#include "markov_chain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORD_MAX_LENGTH 100
#define BUFFER_SIZE 1000
#define FULL_AMOUNT_OF_ARGC 5
#define ACCEPTED_AMOUNT_OF_ARGC 4
#define TEMP_NUMBER (-100)
#define DELIMITERS "\n\r\t "

typedef enum Program {
    SEED = 1,
    TWEETS_NUMBER,
    TEXT_CORPUS_PATH,
    WORD_TO_READ
} Program;


// ERROR MESSAGE'S SECTION:

#define ERR_MSG_FILE_PATH "Error: the program have an invalid file path.\n"

#define ERR_MSG_USAGE_PROBLEM "Usage: Please fill the following command's \
./tweets_generator_logic <seed> <number of tweets> <text corpus path> \
[words to read].\n"

#define ERR_MSG_ALLOCATION_FAILURE \
  "Allocation failure: Something went wrong! we couldn't allocate enough "\
  "memory for your program, pleas try again.\n"

#define END_TWIT_CONST '.'
// the ASCII value of 46

// COMPILATION & DECLARATION SECTION:

static bool ok_arguments_amount (int argc);
static int validate_input (int argc, char *argv[], int *seed, int
*tweets_amount, int *words_to_read);
static bool parse_integer_from_string (int *changed_source, char *source);
static int tweets_generator_logic (unsigned int seed, unsigned int
tweets_number, char *text_corpus_path, int words_to_read);
static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain);

static void print_str (const void *ptr)
{
  const char *str = (const char *) ptr;
  printf ("%s", str);
}

static int comp_str (const void *ptr1, const void *ptr2)
{
  const char *str1 = (const char *) ptr1;
  const char *str2 = (const char *) ptr2;
  return strcmp (str1, str2);
}

static void free_str (void *ptr)
{
  char *str = (char *) ptr;
  free ((char *) str);
}

static void *copy_str (const void *ptr)
{
  const char *str = (const char *) ptr;
  if (str == NULL)
    {
      return NULL;
    }
  unsigned long length = strlen ((char *) str);
  char *new_str = malloc (sizeof (char) * length + 1);
  if (new_str == NULL)
    {
      return NULL;
    }
  strcpy (new_str, (const char *) str);
  return (void *) new_str;
}

static bool is_last_str (const void *ptr)
{
  const char *str = (const char *) ptr;
  unsigned long length = strlen (str);
  unsigned long last_char_loc = length - 1;

  bool didnt_found_colon = false;

  if (str[last_char_loc] == END_TWIT_CONST)
    {
      didnt_found_colon = true;
    }
  return !didnt_found_colon;
}


// _______________________________starts____________________________________ //

int main (int argc, char *argv[])
{
  // setting the params.
  int seed = TEMP_NUMBER;
  int tweets_amount = TEMP_NUMBER;
  int words_to_read = TEMP_NUMBER;
  char *text_corpus_path = NULL;
  if (validate_input (argc, argv, &seed, &tweets_amount, &words_to_read)
      == EXIT_FAILURE)

    { return EXIT_FAILURE; }

  text_corpus_path = argv[TEXT_CORPUS_PATH];

  return tweets_generator_logic (seed, tweets_amount,
                                 text_corpus_path, words_to_read);
}

static int validate_input (int argc, char *argv[], int *seed, int
*tweets_amount, int *words_to_read)
{
  if (!ok_arguments_amount (argc))
    {
      fprintf (stdout, ERR_MSG_USAGE_PROBLEM);
      return EXIT_FAILURE;
    }

  if (parse_integer_from_string (seed, argv[SEED]) == false)
    {
      return EXIT_FAILURE;
    }
  if (parse_integer_from_string (tweets_amount, argv[TWEETS_NUMBER]) ==
      false)
    {
      return EXIT_FAILURE;
    }

  if (argc == FULL_AMOUNT_OF_ARGC)
    {
      if (parse_integer_from_string (words_to_read, argv[WORD_TO_READ]) ==
          false)
        {
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}

static bool ok_arguments_amount (int argc)
{
  bool valid = true;
  if (!((argc == ACCEPTED_AMOUNT_OF_ARGC) || (argc == FULL_AMOUNT_OF_ARGC)))
    { valid = false; }

  return valid;
}

static bool parse_integer_from_string (int *changed_source, char *source)
{
  bool flag = true;
  if (sscanf (source, "%d", changed_source) != 1)
    { flag = false; }

  return flag;
}

int tweets_generator_logic (unsigned int seed, unsigned int
tweets_number, char *text_corpus_path, int words_to_read)
{
// opening the file and define the seed.
  FILE *fp = fopen (text_corpus_path, "r");
  if (fp == NULL)
    {
      fprintf (stdout, ERR_MSG_FILE_PATH);
      return EXIT_FAILURE;
    }
  srand (seed);

  // defining the params.
  LinkedList linked_list = {NULL, NULL, 0};
  MarkovChain markov_chain = {0};
  markov_chain.database = &linked_list;
  markov_chain.comp_func = comp_str;
  markov_chain.free_data = free_str;
  markov_chain.copy_func = copy_str;
  markov_chain.is_last = is_last_str;
  markov_chain.print_func = print_str;
  MarkovChain *markov_chain_pointer = &markov_chain;

  int ans = fill_database (fp, words_to_read, markov_chain_pointer);
  if (ans == EXIT_SUCCESS)
    {
      for (unsigned int index_of_tweet = 0;
           index_of_tweet < tweets_number; index_of_tweet++)
        {
          fprintf (stdout, "Tweet %d: ", index_of_tweet + 1);
          generate_tweet (markov_chain_pointer, NULL, WORD_MAX_LENGTH);
        }

      fclose (fp);
      free_database (&markov_chain_pointer);
      return EXIT_SUCCESS;
    }
  else
    {
      fclose (fp);
      free_database (&markov_chain_pointer);
      return EXIT_FAILURE;
    }
}

static bool continue_reading (int count, int max)
{
  if (max == TEMP_NUMBER)
    {
      return true;
    }

  if (count < max)
    {
      return true;
    }
  return false;
}

/***
 * @param line the current line to parse and add to the chain
 * @param markov_chain the chain to add the words into
 * @return the amount of words added to the chain
 */
static int parse_one_line (char *line, MarkovChain *markov_chain,
                           const int words_to_read, int word_count)
{
  char *current_word;
  Node *curr = NULL;
  Node *prev = NULL;
  current_word = strtok (line, DELIMITERS);
  while (current_word != NULL && continue_reading (word_count,
                                                   words_to_read))
    {
      curr = add_to_database (markov_chain, current_word);
      word_count++;
      if (curr == NULL)
        {
          return EXIT_FAILURE;
        }

      if (prev)
        {
          add_node_to_frequencies_list ((prev)->data,
                                        (curr)->data, markov_chain);
        }

      prev = curr;
      current_word = strtok (NULL, DELIMITERS);
    }
  return word_count;
}

static int fill_database (FILE *fp, int words_to_read, MarkovChain
*markov_chain)
{
  int word_count = 0;

  if (fp == NULL)
    {
      return EXIT_FAILURE;
    }

  char line_buffer[BUFFER_SIZE];

  while (fgets (line_buffer, BUFFER_SIZE, fp) != NULL)
    {
      word_count = parse_one_line (line_buffer, markov_chain,
                                   words_to_read, word_count);
      if (word_count == EXIT_FAILURE)
        {
          return EXIT_FAILURE;
        }

      if (words_to_read != TEMP_NUMBER && word_count >= words_to_read)
        {
          break;
        }
    }

  return EXIT_SUCCESS;
}

