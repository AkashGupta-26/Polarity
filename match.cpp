#include "constants.h"
#include "board.h"
#include "precalculated_move_tables.h"
#include "moves.h"
#include "evaluate.h"

#include <atomic>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

struct UCIEngine {
    FILE *childIn = nullptr;
    FILE *childOut = nullptr;
    pid_t pid = -1;

    bool launch(const char *path) {
        int toChild[2], fromChild[2];
        if (pipe(toChild) || pipe(fromChild)) return false;
        pid = fork();
        if (pid == 0) {
            dup2(toChild[0], STDIN_FILENO);
            dup2(fromChild[1], STDOUT_FILENO);
            close(toChild[0]); close(toChild[1]);
            close(fromChild[0]); close(fromChild[1]);
            execl(path, path, nullptr);
            _exit(127);
        }
        close(toChild[0]);
        close(fromChild[1]);
        childIn = fdopen(toChild[1], "w");
        childOut = fdopen(fromChild[0], "r");
        setvbuf(childIn, nullptr, _IONBF, 0);
        return childIn && childOut;
    }

    void cmd(const std::string &s) { fputs((s + "\n").c_str(), childIn); }

    std::string drainUntil(const std::string &token, int timeoutMs) {
        std::string lastLine;
        long long deadline = TIME_IN_MILLISECONDS + timeoutMs;
        char buf[4096];
        while (TIME_IN_MILLISECONDS < deadline) {
            if (!fgets(buf, sizeof(buf), childOut)) break;
            std::string line(buf);
            while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
            if (line.rfind("info", 0) == 0) continue;
            lastLine = line;
            if (line.rfind(token, 0) == 0) return line;
        }
        return lastLine;
    }

    std::string bestmove(int movetimeMs) {
        cmd("isready");
        drainUntil("readyok", 5000);
        cmd("go movetime " + std::to_string(movetimeMs));
        return drainUntil("bestmove", movetimeMs + 10000);
    }

    void newGame() {
        cmd("ucinewgame");
        cmd("isready");
        drainUntil("readyok", 5000);
    }

    void shutdown() {
        if (childIn) { cmd("quit"); fclose(childIn); childIn = nullptr; }
        if (childOut) { fclose(childOut); childOut = nullptr; }
        if (pid > 0) { waitpid(pid, nullptr, 0); pid = -1; }
    }
};

static std::mutex boardMutex;

static int countPositionOccurrences(U64 hash, const std::vector<U64> &history) {
    int count = 0;
    for (U64 h : history)
        if (h == hash) count++;
    return count;
}

static int countLegalMoves(Board *board) {
    MoveList moveList;
    generateMoves(board, &moveList);
    copyBoard(board);
    int legalMoves = 0;
    for (int i = 0; i < moveList.count; ++i) {
        if (makeMove(board, moveList.moves[i]) == 0) continue;
        legalMoves++;
        takeBack(board, backup);
    }
    return legalMoves;
}

static int parseMoveStr(Board *board, const std::string &moveStr) {
    MoveList moveList[1];
    generateMoves(board, moveList);
    if (moveStr.length() < 4) return 0;

    int source = (moveStr[0] - 'a') + (moveStr[1] - '1') * 8;
    int target = (moveStr[2] - 'a') + (moveStr[3] - '1') * 8;
    char promoChar = moveStr.length() == 5 ? moveStr[4] : ' ';

    for (int i = 0; i < moveList->count; ++i) {
        int move = moveList->moves[i];
        if (decodeSource(move) == source && decodeTarget(move) == target) {
            int promo = decodePromoted(move);
            if (promo) {
                if ((promoChar == 'q' && (promo == Q || promo == q)) ||
                    (promoChar == 'r' && (promo == R || promo == r)) ||
                    (promoChar == 'b' && (promo == B || promo == b)) ||
                    (promoChar == 'n' && (promo == N || promo == n)))
                    return move;
                continue;
            }
            return move;
        }
    }
    return 0;
}

