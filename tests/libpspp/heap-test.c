/* PSPP - a program for statistical analysis.
   Copyright (C) 2007, 2010, 2012 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* This is a test program for the routines defined in heap.c.
   This test program aims to be as comprehensive as possible.
   With -DNDEBUG, "gcov -b" should report 100% coverage of lines
   and branches in heap.c routines, except for the is_heap
   function, which is not called at all with -DNDEBUG.  (Without
   -DNDEBUG, branches caused by failed assertions will also not
   be taken.)  "valgrind --leak-check=yes --show-reachable=yes"
   should give a clean report, both with and without -DNDEBUG. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libpspp/heap.h>

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libpspp/compiler.h>

#include "xalloc.h"

/* Exit with a failure code.
   (Place a breakpoint on this function while debugging.) */
static void
check_die (void)
{
  exit (EXIT_FAILURE);
}

/* If OK is not true, prints a message about failure on the
   current source file and the given LINE and terminates. */
static void
check_func (bool ok, int line)
{
  if (!ok)
    {
      fprintf (stderr, "%s:%d: check failed\n", __FILE__, line);
      check_die ();
    }
}

/* Verifies that EXPR evaluates to true.
   If not, prints a message citing the calling line number and
   terminates. */
#define check(EXPR) check_func ((EXPR), __LINE__)

/* Node type and support routines. */

/* Test data element. */
struct element
  {
    struct heap_node node;      /* Embedded heap element. */
    int x;                      /* Primary value. */
  };

static int aux_data;

/* Returns the `struct element' that NODE is embedded within. */
static struct element *
heap_node_to_element (const struct heap_node *node)
{
  return heap_data (node, struct element, node);
}

/* Compares the `x' values in A and B and returns a strcmp-type
   return value.  Verifies that AUX points to aux_data. */
static int
compare_elements (const struct heap_node *a_, const struct heap_node *b_,
                  const void *aux)
{
  const struct element *a = heap_node_to_element (a_);
  const struct element *b = heap_node_to_element (b_);

  check (aux == &aux_data);
  return a->x < b->x ? -1 : a->x > b->x;
}

/* Returns the smallest of the N integers in ARRAY. */
static int
min_int (int *array, size_t n)
{
  int min;
  size_t i;

  min = INT_MAX;
  for (i = 0; i < n; i++)
    if (array[i] < min)
      min = array[i];
  return min;
}

/* Swaps *A and *B. */
static void
swap (int *a, int *b)
{
  int t = *a;
  *a = *b;
  *b = t;
}

/* Reverses the order of the N integers starting at VALUES. */
static void
reverse (int *values, size_t n)
{
  size_t i = 0;
  size_t j = n;

  while (j > i)
    swap (&values[i++], &values[--j]);
}

/* Arranges the N elements in VALUES into the lexicographically
   next greater permutation.  Returns true if successful.
   If VALUES is already the lexicographically greatest
   permutation of its elements (i.e. ordered from greatest to
   smallest), arranges them into the lexicographically least
   permutation (i.e. ordered from smallest to largest) and
   returns false. */
static bool
next_permutation (int *values, size_t n)
{
  if (n > 0)
    {
      size_t i = n - 1;
      while (i != 0)
        {
          i--;
          if (values[i] < values[i + 1])
            {
              size_t j;
              for (j = n - 1; values[i] >= values[j]; j--)
                continue;
              swap (values + i, values + j);
              reverse (values + (i + 1), n - (i + 1));
              return true;
            }
        }

      reverse (values, n);
    }

  return false;
}

/* Returns N!. */
static unsigned int
factorial (unsigned int n)
{
  unsigned int value = 1;
  while (n > 1)
    value *= n--;
  return value;
}

/* Returns the number of permutations of the N values in
   VALUES.  If VALUES contains duplicates, they must be
   adjacent. */
static unsigned int
expected_perms (int *values, size_t n)
{
  size_t i, j;
  unsigned int n_perms;

  n_perms = factorial (n);
  for (i = 0; i < n; i = j)
    {
      for (j = i + 1; j < n; j++)
        if (values[i] != values[j])
          break;
      n_perms /= factorial (j - i);
    }
  return n_perms;
}

/* Tests whether PARTS is a K-part integer composition of N.
   Returns true if so, false otherwise. */
