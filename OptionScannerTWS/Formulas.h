#pragma once

//==================================================
// Useful formulas to be used throughout the program
//==================================================

#include <cmath>
#include <algorithm>
#include <chrono>
#include <ctime>

class StandardDeviation {
private:
    int n;

    double sum_{ 0 };
    double sumSq_{ 0 };
    double mean_{ 0 };
    double variance_{ 0 };
    double stdDev_{ 0 };

public:
    StandardDeviation() : n(0) {}

    void addValue(double x) {
        n++;
        sum_ += x;
        sumSq_ += x * x;
        mean_ = sum_ / n;
        variance_ = (sumSq_ - n * mean_ * mean_) / n;
        stdDev_ = std::sqrt(variance_);
    }

    double sum() const { return n; }
    double stDev() const { return stdDev_; }
    double mean() const { return mean_; }
    double numStDev(double val) {
        if (stdDev_ == 0) return 0;
        else return (val - mean_) / stdDev_;
    }
};

template <typename T>
bool isWithinXPercent(T value1, T value2, T percent) {
    // Calculate the absolute difference between the two values
    T diff = std::abs(value1 - value2) / std::abs(value1);

    // Check if the absolute difference is within the threshold
    return diff <= percent;
}