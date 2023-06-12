#pragma once

//==================================================
// Useful formulas to be used throughout the program
//==================================================

#include <cmath>

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

    double getStDev() const {
        return stdDev;
    }

    double getMean() const {
        return mean;
    }
};