#include "objective5d.h"
#include <algorithm>
#include <vector>
#include <cmath>

inline double voltage(const std::vector<double> &x, double i)
{
    return x[0] - x[1] * log(i) - x[2] * i + x[3] * log(1 - x[4] * i);
}

// Función a minimizar
// Se quitó 'inline' para permitir el enlace (linking) externo
double function(const std::vector<double> &x, const std::vector<std::pair<double, double>> &data)
{
    const int m = data.size();
    std::vector<double> v(m);
    for (int i = 0; i < m; i++)
    {
        v[i] = std::abs(data[i].second - voltage(x, data[i].first));
    }
    std::sort(v.begin(), v.end());
    // Calcula el error mediano absoluto
    return m % 2 == 0 ? (v[m / 2] + v[m / 2 - 1]) / 2.0 : v[m / 2];
}