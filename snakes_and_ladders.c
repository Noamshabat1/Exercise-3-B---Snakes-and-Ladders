#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

typedef enum Program {
    SEED = 1,
    PATH_AMOUNT,
} Program;

#define TEMP_NUMBER (-100)

#define EMPTY (-1)
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20

#define ACCEPTED_ARG_COUNT 3


// ERROR MESSAGE'S SECTION:
#define ERR_MSG_USAGE_PROBLEM "Usage: Please fill the following command's" \
" ./snakes_and_ladders <seed> <number of paths>"

// COMPILATION & DECLARATION SECTION:
int snakes_and_ladders_logic (int seed, int paths_amount);

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number;
    // Cell number 1-100
    int ladder_to;
    // ladder_to represents the jump of the -
    // ladder in case there is one from this square
    int snake_to;
    // snake_to represents the jump of the snake in case there is -
    // one from this square

    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

static bool is_last_struct_cell (const void *ptr)
{
  Cell *p_cell = (Cell *) ptr;

  return (p_cell->number != BOARD_SIZE);
}

static void print_struct_cell (const void *ptr)
{
  Cell *p_cell = (Cell *) ptr;

  if (p_cell->ladder_to == -1 && p_cell->snake_to == -1)
    {
      printf ("[%d]", p_cell->number);
    }
  else
    {
      printf ("[%d]", p_cell->number);
      if (p_cell->ladder_to != -1)
        {
          printf ("-ladder to %d", p_cell->ladder_to);
        }
      else if (p_cell->snake_to != -1)
        {
          printf ("-snake to %d", p_cell->snake_to);
        }
    }

  if (is_last_struct_cell (ptr))
    {
      printf (" ->");
    }
}

static int comp_struct_cell (const void *ptr1, const void *ptr2)
{
  Cell *comp1 = (Cell *) ptr1;
  Cell *comp2 = (Cell *) ptr2;

  int ans = comp1->number - comp2->number;
  return ans;
}

static void free_struct_cell (void *ptr)
{
  Cell *p_cell = (Cell *) ptr;
  free (p_cell);
}

static void *copy_struct_cell (const void *ptr)
{
  Cell *p_cell = (Cell *) ptr;

  Cell *copy_p_cell = malloc (sizeof (*copy_p_cell));

  if (copy_p_cell == NULL)
    { return NULL; }

  *copy_p_cell = *p_cell;
  return copy_p_cell;
}

/** Error handler **/
static int handle_error (char *error_msg, MarkovChain **database)
{
  printf ("%s", error_msg);
  if (database != NULL)
    {
      free_database (database);
    }
  return EXIT_FAILURE;
}

static int create_board (Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
    {
      cells[i] = malloc (sizeof (Cell));
      if (cells[i] == NULL)
        {
          for (int j = 0; j < i; j++)
            {
              free (cells[j]);

            }
          handle_error (ALLOCATION_ERROR_MASSAGE, NULL);
          return EXIT_FAILURE;
        }
      *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
      int from = transitions[i][0];
      int to = transitions[i][1];
      if (from < to)
        {
          cells[from - 1]->ladder_to = to;
        }
      else
        {
          cells[from - 1]->snake_to = to;
        }
    }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell *cells[BOARD_SIZE];
  if (create_board (cells) == EXIT_FAILURE)
    {
      return EXIT_FAILURE;
    }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
    {
      add_to_database (markov_chain, cells[i]);
    }

  for (size_t i = 0; i < BOARD_SIZE; i++)
    {
      from_node = get_node_from_database (markov_chain, cells[i])->data;

      if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
          index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
          to_node = get_node_from_database (markov_chain, cells[index_to])
              ->data;
          add_node_to_frequencies_list (from_node, to_node, markov_chain);
        }
      else
        {
          for (int j = 1; j <= DICE_MAX; j++)
            {
              index_to = ((Cell *) (from_node->data))->number + j - 1;
              if (index_to >= BOARD_SIZE)
                {
                  break;
                }
              to_node = get_node_from_database (markov_chain, cells[index_to])
                  ->data;
              add_node_to_frequencies_list (from_node, to_node, markov_chain);
            }
        }
    }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
    {
      free (cells[i]);
    }
  return EXIT_SUCCESS;
}

static bool ok_arguments_amount_s (int argc)
{
  bool valid = true;
  if (argc != ACCEPTED_ARG_COUNT)
    { valid = false; }

  return valid;
}

static bool parse_integer_from_string_s (int *changed_source, char *source)
{
  bool flag = true;
  if (sscanf (source, "%d", changed_source) != 1)
    { flag = false; }

  return flag;
}

static int validate_input_s (int argc, char *argv[], int *seed, int
*paths_amount)
{
  if (!ok_arguments_amount_s (argc))
    {
      fprintf (stdout, ERR_MSG_USAGE_PROBLEM);
      return EXIT_FAILURE;
    }

  if (parse_integer_from_string_s (seed, argv[SEED]) == false)
    { return EXIT_FAILURE; }

  if (parse_integer_from_string_s (paths_amount, argv[PATH_AMOUNT]) ==
      false)
    { return EXIT_FAILURE; }

  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  int seed = TEMP_NUMBER;
  int paths_amount = TEMP_NUMBER;
  if (validate_input_s (argc, argv, &seed, &paths_amount) == EXIT_FAILURE)
    { return EXIT_FAILURE; }

  return snakes_and_ladders_logic (seed, paths_amount);
}

int snakes_and_ladders_logic (int seed, int paths_amount)
{
  srand (seed);

  // defining the params.
  LinkedList linked_list = {.first = NULL, .last = NULL, .size = 0};
  MarkovChain markov_chain = {0};
  markov_chain.database = &linked_list;
  markov_chain.print_func = print_struct_cell;
  markov_chain.comp_func = comp_struct_cell;
  markov_chain.free_data = free_struct_cell;
  markov_chain.copy_func = copy_struct_cell;
  markov_chain.is_last = is_last_struct_cell;
  MarkovChain *markov_chain_ptr = &markov_chain;

  int ans = fill_database (markov_chain_ptr);
  if (ans == EXIT_FAILURE) // check!
    {
      return EXIT_FAILURE;
    }

  MarkovNode *first = markov_chain_ptr->database->first->data;
  for (int i = 0; i < paths_amount; i++)
    {
      printf ("Random Walk %d: ", i + 1);
      generate_tweet (markov_chain_ptr, first, MAX_GENERATION_LENGTH);
    }
  free_database (&markov_chain_ptr);
  return EXIT_SUCCESS;
}
