#include <stdio.h>
#include <stdlib.h>

#include "avl.h"

int
compare_longs (void * compare_arg, void * a, void * b)
{
  long la, lb;
  la = (long) a;
  lb = (long) b;
  
  if (la < lb) {
    return -1;
  } else if (la > lb) {
    return +1;
  } else {
    return 0;
  }
}

int
long_printer (char * buffer, void * key)
{
  return sprintf (buffer, "%ld", (long) key);
}

int
null_key_free (void * key) {
  return 0;
}

int
main (int argc, char ** argv)
{
  avl_tree * tree;

  tree = avl_new_avl_tree (compare_longs, NULL);

  avl_insert_by_key (tree, (void *) 50); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 45); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 15); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 10); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 75); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 55); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 70); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 80); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 60); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 32); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 20); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 40); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 25); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 22); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 31); avl_print_tree (tree, long_printer); avl_verify (tree);
  avl_insert_by_key (tree, (void *) 30); avl_print_tree (tree, long_printer); avl_verify (tree);
  while (tree->length) {
    int num = 0;
    int any = 0;
    any = fscanf (stdin, "%d", &num);
    if (any < 1) {
      return 0;
    } else {
      fprintf (stdout, "deleting %d\n", num);
      avl_remove_by_key (tree, (void *) num, null_key_free);
      avl_print_tree (tree, long_printer);
      avl_verify (tree);
    }
  }
  return 0;
}
