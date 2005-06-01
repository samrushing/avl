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

  tree = new_avl_tree (compare_longs, NULL);

  insert_by_key (tree, (void *) 50); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 45); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 15); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 10); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 75); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 55); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 70); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 80); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 60); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 32); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 20); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 40); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 25); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 22); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 31); print_tree (tree, long_printer); verify (tree);
  insert_by_key (tree, (void *) 30); print_tree (tree, long_printer); verify (tree);
  while (tree->length) {
    int num = 0;

    fscanf (stdin, "%d", &num);
    fprintf (stdout, "deleting %d\n", num);
    remove_by_key (tree, (void *) num, null_key_free);
    print_tree (tree, long_printer);
    verify (tree);
  }
  return 0;
}
