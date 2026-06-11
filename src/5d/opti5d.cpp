#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mpi.h>
#include <random>
#include <utility>
#include <vector>

#include "objective5d.h"

struct Individual
{
    int index;
    double fitness;
};

template <typename Comparator>
int optimize_island(int &solution_generation, std::vector<double> &solution, int dimensions,
                    const std::vector<std::pair<double, double>> &bounds,
                    const std::vector<std::pair<double, double>> &dataset, int rank, int num_procs, Comparator comp,
                    std::vector<int> &arguments)
{
    std::random_device rd;
    std::mt19937 rng(rd() + rank);

    const int population = arguments[0];
    const int epochs = arguments[1];
    const int generations_per_epoch = arguments[2];
    const int mutation_chance_1_in = arguments[3];
    const int tournament_size = arguments[4];
    const int num_migrants = arguments[5];

    std::uniform_int_distribution<int> uniform_crossovers(0, population - 1);
    std::uniform_int_distribution<int> uniform_mutation(1, mutation_chance_1_in);
    std::normal_distribution<double> normal_mutation(0, 0.1);
    std::uniform_real_distribution<double> uniform_01(0.0, 1.0);

    std::vector<std::vector<double>> gen(population, std::vector<double>(dimensions));
    std::vector<std::vector<double>> new_gen(population, std::vector<double>(dimensions));

    for (int i = 0; i < population; i++)
    {
        for (int d = 0; d < dimensions; d++)
        {
            std::uniform_real_distribution<double> dist(bounds[d].first, bounds[d].second);
            gen[i][d] = dist(rng);
        }
    }

    int total_generations = 0;
    std::vector<double> elite = gen[0];
    solution = elite;
    solution_generation = 0;

    for (int epoch = 0; epoch < epochs; epoch++)
    {
        for (int g = 0; g < generations_per_epoch; g++)
        {
            total_generations++;
            elite = gen[0];

            for (int i = 0; i < population; i++)
            {
                int best = uniform_crossovers(rng);
                for (int j = 0; j < tournament_size - 1; j++)
                {
                    const int current = uniform_crossovers(rng);
                    // Actualizado para pasar 'dataset'
                    best = comp(function(gen[current], dataset), function(gen[best], dataset)) ? current : best;
                }

                new_gen[i] = gen[best];

                // Actualizado para pasar 'dataset'
                elite = comp(function(gen[i], dataset), function(elite, dataset)) ? gen[i] : elite;
                if (comp(function(elite, dataset), function(solution, dataset)))
                {
                    solution = elite;
                    solution_generation = total_generations;
                }
            }
            new_gen[0] = elite;

            for (int i = 0; i < population - 1; i += 2)
            {
                const double alpha = uniform_01(rng);
                for (int d = 0; d < dimensions; d++)
                {
                    double p1 = new_gen[i][d];
                    double p2 = new_gen[i + 1][d];

                    new_gen[i][d] = (alpha * p1) + ((1.0 - alpha) * p2);
                    new_gen[i + 1][d] = ((1.0 - alpha) * p1) + (alpha * p2);

                    if (uniform_mutation(rng) == 1)
                        new_gen[i][d] += normal_mutation(rng);
                    if (uniform_mutation(rng) == 1)
                        new_gen[i + 1][d] += normal_mutation(rng);

                    new_gen[i][d] = std::clamp(new_gen[i][d], bounds[d].first, bounds[d].second);
                    new_gen[i + 1][d] = std::clamp(new_gen[i + 1][d], bounds[d].first, bounds[d].second);
                }
            }
            gen.swap(new_gen);
        }

        // --- MIGRACIÓN ---
        if (num_procs > 1)
        {
            std::vector<Individual> pop_eval(population);
            for (int i = 0; i < population; i++)
            {
                // Actualizado para pasar 'dataset'
                pop_eval[i] = {i, function(gen[i], dataset)};
            }

            std::sort(pop_eval.begin(), pop_eval.end(),
                      [&comp](const Individual &a, const Individual &b) { return comp(a.fitness, b.fitness); });

            std::vector<double> send_buffer(num_migrants * dimensions);
            for (int m = 0; m < num_migrants; m++)
            {
                for (int d = 0; d < dimensions; d++)
                {
                    send_buffer[m * dimensions + d] = gen[pop_eval[m].index][d];
                }
            }

            std::vector<double> recv_buffer(num_migrants * dimensions);
            int dest = (rank + 1) % num_procs;
            int source = (rank - 1 + num_procs) % num_procs;

            MPI_Sendrecv(send_buffer.data(), num_migrants * dimensions, MPI_DOUBLE, dest, 0, recv_buffer.data(),
                         num_migrants * dimensions, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int m = 0; m < num_migrants; m++)
            {
                int worst_index = pop_eval[population - 1 - m].index;
                for (int d = 0; d < dimensions; d++)
                {
                    gen[worst_index][d] = recv_buffer[m * dimensions + d];
                }
            }
        }
    }

    return total_generations;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<std::pair<double, double>> vfc;

    int num_datos = 0;
    std::vector<double> flat_vfc;

    if (rank == 0)
    {
        std::ifstream data("datos_358.txt");
        if (!data.is_open())
        {
            std::cerr << "Error al abrir el archivo en el rank 0.\n";
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        double dat_i, dat_vi;
        while (data >> dat_i >> dat_vi)
        {
            vfc.push_back({dat_i, dat_vi});
            flat_vfc.push_back(dat_i);
            flat_vfc.push_back(dat_vi);
        }
        num_datos = vfc.size();
    }

    MPI_Bcast(&num_datos, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        flat_vfc.resize(num_datos * 2);
    }

    MPI_Bcast(flat_vfc.data(), num_datos * 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        for (int i = 0; i < num_datos; i++)
        {
            vfc.push_back({flat_vfc[i * 2], flat_vfc[i * 2 + 1]});
        }
    }

    int generation;
    const int dimensions = 5;

    std::vector<std::pair<double, double>> bounds(dimensions);
    bounds[0] = {0.0, 0.5};
    bounds[1] = {0.0, 0.2};
    bounds[2] = {0.0, 1.0};
    bounds[3] = {0.0, 1.14};
    bounds[4] = {0.0, 29.5};

    std::vector<int> parameters(6);
    if (rank == 0)
    {
        std::ifstream param("parameters.cfg");
        if (!param.is_open())
        {
            std::cerr << "Fallo al abrir el archivo 'parameters.cfg'";
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        std::string dummy;
        for (int i = 0; i < 6; i++)
        {
            param >> dummy >> parameters[i];
        }

        parameters[0] /= size;
    }

    MPI_Bcast(parameters.data(), 6, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<double> local_best_solution;

    auto start = std::chrono::high_resolution_clock::now();

    optimize_island(generation, local_best_solution, dimensions, bounds, vfc, rank, size, std::less<double>(),
                    parameters);

    struct
    {
        double val;
        int rank;
    } local_min, global_min;

    local_min.val = function(local_best_solution, vfc);
    local_min.rank = rank;

    MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);

    std::vector<double> global_best_solution = local_best_solution;
    MPI_Bcast(global_best_solution.data(), dimensions, MPI_DOUBLE, global_min.rank, MPI_COMM_WORLD);

    auto stop = std::chrono::high_resolution_clock::now();

    if (rank == 0)
    {
        auto runtime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

        std::ofstream output("out.log");
        output << "La isla ganadora fue el proceso [" << global_min.rank << "] de " << size << "\n";
        output << "Coordenadas del optimo global:\n";
        for (int d = 0; d < dimensions; d++)
        {
            output << "x[" << d << "] = " << std::setprecision(6) << global_best_solution[d] << " (Límites: ["
                   << bounds[d].first << ", " << bounds[d].second << "])\n";
        }
        output << "Valor de la funcion = " << std::setprecision(10) << global_min.val << "\n";
        output << "Tiempo de ejecución: " << runtime.count() << " ms\n";

        output << global_best_solution[0] << " - " << global_best_solution[1] << " * log(i) - "
               << global_best_solution[2] << " * i + " << global_best_solution[3] << " * log(1 - "
               << global_best_solution[4] << " * i)";
    }

    MPI_Finalize();
    return 0;
}