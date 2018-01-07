#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pso.h"

int main()
{
    srand((unsigned)time(NULL));
    usage_tests();

    Pso_config pso_config;
    pso_config.beta = 3;
    pso_config.c1 = 2;
    pso_config.c2 = 2;
    pso_config.iterations = 500;
    pso_config.num_particulas = 1500;
    pso_config.w = 0.95;
    pso_config.collision_handler = PARTIAL_COPY;

    Polarity seq[] = {
        H, H, H, H, H, H, H, H, H, H, H, H, P, H, P, H, P, P, H, H, P, P, H, H, P, P, H, P, P, H, H, P,
        P, H, H, P, P, H, P, P, H, H, P, P, H, H, P, P, H, P, H, P, H, H, H, H, H, H, H, H, H, H, H, H
    };

    int num_dimensions = 64;

    pso_run(pso_config, seq, num_dimensions);
}
