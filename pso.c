#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "pso.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

enum direction
{
    LEFT = 0,
    RIGHT = 1,
    STRAIGHT = 2,
    NONE = 3
};

enum pm_type
{
    ORIGINAL = 0,
    INVERSE = 1
};

struct position
{
    enum direction *dir;
    struct coord *coord;
    int *fitness_by_edge;
    int fitness;
    int feasible;
};

struct particle
{
    struct velocity *velocity;
    struct position position;
    struct position pbest;
};

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
    int readjusted;
};

struct velocity
{
    double l;
    double r;
    double s;
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

void* smalloc(int mem_size)
/* ====================================
 * Allocates memory
 * ====================================
 */
{
    void *mem_pos = (void*) malloc(mem_size);

    if (mem_pos == NULL)
    {
        printf("ERROR: aco.c/smalloc(): \"Unable to allocate memory\"\n");
        exit(1);
    }
    else
    {
        return mem_pos;
    }
}

struct coord create_new_coord(int x, int y)
/* ===========================================================
 * Returns a new variables of type struct coord with the given values
 * ===========================================================
 */
{
    struct coord coord;
    coord.x = x;
    coord.y = y;
    return coord;
}

struct coord subtract_coord
(
    struct coord c1,
    struct coord c2
)
/* =======================================================
 * Calculates the difference between two given coordinates
 * =======================================================
 */
{
    struct coord c3;
    c3.x = c1.x - c2.x;
    c3.y = c1.y - c2.y;
    return c3;
}

int lateral_adj(struct coord c1, struct coord c2)
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

int abs_direction_by_move(struct coord move)
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
            printf("ERROR: aco.c/abs_direction_by_move(): \"Invalid value for parameter move\"\n");
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
            printf("ERROR: aco.c/abs_direction_by_move(): \"Invalid value for parameter move\"\n");
            exit(1);
        }
    }
    else
    {
        printf("ERROR: aco.c/abs_direction_by_move(): \"Invalid value for parameter move\"\n");
        exit(1);
    }
}

int direction_by_move(struct coord prev_move, struct coord move)
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
            printf("ERROR: aco.c/direction_by_move(): \"Invalid values for parameters\"\n");
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
            printf("ERROR: aco.c/direction_by_move(): \"Invalid values for parameters\"\n");
            exit(1);
        }
    }
}

struct coord straight(struct coord prev_move)
/* =================================================================
 * Calculates straight move corresponding to the given previous move
 * =================================================================
 */
{
    struct coord move;

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
        printf("ERROR: aco.c/straight(): \"Invalid value for parameter prev_move\"\n");
        exit(1);
    }

    return move;
}

struct coord left(struct coord prev_move)
/* =============================================================
 * Calculates left move corresponding to the given previous move
 * =============================================================
 */
{
    struct coord move;

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
        printf("ERROR: aco.c/left(): \"Invalid value for parameter prev_move\"\n");
        exit(1);
    }

    return move;
}

struct coord right(struct coord prev_move)
/* ==============================================================
 * Calculates right move corresponding to the given previous move
 * ==============================================================
 */
{
    struct coord move;

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
        printf("ERROR: aco.c/right(): \"Invalid value for parameter prev_move\"\n");
        exit(1);
    }

    return move;
}

char* position_to_string
(
    struct position position,
    int num_dimensions
)
/* ===============================================
 * Extracts conformation info from Ant to Solution
 * ===============================================
 */
{
    struct coord move, prev_move;
    int i, direction;
    char *directions;

    directions = (char*) smalloc(sizeof(char) * num_dimensions);

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
            directions[i] = 'L';
            break;
        case RIGHT:
            directions[i] = 'R';
            break;
        case STRAIGHT:
            directions[i] = 'S';
            break;
        default:
            break;
        }

        prev_move = move;
    }

    directions[num_dimensions - 1] = '\0';
    return directions;
}

void init_position
(
    struct position *position,
    int num_dimensions
)
/* ================================================
 * Allocates memory to struct position structure variables
 * ================================================
 */
{
    int i;

    position->coord = (struct coord*) smalloc(sizeof(struct coord) * num_dimensions);
    position->dir = (enum direction*) smalloc(sizeof(enum direction) * (num_dimensions - 1));
    position->fitness_by_edge = (int*) smalloc(sizeof(int) * (num_dimensions - 1));
    position->feasible = 0;

    for (i = 0; i < num_dimensions - 1; ++i)
    {
        position->dir[i] = NONE;
    }
}

