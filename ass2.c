/*
  Program to plan agent routes in grids with blocks.

  Skeleton written by Artem Polyvyanyy, artem.polyvyanyy@unimelb.edu.au,
  in August 2026, with the intention that it be modified by students
  to add functionality, as required by the assignment specification.

  Student Authorship Declaration:

  (1) I certify that except for the code provided in the initial
  skeleton file the program contained in this submission is my
  own work, except where explicitly noted below in connection with
  the use of AI.  I understand that work that has been developed
  by another student, or by me in collaboration with other students,
  or by non-students as a result of request, solicitation, or
  payment, may not be submitted for assessment in this subject.

  (2) I also certify that I have not provided a copy of this work
  in either softcopy or hardcopy or any other form to any other
  student, and nor will I do so until after the marks are released.
  I understand that providing my work to other students, regardless
  of my intention or any undertakings made to me by that other
  student, is also Academic Misconduct.

  (3) I declare that I have used generative AI tools in connection
  with the following aspects of the assignment (please select the
  option that applies best in your case and delete the other three
  choices):

  -- I prompted an AI system with the full assignment specification
  and example input/output files, and requested a complete solution.
  I then reviewed what I was provided with, made further alterations
  to make sure it addressed the specification, including adding
  suitable comments and this Authorship Declaration.

  -- I developed the program structure for myself, and then used
  a sequence of prompts to an AI system to write parts of my design
  as individual functions (including, for example, via GitHub
  copilot), but without including the assignment specification in
  any of the prompts.

  -- I developed my own complete solution to the problem, and then
  used AI to help debug individual functions, find errors, and
  provide other guidance in regard to structure and so on.

  -- I made zero use of AI, and the program is wholly the result
  my own knowledge and development.

  (4) The AI system(s) that I used are:

  None.

  (5) I understand that submitting for assessment work developed
  in collaboration with other people constitutes Academic Misconduct,
  and may be penalized by mark deductions, or by other penalties
  determined via the University of Melbourne Academic Honesty
  Policy, as described at https://academicintegrity.unimelb.edu.au.

  Signed by: Sutong Zhao 1884449
  Dated:     2026-10-07

*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// various separators used in input/output
#define SEP0 '$'
#define SEP1 "==STAGE %d=======================================\n"
#define SEP2 "------------------------------------------------\n"
#define SEP3 "================================================\n"

// ASCII codes used in grid visualizations
#define CELL_CODE_INITIAL       73 // 'I' - initial cell
#define CELL_CODE_GOAL          71 // 'G' - goal cell
#define CELL_CODE_BLOCK         35 // '#' - block cell
#define CELL_CODE_EMPTY         32 // ' ' - empty cell
#define CELL_CODE_VISITED       42 // '*' - visited cell

// route statuses
#define ROUTE_STATUS_UNKNOWN    0 // Unknown route status
#define ROUTE_INVALID_INITIAL   1 // Initial cell in the route is wrong!
#define ROUTE_INVALID_GOAL      2 // Goal cell in the route is wrong!
#define ROUTE_INVALID_MOVE      3 // There is an illegal move in this route!
#define ROUTE_INVALID_BLOCK     4 // There is a block on this route!
#define ROUTE_VALID             5 // The route is valid!

#define MAX_CELLS_PER_LINE      5 // max number of cells to print per line
#define MAX_LINE_LENGTH         256 // maximum input line buffer size

/* type definitions --------------------------------------------------------*/

// grid cell
typedef struct {
    short row;              // row of the cell
    short col;              // column of the cell
    unsigned int counter;   // counter value associated with this cell
} cell_t;

// state in a route
typedef struct state state_t;
struct state {
    cell_t *cell;           // pointer to a cell
    state_t *next;          // pointer to the next state in the route
};

// route (implemented as a linked list)
typedef struct {
    state_t *head;  // pointer to the node in the head of the linked list
    state_t *tail;  // pointer to the node in the tail of the linked list
} route_t;


/* function prototypes -----------------------------------------------------*/

// linked-list functions for storing and freeing the route
route_t* make_empty_route(void);
void free_state(state_t*);
void free_route(route_t* R);
route_t* insert_at_head(route_t*, cell_t*);
route_t* insert_at_tail(route_t*, cell_t*);

// input functions for cells and route lines
cell_t *make_cell(short row, short col);
int read_cell_line(const char *line, cell_t *cell);
void read_route_line(route_t *R, const char *line);

