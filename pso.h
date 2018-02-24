enum daemon
{
    WITHOUT_DAEMON,
    PULL_MOVE
};

enum collision_handler
{
    PARTIAL_COPY = 0
};

enum polarity
{
    P = 0,
    H = 1
};

struct pso_config
{
    int population;
    int iterations;
    double c1;
    double c2;
    double w;
    double beta;
    enum collision_handler collision_handler;
    enum daemon daemon;
};

struct pso_result
{
    char *directions;
    int *energy_evolution;
    int energy;
    double time;
    int found_on_iteration;
    float final_population_avg;
    float final_population_stddev;
    float final_population_solution_rate;
};

typedef enum daemon Daemon;
typedef enum polarity Polarity;
typedef struct pso_config Pso_config;
typedef struct pso_result Pso_result;

Pso_result pso_run(Pso_config pso_config, Polarity *seq, int num_dimensions, int *seed);
