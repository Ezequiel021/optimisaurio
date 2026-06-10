#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <random>
#include <vector>

inline double function(double x)
{
    return -std::cos(x) * std::exp(-(x - std::numbers::pi) * (x - std::numbers::pi));
}

template <typename Comparator>
int optimize(int &solution_generation, double &solution, Comparator comp)
{
    std::random_device rd;
    std::mt19937 rng(rd());

    const double interval_l = -10.0;
    const double interval_r = 10.0;
    const int population = 100;
    const int max_generations = 10000;
    const int mutation_chance = 2; // 1 en 100 prob de mutar
    const int tournament_size = 3;

    std::uniform_int_distribution<int> uniform_crossovers(0, population - 1);
    std::uniform_int_distribution<int> uniform_mutation(1, mutation_chance);
    std::uniform_real_distribution<double> uniform_solution(interval_l, interval_r);
    std::normal_distribution<double> normal_mutation(0, 0.1);
    std::uniform_real_distribution<double> uniform_01(0.0, 1.0);

    std::vector<double> gen(population);
    std::vector<double> new_gen(population);

    for (int i = 0; i < population; i++)
    {
        gen[i] = uniform_solution(rng);
        new_gen[i] = gen[i];
    }

    int generation = 0;
    double elite = gen[0];
    solution = elite;
    solution_generation = 0;
    while (generation++ < max_generations)
    {
        // Selección de torneo (Clasificación)
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
            elite = comp(function(elite), function(gen[i])) ? elite : gen[i];
            if (comp(function(elite), function(solution)))
            {
                solution = elite;
                solution_generation = generation;
            }
            solution = comp(function(elite), function(solution)) ? elite : solution;
        }
        new_gen[0] = elite;

        // Cruzamiento entre padres contiguos
        for (int i = 0; i < population - 1; i += 2)
        {
            const double alpha = uniform_01(rng);
            const double p1 = new_gen[i];
            const double p2 = new_gen[i + 1];

            new_gen[i]      = alpha * p1 + (1.0 - alpha) * p2;
            new_gen[i + 1]  = (1.0 - alpha) * p1 + alpha * p2;
            
            if (uniform_mutation(rng) == 1)
            {
                new_gen[i] += normal_mutation(rng);
            }
            if (uniform_mutation(rng) == 1)
            {
                new_gen[i + 1] += normal_mutation(rng);
            }

            if (new_gen[i] < interval_l) new_gen[i] = interval_l;
            if (new_gen[i] > interval_r) new_gen[i] = interval_r;

            if (new_gen[i + 1] < interval_l) new_gen[i + 1] = interval_l;
            if (new_gen[i + 1] > interval_r) new_gen[i + 1] = interval_r;
        }

        gen.swap(new_gen);
    }

    std::cout << "Best solution found in gen " << solution_generation << " = " << std::setprecision(20) << solution << "\n";

    return 0;
}

int main()
{
    int generation;
    double solution;
    std::ofstream results("results.csv");
    results << "Run, Generation, Solution\n";
    for (int i = 0; i < 100; i++)
    {
        optimize(generation, solution, std::greater<double>());
        results << i << ", " << generation << ", " << solution << "\n";
    }
    return 0;
}