static bool UNUSED
is_k_composition (int n, int k, const int parts[])
{
  int sum;
  int i;

  sum = 0;
  for (i = 0; i < k; i++)
    {
      if (parts[i] < 1 || parts[i] > n)
        return false;
      sum += parts[i];
    }
  return sum == n;
}

/* Advances the K-part integer composition of N stored in PARTS
   to the next lexicographically greater one.
   Returns true if successful, false if the composition was
   already the greatest K-part composition of N (in which case
   PARTS is unaltered). */
static bool
next_k_composition (int n UNUSED, int k, int parts[])
{
  int x, i;

  assert (is_k_composition (n, k, parts));
  if (k == 1)
    return false;

  for (i = k - 1; i > 0; i--)
    if (parts[i] > 1)
      break;
  if (i == 0)
    return false;

  x = parts[i] - 1;
  parts[i] = 1;
  parts[i - 1]++;
  parts[k - 1] = x;

  assert (is_k_composition (n, k, parts));
  return true;
}

/* Advances *K and PARTS to the next integer composition of N.
   Compositions are ordered from shortest to longest and in
   lexicographical order within a given length.
   Before the first call, initialize *K to 0.
   After each successful call, *K contains the length of the
   current composition and the *K elements in PARTS contain its
   parts.
   Returns true if successful, false if the set of compositions
   has been exhausted. */
static bool
next_composition (int n, int *k, int parts[])
{
  if (*k >= 1 && next_k_composition (n, *k, parts))
    return true;
  else if (*k < n)
    {
      int i;
      for (i = 0; i < *k; i++)
        parts[i] = 1;
      parts[i] = n - *k;
      (*k)++;
      return true;
    }
  else
    return false;
}

/* Inserts sequences without duplicates into a heap, and then
   ensures that they appear as the minimum element in the correct
   order as we delete them.  Exhaustively tests every input
   permutation up to 'max_elems' elements. */
static void
test_insert_no_dups_delete_min (void)
{
  const int max_elems = 8;
  int n;

  for (n = 0; n <= max_elems; n++)
    {
      struct heap *h;
      struct element *elements;
      int *values;
      unsigned int n_permutations;
      int i;

      values = xnmalloc (n, sizeof *values);
      elements = xnmalloc (n, sizeof *elements);
      for (i = 0; i < n; i++)
        values[i] = i;

      h = heap_create (compare_elements, &aux_data);
      n_permutations = 0;
      while (n_permutations == 0 || next_permutation (values, n))
        {
          int i;

          for (i = 0; i < n; i++)
            elements[i].x = values[i];

          check (heap_is_empty (h));
          for (i = 0; i < n; i++)
            {
              heap_insert (h, &elements[i].node);
              check (heap_node_to_element (heap_minimum (h))->x
                     == min_int (values, i + 1));
              check (heap_count (h) == i + 1);
            }

          for (i = 0; i < n; i++)
            {
              check (heap_node_to_element (heap_minimum (h))->x == i);
              heap_delete (h, heap_minimum (h));
            }
          check (heap_is_empty (h));
          n_permutations++;
        }
      check (n_permutations == factorial (n));
      heap_destroy (h);
      free (values);
      free (elements);
    }
}

/* Inserts sequences with duplicates into a heap, and then
   ensures that they appear as the minimum element in the correct
   order as we delete them.  Exhaustively tests every input
   permutation up to 'max_elems' elements.

   See Usenet article <87mz4utika.fsf@blp.benpfaff.org> for
   details of the algorithm used here. */
