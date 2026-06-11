#include "evaluate.h"
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

struct TunePosition {
    std::string fen;
    double result; // 1.0 = white win, 0.5 = draw, 0.0 = black win
};

struct TuneParam {
    int* ptr;
    std::string name;
    int minVal;
    int maxVal;
};

double sigK = 1.13;

double sigmoid(int eval) {
    return 1.0 / (1.0 + pow(10.0, -sigK * eval / 400.0));
}

double computeError(const std::vector<TunePosition>& positions) {
    double totalError = 0.0;
    Board board;
    for (const auto& pos : positions) {
        parseFEN(&board, pos.fen);
        int eval = evaluate(&board);
        if (board.sideToMove == black) eval = -eval;
        double predicted = sigmoid(eval);
        double diff = pos.result - predicted;
        totalError += diff * diff;
    }
    return totalError / positions.size();
}

double findOptimalK(const std::vector<TunePosition>& positions) {
    double bestK = 1.0;
    double bestError = 1e9;
    for (double k = 0.5; k <= 2.0; k += 0.01) {
        sigK = k;
        double err = computeError(positions);
        if (err < bestError) {
            bestError = err;
            bestK = k;
        }
    }
    sigK = bestK;
    std::cout << "Optimal K: " << bestK << " (error: " << bestError << ")" << std::endl;
    return bestK;
}

std::vector<TunePosition> loadPositions(const std::string& filename) {
    std::vector<TunePosition> positions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open " << filename << std::endl;
        return positions;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Format: FEN [result] or FEN ; result
        // Support: "fen [1.0]", "fen [0.5]", "fen [0.0]"
        // Also: "fen ; 1-0", "fen ; 1/2-1/2", "fen ; 0-1"
        size_t bracketOpen = line.find('[');
        size_t bracketClose = line.find(']');
        size_t semicolon = line.find(';');

        TunePosition pos;
        if (bracketOpen != std::string::npos && bracketClose != std::string::npos) {
            pos.fen = line.substr(0, bracketOpen);
            while (!pos.fen.empty() && pos.fen.back() == ' ') pos.fen.pop_back();
            std::string resultStr = line.substr(bracketOpen + 1, bracketClose - bracketOpen - 1);
            if (resultStr == "1.0" || resultStr == "1-0") pos.result = 1.0;
            else if (resultStr == "0.0" || resultStr == "0-1") pos.result = 0.0;
            else pos.result = 0.5;
        } else if (semicolon != std::string::npos) {
            pos.fen = line.substr(0, semicolon);
            while (!pos.fen.empty() && pos.fen.back() == ' ') pos.fen.pop_back();
            std::string resultStr = line.substr(semicolon + 1);
            while (!resultStr.empty() && resultStr.front() == ' ') resultStr.erase(resultStr.begin());
            if (resultStr == "1-0" || resultStr == "1.0") pos.result = 1.0;
            else if (resultStr == "0-1" || resultStr == "0.0") pos.result = 0.0;
            else pos.result = 0.5;
        } else {
            continue;
        }

        positions.push_back(pos);
    }

    std::cout << "Loaded " << positions.size() << " positions" << std::endl;
    return positions;
}

void registerParams(std::vector<TuneParam>& params) {
    params.push_back({&doublePawnPenalty, "doublePawnPenalty", -30, 0});
    params.push_back({&isolatedPawnPenalty, "isolatedPawnPenalty", -30, 0});

    for (int r = 1; r <= 6; r++) {
        params.push_back({&passedPawnBonus[0][r], "passedPawnBonus_MG_" + std::to_string(r), 0, 80});
        params.push_back({&passedPawnBonus[1][r], "passedPawnBonus_EG_" + std::to_string(r), 0, 150});
    }

    params.push_back({&passedPawnFriendlyKingBonus, "passedPawnFriendlyKingBonus", 0, 10});
    params.push_back({&passedPawnEnemyKingPenalty, "passedPawnEnemyKingPenalty", 0, 15});

    params.push_back({&bishopPairBonus[0], "bishopPairBonus_MG", 10, 60});
    params.push_back({&bishopPairBonus[1], "bishopPairBonus_EG", 20, 80});

    params.push_back({&tempoBonus, "tempoBonus", 5, 25});

    params.push_back({&semiOpenFileBonus, "semiOpenFileBonus", 5, 30});
    params.push_back({&openFileBonus, "openFileBonus", 10, 40});

    params.push_back({&pawnShieldBonus, "pawnShieldBonus", 3, 20});
    params.push_back({&pawnShieldMissingPenalty, "pawnShieldMissingPenalty", -25, 0});

    params.push_back({&backwardPawnPenalty[0], "backwardPawnPenalty_MG", -20, 0});
    params.push_back({&backwardPawnPenalty[1], "backwardPawnPenalty_EG", -25, 0});

    params.push_back({&rookOn7thBonus[0], "rookOn7thBonus_MG", 5, 35});
    params.push_back({&rookOn7thBonus[1], "rookOn7thBonus_EG", 10, 50});

    params.push_back({&knightOutpostBonus[0], "knightOutpostBonus_MG", 5, 40});
    params.push_back({&knightOutpostBonus[1], "knightOutpostBonus_EG", 0, 25});

    params.push_back({&rookBehindPassedBonus[0], "rookBehindPassedBonus_MG", 0, 25});
    params.push_back({&rookBehindPassedBonus[1], "rookBehindPassedBonus_EG", 5, 40});

    params.push_back({&connectedPassedBonus[0], "connectedPassedBonus_MG", 0, 25});
    params.push_back({&connectedPassedBonus[1], "connectedPassedBonus_EG", 5, 35});

    params.push_back({&badBishopPenalty[0], "badBishopPenalty_MG", -10, 0});
    params.push_back({&badBishopPenalty[1], "badBishopPenalty_EG", -15, 0});

    params.push_back({&blockedPasserPenalty[0], "blockedPasserPenalty_MG", -20, 0});
    params.push_back({&blockedPasserPenalty[1], "blockedPasserPenalty_EG", -30, 0});
}