static std::string extractBestmove(const std::string &line) {
    std::istringstream ss(line);
    std::string tag, move;
    ss >> tag >> move;
    return move;
}

enum GameResult { WHITE_WIN, BLACK_WIN, DRAW, ABORT };

static GameResult playGame(UCIEngine &whiteEngine, UCIEngine &blackEngine, int movetimeMs,
                           std::vector<std::string> &moveHistory, std::string &resultNote) {
    Board gameBoard;
    std::vector<U64> positionHistory;
    {
        std::lock_guard<std::mutex> lock(boardMutex);
        parseFEN(&gameBoard, start_position);
        positionHistory.push_back(gameBoard.zobristHash);
    }
    moveHistory.clear();
    resultNote.clear();

    whiteEngine.newGame();
    blackEngine.newGame();

    for (;;) {
        GameResult terminal;
        bool gameOver = false;
        int sideToMove;
        {
            std::lock_guard<std::mutex> lock(boardMutex);
            Board *pos = &gameBoard;
            if (countLegalMoves(pos) == 0) {
                if (isBoardInCheck(pos)) {
                    terminal = (gameBoard.sideToMove == white ? BLACK_WIN : WHITE_WIN);
                    resultNote = "checkmate";
                } else {
                    terminal = DRAW;
                    resultNote = "stalemate";
                }
                gameOver = true;
            }
            sideToMove = gameBoard.sideToMove;
        }
        if (gameOver) return terminal;

        UCIEngine &engine = (sideToMove == white) ? whiteEngine : blackEngine;
        std::string posCmd = "position startpos";
        if (!moveHistory.empty()) {
            posCmd += " moves";
            for (const auto &m : moveHistory) posCmd += " " + m;
        }
        engine.cmd(posCmd);

        std::string response = engine.bestmove(movetimeMs);
        std::string uciMove = extractBestmove(response);
        if (uciMove.empty() || uciMove == "0000" || uciMove == "(none)")
            return ABORT;

        {
            std::lock_guard<std::mutex> lock(boardMutex);
            int move = parseMoveStr(&gameBoard, uciMove);
            if (!move) return ABORT;

            if (!makeMove(&gameBoard, move)) return ABORT;

            positionHistory.push_back(gameBoard.zobristHash);
            if (countPositionOccurrences(gameBoard.zobristHash, positionHistory) >= 3) {
                resultNote = "threefold repetition";
                return DRAW;
            }

            if (gameBoard.halfMoveClock >= 100) {
                resultNote = "50-move rule";
                return DRAW;
            }
        }

        moveHistory.push_back(uciMove);
    }
}

struct GameOutcome {
    int gameNum = 0;
    int plies = 0;
    int mainPoint = 0;   // 1 win, 0 draw, -1 loss
    bool aborted = false;
    std::string line;
};

struct MatchTotals {
    std::atomic<int> mainWins{0};
    std::atomic<int> searchWins{0};
    std::atomic<int> draws{0};
    std::atomic<int> aborted{0};
};

