#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "pso.h"



#define MAX(a,b) (((a) > (b)) ? (a) : (b))



struct coord
{
    int x;
    int y;
};

struct candidate
{
    int heuristic;
    struct coord coord;
    struct coord move;
};

struct velocity
{
    double l;
    double r;
    double s;
};

struct particle
{
    struct velocity *velocity;
    struct position position;
    struct position pbest;
};

enum pm_type
{
    ORIGINAL = 0,
    INVERSE = 1
};

struct pm_config
{
    int amino_acid_index;
    struct coord curr;
    struct coord prev;
    struct coord next;
    struct coord f;
    struct coord c;
    enum pm_type pm_type;

};


typedef struct coord Coord;
typedef struct candidate Candidate;
typedef struct velocity Velocity;
typedef struct particle Particle;
typedef struct pm_config Pm_config;
typedef enum pm_type Pm_type;




void init_solution(Solution *solution, int num_dimensions)
/* ====================================================
 * Allocates memory to Solution structure variables
 * ====================================================
 */
{
    solution->directions = (char*) malloc(sizeof(char) * (num_dimensions + 1));
}



void free_solution(Solution solution)
/* ===========================================
 * Free memory of Solution structure variables
 * ===========================================
 */
{
    free(solution.directions);
}



Coord create_new_coord(int x, int y)
/* ===========================================================
 * Returns a new variables of type Coord with the given values
 * ===========================================================
 */
{
    Coord coord;
    coord.x = x;
    coord.y = y;
    return coord;
}



Coord subtract_coord
(
    Coord c1,
    Coord c2
)
/* =======================================================
 * Calculates the difference between two given coordinates
 * =======================================================
 */
{
    Coord c3;

    c3.x = c1.x - c2.x;
    c3.y = c1.y - c2.y;

    return c3;
}



int lateral_adj(Coord c1, Coord c2)
/* ===============================================
 * Check if two coordinates are laterally adjacent
 * ===============================================
 */
{
    if (abs(c1.x - c2.x) + abs(c1.y - c2.y) == 1) {
        return 1;
    } else {
        return 0;
    }

}



int abs_direction_by_move(Coord move)
/* =====================================
 * Calculates absolute direction by move
 * =====================================
 */
{
    if (move.x == 0)
    {
        if (move.y == 1 || move.y == -1)
        {
            return STRAIGHT;
        }
        else
        {
            printf("Error in function aco.c/abs_direction_by_move: Invalid value for parameter move\n");
            exit(1);
        }
    }
    else if (move.y == 0)
    {
        if (move.x == 1)
        {
            return LEFT;
        }
        else if (move.x == -1)
        {
            return RIGHT;
        }
        else
        {
            printf("Error in function aco.c/abs_direction_by_move: Invalid value for parameter move\n");
            exit(1);
        }
    }
    else
    {
        printf("Error in function aco.c/abs_direction_by_move: Invalid value for parameter move\n");
        exit(1);
    }
}



int direction_by_move(Coord prev_move, Coord move)
/* ===========================================================
 * Calculates relative direction between two consecutive moves
 * ===========================================================
 */
{

    if (prev_move.x == move.x && prev_move.y == move.y)
    {
        return STRAIGHT;
    }
    else if (prev_move.y == 0)
    {
        if (move.y == prev_move.x)
        {
            return LEFT;
        }
        else if (abs(move.y) == abs(prev_move.x))
        {
            return RIGHT;
        }
        else
        {
            printf("Error in function aco.c/direction_by_move: Invalid values for parameters\n");
            exit(1);
        }
    }
    else
    {
        if (move.x == prev_move.y)
        {
            return RIGHT;
        }
        else if (abs(move.x) == abs(prev_move.y))
        {
            return LEFT;
        }
        else
        {
            printf("Error in function aco.c/direction_by_move: Invalid values for parameters\n");
            exit(1);
        }
    }
}



Coord straight(Coord prev_move)
/* =================================================================
 * Calculates straight move corresponding to the given previous move
 * =================================================================
 */
{
    Coord move;

    if (prev_move.x == 1)
    {
        move.x = 1;
        move.y = 0;
    }
    else if (prev_move.x == -1)
    {
        move.x = -1;
        move.y = 0;
    }
    else if (prev_move.y == -1)
    {
        move.x = 0;
        move.y = -1;
    }
    else if (prev_move.y == 1)
    {
        move.x = 0;
        move.y = 1;
    }
    else
    {
        printf("Error in function aco.c/straight: Invalid value for parameter prev_move\n");
        exit(1);
    }

    return move;
}



Coord left(Coord prev_move)
/* =============================================================
 * Calculates left move corresponding to the given previous move
 * =============================================================
 */
{
    Coord move;

    if (prev_move.x == 1)
    {
        move.x = 0;
        move.y = 1;
    }
    else if (prev_move.x == -1)
    {
        move.x = 0;
        move.y = -1;
    }
    else if (prev_move.y == -1)
    {
        move.x = 1;
        move.y = 0;
    }
    else if (prev_move.y == 1)
    {
        move.x = -1;
        move.y = 0;
    }
    else
    {
        printf("Error in function aco.c/left: Invalid value for parameter prev_move\n");
        exit(1);
    }

    return move;
}



Coord right(Coord prev_move)
/* ==============================================================
 * Calculates right move corresponding to the given previous move
 * ==============================================================
 */
{
    Coord move;

    if (prev_move.x == 1)
    {
        move.x = 0;
        move.y = -1;
    }
    else if (prev_move.x == -1)
    {
        move.x = 0;
        move.y = 1;
    }
    else if (prev_move.y == -1)
    {
        move.x = -1;
        move.y = 0;
    }
    else if (prev_move.y == 1)
    {
        move.x = 1;
        move.y = 0;
    }
    else
    {
        printf("Error in function aco.c/right: Invalid value for parameter prev_move\n");
        exit(1);
    }

    return move;
}



