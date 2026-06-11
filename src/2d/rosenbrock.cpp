#include <mpi.h>
#include <vector>

// Función a minimizar: Función Rosenbrock
double function(const std::vector<double> &x)
{
    return (1.0 - x[0]) * (1.0 - x[0]) + 100.0 * (x[1] - x[0] * x[0]) * (x[1] - x[0] * x[0]);
}