static void
test_insert_with_dups_delete_min (void)
{
  const int max_elems = 7;
  for (int n_elems = 1; n_elems <= max_elems; n_elems++)
    {
      unsigned int n_compositions;
      int *dups;
      int n_uniques;
      int *values;
      int *sorted_values;
      struct element *elements;
      int n = 0;

      dups = xnmalloc (n_elems, sizeof *dups);
      values = xnmalloc (n_elems, sizeof *values);
      sorted_values = xnmalloc (n_elems, sizeof *sorted_values);
      elements = xnmalloc (n_elems, sizeof *elements);

      n_uniques = 0;
      n_compositions = 0;
      while (next_composition (n_elems, &n_uniques, dups))
        {
          struct heap *h;
          int i, j, k;
          unsigned int n_permutations;

          k = 0;
          for (i = 0; i < n_uniques; i++)
            for (j = 0; j < dups[i]; j++)
              {
                values[k] = i;
                sorted_values[k] = i;
                k++;
              }
          check (k == n_elems);

          h = heap_create (compare_elements, &aux_data);
          n_permutations = 0;
          while (n_permutations == 0 || next_permutation (values, n_elems))
            {
              int min = INT_MAX;

              for (i = 0; i < n_elems; i++)
                elements[i].x = values[i];
              n++;

              check (heap_is_empty (h));
              for (i = 0; i < n_elems; i++)
                {
                  heap_insert (h, &elements[i].node);
                  if (values[i] < min)
                    min = values[i];
                  check (heap_node_to_element (heap_minimum (h))->x == min);
                  check (heap_count (h) == i + 1);
                }

              for (i = 0; i < n_elems; i++)
                {
                  struct element *min = heap_node_to_element (heap_minimum (h));
                  check (min->x == sorted_values[i]);
                  heap_delete (h, heap_minimum (h));
                }
              check (heap_is_empty (h));
              n_permutations++;
            }
          check (n_permutations == expected_perms (values, n_elems));
          heap_destroy (h);

          n_compositions++;
        }
      check (n_compositions == 1 << (n_elems - 1));

      free (dups);
      free (values);
      free (sorted_values);
      free (elements);
    }
}

/* Inserts a sequence without duplicates into a heap, then
   deletes them in a different order. */
static void
test_insert_no_dups_delete_random (void)
{
  const int max_elems = 5;
  int n;

  for (n = 0; n <= max_elems; n++)
    {
      struct heap *h;
      struct element *elements;
      int *insert, *delete;
      unsigned int insert_n_perms;
      int i;

      insert = xnmalloc (n, sizeof *insert);
      delete = xnmalloc (n, sizeof *delete);
      elements = xnmalloc (n, sizeof *elements);
      for (i = 0; i < n; i++)
        {
          insert[i] = i;
          delete[i] = i;
          elements[i].x = i;
        }

      h = heap_create (compare_elements, &aux_data);
      insert_n_perms = 0;
      while (insert_n_perms == 0 || next_permutation (insert, n))
        {
          unsigned int delete_n_perms = 0;

          while (delete_n_perms == 0 || next_permutation (delete, n))
            {
              int min;
              int i;

              check (heap_is_empty (h));
              min = INT_MAX;
              for (i = 0; i < n; i++)
                {
                  heap_insert (h, &elements[insert[i]].node);
                  if (insert[i] < min)
                    min = insert[i];
                  check (heap_node_to_element (heap_minimum (h))->x == min);
                  check (heap_count (h) == i + 1);
                }

              for (i = 0; i < n; i++)
                {
                  int new_min = min_int (delete + i + 1, n - i - 1);
                  heap_delete (h, &elements[delete[i]].node);
                  check (heap_count (h) == n - i - 1);
                  if (!heap_is_empty (h))
                    check (heap_node_to_element (heap_minimum (h))->x == new_min);
                }
              check (heap_is_empty (h));
              delete_n_perms++;
            }
          check (delete_n_perms == factorial (n));
          insert_n_perms++;
        }
      check (insert_n_perms == factorial (n));
      heap_destroy (h);
      free (insert);
      free (delete);
      free (elements);
    }
}

/* Inserts a set of values into a heap, then changes them to a
   different random set of values, then removes them in sorted
   order. */
static void
test_inc_dec (void)
{
  const int max_elems = 8;
  int n;

  for (n = 0; n <= max_elems; n++)
    {
      struct heap *h;
      struct element *elements;
      int *insert, *delete;
      unsigned int insert_n_perms;
      int i;

      insert = xnmalloc (n, sizeof *insert);
      delete = xnmalloc (n, sizeof *delete);
      elements = xnmalloc (n, sizeof *elements);
      for (i = 0; i < n; i++)
        insert[i] = i;

      h = heap_create (compare_elements, &aux_data);
      insert_n_perms = 0;
      while (insert_n_perms == 0 || next_permutation (insert, n))
        {
          for (i = 0; i < n; i++)
            elements[i].x = insert[i];

          check (heap_is_empty (h));
          for (i = 0; i < n; i++)
            {
              int new_min = min_int (insert, i + 1);
              heap_insert (h, &elements[i].node);
              check (heap_node_to_element (heap_minimum (h))->x == new_min);
              check (heap_count (h) == i + 1);
            }

          for (i = 0; i < n; i++)
            delete[i] = insert[i];
          for (i = 0; i < n; i++)
            {
              elements[i].x = delete[i] = rand () % (n + 2) - 1;
              heap_changed (h, &elements[i].node);
              check (heap_node_to_element (heap_minimum (h))->x
                     == min_int (delete, n));
            }

          for (i = 0; i < n; i++)
            {
              int new_min = min_int (delete + i + 1, n - i - 1);
              heap_delete (h, &elements[i].node);
              check (heap_count (h) == n - i - 1);
              if (!heap_is_empty (h))
                check (heap_node_to_element (heap_minimum (h))->x == new_min);
            }
          check (heap_is_empty (h));
          insert_n_perms++;
        }
      check (insert_n_perms == factorial (n));
      heap_destroy (h);
      free (insert);
      free (delete);
      free (elements);
    }
}

