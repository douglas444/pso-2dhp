#include <stdio.h>
#include <stdlib.h>
#include "pso.h"


#define MAX(a,b) (((a) > (b)) ? (a) : (b))

typedef enum
{
    L, R, F, B, NONE
} Position;

typedef struct pso_config
{
    int num_particulas;
    int iterations;
    double c1;
    double c2;
    double w;
} Pso_config;

typedef struct coord
{
    int x;
    int y;
} Coord;

typedef struct velocity
{
    double p_L;
    double p_R;
    double p_F;
    double p_B;
} Velocity;

typedef struct particle
{
    Velocity *velocity;
    Position *position;
    Position *pbest;
    int fitness;
    int fitness_pbest;
} Particle;



void fitness
(
    Particle *particle,
    int **lattice
)
/* ====================================
 *
 * ====================================
 */
{
}



void initializes_empty_velocity
(
    Velocity *velocity,
    int num_dimensions
)
/* =====================================================
 * Sets default values to a velocity passed by reference
 * =====================================================
 */
{
    int i;
    for (i = 0; i < num_dimensions; ++i)
    {
        velocity[i].p_L = -1;
        velocity[i].p_R = -1;
        velocity[i].p_F = -1;
        velocity[i].p_B = -1;
    }
}



void initializes_empty_position
(
    Position *position,
    int num_dimensions
)
/* =====================================================
 * Sets default values to a position passed by reference
 * =====================================================
 */
{
    int i;
    for (i = 0; i < num_dimensions; ++i)
    {
        position[i] = NONE;
    }
}



void multiplies_coefficient_by_velocity
(
    double coefficient,
    Velocity *velocity,
    int num_dimensions
)
/* =========================================================================================
 * Sets the value of velocity to the result of the multiplication of velocity by coefficient
 * =========================================================================================
 */
{
    int i;

    for (i = 0; i < num_dimensions; ++i)
    {
        if (velocity[i].p_L != -1)
        {
            velocity[i].p_L = velocity[i].p_L * coefficient;
            if (velocity[i].p_L > 1)
            {
                velocity[i].p_L = 1;
            }
        }

        if (velocity[i].p_R != -1)
        {
            velocity[i].p_R = velocity[i].p_R * coefficient;
            if (velocity[i].p_R > 1)
            {
                velocity[i].p_R = 1;
            }
        }
        if (velocity[i].p_B != -1)
        {
            velocity[i].p_B = velocity[i].p_B * coefficient;
            if (velocity[i].p_B > 1)
            {
                velocity[i].p_B = 1;
            }
        }
        if (velocity[i].p_F != -1)
        {
            velocity[i].p_F = velocity[i].p_F * coefficient;
            if (velocity[i].p_F > 1)
            {
                velocity[i].p_F = 1;
            }
        }
    }
}



void sum_velocities
(
    Velocity *v1,
    Velocity *v2,
    int num_dimensions
)
/* =======================================================
 * Set the value of velocity v1 to the sum of v1 and v2
 * =======================================================
 */
{
    int i;

    for (i = 0; i < num_dimensions; ++i)
    {
        v1[i].p_L = MAX(v1[i].p_L, v2[i].p_L);
        v1[i].p_R = MAX(v1[i].p_R, v2[i].p_R);
        v1[i].p_B = MAX(v1[i].p_B, v2[i].p_B);
        v1[i].p_F = MAX(v1[i].p_F, v2[i].p_F);
    }
}


void subtract_positions
(
    Position *p1,
    Position *p2,
    int num_dimensions
)
/* ============================================================
 * Set the value of position p1 to the subtraction of v1 and v2
 * ============================================================
 */
{
    int i, j;

    for (i = 0; i < num_dimensions; ++i)
    {
        for (j = 0; j < num_dimensions; ++j)
        {
            if (p1[i] == p2[i] && p1[i] != NONE)
            {
                p1[i] = NONE;
            }
        }
    }
}



