#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
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
int optimize_island(int &solution_generation, std::vector<double> &solution, int dimensions, int rank, int num_procs,
                    Comparator comp)
{
    // Usamos el rank en la semilla para que cada isla tenga evolución distinta
    std::random_device rd;
    std::mt19937 rng(rd() + rank);

    const double interval_l = -2.0;
    const double interval_r = 2.0;
    const int population = 1000;
    const int epochs = 50;                 // Número de fases de evolución independiente
    const int generations_per_epoch = 200; // Generaciones antes de cada migración
    const int mutation_chance = 100;
    const int tournament_size = 3;
    const int num_migrants = 5; // Cuántos individuos viajan a la siguiente isla (5% de la pob)

    std::uniform_int_distribution<int> uniform_crossovers(0, population - 1);
    std::uniform_int_distribution<int> uniform_mutation(1, mutation_chance);
    std::uniform_real_distribution<double> uniform_solution(interval_l, interval_r);
    std::normal_distribution<double> normal_mutation(0, 0.1);
    std::uniform_real_distribution<double> uniform_01(0.0, 1.0);

    std::vector<std::vector<double>> gen(population, std::vector<double>(dimensions));
    std::vector<std::vector<double>> new_gen(population, std::vector<double>(dimensions));

    // Inicialización N-dimensional
    for (int i = 0; i < population; i++)
    {
        for (int d = 0; d < dimensions; d++)
        {
            gen[i][d] = uniform_solution(rng);
        }
    }

    int total_generations = 0;
    std::vector<double> elite = gen[0];
    solution = elite;
    solution_generation = 0;

    // Bucle de Épocas (Evolución -> Migración -> Evolución...)
    for (int epoch = 0; epoch < epochs; epoch++)
    {
        // Evolución local de la Isla
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

                // Elitismo
                elite = comp(function(gen[i]), function(elite)) ? gen[i] : elite;
                if (comp(function(elite), function(solution)))
                {
                    solution = elite;
                    solution_generation = total_generations;
                }
            }
            new_gen[0] = elite;

            // Cruzamiento y mutación
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

                    new_gen[i][d] = std::clamp(new_gen[i][d], interval_l, interval_r);
                    new_gen[i + 1][d] = std::clamp(new_gen[i + 1][d], interval_l, interval_r);
                }
            }
            gen.swap(new_gen);
        } // Fin de la época

        // --- MIGRACIÓN (Topología de Anillo) ---
        if (num_procs > 1)
        {
            // 1. Evaluar y ordenar a la población actual por fitness
            std::vector<Individual> pop_eval(population);
            for (int i = 0; i < population; i++)
            {
                pop_eval[i] = {i, function(gen[i])};
            }

            // Ordenar de mejor a peor usando la función lambda
            std::sort(pop_eval.begin(), pop_eval.end(),
                      [&comp](const Individual &a, const Individual &b) { return comp(a.fitness, b.fitness); });

            // 2. Empaquetar los mejores 'num_migrants' individuos en un array 1D
            std::vector<double> send_buffer(num_migrants * dimensions);
            for (int m = 0; m < num_migrants; m++)
            {
                for (int d = 0; d < dimensions; d++)
                {
                    send_buffer[m * dimensions + d] = gen[pop_eval[m].index][d];
                }
            }

            // 3. Preparar el búfer de recepción y calcular vecinos del anillo
            std::vector<double> recv_buffer(num_migrants * dimensions);
            int dest = (rank + 1) % num_procs;
            int source = (rank - 1 + num_procs) % num_procs;

            // 4. Enviar y recibir simultáneamente (evita deadlocks)
            MPI_Sendrecv(send_buffer.data(), num_migrants * dimensions, MPI_DOUBLE, dest, 0, recv_buffer.data(),
                         num_migrants * dimensions, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // 5. Reemplazar a los PEORES individuos con los inmigrantes recibidos
            for (int m = 0; m < num_migrants; m++)
            {
                int worst_index = pop_eval[population - 1 - m].index;
                for (int d = 0; d < dimensions; d++)
                {
                    gen[worst_index][d] = recv_buffer[m * dimensions + d];
                }
            }
        }
    } // Fin del bucle global

    return total_generations;
}

int main(int argc, char **argv)
{
    // Inicializar MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int generation;
    int dimensions = 2;
    std::vector<double> local_best_solution;

    // Ejecutar el Algoritmo Genético en esta isla
    optimize_island(generation, local_best_solution, dimensions, rank, size, std::less<double>());

    // Estructura para encontrar el mínimo global y qué proceso (isla) lo tiene
    struct
    {
        double val;
        int rank;
    } local_min, global_min;

    local_min.val = function(local_best_solution);
    local_min.rank = rank;

    // MPI_MINLOC encuentra el valor mínimo y su rango asociado
    MPI_Allreduce(&local_min, &global_min, 1, MPI_DOUBLE_INT, MPI_MINLOC, MPI_COMM_WORLD);

    // El proceso que obtuvo el mínimo global transmite sus coordenadas al resto
    std::vector<double> global_best_solution = local_best_solution;
    MPI_Bcast(global_best_solution.data(), dimensions, MPI_DOUBLE, global_min.rank, MPI_COMM_WORLD);

    // Solo el proceso 0 imprime los resultados finales
    if (rank == 0)
    {
        std::cout << "Coordenadas del optimo global:\n";
        for (int d = 0; d < dimensions; d++)
        {
            std::cout << "x[" << d << "] = " << std::setprecision(6) << global_best_solution[d] << "\n";
        }
        std::cout << "Valor de la funcion (Fitness global) = " << std::setprecision(10) << global_min.val << "\n";
    }

    MPI_Finalize();
    return 0;
}