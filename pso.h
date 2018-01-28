enum daemon
{
    WITHOUT_DAEMON,
    PULL_MOVE
};

enum collision_handler
{
    PARTIAL_COPY = 1
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

struct solution
{
    int energy;
    char *directions;
};

typedef enum daemon daemon;
typedef enum polarity Polarity;
typedef struct pso_config Pso_config;
typedef struct solution Solution;

void free_solution(Solution solution);
Solution pso_run(Pso_config pso_config, Polarity *seq, int num_dimensions, int *seed);