void free_position(struct position position)
/* ============================================
 * Frees memory of struct position structure variables
 * ============================================
 */
{
    free(position.coord);
    free(position.dir);
    free(position.fitness_by_edge);
}

void init_particle
(
    struct particle *particle,
    int num_dimensions
)
/* ================================================
 * Allocates memory to struct particle structure variables
 * ================================================
 */
{
    init_position(&(particle->position), num_dimensions);
    init_position(&(particle->pbest), num_dimensions);
    particle->velocity = (struct velocity*) smalloc(sizeof(struct velocity) * (num_dimensions - 1));
}

void free_particle(struct particle particle)
/* ============================================
 * Frees memory of struct particle structure variables
 * ============================================
 */
{
    free_position(particle.position);
    free_position(particle.pbest);
    free(particle.velocity);
}

void init_variables
(
    struct pso_config pso_config,
    struct pm_config **pm_configs,
    int ***lattice,
    int **best_particle_by_edge,
    struct particle **particles,
    struct position *gbest,
    struct position *pm_best_position,
    struct position *pm_position,
    int num_dimensions
)
/* =======================================
 * Allocates all pso.c exclusive variables
 * =======================================
 */
{
    int i;
    int j;

    *pm_configs = (struct pm_config*) smalloc(sizeof(struct pm_config) * 4 * (num_dimensions - 2));

    *lattice = (int**) smalloc(sizeof(int*) * (2 * num_dimensions + 1));
    for (i = 0; i < 2 * num_dimensions + 1; ++i)
    {
        (*lattice)[i] = (int*) smalloc(sizeof(int) * (2 * num_dimensions + 1));
        for (j = 0; j < 2 * num_dimensions + 1; ++j)
        {
            (*lattice)[i][j] = -1;
        }
    }

    *best_particle_by_edge = (int*) smalloc(sizeof(int) * (num_dimensions - 1));
    for (i = 0; i < num_dimensions - 1; ++i)
    {
        (*best_particle_by_edge)[i] = -1;
    }

    *particles = (struct particle*) smalloc(sizeof(struct particle) * pso_config.population);
    for (i = 0; i < pso_config.population; ++i)
    {
        init_particle(&((*particles)[i]), num_dimensions);
    }

    init_position(gbest, num_dimensions);
    init_position(pm_best_position, num_dimensions);
    init_position(pm_position, num_dimensions);
}

void free_variables
(
    struct pso_config pso_config,
    int **lattice,
    struct particle *particles,
    struct position pm_best_position,
    struct position pm_position,
    struct position gbest,
    struct pm_config *pm_configs,
    int *best_particle_by_edge,
    int num_dimensions
)
/* ==========================================
 * Frees all pso.c exclusive variables memory
 * ==========================================
 */
{
    int i;
    for (i = 0; i < 2 * num_dimensions + 1; ++i)
    {
        free(lattice[i]);
    }
    for (i = 0; i < pso_config.population; ++i)
    {
        free_particle(particles[i]);
    }

    free_position(pm_best_position);
    free_position(pm_position);
    free_position(gbest);
    free(pm_configs);
    free(particles);
    free(lattice);
    free(best_particle_by_edge);
}

void set_default_velocity
(
    struct velocity *velocity,
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
    struct velocity *velocity,
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
    struct velocity *v1,
    struct velocity *v2,
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
    struct position *p1,
    struct position p2,
    int num_dimensions
)
/* ============================================================
 * Set the value of position p1 to the subtraction of p1 and p2
 * ============================================================
 */
{
    int i;

    p1->feasible = 0;

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
    struct velocity *velocity,
    double coefficient,
    struct position position,
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
            velocity[i].r = 0;
            velocity[i].s = 0;
            velocity[i].l = coefficient;
            if (velocity[i].l > 1) velocity[i].l = 1;
            break;
        case RIGHT:
            velocity[i].l = 0;
            velocity[i].s = 0;
            velocity[i].r = coefficient;
            if (velocity[i].r > 1) velocity[i].r = 1;
            break;
        case STRAIGHT:
            velocity[i].l = 0;
            velocity[i].r = 0;
            velocity[i].s = coefficient;
            if (velocity[i].s > 1) velocity[i].s = 1;
            break;
        default:
            velocity[i].l = 0;
            velocity[i].r = 0;
            velocity[i].s = 0;
            break;
        }
    }
}