void multiplies_coefficient_by_position
(
    Velocity *velocity,
    double coefficient,
    Position *position, int num_dimensions
)
/* =========================================================================================
 * Sets the value of velocity to the result of the multiplication of coefficient by position
 * =========================================================================================
 */
{
    int i;

    for (i = 0; i < num_dimensions; ++i)
    {
        switch(position[i])
        {
        case L:
            velocity[i].p_L = coefficient;
            break;
        case R:
            velocity[i].p_R = coefficient;
            break;
        case F:
            velocity[i].p_F = coefficient;
            break;
        case B:
            velocity[i].p_B = coefficient;
            break;
        default:
            break;
        }
    }
}



void copy_velocity
(
    Velocity *copy,
    Velocity *velocity,
    int num_dimensions
)
/* ==============================================
 * Set the value of copy to the value of velocity
 * ==============================================
 */
{
    int i;
    for (i = 0; i < num_dimensions; ++i)
    {
        copy[0].p_B = velocity[0].p_B;
        copy[0].p_F = velocity[0].p_F;
        copy[0].p_L = velocity[0].p_L;
        copy[0].p_R = velocity[0].p_R;
    }
}



void copy_position
(
    Position *copy,
    Position *position,
    int num_dimensions
)
/* ==============================================
 * Set the value of copy to the value of position
 * ==============================================
 */
{
    int i;
    for (i = 0; i < num_dimensions; ++i)
    {
        copy[i] = position[i];
    }
}



void calculate_velocity
(
    Pso_config pso_config,
    Particle *particle,
    Position *gbest,
    int num_dimensions
)
/* ==================================================
 * Adjust particle velocity following the PSO formula
 * ==================================================
 */
{
    double r1;
    double r2;

    Velocity *cognitive_velocity;
    Velocity *social_velocity;
    Position *cognitive_position;
    Position *social_position;

    cognitive_velocity = (Velocity*) malloc(sizeof(Velocity) * num_dimensions);
    social_velocity = (Velocity*) malloc(sizeof(Velocity) * num_dimensions);
    cognitive_position = (Position*) malloc(sizeof(Position) * num_dimensions);
    social_position = (Position*) malloc(sizeof(Position) * num_dimensions);

    //velocity = w * velocity
    multiplies_coefficient_by_velocity(pso_config.w, particle->velocity, num_dimensions);

    //cognitive_velocity = c1 * r1 * (pbest - xi)
    r1 = rand()/RAND_MAX;
    copy_position(cognitive_position, particle->pbest, num_dimensions);
    subtract_positions(cognitive_position, particle->position, num_dimensions);
    multiplies_coefficient_by_position(cognitive_velocity, pso_config.c1 * r1, cognitive_position, num_dimensions);

    //social_velocity = c2 * r2 * (gbest - xi)
    r2 = rand()/RAND_MAX;
    copy_position(social_position, gbest, num_dimensions);
    subtract_positions(social_position, particle->position, num_dimensions);
    multiplies_coefficient_by_position(social_velocity, pso_config.c2 * r2, social_position, num_dimensions);

    //velocity = velocity + cognitive_velocity  + social_velocity
    sum_velocities(cognitive_velocity, social_velocity, num_dimensions);
    sum_velocities(particle->velocity, cognitive_velocity, num_dimensions);

    free(cognitive_velocity);
    free(social_velocity);
    free(cognitive_position);
    free(social_position);
}



void randomize_position
(
    Position *position,
    int num_dimensions
)
/* ====================================
 * Set position to a random value
 * ====================================
 */
{
    int i;
    int r;

    for (i = 0; i < num_dimensions; ++i)
    {
        r = rand()%5;

        switch(r)
        {
        case 0:
            position[i] = L;
            break;
        case 1:
            position[i] = R;
            break;
        case 2:
            position[i] = F;
            break;
        case 3:
            position[i] = B;
            break;
        case 4:
            position[i] = NONE;
            break;
        default:
            break;
        }
    }
}



