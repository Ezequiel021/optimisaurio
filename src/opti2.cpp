#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <random>

struct vector
{
    int dim;
    double *data;

    double & operator[](size_t index)
    {
        if (index >= dim)
        {
            std::out_of_range("Index out of bounds");
        }

        return data[index];
    }

    vector(int dim)
    {
        this->dim = dim;
    }
};

int main()
{
    const int population = 100;
    std::vector<double> initial_solutions(population);
}