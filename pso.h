enum daemon
{
    WITHOUT_DAEMON,
    PULL_MOVE
};

enum constructor
{
    XIAO_LI_HU_2014 = 0,
    HU_ZHANG_LI_2009 = 1,
    SHMYGELSKA_HOOS_2003 = 2
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
    double min_probability;
    enum constructor constructor;
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

struct pso_result pso_run(struct pso_config pso_config, enum polarity *seq, int num_dimensions, int *seed);