// route checking and printing functions
int route_status(route_t *R, cell_t initial, cell_t goal,
                 unsigned char **grid, int rows, int cols);
void print_cell(cell_t *cell);
void print_route(route_t *R);
void print_route_status(int status);

// grid creation, clearing and printing functions
unsigned char **make_grid(int rows, int cols);
void free_grid(unsigned char **grid, int rows);
void clear_grid(unsigned char **grid, int rows, int cols);
void print_grid(unsigned char **grid, int rows, int cols,
                cell_t initial, cell_t goal, route_t *R);

// functions used to read and process each block configuration
int read_block_configuration(char *line, unsigned char **grid,
                             int rows, int cols);
int count_blocks(unsigned char **grid, int rows, int cols);
void print_stage0(int rows, int cols, int blocks,
                  cell_t initial, cell_t goal, route_t *R,
                  int status);
void process_stage1(unsigned char **grid, int rows, int cols,
                    cell_t initial, cell_t goal, route_t *R,
                    int status, int has_stage2);
void process_stage2(unsigned char **grid, int rows, int cols,
                    cell_t initial, cell_t goal, route_t *R,
                    char *line);

// functions used by the route-repair breadth-first search
int repair_route(route_t *R, unsigned char **grid, int rows, int cols);
int in_queue(cell_t *queue, int n, short row, short col);
state_t *route_target(state_t *after, short row, short col);

/* where it all happens ----------------------------------------------------*/
int
main(int argc, char *argv[]) {
    char line[MAX_LINE_LENGTH];
    int rows, cols, status;
    cell_t initial, goal;
    unsigned char **grid;
    route_t *route;

    // these parameters are not used, but are part of the standard main form
    (void)argc;
    (void)argv;

    // read the grid dimensions from the first input line
    fgets(line, sizeof(line), stdin);
    sscanf(line, "%dx%d", &rows, &cols);

    // read the initial and goal cells from the next two lines
    fgets(line, sizeof(line), stdin);
    read_cell_line(line, &initial);
    fgets(line, sizeof(line), stdin);
    read_cell_line(line, &goal);

    // create an empty grid before reading the first block configuration
    grid = make_grid(rows, cols);
    clear_grid(grid, rows, cols);

    // read blocks until the '$' which separates blocks from the route
    read_block_configuration(line, grid, rows, cols);

    // read the proposed route until the '$' which starts Stage 2
    route = make_empty_route();
    int stage2_started = 0;
    while (fgets(line, sizeof(line), stdin)) {
        if (line[0] == SEP0) {
            stage2_started = 1;
            break;
        }

        // a route may be split over any number of input lines
        read_route_line(route, line);
    }

    // check the original route and print all Stage 0 information
    status = route_status(route, initial, goal, grid, rows, cols);
    print_stage0(rows, cols, count_blocks(grid, rows, cols),
                 initial, goal, route, status);
    
    // Stage 1 repairs at most the first broken segment
    process_stage1(grid, rows, cols, initial, goal, route, status,
                   stage2_started);

    // Stage 2 uses the repaired route from the previous configuration
    if (stage2_started) {
        process_stage2(grid, rows, cols, initial, goal, route, line);
    }

    // the route and grid are dynamically allocated, so both are freed here
    free_route(route);
    free_grid(grid, rows);
    return EXIT_SUCCESS;        // we are done! algorithms are fun!!!
}

// read one complete block configuration and leave the '$' consumed
int
read_block_configuration(char *line, unsigned char **grid,
                         int rows, int cols) {
    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        // '$' marks the end of the current block configuration
        if (line[0] == SEP0) {
            return 1;
        }

        cell_t block;

        // store only coordinates which are valid grid positions
        if (read_cell_line(line, &block) &&
            block.row >= 0 && block.row < rows &&
            block.col >= 0 && block.col < cols) {
            grid[block.row][block.col] = 1;
        }
    }
    return 0;
}

// count the occupied cells in the current grid
int
count_blocks(unsigned char **grid, int rows, int cols) {
    int blocks = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c]) {
                blocks++;
            }
        }
    }
    return blocks;
}