void extract_solution
(
    Position position,
    Solution *solution,
    int num_dimensions
)
/* ===============================================
 * Extracts conformation info from Ant to Solution
 * ===============================================
 */
{
    Coord move, prev_move;
    int i, direction;

    solution->energy = position.fitness;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        move = subtract_coord(position.coord[i + 1], position.coord[i]);

        if (i == 0)
        {
            direction = abs_direction_by_move(move);
        }
        else
        {
            direction = direction_by_move(prev_move, move);
        }

        switch(direction)
        {
        case LEFT:
            solution->directions[i] = 'L';
            break;
        case RIGHT:
            solution->directions[i] = 'R';
            break;
        case STRAIGHT:
            solution->directions[i] = 'S';
            break;
        default:
            break;
        }

        prev_move = move;
    }

    solution->directions[num_dimensions - 1] = '\0';
}



void init_position
(
    Position *position,
    int num_dimensions
)
/* ================================================
 * Allocates memory to Position structure variables
 * ================================================
 */
{
    int i;

    position->coord = (Coord*) malloc(sizeof(Coord) * num_dimensions);
    position->dir = (Direction*) malloc(sizeof(Direction) * (num_dimensions - 1));
    position->fitness_by_edge = (int*) malloc(sizeof(int) * (num_dimensions - 1));
    position->feasible = FALSE;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        position->dir[i] = NONE;
    }
}



void free_position(Position position)
/* ============================================
 * Frees memory of Position structure variables
 * ============================================
 */
{
    free(position.coord);
    free(position.dir);
    free(position.fitness_by_edge);
}



void init_particle
(
    Particle *particle,
    int num_dimensions
)
/* ================================================
 * Allocates memory to Particle structure variables
 * ================================================
 */
{
    init_position(&(particle->position), num_dimensions);
    init_position(&(particle->pbest), num_dimensions);
    particle->velocity = (Velocity*) malloc(sizeof(Velocity) * (num_dimensions - 1));
}



void free_particle(Particle particle)
/* ============================================
 * Frees memory of Particle structure variables
 * ============================================
 */
{
    free_position(particle.position);
    free_position(particle.pbest);
    free(particle.velocity);
}



void set_default_velocity
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
    for (i = 0; i < num_dimensions - 1; ++i)
    {
        velocity[i].l = 0;
        velocity[i].r = 0;
        velocity[i].s = 0;
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

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        velocity[i].l = velocity[i].l * coefficient;
        velocity[i].r = velocity[i].r * coefficient;
        velocity[i].s = velocity[i].s * coefficient;

        if (velocity[i].l > 1) velocity[i].l = 1;
        if (velocity[i].r > 1) velocity[i].r = 1;
        if (velocity[i].s > 1) velocity[i].s = 1;
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

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        v1[i].l = MAX(v1[i].l, v2[i].l);
        v1[i].r = MAX(v1[i].r, v2[i].r);
        v1[i].s = MAX(v1[i].s, v2[i].s);
    }
}



void subtract_positions
(
    Position *p1,
    Position p2,
    int num_dimensions
)
/* ============================================================
 * Set the value of position p1 to the subtraction of p1 and p2
 * ============================================================
 */
{
    int i;

    p1->feasible = FALSE;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        if (p1->dir[i] == p2.dir[i] && p1->dir[i] != NONE)
        {
            p1->dir[i] = NONE;
        }
    }
}



