#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "pso.h"
#include "file.h"

void set_max_priority();
Daemon string_to_daemon(char *daemon);
Daemon string_to_collision_handler(char *collision_handler);
Daemon char_to_polarity(char polarity);
void read_inputs(Pso_config *pso_config, Polarity **sequence, int *num_dimensions, char **argv, char **char_sequence);

int main(int argc, char **argv)
{
    set_max_priority();

    int i;
    char *char_sequence;
    Pso_config pso_config;
    Pso_result pso_result;
    Polarity *sequence;
    int num_dimensions;
    int seed = -1;

    if (argc < 3)
    {
        printf("ERROR: main.c/main(): \"Invalid application parameters\"\n");
        exit(1);
    }

    seed = -1;
    read_inputs(&pso_config, &sequence, &num_dimensions, argv, &char_sequence);
    pso_result = pso_run(pso_config, sequence, num_dimensions, &seed);

    //output
    printf("%d|", pso_config.iterations);
    printf("%s|", char_sequence);
    printf("%s|", pso_result.directions);
    printf("%d|", pso_result.energy);
    printf("%.2f|", pso_result.final_population_avg);
    printf("%.2f|", pso_result.final_population_stddev);
    printf("%.2f|", pso_result.final_population_solution_rate);
    printf("%d|", pso_result.found_on_iteration);
    printf("%.2f|", pso_result.time);
    for (i = 0; i < pso_config.iterations; ++i) {
        if (pso_result.energy_evolution[i] != 1) {
            printf("%d,%d/", i, pso_result.energy_evolution[i]);
        }
    }

    //free
    free(pso_result.directions);
    free(pso_result.energy_evolution);
    free(sequence);
    free(char_sequence);

    return 0;
}

void set_max_priority() {

    if (setpriority(PRIO_PROCESS, 0, -20) == -1) {
        printf("ERROR: main.c/set_max_priority(): \"Setpriority() failed\"\n");
        exit(1);
    }

}

Daemon string_to_daemon(char *daemon)
{
    if (strcmp(daemon, "WITHOUT_DAEMON") == 0)
    {
        return WITHOUT_DAEMON;
    }
    else if (strcmp(daemon, "PULL_MOVE") == 0)
    {
        return PULL_MOVE;
    }
    else
    {
        printf("ERROR: main.c/string_to_daemon(): \"Invalid value for daemon parameter\"\n");
        exit(1);
    }
}

Daemon string_to_collision_handler(char *collision_handler)
{
    if (strcmp(collision_handler, "PARTIAL_COPY") == 0)
    {
        return PARTIAL_COPY;
    }
    else
    {
        printf("ERROR: main.c/string_to_collision_handler(): \"Invalid value for collision_handler parameter\"\n");
        exit(1);
    }
}

Daemon char_to_polarity(char polarity)
{
    if (polarity == 'H')
    {
        return H;
    }
    else if (polarity == 'P')
    {
        return P;
    }
    else
    {
        printf("ERROR: main.c/char_to_polarity(): \"Invalid value for polarity parameter\"\n");
        exit(1);
    }

}

void read_inputs(Pso_config *pso_config, Polarity **sequence, int *num_dimensions, char **argv, char **char_sequence)
{
    int i;
    char *input_file;
    char *collision_handler;
    char *daemon;
    char *sequence_key;

    sequence_key = (char*) malloc(sizeof(char) * (strlen(argv[2]) + 1));
    if (sequence_key == NULL)
    {
        printf("ERROR: main.c/read_inputs(): \"Unable to allocate memory\"\n");
        exit(1);
    }

    //get inputs
    strcpy(sequence_key, argv[2]);
    input_file = load_file_content(argv[1]);

    //extract from inputs
    pso_config->beta = char_to_double(get_key_value(input_file, "beta"));
    pso_config->c1 = char_to_double(get_key_value(input_file, "c1"));
    pso_config->c2 = char_to_double(get_key_value(input_file, "c2"));
    pso_config->w = char_to_double(get_key_value(input_file, "w"));
    pso_config->iterations = char_to_int(get_key_value(input_file, "iterations"));

    *char_sequence = get_key_value(input_file, sequence_key);
    collision_handler = get_key_value(input_file, "collision-handler");
    daemon = get_key_value(input_file, "daemon");

    *num_dimensions = strlen(*char_sequence);
    pso_config->population = char_to_int(get_key_value(input_file,
        (*num_dimensions <= 25) ? "small-instances-population" : "big-instances-population"));
    pso_config->daemon = string_to_daemon(daemon);
    pso_config->collision_handler = string_to_collision_handler(collision_handler);

    *sequence = (Polarity*) malloc(*num_dimensions * sizeof(Polarity));
    if (*sequence == NULL)
    {
        printf("ERROR: main.c/read_inputs(): \"Unable to allocate memory\"\n");
        exit(1);
    }
    for (i = 0; i < *num_dimensions; ++i)
    {
        (*sequence)[i] = char_to_polarity((*char_sequence)[i]);
    }

    //free
    free(input_file);
    free(collision_handler);
    free(daemon);
    free(sequence_key);
}