// print all information required for Stage 0
void
print_stage0(int rows, int cols, int blocks,
             cell_t initial, cell_t goal, route_t *R,
             int status) {
    printf(SEP1, 0);

    // print the grid dimensions and number of blocks first
    printf("The grid has %d rows and %d columns.\n", rows, cols);
    printf("The grid has %d block(s).\n", blocks);

    // print the initial and goal cells using the same cell format as routes
    printf("The initial cell in the grid is ");
    print_cell(&initial);
    printf(".\n");

    printf("The goal cell in the grid is ");
    print_cell(&goal);
    printf(".\n");

    // print the proposed route before reporting its status
    printf("The proposed route in the grid is:\n");
    print_route(R);
    print_route_status(status);
}

// perform the single repair required by Stage 1, if necessary
void
process_stage1(unsigned char **grid, int rows, int cols,
               cell_t initial, cell_t goal, route_t *R,
               int status, int has_stage2) {
    printf(SEP1, 1);
    print_grid(grid, rows, cols, initial, goal, R);

    // a valid route or a non-block error does not enter the repair algorithm
    if (status == ROUTE_INVALID_BLOCK) {
        if (repair_route(R, grid, rows, cols)) {
            // show the grid after the route has been repaired
            printf(SEP2);
            print_grid(grid, rows, cols, initial, goal, R);

            // print the repaired route and its new status
            printf(SEP2);
            status = route_status(R, initial, goal, grid, rows, cols);
            print_route(R);
            print_route_status(status);
        } else {
            // if no repair exists, the route remains the original route
            printf(SEP2);
            print_grid(grid, rows, cols, initial, goal, R);
            printf("The route cannot be repaired!\n");
        }
    }

    // the final separator is printed only when Stage 2 does not follow
    if (!has_stage2) {
        printf(SEP3);
    }
}

// process every Stage 2 block configuration in order;
// the route is deliberately not reset between configurations
void
process_stage2(unsigned char **grid, int rows, int cols,
               cell_t initial, cell_t goal, route_t *R,
               char *line) {
    int more_input = 1;

    printf(SEP1, 2);

    while (more_input) {
        // each configuration starts immediately after the previous '$'
        clear_grid(grid, rows, cols);

        // read blocks until the '$' terminating this configuration
        more_input = read_block_configuration(line, grid, rows, cols);

        // show the grid before attempting any repair
        print_grid(grid, rows, cols, initial, goal, R);

        int status = route_status(R, initial, goal, grid, rows, cols);

        // a configuration with a valid route needs no repair grids
        if (status == ROUTE_VALID) {
            printf(SEP3);
        } else if (status == ROUTE_INVALID_BLOCK) {
            // keep repairing the first blocked segment until the route is
            // valid or no further repair can be found
            while (status == ROUTE_INVALID_BLOCK) {
                // apply the repair procedure without printing
                // intermediate grids
                if (!repair_route(R, grid, rows, cols)) {
                    break;
                }
                status = route_status(R, initial, goal, grid, rows, cols);
            }

            // print only the final grid after all repair attempts
            printf(SEP2);
            print_grid(grid, rows, cols, initial, goal, R);
            printf(SEP2);

            // a fully repaired route is followed by its route and status
            if (status == ROUTE_VALID) {
                print_route(R);
                print_route_status(status);
            } else {
                printf("The route cannot be repaired!\n");
            }
            printf(SEP3);
        } else {
            // initial/goal/move errors are not repaired by Stage 2
            printf(SEP2);
            printf("The route cannot be repaired!\n");
            printf(SEP3);
        }
    }
}

// free one state and the cell stored inside it
void
free_state(state_t* state) {
    if (state) {
        free(state->cell);
        free(state);
    }
}

// Adapted version of the make_empty_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
route_t
*make_empty_route(void) {
    route_t *R;

    // allocate the route structure and start with an empty linked list
    R = (route_t*)malloc(sizeof(*R));
    assert(R!=NULL);
    R->head = R->tail = NULL;
    return R;
}

// Adapted version of the free_list function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
void
free_route(route_t* R) {
    state_t *curr, *prev;
    assert(R!=NULL);
    curr = R->head;

    // visit every linked-list node and free its cell and node
    while (curr) {
        prev = curr;
        curr = curr->next;
        free_state(prev);
    }
    free(R);
}

// Adapted version of the insert_at_head function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
route_t
*insert_at_head(route_t* R, cell_t* addr) {
    state_t *new;
    assert(R != NULL);

    // create a new state and make it the first node in the route
    new = (state_t*)malloc(sizeof(*new));
    assert(new != NULL);
    new->cell = addr;
    new->next = R->head;
    R->head = new;

    // if the route was empty, the new node is also its tail
    if (R->tail == NULL) {
        R->tail = new;
    }
    return R;
}

