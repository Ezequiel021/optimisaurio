#include <cmath>
#include <cstdlib>
#include <mpi.h>
#include <utility>
#include <vector>
#include "objective5d.h"

inline double voltage(const std::vector<double> &x, double i)
{
    return x[0] - x[1] * log10(i) - x[2] * i + x[3] * log(1 - x[4] * i);
}

// Función a minimizar: SSE
double function(const std::vector<double> &x, const std::vector<std::pair<double, double>> &data)
{
    const int m = data.size();
    double sum = 0.0;
    for (int i = 0; i < m; i++)
    {
        sum += pow(std::abs(data[i].second - voltage(x, data[i].first)), 2.0);
    }
    return sum;
}