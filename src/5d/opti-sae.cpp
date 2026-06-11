#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <random>
#include <utility>
#include <vector>
#include <chrono>
#include "objective5d.h"

// Datos experimentales
// std::vector<std::pair<double, double>> vfc;

inline double voltage(const std::vector<double> &x, double i)
{
    return x[0] - x[1] * log10(i) - x[2] * i + x[3] * log(1 - x[4] * i);
}

// Función a minimizar: SAE
double function(const std::vector<double> &x, const std::vector<std::pair<double, double>> &data)
{
    const int m = data.size();
    double sum = 0.0;
    for (int i = 0; i < m; i++)
    {
        sum += std::abs(data[i].second - voltage(x, data[i].first));
    }
    return sum;
}