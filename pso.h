enum direction
{
    LEFT = 0,
    RIGHT = 1,
    STRAIGHT = 2,
    NONE = 3
};

enum boolean
{
    FALSE = 0,
    TRUE = 1
};

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

struct solution
{
    int energy;
    char *directions;
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

struct position
{
    enum direction *dir;
    struct coord *coord;
    int *fitness_by_edge;
    int fitness;
    enum boolean feasible;
};


typedef enum direction Direction;
typedef enum boolean Boolean;
typedef enum daemon daemon;
typedef enum polarity Polarity;
typedef struct pso_config Pso_config;
typedef struct solution Solution;
typedef struct position Position;

void usage_tests();
void init_solution(Solution *solution, int num_dimensions);
void free_solution(Solution solution);
void init_position(Position *position, int num_dimensions);
void free_position(Position position);
void extract_solution(Position position, Solution *solution, int seq_len);
Position pso_run(Pso_config pso_config, Polarity *seq, int num_dimensions, int *seed);
