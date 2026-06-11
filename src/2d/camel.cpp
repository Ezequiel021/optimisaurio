#include <cmath>
#include <mpi.h>
#include <vector>

#include "objective2d.h"

// Función a minimizar: Six hum camel back
double function(const std::vector<double> &x)
{
    return (4.0 - 2.1 * x[0] * x[0] + 0.5 * pow(x[0], 4.0)) * pow(x[0], 2.0) + x[0] * x[1] +
           4 * (x[1] * x[1] - 1) * x[1] * x[1];
}