// Adapted version of the insert_at_foot function by Alistair Moffat:
// https://people.eng.unimelb.edu.au/ammoffat/ppsaa/c/listops.c
// Data type and variable names changed
route_t
*insert_at_tail(route_t* R, cell_t* addr) {
    state_t *new;
    assert(R != NULL);

    // create a new final state with no successor
    new = (state_t*)malloc(sizeof(*new));
    assert(new != NULL);
    new->cell = addr;
    new->next = NULL;

    // update both head and tail when the route was previously empty
    if (R->tail == NULL) {
        R->head = R->tail = new;
    } else {
        // otherwise attach the new state after the old tail
        R->tail->next = new;
        R->tail = new;
    }
    return R;
}

/* input functions ---------------------------------------------------------*/

// allocate and initialise one grid-cell record
cell_t
*make_cell(short row, short col) {
    cell_t *cell = (cell_t*)malloc(sizeof(*cell));
    assert(cell != NULL);

    // the counter is used later by the breadth-first search
    cell->row = row;
    cell->col = col;
    cell->counter = 0;
    return cell;
}

// read one cell written in the form [row,column]
int
read_cell_line(const char *line, cell_t *cell) {
    return sscanf(line, "[%hd,%hd]", &cell->row, &cell->col) == 2;
}

// read every [row,column] pair appearing on one route line;
// the input may contain several cells on the same line
void
read_route_line(route_t *R, const char *line) {
    const char *p = line;
    short row, col;

    while (sscanf(p, "[%hd,%hd]", &row, &col) == 2) {
        // add each parsed cell to the end so input order is preserved
        insert_at_tail(R, make_cell(row, col));

        // move to the next '[' before trying to read another cell
        p++;
        while (*p != '\0' && *p != '[') {
            p++;
        }
    }
}

/* route functions ---------------------------------------------------------*/

// check a route in the required order:
// initial cell, goal cell, moves, then blocks
int
route_status(route_t *R, cell_t initial, cell_t goal,
             unsigned char **grid, int rows, int cols) {
    state_t *curr, *prev;

    // first check that the route begins at the required initial cell
    if (R->head == NULL || R->head->cell->row != initial.row ||
        R->head->cell->col != initial.col) {
        return ROUTE_INVALID_INITIAL;
    }

    // the route must end at the required goal cell
    if (R->tail->cell->row != goal.row ||
        R->tail->cell->col != goal.col) {
        return ROUTE_INVALID_GOAL;
    }

    // check every pair of consecutive cells for a one-step move
    prev = R->head;
    curr = R->head->next;
    while (curr != NULL) {
        int dr = curr->cell->row - prev->cell->row;
        int dc = curr->cell->col - prev->cell->col;

        // a legal move has Manhattan distance exactly one
        if ((dr < 0 ? -dr : dr) + (dc < 0 ? -dc : dc) != 1) {
            return ROUTE_INVALID_MOVE;
        }
        prev = curr;
        curr = curr->next;
    }

    // only after all moves are legal do we check for blocks
    for (curr = R->head; curr != NULL; curr = curr->next) {
        if (curr->cell->row >= 0 && curr->cell->row < rows &&
            curr->cell->col >= 0 && curr->cell->col < cols &&
            grid[curr->cell->row][curr->cell->col]) {
            return ROUTE_INVALID_BLOCK;
        }
    }
    return ROUTE_VALID;
}

// print one cell in the exact [row,column] format used by the assignment
void
print_cell(cell_t *cell) {
    printf("[%d,%d]", cell->row, cell->col);
}

// print the linked-list route, putting at most five cells on each line;
// the final cell is followed by a full stop instead of an arrow
void
print_route(route_t *R) {
    state_t *curr = R->head;
    int count = 0;

    while (curr != NULL) {
        print_cell(curr->cell);
        count++;

        if (curr->next != NULL) {
            printf("->");

            // start a new line after every fifth printed cell
            if (count == MAX_CELLS_PER_LINE) {
                printf("\n");
                count = 0;
            }
        } else {
            // the final cell ends the route with a period
            printf(".\n");
        }
        curr = curr->next;
    }
}