/* Performs a random sequence of insertions and deletions in a
   heap. */
static void
test_random_insert_delete (void)
{
  const int max_elems = 64;
  const int num_actions = 250000;
  struct heap *h;
  int *values;
  struct element *elements;
  int n;
  int insert_chance;
  int i;

  values = xnmalloc (max_elems, sizeof *values);
  elements = xnmalloc (max_elems, sizeof *elements);
  n = 0;
  insert_chance = 5;

  h = heap_create (compare_elements, &aux_data);
  for (i = 0; i < num_actions; i++)
    {
      enum { INSERT, DELETE } action;

      if (n == 0)
        {
          action = INSERT;
          if (insert_chance < 9)
            insert_chance++;
        }
      else if (n == max_elems)
        {
          action = DELETE;
          if (insert_chance > 0)
            insert_chance--;
        }
      else
        action = rand () % 10 < insert_chance ? INSERT : DELETE;

      if (action == INSERT)
        {
          int new_value;

          new_value = rand () % max_elems;
          values[n] = new_value;
          elements[n].x = new_value;

          heap_insert (h, &elements[n].node);

          n++;
        }
      else if (action == DELETE)
        {
          int del_idx;

          del_idx = rand () % n;
          heap_delete (h, &elements[del_idx].node);

          n--;
          if (del_idx != n)
            {
              values[del_idx] = values[n];
              elements[del_idx] = elements[n];
              heap_moved (h, &elements[del_idx].node);
            }
        }
      else
        abort ();

      check (heap_count (h) == n);
      check (heap_is_empty (h) == (n == 0));
      if (n > 0)
        check (heap_node_to_element (heap_minimum (h))->x
               == min_int (values, n));
    }
  heap_destroy (h);
  free (elements);
  free (values);
}

/* Main program. */

struct test
  {
    const char *name;
    const char *description;
    void (*function) (void);
  };

static const struct test tests[] =
  {
    {
      "insert-no-dups-delete-min",
      "insert (no dups), delete minimum values",
      test_insert_no_dups_delete_min
    },
    {
      "insert-with-dups-delete-min",
      "insert with dups, delete minimum values",
      test_insert_with_dups_delete_min
    },
    {
      "insert-no-dups-delete-random",
      "insert (no dups), delete in random order",
      test_insert_no_dups_delete_random
    },
    {
      "inc-dec",
      "increase and decrease values",
      test_inc_dec
    },
    {
      "random-insert-delete",
      "random insertions and deletions",
      test_random_insert_delete
    }
  };

enum { N_TESTS = sizeof tests / sizeof *tests };

int
main (int argc, char *argv[])
{
  int i;

  if (argc != 2)
    {
      fprintf (stderr, "exactly one argument required; use --help for help\n");
      return EXIT_FAILURE;
    }
  else if (!strcmp (argv[1], "--help"))
    {
      printf ("%s: test heap library\n"
              "usage: %s TEST-NAME\n"
              "where TEST-NAME is one of the following:\n",
              argv[0], argv[0]);
      for (i = 0; i < N_TESTS; i++)
        printf ("  %s\n    %s\n", tests[i].name, tests[i].description);
      return 0;
    }
  else
    {
      for (i = 0; i < N_TESTS; i++)
        if (!strcmp (argv[1], tests[i].name))
          {
            tests[i].function ();
            return 0;
          }

      fprintf (stderr, "unknown test %s; use --help for help\n", argv[1]);
      return EXIT_FAILURE;
    }
}
