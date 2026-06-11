#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include <vector>
#include <utility> // Necesario para std::pair

double function(const std::vector<double> &x, const std::vector<std::pair<double, double>> &data);

#endif