// print the required message for the route status code
void
print_route_status(int status) {
    if (status == ROUTE_INVALID_INITIAL) {
        printf("The initial cell in the route is wrong!\n");
    } else if (status == ROUTE_INVALID_GOAL) {
        printf("The goal cell in the route is wrong!\n");
    } else if (status == ROUTE_INVALID_MOVE) {
        printf("There is an illegal move in this route!\n");
    } else if (status == ROUTE_INVALID_BLOCK) {
        printf("There is a block on this route!\n");
    } else if (status == ROUTE_VALID) {
        printf("The route is valid!\n");
    }
}

/* grid functions ----------------------------------------------------------*/

// allocate a two-dimensional grid as an array of row pointers
unsigned char
**make_grid(int rows, int cols) {
    unsigned char **grid;

    // first allocate the array containing the row pointers
    grid = (unsigned char**)malloc(rows * sizeof(*grid));
    assert(grid != NULL);

    // then allocate each row separately
    for (int r = 0; r < rows; r++) {
        grid[r] = (unsigned char*)malloc(cols * sizeof(*grid[r]));
        assert(grid[r] != NULL);
    }
    return grid;
}

// free every row of a grid and then free the row-pointer array
void
free_grid(unsigned char **grid, int rows) {
    for (int r = 0; r < rows; r++) {
        free(grid[r]);
    }
    free(grid);
}

// set every cell in a grid to zero, removing all previous blocks
void
clear_grid(unsigned char **grid, int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            grid[r][c] = 0;
        }
    }
}

// print the grid with the column header, row labels, blocks and route;
// a temporary visited grid lets us mark every route cell without changing
// the actual block configuration.

void
print_grid(unsigned char **grid, int rows, int cols,
           cell_t initial, cell_t goal, route_t *R) {
    unsigned char **visited = make_grid(rows, cols);
    clear_grid(visited, rows, cols);

    // mark every route cell which lies inside the displayed grid
    for (state_t *curr = R->head; curr != NULL; curr = curr->next) {
        if (curr->cell->row >= 0 && curr->cell->row < rows &&
            curr->cell->col >= 0 && curr->cell->col < cols) {
            visited[curr->cell->row][curr->cell->col] = 1;
        }
    }

    // a leading space before the column numbers
    putchar(' ');
    for (int c = 0; c < cols; c++) {
        putchar('0' + c % 10);
    }
    putchar('\n');

    for (int r = 0; r < rows; r++) {
        // row labels also repeat their final digit for large grids
        putchar('0' + r % 10);

        for (int c = 0; c < cols; c++) {
            // blocks have priority over all other symbols
            if (grid[r][c]) {
                putchar(CELL_CODE_BLOCK);
            } else if (r == initial.row && c == initial.col) {
                putchar(CELL_CODE_INITIAL);
            } else if (r == goal.row && c == goal.col) {
                putchar(CELL_CODE_GOAL);
            } else if (visited[r][c]) {
                putchar(CELL_CODE_VISITED);
            } else {
                putchar(CELL_CODE_EMPTY);
            }
        }
        putchar('\n');
    }

    // the temporary grid is no longer needed after printing
    free_grid(visited, rows);
}

/* route repair ------------------------------------------------------------*/

// return 1 when the given cell has already been added to the BFS queue
int
in_queue(cell_t *queue, int n, short row, short col) {
    for (int i = 0; i < n; i++) {
        if (queue[i].row == row && queue[i].col == col) {
            return 1;
        }
    }
    return 0;
}

// find a cell in the unbroken suffix of the old route;
// the returned state is used as the endpoint of the new repair path
state_t
*route_target(state_t *after, short row, short col) {
    for (state_t *curr = after; curr != NULL; curr = curr->next) {
        if (curr->cell->row == row && curr->cell->col == col) {
            return curr;
        }
    }
    return NULL;
}