void copy_velocity
(
    struct velocity *copy,
    struct velocity *velocity,
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
    struct position *copy,
    struct position position,
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
    struct pso_config pso_config,
    struct particle *particle,
    struct position gbest,
    int num_dimensions
)
/* ==================================================
 * Adjust particle velocity following the PSO formula
 * ==================================================
 */
{
    double r1;
    double r2;

    struct velocity *cognitive_velocity;
    struct velocity *social_velocity;
    struct position cognitive_position;
    struct position social_position;

    cognitive_velocity = (struct velocity*) smalloc(sizeof(struct velocity) * num_dimensions);
    social_velocity = (struct velocity*) smalloc(sizeof(struct velocity) * num_dimensions);
    init_position(&cognitive_position, num_dimensions);
    init_position(&social_position, num_dimensions);

    //velocity = w * velocity
    multiplies_coefficient_by_velocity(pso_config.w, particle->velocity, num_dimensions);

    //cognitive_velocity = c1 * r1 * (pbest - xi)
    r1 = (double)rand()/RAND_MAX;
    copy_position(&cognitive_position, particle->pbest, num_dimensions);
    subtract_positions(&cognitive_position, particle->position, num_dimensions);
    multiplies_coefficient_by_position(cognitive_velocity, pso_config.c1 * r1, cognitive_position, num_dimensions);

    //social_velocity = c2 * r2 * (gbest - xi)
    r2 = (double)rand()/RAND_MAX;
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
    struct coord pos,
    enum polarity *seq
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
    struct coord pos,
    enum polarity *seq
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
    struct position *position,
    enum polarity *seq,
    int **lattice,
    int num_dimensions
)
/* ================================================
 * Adjust direction and energy based on coordinates
 * ================================================
 */
{
    int i;
    struct coord move;
    struct coord prev_move;
    enum direction direction;
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

int execute_look_ahead(int **lattice, struct coord position, int num_dimensions, int amin_index)
{
    int right;
    int left;
    int down;
    int up;

    int position_is_invalid;

    right = lattice[position.x + 1][position.y];
    left = lattice[position.x - 1][position.y];
    up = lattice[position.x][position.y + 1];
    down = lattice[position.x][position.y - 1];

    if (right > -1 && left > -1 && up > -1 && down > -1) {
        position_is_invalid = 1;
    } else {
        position_is_invalid = 0;
    }

    if (amin_index == 0 || amin_index == num_dimensions - 1)
    {
        position_is_invalid = 0;
    }

    return !position_is_invalid;
}

void update_position_1
(
    struct particle *particle,
    int **lattice,
    enum polarity *seq,
    int num_dimensions,
    struct pso_config pso_config,
    int *best_particle_by_edge,
    int particle_index,
    struct particle *particles
)
/* ============================================================================
 * Sets particles position to feasible position built using particle's velocity
 * ============================================================================
 */
{
    int i;

    int num_candidates;
    int selected_candidate;
    double sum_probabilities;
    double probabilities[3];
    struct coord candidate_moves[3];
    struct candidate candidates[3];

    struct coord foward_position;
    struct coord last_move;

    int curr_amin;
    int prev_amin;
    int next_amin;

    int left_extremity;
    int right_extremity;
    int *extremity;

    int side;
    int last_unfold_side;
    int unfold_size;
    particle->pbest.fitness = 0;

    //Defines start edge direction

    left_extremity = ceil(num_dimensions / 2);
    right_extremity = ceil(num_dimensions / 2) + 1;

    particle->position.coord[left_extremity] = create_new_coord(num_dimensions, num_dimensions);
    particle->position.coord[right_extremity] = create_new_coord(num_dimensions + 1, num_dimensions);

    lattice[particle->position.coord[left_extremity].x][particle->position.coord[left_extremity].y] = left_extremity;
    lattice[particle->position.coord[right_extremity].x][particle->position.coord[right_extremity].y] = right_extremity;

    side = 0;//left
    last_unfold_side = -1;


    //Constructor loop

    while (left_extremity > 0 || right_extremity < num_dimensions - 1)
    {
        if (side == 0 && left_extremity == 0)
        {
            side = 1;
        }
        else if (side == 1 && right_extremity == num_dimensions - 1)
        {
            side = 0;
        }

        if (side == 0)
        {
            curr_amin = left_extremity;
            next_amin = left_extremity - 1;
            prev_amin = left_extremity + 1;
            extremity = &left_extremity;
        }
        else
        {
            curr_amin = right_extremity;
            next_amin = right_extremity + 1;
            prev_amin = right_extremity - 1;
            extremity = &right_extremity;
        }

        last_move = subtract_coord(particle->position.coord[curr_amin], particle->position.coord[prev_amin]);

        sum_probabilities = 0;
        num_candidates = 0;

        candidate_moves[0] = left(last_move);
        candidate_moves[1] = right(last_move);
        candidate_moves[2] = straight(last_move);

        //Defines candidate directions

        //For each direction
        for (i = 0; i < 3; ++i)
        {

            foward_position = create_new_coord(particle->position.coord[curr_amin].x + candidate_moves[i].x,
                                               particle->position.coord[curr_amin].y + candidate_moves[i].y);

            //If the next position in this direction is not occupied, turns current direction into a candidate
            if (lattice[foward_position.x][foward_position.y] == -1 &&
                execute_look_ahead(lattice, foward_position, num_dimensions, next_amin))
            {
                candidates[num_candidates].move = candidate_moves[i];
                candidates[num_candidates].coord = foward_position;

                if (seq[next_amin] == H)
                {
                    candidates[num_candidates].heuristic =
                        calculate_heuristic(lattice, next_amin, foward_position, seq);
                }
                else
                {
                    candidates[num_candidates].heuristic = 0;
                }



                probabilities[num_candidates] = pow(exp((double) candidates[num_candidates].heuristic / 0.3), pso_config.beta);

                switch(i) {
                case LEFT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].l;
                    break;
                case RIGHT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].r;
                    break;
                case STRAIGHT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].s;
                    break;
                default:
                    break;
                }

                sum_probabilities += probabilities[num_candidates];
                ++num_candidates;
            }

        }

        for (i = 0; i < num_candidates; ++i)
        {
            probabilities[i] = probabilities[i] / sum_probabilities;
        }

        //Selects a candidate

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

        //Update conformation

        if (selected_candidate != -1)
        {
            *extremity = next_amin;

            particle->position.fitness -= candidates[selected_candidate].heuristic;
            particle->position.coord[*extremity] = candidates[selected_candidate].coord;
            lattice[particle->position.coord[*extremity].x][particle->position.coord[*extremity].y] = *extremity;
            side = !side;

        }
        else
        {
            //When is impossible continue the fold process

            if (right_extremity == ceil(num_dimensions / 2) + 1 ||
                (last_unfold_side == 1 && left_extremity != ceil(num_dimensions / 2)))

            {
                unfold_size = 1 + rand() % ((int) ceil(num_dimensions / 2) - left_extremity);
                extremity = &left_extremity;
                last_unfold_side = 0;

                curr_amin = *extremity;
                next_amin = curr_amin - 1;
                prev_amin = curr_amin + 1;
            }
            else
            {

                unfold_size = 1 + rand() % (right_extremity - ((int) ceil(num_dimensions / 2) + 1));
                extremity = &right_extremity;
                last_unfold_side = 1;

                curr_amin = *extremity;
                next_amin = curr_amin + 1;
                prev_amin = curr_amin - 1;
            }


            for (i = 0; i < unfold_size; ++i)
            {
                if (seq[*extremity] == H)
                {
                    particle->position.fitness += calculate_heuristic(lattice, *extremity, particle->position.coord[*extremity], seq);
                }
                lattice[particle->position.coord[*extremity].x][particle->position.coord[*extremity].y] = -1;

                next_amin = *extremity;
                curr_amin = prev_amin;
                prev_amin = curr_amin - (next_amin - curr_amin);

                *extremity = curr_amin;
            }
        }
    }


    adjust_particle_by_coord(&(particle->position), seq, lattice, num_dimensions);

    particle->position.feasible = 1;


    for (i = 0; i < num_dimensions; ++i)
    {
        lattice[particle->position.coord[i].x][particle->position.coord[i].y] = -1;
    }
}

