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
	double sum;
	double sumSquared;
	double mean;
	double variance;
	double stdDev;

public:
	StandardDeviation() : n(0), sum(0.0), sumSquared(0.0), mean(0.0), variance(0.0), stdDev(0.0) {}

    void addValue(double x) {
        n++;
        sum += x;
        sumSquared += x * x;
        mean = sum / n;
        variance = (sumSquared - (sum * sum) / n) / n;
        stdDev = std::sqrt(variance);
    }

    double getTotal() const { return n; }
    double getStDev() const { return stdDev; }
    double getMean() const { return mean; }
    bool checkDeviation(double val, double n) const { return val > ((n * stdDev) + mean); }
    double numStDev(double val) { return ((val - mean) / stdDev); }
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