// repair the first block-broken segment of the route using BFS;
// the search starts at the route cell immediately before the first block;
// neighbours are considered in the required order: above, below, left, right;
// the first cell from the old route suffix reached by BFS is used
// as the endpoint, giving the shortest repair with the required tie-break
int
repair_route(route_t *R, unsigned char **grid, int rows, int cols) {
    state_t *broken = NULL;
    state_t *start, *after;

    // find the first route state which lies on a block
    for (state_t *curr = R->head; curr != NULL; curr = curr->next) {
        if (curr->cell->row >= 0 && curr->cell->row < rows &&
            curr->cell->col >= 0 && curr->cell->col < cols &&
            grid[curr->cell->row][curr->cell->col]) {
            broken = curr;
            break;
        }
    }

    // a repair needs an unblocked route cell before the broken segment
    if (broken == NULL || broken == R->head) {
        return 0;
    }

    // find the route state immediately before the first broken cell
    start = R->head;
    while (start->next != broken) {
        start = start->next;
    }

    // skip the consecutive blocked part of the old route
    after = broken;
    while (after != NULL &&
           after->cell->row >= 0 && after->cell->row < rows &&
           after->cell->col >= 0 && after->cell->col < cols &&
           grid[after->cell->row][after->cell->col]) {
        after = after->next;
    }

    // there is no route suffix to reconnect to
    if (after == NULL) {
        return 0;
    }

    // every grid cell needs at most one queue entry
    int capacity = rows * cols;
    cell_t *queue = (cell_t*)malloc(capacity * sizeof(*queue));
    assert(queue != NULL);

    int front = 0;
    int back = 0;

    // start BFS from the last valid cell before the broken segment
    cell_t first = *start->cell;
    first.counter = 0;
    queue[back++] = first;

    state_t *target = NULL;

    while (front < back && target == NULL) {
        // remove the oldest queue entry, which is the BFS property
        cell_t current = queue[front++];

        // neighbour order: above, below, left, right
        int nr[4] = {current.row - 1, current.row + 1,
                     current.row, current.row};
        int nc[4] = {current.col, current.col,
                     current.col - 1, current.col + 1};

        for (int i = 0; i < 4; i++) {
            // ignore neighbours outside the grid
            if (nr[i] < 0 || nr[i] >= rows ||
                nc[i] < 0 || nc[i] >= cols) {
                continue;
            }

            // blocks and previously queued cells cannot be used again
            if (grid[nr[i]][nc[i]] ||
                in_queue(queue, back, nr[i], nc[i])) {
                continue;
            }

            // record this cell and its BFS distance from the start
            cell_t next;
            next.row = nr[i];
            next.col = nc[i];
            next.counter = current.counter + 1;
            queue[back++] = next;

            // stop when the newly reached cell is on the old route suffix
            target = route_target(after, nr[i], nc[i]);
            if (target != NULL) {
                break;
            }
        }
    }

    // if the BFS never reached the old route, the segment cannot be repaired
    if (target == NULL) {
        free(queue);
        return 0;
    }

    // reconstruct the shortest path from target back to start;
    // the smallest counter identifies a predecessor one step closer to start
    route_t *replacement = make_empty_route();
    cell_t current = *target->cell;

    while (current.row != start->cell->row ||
           current.col != start->cell->col) {
        // add the current path cell at the front to reverse the path
        insert_at_head(replacement, make_cell(current.row, current.col));

        int best = -1;
        int nr[4] = {current.row - 1, current.row + 1,
                     current.row, current.row};
        int nc[4] = {current.col, current.col,
                     current.col - 1, current.col + 1};

        // look for an adjacent queued cell with the smallest counter
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < back; j++) {
                if (queue[j].row == nr[i] && queue[j].col == nc[i] &&
                    (best == -1 || queue[j].counter < queue[best].counter)) {
                    best = j;
                }
            }
        }

        // this should not happen for a valid BFS path
        if (best == -1) {
            free_route(replacement);
            free(queue);
            return 0;
        }
        current = queue[best];
    }

    // include the starting route cell in the temporary replacement path
    insert_at_head(replacement,
                   make_cell(start->cell->row, start->cell->col));

    // delete the old broken segment, but keep target and the route after it
    state_t *curr = start->next;
    while (curr != target) {
        state_t *next = curr->next;
        free_state(curr);
        curr = next;
    }

    // insert the replacement cells after start and before target;
    // the start cell already exists in the original route, so skip it here;
    // the old broken segment has already been deleted, so reconnect start
    // directly to target before inserting any intermediate cells
    start->next = target;

    state_t *new_curr = replacement->head->next;
    state_t *previous = start;

    while (new_curr != NULL && new_curr->next != NULL) {
        state_t *new_state = (state_t*)malloc(sizeof(*new_state));
        assert(new_state != NULL);

        // copy only the coordinate information needed by the route
        new_state->cell = make_cell(new_curr->cell->row,
                                    new_curr->cell->col);
        new_state->next = target;
        previous->next = new_state;
        previous = new_state;
        new_curr = new_curr->next;
    }

    // the temporary replacement list and BFS queue are no longer needed
    free_route(replacement);
    free(queue);
    return 1;
}