void update_position_0
(
    struct particle *particle,
    int **lattice,
    enum polarity *seq,
    int num_dimensions,
    struct pso_config pso_config,
    int *best_particle_by_edge,
    int particle_index,
    struct particle *particles
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
    struct coord candidate_moves[3];
    struct candidate candidates[3];

    struct coord curr_coord;
    struct coord foward_coord;
    struct coord last_move;

    struct position copied_position;

    particle->position.fitness = 0;
    particle->position.fitness_by_edge[0] = 0;

    //DEFINES FIRST EDGE

    lattice[num_dimensions][num_dimensions] = 0;
    particle->position.coord[0] = create_new_coord(num_dimensions, num_dimensions);

    last_move = create_new_coord(0, 1);
    curr_coord = create_new_coord(particle->position.coord[0].x + last_move.x,
                                  particle->position.coord[0].y + last_move.y);
    lattice[curr_coord.x][curr_coord.y] = 1;
    particle->position.coord[1] = curr_coord;


    //CONSTRUCTOR LOOP

    //For each edge except the first
    for (i = 1; i < num_dimensions - 1; ++i)
    {
        sum_probabilities = 0;
        num_candidates = 0;

        candidate_moves[0] = left(last_move);
        candidate_moves[1] = right(last_move);
        candidate_moves[2] = straight(last_move);

        //DEFINES CANDIDATES DIRECTION

        //For each direction
        for (j = 0; j < 3; ++j)
        {

            foward_coord = create_new_coord(curr_coord.x + candidate_moves[j].x,
                                            curr_coord.y + candidate_moves[j].y);

            //If the next coord in this direction is not occupied, turns current direction into a candidate
            if (lattice[foward_coord.x][foward_coord.y] == -1)
            {
                candidates[num_candidates].move = candidate_moves[j];
                candidates[num_candidates].coord = foward_coord;

                if (seq[i + 1] == H)
                {
                    candidates[num_candidates].heuristic =
                        calculate_heuristic(lattice, i + 1, foward_coord, seq);
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
            last_move = candidates[selected_candidate].move;
            curr_coord = candidates[selected_candidate].coord;
            particle->position.fitness_by_edge[i] = particle->position.fitness;
            particle->position.fitness -= candidates[selected_candidate].heuristic;
            particle->position.coord[i + 1] = curr_coord;
            lattice[curr_coord.x][curr_coord.y] = i + 1;

        }
        else
        {
            //WHEN IS IMPOSSIBLE CONTINUE THE FOLD PROCESS

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

                last_move = create_new_coord(0, 1);
                curr_coord = create_new_coord(particle->position.coord[0].x + last_move.x,
                                              particle->position.coord[0].y + last_move.y);
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

                last_move = create_new_coord(curr_coord.x - copied_position.coord[i - 1].x,
                                        curr_coord.y - copied_position.coord[i - 1].y);
                --i;
            }
        }
    }

    adjust_particle_by_coord(&(particle->position), seq, lattice, num_dimensions);

    particle->position.feasible = 1;

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

void update_position_2
(
    struct particle *particle,
    int **lattice,
    enum polarity *seq,
    int num_dimensions,
    struct pso_config pso_config,
    int *best_particle_by_edge,
    int particle_index,
    struct particle *particles
)
/* ====================================================
 * Builds ant conformation. Based in Xiao, Li & Hu 2014
 * ====================================================
 */
{
    int i;

    int num_candidates;
    int selected_candidate;
    double sum_probabilities;
    double probabilities[3];
    struct coord candidate_moves[3];
    struct candidate candidates[3];

    struct coord last_move;
    struct coord foward_position;
    enum direction forbidden_dir;
    double left_unfolded_percentage;

    int curr_amin;
    int prev_amin;
    int next_amin;
    int left_extremity;
    int right_extremity;
    int *extremity;
    int side;
    int unfold_size;
    int start_point;

    double readjust;

    forbidden_dir = -1;
    particle->position.fitness = 0;

    //Defines start edge direction

    start_point = rand()%(num_dimensions - 1);

    left_extremity = start_point;
    right_extremity = start_point + 1;

    particle->position.coord[left_extremity] = create_new_coord(num_dimensions, num_dimensions);
    particle->position.coord[right_extremity] = create_new_coord(num_dimensions + 1, num_dimensions);

    lattice[particle->position.coord[left_extremity].x][particle->position.coord[left_extremity].y] = left_extremity;
    lattice[particle->position.coord[right_extremity].x][particle->position.coord[right_extremity].y] = right_extremity;

    //Constructor loop

    while (left_extremity > 0 || right_extremity < num_dimensions - 1)
    {

        left_unfolded_percentage = (double) left_extremity / (left_extremity + (num_dimensions - right_extremity));

        if ((double) rand() / RAND_MAX < left_unfolded_percentage) {
            side = 0;//left
        } else {
            side = 1;//right
        }


        if (side == 0 && left_extremity == 0)
        {
            side = 1;
        }
        else if (side == 1 && right_extremity == num_dimensions - 1)
        {
            side = 0;
        }

        if (side == 0)
        {
            curr_amin = left_extremity;
            next_amin = left_extremity - 1;
            prev_amin = left_extremity + 1;
            extremity = &left_extremity;
        }
        else
        {
            curr_amin = right_extremity;
            next_amin = right_extremity + 1;
            prev_amin = right_extremity - 1;
            extremity = &right_extremity;
        }

        last_move = subtract_coord(particle->position.coord[curr_amin], particle->position.coord[prev_amin]);

        sum_probabilities = 0;
        num_candidates = 0;

        candidate_moves[0] = left(last_move);
        candidate_moves[1] = right(last_move);
        candidate_moves[2] = straight(last_move);

        //Defines candidate direction

        //For each direction
        for (i = 0; i < 3; ++i)
        {

            foward_position = create_new_coord(particle->position.coord[curr_amin].x + candidate_moves[i].x,
                                               particle->position.coord[curr_amin].y + candidate_moves[i].y);

            //If the next position in this direction is not occupied, turns current direction into a candidate
            if (lattice[foward_position.x][foward_position.y] == -1 &&
                execute_look_ahead(lattice, foward_position, num_dimensions, next_amin) &&
                i != forbidden_dir)
            {
                candidates[num_candidates].move = candidate_moves[i];
                candidates[num_candidates].coord = foward_position;
                candidates[num_candidates].readjusted = 0;

                if (seq[next_amin] == H)
                {
                    candidates[num_candidates].heuristic =
                        calculate_heuristic(lattice, next_amin, foward_position, seq);
                }
                else
                {
                    candidates[num_candidates].heuristic = 0;
                }

                probabilities[num_candidates] = pow(exp((double) candidates[num_candidates].heuristic / 0.3), pso_config.beta);

                switch(i) {
                case LEFT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].l;
                    break;
                case RIGHT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].r;
                    break;
                case STRAIGHT:
                    probabilities[num_candidates] *= particle->velocity[curr_amin].s;
                    break;
                default:
                    break;
                }


                sum_probabilities += probabilities[num_candidates];
                ++num_candidates;
            }

        }

        forbidden_dir = -1;

        readjust = 0;

        for (i = 0; i < num_candidates; ++i)
        {
            probabilities[i] = probabilities[i] / sum_probabilities;

            if (probabilities[i] < pso_config.min_probability)
            {
                candidates[i].readjusted = 1;
                readjust += pso_config.min_probability - probabilities[i];
                probabilities[i] = pso_config.min_probability;
            }
        }

        if (readjust > 0)
        {
            for (i = 0; i < num_candidates; ++i)
            {
                if (candidates[i].readjusted == 0)
                {
                    probabilities[i] -= (readjust * probabilities[i]);
                }
            }
        }

        //Selects a candidate

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

        //Update conformation

        if (selected_candidate != -1)
        {
            *extremity = next_amin;

            particle->position.fitness -= candidates[selected_candidate].heuristic;
            particle->position.coord[*extremity] = candidates[selected_candidate].coord;
            lattice[particle->position.coord[*extremity].x][particle->position.coord[*extremity].y] = *extremity;
            side = !side;

        }
        else
        {
            unfold_size = ceil((right_extremity - left_extremity) / 2);

            if (side == 0) {


                extremity = &left_extremity;
                curr_amin = *extremity;
                next_amin = curr_amin - 1;
                prev_amin = curr_amin + 1;

            } else {

                extremity = &right_extremity;
                curr_amin = *extremity;
                next_amin = curr_amin + 1;
                prev_amin = curr_amin - 1;

            }

            for (i = 0; i < unfold_size; ++i)
            {
                if (seq[*extremity] == H)
                {
                    particle->position.fitness += calculate_heuristic(lattice, *extremity, particle->position.coord[*extremity], seq);
                }
                lattice[particle->position.coord[*extremity].x][particle->position.coord[*extremity].y] = -1;

                next_amin = *extremity;
                curr_amin = prev_amin;
                prev_amin = curr_amin - (next_amin - curr_amin);

                *extremity = curr_amin;
            }

            forbidden_dir = direction_by_move(
                subtract_coord(particle->position.coord[curr_amin], particle->position.coord[prev_amin]),
                subtract_coord(particle->position.coord[next_amin], particle->position.coord[curr_amin]));
        }
    }

    adjust_particle_by_coord(&(particle->position), seq, lattice, num_dimensions);

    particle->position.feasible = 1;

    for (i = 0; i < num_dimensions; ++i)
    {
        lattice[particle->position.coord[i].x][particle->position.coord[i].y] = -1;
    }
}