void initializes_population
(
    Pso_config pso_config,
    Particle *particles,
    Position *gbest,
    int fitness_gbest,
    int num_dimensions,
    int **lattice
)
/* ===========================================================================================
 * Sets a random position to each particles, choose the gbest and calculate initial velocities
 * ===========================================================================================
 */
{
    int i;

    fitness_gbest = 1;

    for (i = 0; i < pso_config.num_particulas; ++i)
    {
        //Randomizes position
        randomize_position(particles[i].position, num_dimensions);
        //Calculates fitness
        fitness(&(particles[i]), lattice);
        //Update pbest
        copy_position(particles[i].pbest, particles[i].position, num_dimensions);
        particles[i].fitness_pbest = particles[i].fitness;
        //Update gbest
        if (particles[i].fitness < fitness_gbest)
        {
            fitness_gbest = particles[i].fitness;
            copy_position(gbest, particles[i].pbest, num_dimensions);
        }
    }

    for (i = 0; i < pso_config.num_particulas; ++i)
    {
        initializes_empty_velocity(particles[i].velocity, num_dimensions);
        calculate_velocity(pso_config, &(particles[i]), gbest, num_dimensions);
    }

}



void usage_tests()
/* ====================================================================
 * Tests all functions of pso.c calling each one with example parameters
 * ====================================================================
 */
{

    ///MACRO: MAX
    {
        //Test variables definition
        int a = 1;
        int b = 2;
        int result;
        //Function call
        result = MAX(a, b);
        //Result check
        if (result != b)
        {
            printf("Test failed on macro MAX\n");
            exit(1);
        }
    }

    ///FUNCTION: initializes_empty_velocity
    {
        //Test variables definition
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 1);
        //Function call
        initializes_empty_velocity(velocity, 1);
        //Result check
        if (velocity[0].p_B != -1 ||
                velocity[0].p_F != -1 ||
                velocity[0].p_L != -1 ||
                velocity[0].p_R != -1)
        {
            printf("Test failed on function initializes_empty_velocity\n");
            exit(1);
        }
        //Free memory
        free(velocity);
    }

    ///FUNCTION: initializes_empty_position
    {
        //Test variables definition
        Position *position = (Position*) malloc(sizeof(Position) * 3);
        //Function call
        initializes_empty_position(position, 3);
        //Result check
        if (position[0] != NONE ||
                position[1] != NONE ||
                position[2] != NONE)
        {
            printf("Test failed on function initializes_empty_position\n");
            exit(1);
        }
        //Free memory
        free(position);
    }

    ///FUNCTION: multiplies_coefficient_by_velocity
    {
        //Test variables definition
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 2);
        initializes_empty_velocity(velocity, 2);
        velocity[1].p_L = 0.2;
        velocity[1].p_R = 0.4;
        velocity[1].p_F = 0.6;
        velocity[1].p_B = 0.8;
        //Function call
        multiplies_coefficient_by_velocity(2, velocity, 2);
        //Result check
        if (velocity[1].p_L != 0.4 ||
                velocity[1].p_R != 0.8 ||
                velocity[1].p_B != 1.0 ||
                velocity[1].p_F != 1.0)
        {
            printf("Test failed on function multiplies_coefficient_by_velocity\n");
            exit(1);
        }
        //Free memory
        free(velocity);
    }


    ///FUNCTION: sum_velocities
    {
        //Test variables definition
        Velocity *v1 = (Velocity*) malloc(sizeof(Velocity) * 2);
        initializes_empty_velocity(v1, 2);
        v1[1].p_L = 0.2;
        v1[1].p_R = 0.4;
        v1[1].p_F = 0.6;
        v1[1].p_B = 0.8;
        Velocity *v2 = (Velocity*) malloc(sizeof(Velocity) * 2);
        initializes_empty_velocity(v2, 2);
        v2[1].p_L = 0.3;
        v2[1].p_R = 0.1;
        v2[1].p_F = 0.6;
        v2[1].p_B = 0.9;
        //Function call
        sum_velocities(v1, v2, 2);
        //Result check
        if (v1[1].p_L != 0.3 ||
                v1[1].p_R != 0.4 ||
                v1[1].p_F != 0.6 ||
                v1[1].p_B != 0.9)
        {
            printf("Test failed on function sum_velocities\n");
            exit(1);
        }
        //Free memory
        free(v1);
        free(v2);
    }

    ///FUNCTION: subtract_positions
    {
        //Test variables definition
        Position *p1 = (Position*) malloc(sizeof(Position) * 2);
        initializes_empty_position(p1, 2);
        p1[1] = R;
        Position *p2 = (Position*) malloc(sizeof(Position) * 2);
        initializes_empty_position(p2, 2);
        p2[1] = R;
        //Function call
        subtract_positions(p1, p2, 2);
        //Result check
        if (p1[0] != NONE ||
                p1[1] != NONE)
        {
            printf("Test failed on function subtract_positions\n");
            exit(1);
        }
        //Free memory
        free(p1);
        free(p2);
    }

    ///FUNCTION: copy_velocity
    {
        //Test variables definition
        int i;
        Velocity *copy = (Velocity*) malloc(sizeof(Velocity) * 2);
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 2);
        initializes_empty_velocity(copy, 2);
        initializes_empty_velocity(velocity, 2);
        velocity[1].p_L = 0.2;
        velocity[1].p_R = 0.4;
        velocity[1].p_F = 0.6;
        velocity[1].p_B = 0.8;
        //Function call
        copy_velocity(copy, velocity, 2);
        //Result check
        for (i = 0; i < 2; ++i)
        {
            if (copy[0].p_B != velocity[0].p_B ||
                    copy[0].p_F != velocity[0].p_F ||
                    copy[0].p_L != velocity[0].p_L ||
                    copy[0].p_R != velocity[0].p_R)
            {
                printf("Test failed on function copy_velocity\n");
            }
        }
        //Free memory
        free(copy);
        free(velocity);
    }

    ///FUNCTION: copy_position
    {
        //Test variables definition
        int i;
        Position *copy = (Position*) malloc(sizeof(Position) * 2);
        Position *position = (Position*) malloc(sizeof(Position) * 2);
        initializes_empty_position(copy, 2);
        initializes_empty_position(position, 2);
        position[1] = L;
        //Function call
        copy_position(copy, position, 2);
        //Result check
        for (i = 0; i < 2; ++i)
        {
            if (copy[0] != position[0] ||
                    copy[1] != position[1])
            {
                printf("Test failed on function copy_position\n");
            }
        }
        //Free memory
        free(copy);
        free(position);
    }

    ///FUNCTION: calculate_velocity
    {
        //Test variables definition

        Pso_config pso_config;
        pso_config.w = 0.5;
        pso_config.c1 = 2.1;
        pso_config.c2 = 2.1;

        Position *pbest = (Position*) malloc(sizeof(Position) * 3);
        initializes_empty_position(pbest, 3);
        pbest[0] = F;
        pbest[1] = B;
        pbest[2] = L;

        Position *position = (Position*) malloc(sizeof(Position) * 3);
        initializes_empty_position(position, 3);
        position[0] = B;
        position[1] = R;
        position[2] = F;

        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 3);
        initializes_empty_velocity(velocity, 3);
        velocity[0].p_L = 0.2;
        velocity[1].p_R = 0.4;
        velocity[2].p_F = 0.6;

        Particle particle;
        particle.fitness = -5;
        particle.fitness_pbest = -7;
        particle.pbest = pbest;
        particle.position = position;
        particle.velocity = velocity;

        Position *gbest = (Position*) malloc(sizeof(Position) * 3);
        initializes_empty_position(gbest, 3);
        gbest[0] = R;
        gbest[1] = B;
        gbest[2] = L;

        //Function call
        calculate_velocity(pso_config, &particle, gbest, 3);

        //Free memory
        free(pbest);
        free(position);
        free(velocity);
        free(gbest);
    }

    ///FUNCTION: randomize_position
    {
        //Test variables definition
        Position *position = (Position*) malloc(sizeof(Position) * 3);
        //Function call
        randomize_position(position, 3);
        //Free memory
        free(position);
    }

    ///FUNCTION: initializes_population
    {}

}
