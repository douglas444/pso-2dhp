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
    int num_particulas;
    int iterations;
    double c1;
    double c2;
    double w;
    double beta;
    enum collision_handler collision_handler;
};

typedef enum polarity Polarity;
typedef struct pso_config Pso_config;

void usage_tests();
void pso_run(Pso_config pso_config, Polarity *seq, int num_dimensions);