static void matchWorker(const char *mainPath, const char *searchPath, int movetime, int totalGames,
                        int parallelism, std::atomic<int> &nextGame, MatchTotals &totals,
                        std::vector<GameOutcome> &outcomes, std::mutex &outcomeMutex) {
    UCIEngine mainEngine, searchEngine;
    if (!mainEngine.launch(mainPath) || !searchEngine.launch(searchPath)) {
        std::cerr << "Worker failed to launch engines\n";
        return;
    }
    mainEngine.cmd("uci"); mainEngine.drainUntil("uciok", 3000);
    searchEngine.cmd("uci"); searchEngine.drainUntil("uciok", 3000);

    while (true) {
        int g = nextGame.fetch_add(1);
        if (g >= totalGames) break;

        bool mainWhite = (g % 2 == 0);
        UCIEngine &whiteEngine = mainWhite ? mainEngine : searchEngine;
        UCIEngine &blackEngine = mainWhite ? searchEngine : mainEngine;
        const char *whiteName = mainWhite ? "main" : "search";
        const char *blackName = mainWhite ? "search" : "main";

        std::vector<std::string> moves;
        std::string resultNote;
        GameResult result = playGame(whiteEngine, blackEngine, movetime, moves, resultNote);

        GameOutcome outcome;
        outcome.gameNum = g + 1;
        outcome.plies = (int)moves.size();

        std::ostringstream oss;
        oss << "Game " << (g + 1) << ": " << whiteName << " (W) vs " << blackName << " (B) | "
            << moves.size() << " plies | ";

        if (result == WHITE_WIN) {
            if (mainWhite) { totals.mainWins++; outcome.mainPoint = 1; oss << "main wins"; }
            else { totals.searchWins++; outcome.mainPoint = -1; oss << "search wins"; }
        } else if (result == BLACK_WIN) {
            if (mainWhite) { totals.searchWins++; outcome.mainPoint = -1; oss << "search wins"; }
            else { totals.mainWins++; outcome.mainPoint = 1; oss << "main wins"; }
        } else if (result == DRAW) {
            totals.draws++;
            outcome.mainPoint = 0;
            oss << "draw";
            if (!resultNote.empty()) oss << " (" << resultNote << ")";
        } else {
            totals.aborted++;
            outcome.aborted = true;
            oss << "aborted";
        }

        outcome.line = oss.str();

        {
            std::lock_guard<std::mutex> lock(outcomeMutex);
            outcomes[g] = outcome;
            std::cout << outcome.line << std::endl;
        }
    }

    mainEngine.shutdown();
    searchEngine.shutdown();
}

int main(int argc, char **argv) {
    const char *mainPath = (argc > 1) ? argv[1] : "./engine-main";
    const char *searchPath = (argc > 2) ? argv[2] : "./engine-search";
    int games = (argc > 3) ? std::stoi(argv[3]) : 10;
    int movetime = (argc > 4) ? std::stoi(argv[4]) : 500;
    int parallelism = (argc > 5) ? std::stoi(argv[5]) : 4;

    if (parallelism < 1) parallelism = 1;
    if (parallelism > games) parallelism = games;

    initializeMoveTables();
    initializeRandomKeys();
    initializeTranspositionSize(16);
    initializeEvaluationMasks();

    std::cout << "Match: " << mainPath << " vs " << searchPath
              << " | " << games << " games | movetime " << movetime << "ms"
              << " | parallelism " << parallelism << "\n\n";

    std::atomic<int> nextGame{0};
    MatchTotals totals;
    std::vector<GameOutcome> outcomes(games);
    std::mutex outcomeMutex;

    auto startTime = TIME_IN_MILLISECONDS;
    std::vector<std::thread> workers;
    workers.reserve(parallelism);
    for (int t = 0; t < parallelism; ++t)
        workers.emplace_back(matchWorker, mainPath, searchPath, movetime, games, parallelism,
                             std::ref(nextGame), std::ref(totals), std::ref(outcomes), std::ref(outcomeMutex));

    for (auto &w : workers) w.join();
    auto elapsed = TIME_IN_MILLISECONDS - startTime;

    std::cout << "\n=== Final Score ===\n";
    std::cout << "main:   " << totals.mainWins.load() << " / " << games << "\n";
    std::cout << "search: " << totals.searchWins.load() << " / " << games << "\n";
    std::cout << "draws:  " << totals.draws.load() << "\n";
    if (totals.aborted.load()) std::cout << "aborted:" << totals.aborted.load() << "\n";
    std::cout << "Score:  " << totals.mainWins.load() << " - " << totals.searchWins.load()
              << " - " << totals.draws.load() << " (W-D-L from main's perspective)\n";
    std::cout << "Elapsed: " << elapsed << " ms (" << (elapsed / 1000.0) << " s)\n";

    delete[] TranspositionTable;
    return 0;
}
