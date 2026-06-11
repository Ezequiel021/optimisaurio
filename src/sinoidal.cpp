#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <random>
#include <vector>

// Función a minimizar: Función sinoidal
inline double function(const std::vector<double> &x)
{
    double sinx = sin(5.0 * x[0]);
    sinx *= sinx;
    double siny = sin(5.0 * x[1]);
    siny *= siny;
    return x[0]*x[0] + x[1]*x[1] + 3 * sqrt(sinx + siny) + 0.1;
}

// Estructura auxiliar para ordenar la población por fitness
struct Individual
{
    int index;
    double fitness;
};

template <typename Comparator>
int optimize_island(int &solution_generation, std::vector<double> &solution, int dimensions,
                    const std::vector<std::pair<double, double>> &bounds, int rank, int num_procs, Comparator comp, std::vector<int> &arguments)
{
    std::random_device rd;
    std::mt19937 rng(rd() + rank);

    const int population            = arguments[0];
    const int epochs                = arguments[1];
    const int generations_per_epoch = arguments[2];
    const int mutation_chance_1_in  = arguments[3];
    const int tournament_size       = arguments[4];
    const int num_migrants          = arguments[5];

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
                    best = comp(function(gen[current]), function(gen[best])) ? current : best;
                }

                new_gen[i] = gen[best];

                elite = comp(function(gen[i]), function(elite)) ? gen[i] : elite;
                if (comp(function(elite), function(solution)))
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
                pop_eval[i] = {i, function(gen[i])};
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
    // ====== SETUP ======
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int generation;
    const int dimensions = 2;

    std::vector<std::pair<double, double>> bounds(dimensions);
    bounds[0] = {-2.0, 2.0};
    bounds[1] = {-2.0, 2.0};


    std::vector<int> parameters(6);
    if (rank == 0)
    {
        std::ifstream param("parameters.cfg");
        if (! param.is_open())
        {
            std::cerr << "Fallo al abrir el archivo 'parameters.cfg'";
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        std::string dummy;
        for (int i = 0; i < 6; i++)
        {
            param >> dummy >> parameters[i];
        }

        // Distribuir la población total equitativamente entre las islas
        parameters[0] /= size;
    }

    MPI_Bcast(parameters.data(), 6, MPI_INT, 0, MPI_COMM_WORLD);    
    std::vector<double> local_best_solution;

    // ====== EJECUCIÓN ======
    auto start = std::chrono::high_resolution_clock::now();
    optimize_island(generation, local_best_solution, dimensions, bounds, rank, size, std::less<double>(), parameters);

    struct
    {
        double val;
        int rank;
    } local_min, global_min;

    local_min.val = function(local_best_solution);
    local_min.rank = rank;

    MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);

    std::vector<double> global_best_solution = local_best_solution;
    MPI_Bcast(global_best_solution.data(), dimensions, MPI_DOUBLE, global_min.rank, MPI_COMM_WORLD);

    auto stop = std::chrono::high_resolution_clock::now();

    // ======= SALIDA =======
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
        output << "Tiempo de ejecución: " << runtime.count() << "\n";
    }

    MPI_Finalize();
    return 0;
}