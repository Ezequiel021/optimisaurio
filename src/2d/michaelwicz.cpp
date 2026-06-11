#include <cmath>
#include <mpi.h>
#include <vector>

// Función a minimizar: Función Michaelwicz
double function(const std::vector<double> &x)
{
    const double sinx = pow(sin(x[0] * x[0] / std::numbers::pi), 20.0);
    const double siny = pow(sin(2.0 * x[1] * x[1] / std::numbers::pi), 20.0);
    return - sin(x[0]) * sinx - sin(x[1]) * siny;
}