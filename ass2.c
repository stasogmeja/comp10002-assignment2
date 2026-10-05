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
  Dated:     2026-10-05

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

// grid creation, clearing and printing functions
unsigned char **make_grid(int rows, int cols);
void free_grid(unsigned char **grid, int rows);
void clear_grid(unsigned char **grid, int rows, int cols);

// functions used to read and process each block configuration
int read_block_configuration(char *line, unsigned char **grid,
                             int rows, int cols);
int count_blocks(unsigned char **grid, int rows, int cols);

// functions used by the route-repair breadth-first search


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
        /* Add each parsed cell to the end so input order is preserved. */
        insert_at_tail(R, make_cell(row, col));

        /* Move to the next '[' before trying to read another cell. */
        p++;
        while (*p != '\0' && *p != '[') {
            p++;
        }
    }
}

/* route functions ---------------------------------------------------------*/



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

/* Free every row of a grid and then free the row-pointer array. */
void
free_grid(unsigned char **grid, int rows) {
    for (int r = 0; r < rows; r++) {
        free(grid[r]);
    }
    free(grid);
}

/* Set every cell in a grid to zero, removing all previous blocks. */
void
clear_grid(unsigned char **grid, int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            grid[r][c] = 0;
        }
    }
}

/* route repair ------------------------------------------------------------*/