void multiplies_coefficient_by_position
(
    Velocity *velocity,
    double coefficient,
    Position position,
    int num_dimensions
)
/* =========================================================================================
 * Sets the value of velocity to the result of the multiplication of coefficient by position
 * =========================================================================================
 */
{
    int i;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        switch(position.dir[i])
        {
        case LEFT:
            velocity[i].l = coefficient;
            if (velocity[i].l > 1) velocity[i].l = 1;
            break;
        case RIGHT:
            velocity[i].r = coefficient;
            if (velocity[i].r > 1) velocity[i].r = 1;
            break;
        case STRAIGHT:
            velocity[i].s = coefficient;
            if (velocity[i].s > 1) velocity[i].s = 1;
            break;
        default:
            velocity[i].l = 0;
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
    for (i = 0; i < num_dimensions - 1; ++i)
    {
        copy[0].s = velocity[0].s;
        copy[0].l = velocity[0].l;
        copy[0].r = velocity[0].r;
    }
}



void copy_position
(
    Position *copy,
    Position position,
    int num_dimensions
)
/* ==============================================
 * Set the value of copy to the value of position
 * ==============================================
 */
{
    int i;

    copy->feasible = position.feasible;
    copy->fitness = position.fitness;

    for (i = 0; i < num_dimensions; ++i)
    {
        if (i < num_dimensions - 1)
        {
            copy->dir[i] = position.dir[i];
            copy->fitness_by_edge[i] = position.fitness_by_edge[i];
        }
        copy->coord[i] = position.coord[i];
    }
}



void update_velocity
(
    Pso_config pso_config,
    Particle *particle,
    Position gbest,
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
    Position cognitive_position;
    Position social_position;

    cognitive_velocity = (Velocity*) malloc(sizeof(Velocity) * num_dimensions);
    social_velocity = (Velocity*) malloc(sizeof(Velocity) * num_dimensions);
    init_position(&cognitive_position, num_dimensions);
    init_position(&social_position, num_dimensions);

    set_default_velocity(cognitive_velocity, num_dimensions);
    set_default_velocity(social_velocity, num_dimensions);

    //velocity = w * velocity
    multiplies_coefficient_by_velocity(pso_config.w, particle->velocity, num_dimensions);

    //cognitive_velocity = c1 * r1 * (pbest - xi)
    r1 = rand()/RAND_MAX;
    copy_position(&cognitive_position, particle->pbest, num_dimensions);
    subtract_positions(&cognitive_position, particle->position, num_dimensions);
    multiplies_coefficient_by_position(cognitive_velocity, pso_config.c1 * r1, cognitive_position, num_dimensions);

    //social_velocity = c2 * r2 * (gbest - xi)
    r2 = rand()/RAND_MAX;
    copy_position(&social_position, gbest, num_dimensions);
    subtract_positions(&social_position, particle->position, num_dimensions);
    multiplies_coefficient_by_position(social_velocity, pso_config.c2 * r2, social_position, num_dimensions);

    //velocity = velocity + cognitive_velocity  + social_velocity
    sum_velocities(cognitive_velocity, social_velocity, num_dimensions);
    sum_velocities(particle->velocity, cognitive_velocity, num_dimensions);

    free(cognitive_velocity);
    free(social_velocity);
    free_position(cognitive_position);
    free_position(social_position);
}



int calculate_relative_heuristic
(
    int **lattice,
    int amino_acid_index,
    Coord pos,
    Polarity *seq
)
/* =================================================================================
 * Calculates the number of H-H contacts if a H amino-acid occupy the given position
 * =================================================================================
 */
{
    int heuristic_value = 0;
    int right_neighbor = lattice[pos.x + 1][pos.y];
    int left_neighbor = lattice[pos.x - 1][pos.y];
    int down_neighbor = lattice[pos.x][pos.y - 1];
    int up_neighbor = lattice[pos.x][pos.y + 1];

    if (right_neighbor >= 0 && right_neighbor - amino_acid_index < -1
        && seq[right_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (left_neighbor >= 0 && left_neighbor - amino_acid_index < -1
        && seq[left_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (up_neighbor >= 0 && up_neighbor - amino_acid_index < -1
        && seq[up_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (down_neighbor >= 0 && down_neighbor - amino_acid_index < -1
        && seq[down_neighbor] == H)
    {
        ++heuristic_value;
    }

    return heuristic_value;
}



int calculate_heuristic
(
    int **lattice,
    int amino_acid_index,
    Coord pos,
    Polarity *seq
)
/* =================================================================================
 * Calculates the number of H-H contacts if a H amino-acid occupy the given position
 * =================================================================================
 */
{
    int heuristic_value = 0;
    int right_neighbor = lattice[pos.x + 1][pos.y];
    int left_neighbor = lattice[pos.x - 1][pos.y];
    int down_neighbor = lattice[pos.x][pos.y - 1];
    int up_neighbor = lattice[pos.x][pos.y + 1];

    if (right_neighbor >= 0 && abs(right_neighbor - amino_acid_index) > 1
        && seq[right_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (left_neighbor >= 0 && abs(left_neighbor - amino_acid_index) > 1
        && seq[left_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (up_neighbor >= 0 && abs(up_neighbor - amino_acid_index) > 1
        && seq[up_neighbor] == H)
    {
        ++heuristic_value;
    }
    if (down_neighbor >= 0 && abs(down_neighbor - amino_acid_index) > 1
        && seq[down_neighbor] == H)
    {
        ++heuristic_value;
    }

    return heuristic_value;
}



void adjust_particle_by_coord
(
    Position *position,
    Polarity *seq,
    int **lattice,
    int num_dimensions
)
/* ================================================
 * Adjust direction and energy based on coordinates
 * ================================================
 */
{
    int i;
    Coord move;
    Coord prev_move;
    Direction direction;
    position->fitness = 0;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        move = subtract_coord(position->coord[i + 1], position->coord[i]);

        if (seq[i] == H)
        {
            position->fitness -= calculate_relative_heuristic(lattice, i, position->coord[i], seq);
        }

        position->fitness_by_edge[i] = position->fitness;

        if (i == 0)
        {
            direction = abs_direction_by_move(move);
        }
        else
        {
            direction = direction_by_move(prev_move, move);
        }

        position->dir[i] = direction;
        prev_move = move;
    }

    if (seq[i] == H)
    {
        position->fitness -= calculate_relative_heuristic(lattice, num_dimensions - 1, position->coord[i], seq);
    }
}



int random_select(double *probabilities, int len)
/* =====================================================================
 * Randomly chooses a integer between 0 and len with given probabilities
 * =====================================================================
 */
{
    int i = 0;
    int result = -1;
    double cumulative_probability = 0;
    double r = ((double)rand()/RAND_MAX);

    while(result == -1)
    {
        cumulative_probability += probabilities[i];
        if(r <= cumulative_probability || i == len - 1)
        {
            result = i;
        }
        ++i;
    }
    return result;
}



void update_position
(
    Particle *particle,
    int **lattice,
    Polarity *seq,
    int num_dimensions,
    Pso_config pso_config,
    int *best_particle_by_edge,
    int particle_index,
    Particle *particles
)
/* ============================================================================
 * Sets particles position to feasible position built using particle's velocity
 * ============================================================================
 */
{
    int i;
    int j;
    int num_candidates;
    int selected_candidate;
    double sum_probabilities;
    double probabilities[3];
    Coord curr_coord;
    Coord move;
    Coord candidate_move[3];
    Candidate candidates[3];
    Position copied_position;

    particle->position.fitness = 0;
    particle->position.fitness_by_edge[0] = 0;

    //DEFINES FIRST EDGE

    lattice[num_dimensions][num_dimensions] = 0;
    particle->position.coord[0] = create_new_coord(num_dimensions, num_dimensions);

    move = create_new_coord(0, 1);
    curr_coord = create_new_coord(particle->position.coord[0].x + move.x,
                                  particle->position.coord[0].y + move.y);
    lattice[curr_coord.x][curr_coord.y] = 1;
    particle->position.coord[1] = curr_coord;


    //CONSTRUCTOR LOOP

    //For each edge except the first
    for (i = 1; i < num_dimensions - 1; ++i)
    {
        sum_probabilities = 0;
        num_candidates = 0;

        candidate_move[0] = left(move);
        candidate_move[1] = right(move);
        candidate_move[2] = straight(move);

        //DEFINES CANDIDATES DIRECTION

        //For each direction
        for (j = 0; j < 3; ++j)
        {

            //If the next coord in this direction is not occupied, turns current direction into a candidate
            if (lattice[curr_coord.x + candidate_move[j].x][curr_coord.y + candidate_move[j].y] == -1)
            {
                candidates[num_candidates].move = candidate_move[j];
                candidates[num_candidates].coord.x = curr_coord.x + candidates[num_candidates].move.x;
                candidates[num_candidates].coord.y = curr_coord.y + candidates[num_candidates].move.y;

                if (seq[i + 1] == H)
                {
                    candidates[num_candidates].heuristic =
                        calculate_heuristic(lattice, i + 1, candidates[num_candidates].coord, seq);
                }
                else
                {
                    candidates[num_candidates].heuristic = 0;
                }

                probabilities[num_candidates] = pow(exp((double) candidates[num_candidates].heuristic / 0.3), pso_config.beta);

                switch(j) {
                case LEFT:
                    probabilities[num_candidates] *= particle->velocity[i].l;
                    break;
                case RIGHT:
                    probabilities[num_candidates] *= particle->velocity[i].r;
                    break;
                case STRAIGHT:
                    probabilities[num_candidates] *= particle->velocity[i].s;
                    break;
                default:
                    break;
                }

                sum_probabilities += probabilities[num_candidates];
                ++num_candidates;
            }

        }

        for (j = 0; j < num_candidates; ++j)
        {
            probabilities[j] = probabilities[j]/sum_probabilities;
        }

        //SELECTS A CANDIDATE

        if (num_candidates == 0)
        {
            selected_candidate = -1;
        }
        else if (sum_probabilities == 0)
        {
            selected_candidate = rand() % num_candidates;
        }
        else if (num_candidates > 1)
        {
            selected_candidate = random_select(probabilities, num_candidates);
        }
        else if (num_candidates == 1)
        {
            selected_candidate = 0;
        }

        //UPDATE CONFORMATION

        if (selected_candidate != -1)
        {
            move = candidates[selected_candidate].move;
            curr_coord = candidates[selected_candidate].coord;
            particle->position.fitness_by_edge[i] = particle->position.fitness;
            particle->position.fitness -= candidates[selected_candidate].heuristic;
            particle->position.coord[i + 1] = curr_coord;
            lattice[curr_coord.x][curr_coord.y] = i + 1;

        }
        else
        {
            //WHEN IS IMPOSSIBLE CONTINUE THE FOLD PROCESS

            switch (pso_config.collision_handler)
            {

            case PARTIAL_COPY:

                //If theres no another ant to copy
                if (best_particle_by_edge[i] == -1)
                {
                    for (j = 0; j <= i; ++j)
                    {
                        lattice[particle->position.coord[j].x][particle->position.coord[j].y] = -1;
                    }

                    i = 0;
                    particle->position.fitness = 0;

                    //Reset first edge
                    lattice[num_dimensions][num_dimensions] = 0;
                   particle->position.coord[0] = create_new_coord(num_dimensions, num_dimensions);

                    move = create_new_coord(0, 1);
                    curr_coord = create_new_coord(particle->position.coord[0].x + move.x,
                                                  particle->position.coord[0].y + move.y);
                    lattice[curr_coord.x][curr_coord.y] = 1;
                    particle->position.coord[1] = curr_coord;
                }
                else
                {
                    copied_position = particles[best_particle_by_edge[i]].position;

                    for (j = 0; j <= i; ++j)
                    {
                        lattice[particle->position.coord[j].x][particle->position.coord[j].y] = -1;
                    }

                    //Copy best ant for index i until i th amino-acid*/
                    for (j = 0; j <= i; ++j)
                    {
                        lattice[copied_position.coord[j].x][copied_position.coord[j].y] = j;
                        particle->position.coord[j] = copied_position.coord[j];
                        if (j != i)
                        {
                            particle->position.fitness_by_edge[j] = copied_position.fitness_by_edge[j];
                        }
                    }
                    curr_coord = copied_position.coord[i];
                    particle->position.fitness = copied_position.fitness_by_edge[i];

                    move = create_new_coord(curr_coord.x - copied_position.coord[i - 1].x,
                                            curr_coord.y - copied_position.coord[i - 1].y);
                    --i;
                }
                break;

            default:
                break;
            }
        }
    }

    particle->position.feasible = TRUE;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        if (best_particle_by_edge[i] == -1 ||
            particles[best_particle_by_edge[i]].position.fitness_by_edge[i] > particle->position.fitness_by_edge[i])
        {
            best_particle_by_edge[i] = particle_index;
        }
    }

    for (i = 0; i < num_dimensions; ++i)
    {
        lattice[particle->position.coord[i].x][particle->position.coord[i].y] = -1;
    }
}



void randomize_velocity
(
    Velocity *velocity,
    int num_dimensions
)
/* ====================================
 * Set position to a random value
 * ====================================
 */
{
    int i;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        velocity[i].s = rand()/RAND_MAX;
        velocity[i].l = rand()/RAND_MAX;
        velocity[i].r = rand()/RAND_MAX;
    }
}



void initializes_population
(
    Pso_config pso_config,
    Particle *particles,
    Position *gbest,
    int num_dimensions,
    int *best_particle_by_edge,
    int **lattice,
    Polarity *seq

)
/* ===========================================================================================
 * Sets a random position to each particles, choose the gbest and calculate initial velocities
 * ===========================================================================================
 */
{
    int i;

    gbest->fitness = 1;

    for (i = 0; i < pso_config.population; ++i)
    {
        //Randomizes velocity
        randomize_velocity(particles[i].velocity, num_dimensions);
        //Initialize position using current velocity
        update_position(&(particles[i]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, i, particles);
        //Update pbest
        copy_position(&(particles[i].pbest), particles[i].position, num_dimensions);
        //Update gbest
        if (particles[i].position.fitness < gbest->fitness)
        {
            copy_position(gbest, particles[i].pbest, num_dimensions);
        }
    }

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        best_particle_by_edge[i] = -1;
    }

    for (i = 0; i < pso_config.population; ++i)
    {
        set_default_velocity(particles[i].velocity, num_dimensions);
        update_velocity(pso_config, &(particles[i]), *gbest, num_dimensions);
    }

}


int move_amino_acid
(
    int current_energy,
    int **lattice,
    Polarity *seq,
    int amino_acid_index,
    Coord src,
    Coord dest
)
/* ==========================================================================
 * Moves a amino-acid to another position in the lattice, handling the energy
 * ==========================================================================
 */
{

    int temp;

    if (seq[amino_acid_index] == H)
    {
        current_energy += calculate_heuristic(lattice, amino_acid_index, src, seq);
        current_energy -= calculate_heuristic(lattice, amino_acid_index, dest, seq);
    }

    temp = lattice[src.x][src.y];
    lattice[src.x][src.y] = lattice[dest.x][dest.y];
    lattice[dest.x][dest.y] = temp;

    return current_energy;
}



int apply_pm
(
    Position position,
    Pm_config config,
    Polarity *seq,
    int **lattice,
    int num_dimensions,
    Coord *particle_coord
)
/* ========================================================
 * Applies the pull-move configuration to the given protein
 * ========================================================
 */
{

    int i;
    int last_modified;
    int previous_index;
    int before_previous_index;
    Coord tempCoord;

    if (config.pm_type == ORIGINAL)
    {
        previous_index = config.amino_acid_index - 1;
        before_previous_index = config.amino_acid_index - 2;
        last_modified = 0;
    }
    else
    {
        previous_index = config.amino_acid_index + 1;
        before_previous_index = config.amino_acid_index + 2;
        last_modified = num_dimensions - 1;
    }

    if (config.c.x == config.prev.x && config.c.y == config.prev.y)
    {
        position.fitness = move_amino_acid(position.fitness, lattice, seq,
                                           config.amino_acid_index,
                                           config.curr, config.f);

        position.coord[config.amino_acid_index] = config.f;
        config.curr =  position.coord[config.amino_acid_index];
    }
    else
    {
        position.fitness = move_amino_acid(position.fitness, lattice, seq,
                                           previous_index, config.prev,
                                           config.c);

        position.coord[previous_index] = config.c;

        position.fitness = move_amino_acid(position.fitness, lattice, seq,
                                           config.amino_acid_index,
                                           config.curr, config.f);

        position.coord[config.amino_acid_index] = config.f;

        i = before_previous_index;

        while ((config.pm_type == ORIGINAL && i < num_dimensions - 1 && i >= 0 &&
                lateral_adj(position.coord[i], position.coord[i + 1]) == 0) ||
                (config.pm_type == INVERSE && i > 0 && i <= num_dimensions - 1 &&
                 lateral_adj(position.coord[i], position.coord[i - 1]) == 0))
        {
            position.fitness = move_amino_acid(position.fitness, lattice, seq, i,
                                               position.coord[i], config.curr);

            tempCoord = position.coord[i];
            position.coord[i] = config.curr;
            config.curr = config.prev;
            config.prev = tempCoord;

            last_modified = i;

            if (config.pm_type == ORIGINAL)
            {
                --i;
            }
            else if (config.pm_type == INVERSE)
            {
                ++i;
            }

        }

    }

    //DEVOLVES LATTICE TO ORIGINAL STATE

    int start;
    int end;

    if (config.pm_type == ORIGINAL)
    {
        start = last_modified;
        end = config.amino_acid_index;
    }
    else
    {
        end = last_modified;
        start = config.amino_acid_index;
    }

    for (i = start; i <= end; ++i)
    {
        lattice[position.coord[i].x][position.coord[i].y] = -1;
    }

    for (i = start; i <= end; ++i)
    {
        lattice[particle_coord[i].x][particle_coord[i].y] = i;
    }

    return position.fitness;
}



int generate_pm_config
(
    Pm_config *config,
    int **lattice,
    Pm_type pm_type,
    Coord curr,
    Coord prev,
    Coord next,
    Coord direction
)
/* =====================================================================================
 * Generates a pull-move configuration for a specific direction of a specific amino-acid
 * =====================================================================================
 */
{
    /* F exists? */
    if (lattice[next.x + direction.x][next.y + direction.y] == -1 &&
        next.x + direction.x != curr.x && next.y + direction.y != curr.y)
    {
        config->f.x = next.x + direction.x;
        config->f.y = next.y + direction.y;
        config->c.x = config->f.x + curr.x - next.x;
        config->c.y = config->f.y + curr.y - next.y;

        /*C is empty or is equals do previous amino-acid*/
        if ((config->c.x == prev.x && config->c.y == prev.y) ||
            lattice[config->c.x][config->c.y] == -1)
        {
                config->next = next;
                config->curr = curr;
                config->prev = prev;
                config->pm_type = pm_type;

                return 1;
        }

    }

    /* Impossible generate the pull-move configuration */
    return 0;
}



void generate_pm_configs
(
    int amino_acid_index,
    int **lattice,
    Coord *particle_coord,
    Pm_config *configs,
    int *config_index,
    int num_dimensions
)
/* ============================================================
 * Generates pull-move configurations for a specific amino-acid
 * ============================================================
 */
{
    int result;
    Pm_config config;
    config.amino_acid_index = amino_acid_index;

    Coord right_move = create_new_coord(1, 0);
    Coord left_move = create_new_coord(-1, 0);
    Coord up_move = create_new_coord(0, 1);
    Coord down_move = create_new_coord(0, -1);

    Coord next = particle_coord[amino_acid_index + 1];
    Coord prev = particle_coord[amino_acid_index - 1];
    Coord curr = particle_coord[amino_acid_index];

    result = generate_pm_config(&config, lattice, ORIGINAL, curr, prev, next, right_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, ORIGINAL, curr, prev, next, left_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, ORIGINAL, curr, prev, next, up_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, ORIGINAL, curr, prev, next, down_move);
    if (result) configs[(*config_index)++] = config;

    next = particle_coord[amino_acid_index - 1];
    prev = particle_coord[amino_acid_index + 1];

    result = generate_pm_config(&config, lattice, INVERSE, curr, prev, next, right_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, INVERSE, curr, prev, next, left_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, INVERSE, curr, prev, next, up_move);
    if (result) configs[(*config_index)++] = config;
    result = generate_pm_config(&config, lattice, INVERSE, curr, prev, next, down_move);
    if (result) configs[(*config_index)++] = config;

}



Boolean pm_search
(
    Polarity *seq,
    int num_dimensions,
    Particle *original_particle,
    int **lattice,
    Position best_position,
    Position position,
    Pm_config *configs
)
/* ============================================================
 * Applies pull-moves on the protein while there is improvement
 * ============================================================
 */
{
    int i;
    int j;
    int num_configs = 0;
    int previous_fitness;
    Coord lattice_adjust;
    Boolean change = FALSE;

    best_position.fitness = 0;

    //APPLY PULL-MOVE WHILE THERE IS IMPROVEMENT

    do
    {
        num_configs = 0;
        previous_fitness = original_particle->position.fitness;

        for (i = 0; i < num_dimensions; ++i)
        {
            lattice[original_particle->position.coord[i].x][original_particle->position.coord[i].y] = i;
        }

        //GENERATE ALL POSSIBLE PULL-MOVES CONFIGURATIONS

        for (i = 1; i < num_dimensions - 1; ++i)
        {
            generate_pm_configs(i, lattice, original_particle->position.coord,
                                configs, &num_configs, num_dimensions);
        }

        /*Apply all possible pull-moves and save the best*/
        for (i = 0; i < num_configs; ++i)
        {
            /*Copy original particle*/
            position.fitness = original_particle->position.fitness;
            for (j = 0; j < num_dimensions; ++j)
            {
                position.coord[j] = original_particle->position.coord[j];
            }

            /*Apply pull move on the copy*/
            position.fitness = apply_pm(position, configs[i], seq, lattice,
                                        num_dimensions, original_particle->position.coord);

            if (position.fitness < best_position.fitness)
            {
                best_position.fitness = position.fitness;

                for (j = 0; j < num_dimensions; ++j)
                {
                    best_position.coord[j] = position.coord[j];
                }
            }
        }

        //CHECKS IF NEW CONFORMATION IS BETTER THAN ORIGINAL CONFORMATION

        if (best_position.fitness != 0 && best_position.fitness < original_particle->position.fitness)
        {
            change = TRUE;

            lattice_adjust = create_new_coord(num_dimensions - best_position.coord[0].x,
                                              num_dimensions - best_position.coord[0].y);

            original_particle->position.fitness = previous_fitness;

            for (i = 0; i < num_dimensions; ++i)
            {
                lattice[original_particle->position.coord[i].x][original_particle->position.coord[i].y] = -1;

                /*Move protein to center*/
                best_position.coord[i].x += lattice_adjust.x;
                best_position.coord[i].y += lattice_adjust.y;

                original_particle->position.fitness = best_position.fitness;

                /*Update original original_particle*/
                original_particle->position.coord[i] = best_position.coord[i];

            }
        }
    }
    while(best_position.fitness < previous_fitness);

    for (i = 0; i < num_dimensions; ++i)
    {
        lattice[original_particle->position.coord[i].x][original_particle->position.coord[i].y] = -1;
    }

    return change;
}



Position pso_run
(
    Pso_config pso_config,
    Polarity *seq,
    int num_dimensions,
    int *seed
)
/* ====================================
 * PSO main function
 * ====================================
 */
{

    int i;
    int j;
    int k;
    int *best_particle_by_edge;
    int **lattice;
    Position gbest;
    Particle *particles;
    Position pm_best_position;
    Position pm_position;
    Pm_config* pm_configs;
    Solution solution;
    Position iteration_position;
    Boolean change;

    //Sets seed
    if (*seed == -1)
    {
        *seed = (unsigned) time(NULL);
    }
    srand(*seed);

    pm_configs = (Pm_config*) malloc(sizeof(Pm_config) * 4 * (num_dimensions - 2));

    lattice = (int**) malloc(sizeof(int*) * (2 * num_dimensions + 1));
    for (i = 0; i < 2 * num_dimensions + 1; ++i)
    {
        lattice[i] = (int*) malloc(sizeof(int) * (2 * num_dimensions + 1));
        for (j = 0; j < 2 * num_dimensions + 1; ++j)
        {
            lattice[i][j] = -1;
        }
    }

    particles = (Particle*) malloc(sizeof(Particle) * pso_config.population);
    for (i = 0; i < pso_config.population; ++i)
    {
        init_particle(&(particles[i]), num_dimensions);
    }

    best_particle_by_edge = (int*) malloc(sizeof(int) * (num_dimensions - 1));
    for (i = 0; i < num_dimensions - 1; ++i)
    {
        best_particle_by_edge[i] = -1;
    }

    init_solution(&solution, num_dimensions);
    init_position(&gbest, num_dimensions);
    init_position(&pm_best_position, num_dimensions);
    init_position(&pm_position, num_dimensions);
    init_position(&iteration_position, num_dimensions);
    initializes_population(pso_config, particles, &gbest, num_dimensions, best_particle_by_edge, lattice, seq);

    for (i = 0; i < pso_config.iterations; ++i)
    {
        for (j = 0; j < pso_config.population; ++j)
        {
            update_position(&(particles[j]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, j, particles);
            update_velocity(pso_config, &(particles[j]), gbest, num_dimensions);
        }

        for (j = 0; j < num_dimensions - 1; ++j)
        {
            best_particle_by_edge[j] = -1;
        }

        iteration_position = particles[0].position;

        //Daemon search
        for (j = 0; j < pso_config.population; ++j)
        {

            switch (pso_config.daemon)
            {

            case PULL_MOVE:

                change = pm_search(seq, num_dimensions, &(particles[j]),
                                   lattice, pm_best_position,
                                   pm_position, pm_configs);
                break;

            default:
                break;

            }

            if (change == TRUE)
            {
                for (k = 0; k < num_dimensions; ++k)
                {
                    lattice[particles[j].position.coord[k].x][particles[j].position.coord[k].y] = k;
                }

                adjust_particle_by_coord(&(particles[j].position), seq, lattice, num_dimensions);

                for (k = 0; k < num_dimensions; ++k)
                {
                    lattice[particles[j].position.coord[k].x][particles[j].position.coord[k].y] = -1;
                }

                change = FALSE;
            }

            if (particles[j].position.fitness < iteration_position.fitness)
            {
                iteration_position = particles[j].position;
            }
        }
        if (iteration_position.fitness < gbest.fitness)
        {
            copy_position(&gbest, iteration_position, num_dimensions);
        }
    }

    for (i = 0; i < pso_config.population; ++i)
    {
        free_particle(particles[i]);
    }
    free(particles);
    for (i = 0; i < 2 * num_dimensions + 1; ++i)
    {
        free(lattice[i]);
    }
    free(lattice);
    free(best_particle_by_edge);
    free_solution(solution);

    return gbest;
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

    ///FUNCTION: set_default_velocity
    {
        //Test variables definition
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 1);
        //Function call
        set_default_velocity(velocity, 2);
        //Result check
        if (velocity[0].s != 0 ||
            velocity[0].l != 0 ||
            velocity[0].r != 0)
        {
            printf("Test failed on function set_default_velocity\n");
            exit(1);
        }
        //Free memory
        free(velocity);
    }

    ///FUNCTION: multiplies_coefficient_by_velocity
    {
        //Test variables definition
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 3);
        set_default_velocity(velocity, 3);
        velocity[1].l = 0.2;
        velocity[1].r = 0.4;
        velocity[1].s = 0.8;
        //Function call
        multiplies_coefficient_by_velocity(2, velocity, 3);
        //Result check
        if (velocity[1].l != 0.4 ||
            velocity[1].r != 0.8 ||
            velocity[1].s != 1.0)
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
        set_default_velocity(v1, 3);
        v1[1].l = 0.2;
        v1[1].r = 0.4;
        v1[1].s = 0.8;
        Velocity *v2 = (Velocity*) malloc(sizeof(Velocity) * 2);
        set_default_velocity(v2, 3);
        v2[1].l = 0.3;
        v2[1].r = 0.1;
        v2[1].s = 0.9;
        //Function call
        sum_velocities(v1, v2, 3);
        //Result check
        if (v1[1].l != 0.3 ||
            v1[1].r != 0.4 ||
            v1[1].s != 0.9)
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
        Position p1;
        init_position(&p1, 3);
        p1.dir[0] = NONE;
        p1.dir[1] = RIGHT;

        Position p2;
        init_position(&p2, 3);
        p2.dir[0] = NONE;
        p2.dir[1] = RIGHT;
        //Function call
        subtract_positions(&p1, p2, 3);
        //Result check
        if (p1.dir[0] != NONE ||
            p1.dir[1] != NONE)
        {
            printf("Test failed on function subtract_positions\n");
            exit(1);
        }
        //Free memory
        free_position(p1);
        free_position(p2);
    }

    ///FUNCTION: copy_velocity
    {
        //Test variables definition
        int i;
        Velocity *copy = (Velocity*) malloc(sizeof(Velocity) * 2);
        Velocity *velocity = (Velocity*) malloc(sizeof(Velocity) * 2);
        set_default_velocity(copy, 2);
        set_default_velocity(velocity, 2);
        velocity[1].l = 0.2;
        velocity[1].r = 0.4;
        velocity[1].s = 0.8;
        //Function call
        copy_velocity(copy, velocity, 2);
        //Result check
        for (i = 0; i < 2; ++i)
        {
            if (copy[0].s != velocity[0].s ||
                copy[0].l != velocity[0].l ||
                copy[0].r != velocity[0].r)
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
        Position copy;
        Position position;
        init_position(&copy, 3);
        init_position(&position, 3);
        position.dir[0] = NONE;
        position.dir[1] = LEFT;
        //Function call
        copy_position(&copy, position, 3);
        //Result check
        for (i = 0; i < 2; ++i)
        {
            if (copy.dir[0] != position.dir[0] ||
                copy.dir[1] != position.dir[1])
            {
                printf("Test failed on function copy_position\n");
            }
        }
        //Free memory
        free_position(copy);
        free_position(position);
    }

    ///FUNCTION: update_velocity
    {
        //Test variables definition

        Pso_config pso_config;
        pso_config.w = 0.5;
        pso_config.c1 = 2.1;
        pso_config.c2 = 2.1;

        Particle particle;

        particle.velocity = (Velocity*) malloc(sizeof(Velocity) * 3);
        set_default_velocity(particle.velocity, 4);
        particle.velocity[0].l = 0.2;
        particle.velocity[1].r = 0.4;
        particle.velocity[2].s = 0.6;

        init_position(&(particle.position), 4);
        particle.position.dir[0] = LEFT;
        particle.position.dir[1] = RIGHT;
        particle.position.dir[2] = STRAIGHT;
        particle.position.fitness = -7;

        init_position(&(particle.pbest), 4);
        particle.pbest.dir[0] = STRAIGHT;
        particle.pbest.dir[1] = RIGHT;
        particle.pbest.dir[2] = LEFT;
        particle.pbest.fitness = -5;

        Position gbest;
        init_position(&gbest, 4);
        gbest.dir[0] = RIGHT;
        gbest.dir[1] = STRAIGHT;
        gbest.dir[2] = LEFT;

        //Function call
        update_velocity(pso_config, &particle, gbest, 4);

        //Free memory
        free_position(particle.pbest);
        free_position(particle.position);
        free_position(gbest);
        free(particle.velocity);
    }

    ///FUNCTION: update_position
    {
        //Test variables definition

        int i, j;
        int num_dimensions = 10;
        Pso_config pso_config;
        pso_config.beta = 1;
        pso_config.collision_handler = PARTIAL_COPY;

        Polarity seq[] = {H, H, P, H, P, P, H, H, P, H};

        int **lattice;
        lattice = (int**) malloc(sizeof(int*) * (2 * num_dimensions + 1));
        for (i = 0; i < 2 * num_dimensions + 1; ++i)
        {
            lattice[i] = (int*) malloc(sizeof(int) * (2 * num_dimensions + 1));
            for (j = 0; j < 2 * num_dimensions + 1; ++j)
            {
                lattice[i][j] = -1;
            }
        }

        int *best_particle_by_edge = (int*) malloc(sizeof(int) * (num_dimensions - 1));
        for (i = 0; i < num_dimensions - 1; ++i)
        {
            best_particle_by_edge[i] = -1;
        }

        Particle *particles;
        particles = (Particle*) malloc(sizeof(Particle));
        init_particle(&(particles[0]), num_dimensions);

        randomize_velocity(particles[0].velocity, num_dimensions);

        //Function call
        update_position(&(particles[0]), lattice, seq, num_dimensions, pso_config,
                        best_particle_by_edge, 0, particles);

        //Free memory
        for (i = 0; i < 2 * num_dimensions + 1; ++i) {
            free(lattice[i]);
        }
        free(lattice);
        free_particle(particles[0]);
        free(best_particle_by_edge);
        free(particles);
    }

    ///FUNCTION: initializes_population
    {
        int i, j;
        int num_dimensions = 10;

        Pso_config pso_config;

        pso_config.w = 0.5;
        pso_config.c1 = 2.1;
        pso_config.c2 = 2.1;
        pso_config.beta = 1;
        pso_config.population = 5;
        pso_config.collision_handler = PARTIAL_COPY;


        Polarity seq[] = {H, H, P, H, P, P, H, H, P, H};

        Particle *particles;
        particles = (Particle*) malloc(sizeof(Particle) * pso_config.population);
        for (i = 0; i < pso_config.population; ++i)
        {
            init_particle(&(particles[i]), num_dimensions);
        }

        Position gbest;
        init_position(&gbest, num_dimensions);


        int *best_particle_by_edge = (int*) malloc(sizeof(int) * (num_dimensions - 1));
        for (i = 0; i < num_dimensions - 1; ++i)
        {
            best_particle_by_edge[i] = -1;
        }

        int **lattice;
        lattice = (int**) malloc(sizeof(int*) * (2 * num_dimensions + 1));
        for (i = 0; i < 2 * num_dimensions + 1; ++i)
        {
            lattice[i] = (int*) malloc(sizeof(int) * (2 * num_dimensions + 1));
            for (j = 0; j < 2 * num_dimensions + 1; ++j)
            {
                lattice[i][j] = -1;
            }
        }

        //Function call
        initializes_population(pso_config, particles, &gbest, num_dimensions, best_particle_by_edge, lattice, seq);

        //Free memory
        for (i = 0; i < pso_config.population; ++i)
        {
            free_particle(particles[i]);
        }
        for (i = 0; i < 2 * num_dimensions + 1; ++i)
        {
            free(lattice[i]);
        }
        free(particles);
        free(lattice);
        free(best_particle_by_edge);
        free_position(gbest);

    }

}
