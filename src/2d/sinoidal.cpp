#include <cmath>
#include <mpi.h>
#include <vector>

// Función a minimizar: Función sinoidal
double function(const std::vector<double> &x)
{
    double sinx = sin(5.0 * x[0]);
    sinx *= sinx;
    double siny = sin(5.0 * x[1]);
    siny *= siny;
    return x[0]*x[0] + x[1]*x[1] + 3 * sqrt(sinx + siny) + 0.1;
}