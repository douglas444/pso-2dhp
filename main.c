#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "pso.h"
#include "file.h"

int main(int argc, char **argv)
{
    int i;
    Pso_config pso_config;
    int num_dimensions;
    Polarity *sequence;

    char *input_file;
    char *collision_handler;
    char *daemon;
    char *char_sequence;
    char *sequence_key;

    int seed = -1;
    int *best_energy_evolution;
    Particle *particles;
    Solution best_solution;
    Solution *final_solutions;
    clock_t t0;
    double time;

    //Read file----------------------------------------------------------------

    if (argc < 3)
    {
        printf("Invalid parameters\n");
        exit(1);
    }

    //Extract file content
    input_file = load_file_content(argv[1]);

    //Sequence key
    sequence_key = (char*) malloc(sizeof(char) * (strlen(argv[2]) + 1));
    if (sequence_key == NULL)
    {
        printf("Error in function main: Unable to allocate memory");
        exit(1);
    }
    strcpy(sequence_key, argv[2]);

    //Get parameters from file content
    pso_config.beta = char_to_double(get_key_value(input_file, "beta"));
    pso_config.c1 = char_to_double(get_key_value(input_file, "c1"));
    pso_config.c2 = char_to_double(get_key_value(input_file, "c2"));
    pso_config.w = char_to_double(get_key_value(input_file, "w"));
    pso_config.iterations = char_to_int(get_key_value(input_file, "iterations"));

    char_sequence = get_key_value(input_file, sequence_key);
    num_dimensions= strlen(char_sequence);

    if (num_dimensions <= 25)
    {
        pso_config.population = char_to_int(get_key_value(input_file, "small-instances-population"));
    } else {
        pso_config.population = char_to_int(get_key_value(input_file, "big-instances-population"));
    }
    collision_handler = get_key_value(input_file, "collision-handler");
    daemon = get_key_value(input_file, "daemon");

    //Set local search method
    if (strcmp(daemon, "WITHOUT_DAEMON") == 0)
    {
        pso_config.daemon = WITHOUT_DAEMON;
    }
    else if (strcmp(daemon, "PULL_MOVE") == 0)
    {
        pso_config.daemon = PULL_MOVE;
    }
    else
    {
        printf("Error in function main: Local search parameter not recognized\n");
        exit(1);
    }

    //Set collision handler method
    if (strcmp(collision_handler, "PARTIAL_COPY") == 0)
    {
        pso_config.collision_handler = PARTIAL_COPY;
    }
    else
    {
        printf("Error in function main: Collision handler parameter not recognized\n");
        exit(1);
    }

    //generate protein binary sequence
    sequence = (Polarity*) malloc(num_dimensions * sizeof(Polarity));
    if (sequence == NULL)
    {
        printf("Error in function main: Unable to allocate memory");
        exit(1);
    }
    for (i = 0; i < num_dimensions; ++i)
    {
        sequence[i] = char_sequence[i] == 'H' ? H : P;
    }

    //Run PSO------------------------------------------------------------------

    init_solution(&best_solution, num_dimensions);
    best_energy_evolution = (int*) malloc(sizeof(int) * pso_config.iterations);
    particles = (Particle*) malloc(sizeof(Particle) * pso_config.population);
    final_solutions = (Solution*) malloc(sizeof(Solution) * pso_config.population);

    for (i = 0; i < pso_config.population; ++i)
    {
        init_particle(&(particles[i]), num_dimensions);
        init_solution(&(final_solutions[i]), num_dimensions);

    }

    t0 = clock();
    Position gbest = pso_run(pso_config, sequence, num_dimensions, &seed, particles, best_energy_evolution);
    time = (clock() - t0)/(double)CLOCKS_PER_SEC;

    extract_solution(gbest, &best_solution, num_dimensions);

    //Output ------------------------------------------------------------------

    printf("%d %f %s ", best_solution.energy, time, best_solution.directions);

    for (i = 0; i < num_dimensions; ++i)
    {
        printf("%c", char_sequence[i]);
    }
    printf(" ");

    for (i = 0; i < pso_config.iterations; ++i)
    {
        printf("%d\\n", best_energy_evolution[i]);
    }
    printf(" ");

    for (i = 0; i < pso_config.population; ++i)
    {
        extract_solution(particles[i].position, &(final_solutions[i]), num_dimensions);
        printf("[%d][%s]\\n", final_solutions[i].energy, final_solutions[i].directions);
    }

    //Free memory -------------------------------------------------------------

    for (i = 0; i < pso_config.population; ++i)
    {
        free_particle(particles[i]);
        free_solution(final_solutions[i]);
    }
    free_position(gbest);
    free_solution(best_solution);
    free(particles);
    free(final_solutions);
    free(sequence_key);
    free(input_file);
    free(char_sequence);
    free(sequence);
    free(best_energy_evolution);

    return 0;
}
