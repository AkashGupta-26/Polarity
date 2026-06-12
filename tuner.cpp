#include "evaluate.h"
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <numeric>

struct TunePosition {
    std::string fen;
    double result;
};

struct TuneParam {
    int* ptr;
    std::string name;
    int minVal;
    int maxVal;
    int startVal; // original value for regularization
    double gradient;
    double m;  // Adam first moment
    double v;  // Adam second moment
};

double sigK = 1.13;
double REGULARIZATION = 0.0; // L2 regularization strength (set via CLI)
std::vector<TuneParam>* globalParams = nullptr;
int NUM_THREADS = std::max(1, (int)std::thread::hardware_concurrency() - 1);

double sigmoid(int eval) {
    return 1.0 / (1.0 + pow(10.0, -sigK * eval / 400.0));
}

double computeErrorChunk(const std::vector<TunePosition>& positions, int start, int end) {
    double totalError = 0.0;
    Board board;
    for (int i = start; i < end; i++) {
        parseFEN(&board, positions[i].fen);
        int eval = evaluate(&board);
        if (board.sideToMove == black) eval = -eval;
        double predicted = sigmoid(eval);
        double diff = positions[i].result - predicted;
        totalError += diff * diff;
    }
    return totalError;
}

double computeError(const std::vector<TunePosition>& positions) {
    int n = positions.size();
    int chunkSize = n / NUM_THREADS;
    std::vector<std::thread> threads;
    std::vector<double> errors(NUM_THREADS, 0.0);

    for (int t = 0; t < NUM_THREADS; t++) {
        int start = t * chunkSize;
        int end = (t == NUM_THREADS - 1) ? n : (t + 1) * chunkSize;
        threads.emplace_back([&, t, start, end]() {
            errors[t] = computeErrorChunk(positions, start, end);
        });
    }
    for (auto& th : threads) th.join();

    double total = 0.0;
    for (double e : errors) total += e;
    double mse = total / n;

    // L2 regularization: penalize deviation from starting values
    if (REGULARIZATION > 0.0 && globalParams) {
        double regPenalty = 0.0;
        for (const auto& p : *globalParams) {
            double diff = (*p.ptr - p.startVal);
            regPenalty += diff * diff;
        }
        mse += REGULARIZATION * regPenalty / globalParams->size();
    }

    return mse;
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
    auto reg = [&](int* ptr, const std::string& name, int lo, int hi) {
        params.push_back({ptr, name, lo, hi, *ptr, 0, 0, 0});
    };

    reg(&doublePawnPenalty, "doublePawnPenalty", -40, 0);
    reg(&isolatedPawnPenalty, "isolatedPawnPenalty", -40, 0);

    for (int r = 1; r <= 6; r++) {
        reg(&passedPawnBonus[0][r], "passedPawnBonus_MG_" + std::to_string(r), 0, 120);
        reg(&passedPawnBonus[1][r], "passedPawnBonus_EG_" + std::to_string(r), 0, 180);
    }

    reg(&passedPawnFriendlyKingBonus, "passedPawnFriendlyKingBonus", 0, 12);
    reg(&passedPawnEnemyKingPenalty, "passedPawnEnemyKingPenalty", 0, 18);

    reg(&bishopPairBonus[0], "bishopPairBonus_MG", 10, 80);
    reg(&bishopPairBonus[1], "bishopPairBonus_EG", 20, 100);

    reg(&tempoBonus, "tempoBonus", 0, 25);

    reg(&semiOpenFileBonus, "semiOpenFileBonus", 5, 35);
    reg(&openFileBonus, "openFileBonus", 10, 45);

    reg(&pawnShieldBonus, "pawnShieldBonus", 0, 20);
    reg(&pawnShieldMissingPenalty, "pawnShieldMissingPenalty", -25, 0);

    reg(&backwardPawnPenalty[0], "backwardPawnPenalty_MG", -25, 0);
    reg(&backwardPawnPenalty[1], "backwardPawnPenalty_EG", -30, 0);

    reg(&rookOn7thBonus[0], "rookOn7thBonus_MG", 5, 45);
    reg(&rookOn7thBonus[1], "rookOn7thBonus_EG", 10, 55);

    reg(&knightOutpostBonus[0], "knightOutpostBonus_MG", 5, 50);
    reg(&knightOutpostBonus[1], "knightOutpostBonus_EG", 0, 30);

    reg(&rookBehindPassedBonus[0], "rookBehindPassedBonus_MG", 0, 30);
    reg(&rookBehindPassedBonus[1], "rookBehindPassedBonus_EG", 0, 35);

    reg(&connectedPassedBonus[0], "connectedPassedBonus_MG", 0, 25);
    reg(&connectedPassedBonus[1], "connectedPassedBonus_EG", 0, 35);

    reg(&badBishopPenalty[0], "badBishopPenalty_MG", -12, 0);
    reg(&badBishopPenalty[1], "badBishopPenalty_EG", -18, 0);

    reg(&blockedPasserPenalty[0], "blockedPasserPenalty_MG", -25, 0);
    reg(&blockedPasserPenalty[1], "blockedPasserPenalty_EG", -40, 0);
}

