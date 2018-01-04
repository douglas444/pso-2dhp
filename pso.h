enum collision_handler
{
    PARTIAL_COPY = 1
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

void usage_tests();