void localSearch(std::vector<TunePosition>& positions, std::vector<TuneParam>& params) {
    double bestError = computeError(positions);
    std::cout << "Initial error: " << bestError << std::endl;

    bool improved = true;
    int epoch = 0;

    while (improved) {
        improved = false;
        epoch++;
        auto start = std::chrono::steady_clock::now();

        for (auto& p : params) {
            int original = *p.ptr;

            // Try +1
            if (original + 1 <= p.maxVal) {
                *p.ptr = original + 1;
                double err = computeError(positions);
                if (err < bestError) {
                    bestError = err;
                    improved = true;
                    std::cout << "  " << p.name << ": " << original << " -> " << (original + 1) << " (err: " << err << ")" << std::endl;
                    continue;
                }
            }

            // Try -1
            if (original - 1 >= p.minVal) {
                *p.ptr = original - 1;
                double err = computeError(positions);
                if (err < bestError) {
                    bestError = err;
                    improved = true;
                    std::cout << "  " << p.name << ": " << original << " -> " << (original - 1) << " (err: " << err << ")" << std::endl;
                    continue;
                }
            }

            *p.ptr = original;
        }

        auto end = std::chrono::steady_clock::now();
        double secs = std::chrono::duration<double>(end - start).count();
        std::cout << "Epoch " << epoch << " complete (" << secs << "s), error: " << bestError << std::endl;
    }

    std::cout << "\nOptimization complete after " << epoch << " epochs." << std::endl;
    std::cout << "Final error: " << bestError << std::endl;
}

void printResults(const std::vector<TuneParam>& params) {
    std::cout << "\n=== Tuned Values ===" << std::endl;
    for (const auto& p : params) {
        std::cout << p.name << " = " << *p.ptr << std::endl;
    }
    std::cout << "\n=== Copy-paste for evaluate.h ===" << std::endl;
    std::cout << "doublePawnPenalty = " << doublePawnPenalty << ";" << std::endl;
    std::cout << "isolatedPawnPenalty = " << isolatedPawnPenalty << ";" << std::endl;
    std::cout << "passedPawnBonus[2][8] = {" << std::endl;
    std::cout << "    {";
    for (int i = 0; i < 8; i++) std::cout << passedPawnBonus[0][i] << (i < 7 ? ", " : "");
    std::cout << "}," << std::endl;
    std::cout << "    {";
    for (int i = 0; i < 8; i++) std::cout << passedPawnBonus[1][i] << (i < 7 ? ", " : "");
    std::cout << "}" << std::endl << "};" << std::endl;
    std::cout << "passedPawnFriendlyKingBonus = " << passedPawnFriendlyKingBonus << ";" << std::endl;
    std::cout << "passedPawnEnemyKingPenalty = " << passedPawnEnemyKingPenalty << ";" << std::endl;
    std::cout << "bishopPairBonus[2] = {" << bishopPairBonus[0] << ", " << bishopPairBonus[1] << "};" << std::endl;
    std::cout << "tempoBonus = " << tempoBonus << ";" << std::endl;
    std::cout << "semiOpenFileBonus = " << semiOpenFileBonus << ";" << std::endl;
    std::cout << "openFileBonus = " << openFileBonus << ";" << std::endl;
    std::cout << "pawnShieldBonus = " << pawnShieldBonus << ";" << std::endl;
    std::cout << "pawnShieldMissingPenalty = " << pawnShieldMissingPenalty << ";" << std::endl;
    std::cout << "backwardPawnPenalty[2] = {" << backwardPawnPenalty[0] << ", " << backwardPawnPenalty[1] << "};" << std::endl;
    std::cout << "rookOn7thBonus[2] = {" << rookOn7thBonus[0] << ", " << rookOn7thBonus[1] << "};" << std::endl;
    std::cout << "knightOutpostBonus[2] = {" << knightOutpostBonus[0] << ", " << knightOutpostBonus[1] << "};" << std::endl;
    std::cout << "rookBehindPassedBonus[2] = {" << rookBehindPassedBonus[0] << ", " << rookBehindPassedBonus[1] << "};" << std::endl;
    std::cout << "connectedPassedBonus[2] = {" << connectedPassedBonus[0] << ", " << connectedPassedBonus[1] << "};" << std::endl;
    std::cout << "badBishopPenalty[2] = {" << badBishopPenalty[0] << ", " << badBishopPenalty[1] << "};" << std::endl;
    std::cout << "blockedPasserPenalty[2] = {" << blockedPasserPenalty[0] << ", " << blockedPasserPenalty[1] << "};" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: tuner <positions.txt>" << std::endl;
        std::cerr << "Format: FEN [result]  where result is 1.0, 0.5, or 0.0" << std::endl;
        return 1;
    }

    initializeMoveTables();
    initializeRandomKeys();
    initializeEvaluationMasks();

    auto positions = loadPositions(argv[1]);
    if (positions.empty()) {
        std::cerr << "No positions loaded." << std::endl;
        return 1;
    }

    findOptimalK(positions);

    std::vector<TuneParam> params;
    registerParams(params);
    std::cout << "Tuning " << params.size() << " parameters over " << positions.size() << " positions" << std::endl;

    localSearch(positions, params);
    printResults(params);

    return 0;
}