void computeGradients(std::vector<TunePosition>& positions, std::vector<TuneParam>& params, int step) {
    double baseError = computeError(positions);

    std::vector<std::thread> threads;
    int numParams = params.size();

    for (int i = 0; i < numParams; i++) {
        int original = *params[i].ptr;
        int newVal = std::min(original + step, params[i].maxVal);
        if (newVal == original) newVal = std::max(original - step, params[i].minVal);

        *params[i].ptr = newVal;
        double newError = computeError(positions);
        *params[i].ptr = original;

        params[i].gradient = (newError - baseError) / (newVal - original);
    }
}

void adamOptimize(std::vector<TunePosition>& positions, std::vector<TuneParam>& params) {
    const double alpha = 2.0;    // Learning rate (in integer param units)
    const double beta1 = 0.9;
    const double beta2 = 0.999;
    const double epsilon = 1e-8;
    const int maxEpochs = 200;
    const int patienceLimit = 10;

    double bestError = computeError(positions);
    std::cout << "Initial error: " << bestError << std::endl;

    std::vector<int> bestValues(params.size());
    for (size_t i = 0; i < params.size(); i++) bestValues[i] = *params[i].ptr;

    int patience = 0;
    int step = 3;

    for (int epoch = 1; epoch <= maxEpochs; epoch++) {
        auto start = std::chrono::steady_clock::now();

        if (epoch > 50) step = 2;
        if (epoch > 100) step = 1;

        computeGradients(positions, params, step);

        int paramsChanged = 0;
        for (size_t i = 0; i < params.size(); i++) {
            auto& p = params[i];

            p.m = beta1 * p.m + (1.0 - beta1) * p.gradient;
            p.v = beta2 * p.v + (1.0 - beta2) * p.gradient * p.gradient;

            double mHat = p.m / (1.0 - pow(beta1, epoch));
            double vHat = p.v / (1.0 - pow(beta2, epoch));

            double update = -alpha * mHat / (sqrt(vHat) + epsilon);

            int intUpdate = (int)round(update);
            if (intUpdate == 0 && std::abs(update) > 0.3) {
                intUpdate = (update > 0) ? 1 : -1;
            }

            if (intUpdate != 0) {
                int newVal = *p.ptr + intUpdate;
                newVal = std::max(p.minVal, std::min(p.maxVal, newVal));
                if (newVal != *p.ptr) {
                    *p.ptr = newVal;
                    paramsChanged++;
                }
            }
        }

        double currentError = computeError(positions);

        if (currentError < bestError) {
            bestError = currentError;
            for (size_t i = 0; i < params.size(); i++) bestValues[i] = *params[i].ptr;
            patience = 0;
        } else {
            patience++;
        }

        auto end = std::chrono::steady_clock::now();
        double secs = std::chrono::duration<double>(end - start).count();
        std::cout << "Epoch " << epoch << " (" << secs << "s): error=" << currentError
                  << " best=" << bestError << " changed=" << paramsChanged
                  << " step=" << step << " patience=" << patience << std::endl;

        if (patience >= patienceLimit) {
            std::cout << "Early stopping: no improvement for " << patienceLimit << " epochs" << std::endl;
            break;
        }

        if (paramsChanged == 0 && step == 1) {
            std::cout << "Converged: no parameters changed at minimum step size" << std::endl;
            break;
        }
    }

    for (size_t i = 0; i < params.size(); i++) *params[i].ptr = bestValues[i];
    std::cout << "\nFinal error: " << bestError << std::endl;
}

void localSearch(std::vector<TunePosition>& positions, std::vector<TuneParam>& params) {
    double bestError = computeError(positions);
    std::cout << "\nLocal search refinement from error: " << bestError << std::endl;

    bool improved = true;
    int epoch = 0;

    while (improved) {
        improved = false;
        epoch++;
        auto start = std::chrono::steady_clock::now();

        for (auto& p : params) {
            int original = *p.ptr;

            if (original + 1 <= p.maxVal) {
                *p.ptr = original + 1;
                double err = computeError(positions);
                if (err < bestError) {
                    bestError = err;
                    improved = true;
                    continue;
                }
            }

            if (original - 1 >= p.minVal) {
                *p.ptr = original - 1;
                double err = computeError(positions);
                if (err < bestError) {
                    bestError = err;
                    improved = true;
                    continue;
                }
            }

            *p.ptr = original;
        }

        auto end = std::chrono::steady_clock::now();
        double secs = std::chrono::duration<double>(end - start).count();
        std::cout << "  Refine epoch " << epoch << " (" << secs << "s), error: " << bestError << std::endl;
    }

    std::cout << "Local search complete after " << epoch << " epochs, final error: " << bestError << std::endl;
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
        std::cerr << "Usage: tuner <positions.txt> [threads] [regularization]" << std::endl;
        std::cerr << "Format: FEN [result]  where result is 1.0, 0.5, or 0.0" << std::endl;
        std::cerr << "Regularization: 0.0 = no constraint, 0.001 = conservative" << std::endl;
        return 1;
    }

    if (argc >= 3) NUM_THREADS = std::atoi(argv[2]);
    if (argc >= 4) REGULARIZATION = std::atof(argv[3]);
    std::cout << "Using " << NUM_THREADS << " threads" << std::endl;
    std::cout << "Regularization: " << REGULARIZATION << std::endl;

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
    globalParams = &params;
    std::cout << "Tuning " << params.size() << " parameters over " << positions.size() << " positions" << std::endl;

    adamOptimize(positions, params);
    localSearch(positions, params);
    printResults(params);

    return 0;
}