void randomize_velocity
(
    struct velocity *velocity,
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
        velocity[i].s = (double)rand()/RAND_MAX;
        velocity[i].l = (double)rand()/RAND_MAX;
        velocity[i].r = (double)rand()/RAND_MAX;
    }
}

void initializes_population
(
    struct pso_config pso_config,
    struct particle *particles,
    struct position *gbest,
    int num_dimensions,
    int *best_particle_by_edge,
    int **lattice,
    enum polarity *seq

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
        update_position_1(&(particles[i]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, i, particles);
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
    enum polarity *seq,
    int amino_acid_index,
    struct coord src,
    struct coord dest
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
    struct position position,
    struct pm_config config,
    enum polarity *seq,
    int **lattice,
    int num_dimensions,
    struct coord *particle_coord
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
    struct coord coord;

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

            coord = position.coord[i];
            position.coord[i] = config.curr;
            config.curr = config.prev;
            config.prev = coord;

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
    struct pm_config *config,
    int **lattice,
    enum pm_type pm_type,
    struct coord curr,
    struct coord prev,
    struct coord next,
    struct coord direction
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
    struct coord *particle_coord,
    struct pm_config *configs,
    int *config_index,
    int num_dimensions
)
/* ============================================================
 * Generates pull-move configurations for a specific amino-acid
 * ============================================================
 */
{
    int result;
    struct pm_config config;
    config.amino_acid_index = amino_acid_index;

    struct coord right_move = create_new_coord(1, 0);
    struct coord left_move = create_new_coord(-1, 0);
    struct coord up_move = create_new_coord(0, 1);
    struct coord down_move = create_new_coord(0, -1);

    struct coord next = particle_coord[amino_acid_index + 1];
    struct coord prev = particle_coord[amino_acid_index - 1];
    struct coord curr = particle_coord[amino_acid_index];

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

int pm_search
(
    enum polarity *seq,
    int num_dimensions,
    struct particle *original_particle,
    int **lattice,
    struct position best_position,
    struct position position,
    struct pm_config *configs
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
    struct coord lattice_adjust;
    int change = 0;

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

        if (best_position.fitness != 0 && best_position.fitness <= original_particle->position.fitness)
        {
            change = 1;

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

struct pso_result pso_run
(
    struct pso_config pso_config,
    enum polarity *seq,
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
    struct position gbest;
    struct position pm_best_position;
    struct position pm_position;
    struct pm_config* pm_configs;
    struct position iteration_position;
    int change;
    struct particle *particles;
    struct pso_result pso_result;
    clock_t t0;

    /**/pso_result.energy_evolution = (int*) smalloc(sizeof(int) * pso_config.iterations);
    /**/t0 = clock();

    //Sets seed
    if (*seed == -1)
    {
        *seed = (unsigned) time(NULL);
    }
    srand(*seed);

    init_variables(pso_config, &pm_configs, &lattice, &best_particle_by_edge,
                   &particles, &gbest, &pm_best_position, &pm_position, num_dimensions);

    initializes_population(pso_config, particles, &gbest, num_dimensions, best_particle_by_edge, lattice, seq);

    for (i = 0; i < pso_config.iterations; ++i)
    {
        for (j = 0; j < pso_config.population; ++j)
        {
            switch (pso_config.constructor) {

            case XIAO_LI_HU_2014:
                update_position_0(&(particles[j]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, j, particles);
                break;
            case HU_ZHANG_LI_2009:
                update_position_1(&(particles[j]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, j, particles);
                break;
            case SHMYGELSKA_HOOS_2003:
                update_position_2(&(particles[j]), lattice, seq, num_dimensions, pso_config, best_particle_by_edge, j, particles);
                break;
            default:
                printf("ERROR: aco.c/aco_run(): \"Invalid constructor\"\n");
                exit(1);
                break;

            }
            update_velocity(pso_config, &(particles[j]), gbest, num_dimensions);

        }


        for (j = 0; j < num_dimensions - 1; ++j)
        {
            best_particle_by_edge[j] = -1;
        }

        iteration_position = particles[0].position;

        //enum daemon search
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

            if (change == 1)
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

                change = 0;
            }


            if (particles[j].position.fitness < particles[j].pbest.fitness)
            {
                copy_position(&(particles[j].pbest), particles[j].position, num_dimensions);
            }

            if (particles[j].position.fitness < iteration_position.fitness)
            {
                iteration_position = particles[j].position;
            }
        }

        /**/if (i == 0 || iteration_position.fitness < gbest.fitness) {
        /**/    if (iteration_position.fitness < gbest.fitness) {
        /**/        pso_result.energy_evolution[i] = iteration_position.fitness;
        /**/    } else {
        /**/        pso_result.energy_evolution[i] = gbest.fitness;
        /**/    }
        /**/} else {
        /**/    pso_result.energy_evolution[i] = 1;
        /**/}

        if (iteration_position.fitness < gbest.fitness)
        {
            /**/pso_result.found_on_iteration = i;
            copy_position(&gbest, iteration_position, num_dimensions);
        }
    }

    /**/pso_result.time = (clock() - t0) / (double)CLOCKS_PER_SEC;
    /**/pso_result.energy = gbest.fitness;
    /**/pso_result.final_population_avg = 0;
    /**/pso_result.final_population_solution_rate = 0;
    /**/pso_result.final_population_stddev = 0;

    /**/for (i = 0; i < pso_config.population; ++i) {
    /**/    pso_result.final_population_avg += particles[i].position.fitness;
    /**/    if (particles[i].position.fitness == gbest.fitness)
    /**/    {
    /**/        ++pso_result.final_population_solution_rate;
    /**/    }
    /**/}

    /**/pso_result.final_population_solution_rate /= pso_config.population;
    /**/pso_result.final_population_avg /= pso_config.population;

    /**/for (i = 0; i < pso_config.population; ++i) {
    /**/    pso_result.final_population_stddev +=
    /**/        (pso_result.final_population_avg - particles[i].position.fitness) *
    /**/        (pso_result.final_population_avg - particles[i].position.fitness);
    /**/}

    /**/pso_result.final_population_stddev /= pso_config.population;
    /**/pso_result.final_population_stddev = sqrt(pso_result.final_population_stddev);
    /**/pso_result.final_population_solution_rate *= 100;
    /**/pso_result.directions = position_to_string(gbest, num_dimensions);

    free_variables(pso_config, lattice, particles, pm_best_position, pm_position, gbest,
                   pm_configs, best_particle_by_edge, num_dimensions);

    return pso_result;
}
