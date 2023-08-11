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
	double sum_;
	double sumSquared_;
	double mean_;
	double variance_;
	double stdDev_;

public:
	StandardDeviation() : n(0), sum_(0.0), sumSquared_(0.0), mean_(0.0), variance_(0.0), stdDev_(0.0) {}

    void addValue(double x) {
        n++;
        sum_ += x;
        sumSquared_ += x * x;
        mean_ = sum_ / n;
        variance_ = (sumSquared_ - (sum_ * sum_) / n) / n;
        stdDev_ = std::sqrt(variance_);
    }

    double sum() const { return n; }
    double stDev() const { return stdDev_; }
    double mean() const { return mean_; }
    bool checkDeviation(double val, double n) const { return val > ((n * stdDev_) + mean_); }
    double numStDev(double val) { return ((val - mean_) / stdDev_); }
};

template <typename T>
bool isWithinXPercent(T value1, T value2, T percent) {
    // Calculate the absolute difference between the two values
    T diff = std::abs(value1 - value2);

    // Calculate 5% of the average of the two values
    T threshold = (percent * 1/100) * (std::abs(value1) + std::abs(value2)) / 2;

    // Check if the absolute difference is within the threshold
    return diff <= threshold;
}