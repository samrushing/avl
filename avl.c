/*
 * Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
 */

/* $Id: avl.c,v 2.1 1997/02/21 23:10:05 rushing Exp rushing $ */

/*
 * This is a fairly straightfoward translation of a prototype
 * written in python, 'avl_tree.py'. Read that file first.
 */

#include <stdio.h>
#include <stdlib.h>

#include "avl.h"

avl_node *
new_avl_node (void *		key,
	      avl_node *	parent)
{
  avl_node * node = (avl_node *) malloc (sizeof (avl_node));

  if (!node) {
    return NULL;
  } else {
    node->parent = parent;
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    SET_BALANCE (node, 0);
    SET_RANK (node, 1);
    return node;
  }
}	     

avl_tree *
new_avl_tree (avl_key_compare_fun_type compare_fun,
	      void * compare_arg)
{
  avl_tree * t = (avl_tree *) malloc (sizeof (avl_tree));

  if (!t) {
    return NULL;
  } else {
    avl_node * root = new_avl_node((void *)NULL, (avl_node *) NULL);
    if (!root) {
      return NULL;
    } else {
      t->root = root;
      t->height = 0;
      t->length = 0;
      t->compare_fun = compare_fun;
      t->compare_arg = compare_arg;
      return t;
    }
  }
}
  
void
free_avl_tree_helper (avl_node * node, avl_free_key_fun_type free_key_fun)
{
  if (node->left) {
    free_avl_tree_helper (node->left, free_key_fun);
  }
  free_key_fun (node->key);
  if (node->right) {
    free_avl_tree_helper (node->right, free_key_fun);
  }
  free (node);
}
  
void
free_avl_tree (avl_tree * tree, avl_free_key_fun_type free_key_fun)
{
  if (tree->length) {
    free_avl_tree_helper (tree->root->right, free_key_fun);
  }
  if (tree->root) {
    free (tree->root);
  }
  free (tree);
}

int
insert_by_key (avl_tree * ob,
	       void * key)
{
  if (!(ob->root->right)) {
    avl_node * node = new_avl_node (key, ob->root);
    if (!node) {
      return -1;
    } else {
      ob->root->right = node;
      ob->length = ob->length + 1;
      return 0;
    }
  } else { /* not self.right == None */
    avl_node *t, *p, *s, *q, *r;
    int a;

    t = ob->root;
    s = p = t->right;

    while (1) {
      if (ob->compare_fun (ob->compare_arg, key, p->key) < 1) {
	/* move left */
	SET_RANK (p, (GET_RANK (p) + 1));
	q = p->left;
	if (!q) {
	  /* insert */
	  avl_node * q_node = new_avl_node (key, p);
	  if (!q_node) {
	    return (-1);
	  } else {
	    q = q_node;
	    p->left = q;
	    break;
	  }
	} else if (GET_BALANCE(q)) {
	  t = p;
	  s = q;
	}
	p = q;
      } else {
	/* move right */
	q = p->right;
	if (!q) {
	  /* insert */
	  avl_node * q_node = new_avl_node (key, p);
	  if (!q_node) {
	    return -1;
	  } else {
	    q = q_node;
	    p->right = q;
	    break;
	  }
	} else if (GET_BALANCE(q)) {
	  t = p;
	  s = q;
	}
	p = q;
      }
    }
    
    ob->length = ob->length + 1;
    
    /* adjust balance factors */
    if (ob->compare_fun (ob->compare_arg, key, s->key) < 1) {
      r = p = s->left;
    } else {
      r = p = s->right;
    }
    while (p != q) {
      if (ob->compare_fun (ob->compare_arg, key, p->key) < 1) {
	SET_BALANCE (p, -1);
	p = p->left;
      } else {
	SET_BALANCE (p, +1);
	p = p->right;
      }
    }
    
    /* balancing act */
    
    if (ob->compare_fun (ob->compare_arg, key, s->key) < 1) {
      a = -1;
    } else {
      a = +1;
    }
    
    if (GET_BALANCE (s) == 0) {
      SET_BALANCE (s, a);
      ob->height = ob->height + 1;
      return 0;
    } else if (GET_BALANCE (s) == -a) {
      SET_BALANCE (s, 0);
      return 0;
    } else if (GET_BALANCE(s) == a) {
      if (GET_BALANCE (r) == a) {
	/* single rotation */
	p = r;
	if (a == -1) {
	  s->left = r->right;
	  if (r->right) {
	    r->right->parent = s;
	  }
	  r->right = s;
	  s->parent = r;
	  SET_RANK (s, (GET_RANK (s) - GET_RANK (r)));
	} else {
	  s->right = r->left;
	  if (r->left) {
	    r->left->parent = s;
	  }
	  r->left = s;
	  s->parent = r;
	  SET_RANK (r, (GET_RANK (r) + GET_RANK (s)));
	}
	SET_BALANCE (s, 0);
	SET_BALANCE (r, 0);
      } else if (GET_BALANCE (r) == -a) {
	/* double rotation */
	if (a == -1) {
	  p = r->right;
	  r->right = p->left;
	  if (p->left) {
	    p->left->parent = r;
	  }
	  p->left = r;
	  r->parent = p;
	  s->left = p->right;
	  if (p->right) {
	    p->right->parent = s;
	  }
	  p->right = s;
	  s->parent = p;
	  SET_RANK (p, (GET_RANK (p) + GET_RANK (r)));
	  SET_RANK (s, (GET_RANK (s) - GET_RANK (p)));
	} else {
	  p = r->left;
	  r->left = p->right;
	  if (p->right) {
	    p->right->parent = r;
	  }
	  p->right = r;
	  r->parent = p;
	  s->right = p->left;
	  if (p->left) {
	    p->left->parent = s;
	  }
	  p->left = s;
	  s->parent = p;
	  SET_RANK (r, (GET_RANK (r) - GET_RANK (p)));
	  SET_RANK (p, (GET_RANK (p) + GET_RANK (s)));
	}
	if (GET_BALANCE (p) == a) {
	  SET_BALANCE (s, -a);
	  SET_BALANCE (r, 0);
	} else if (GET_BALANCE (p) == -a) {
	  SET_BALANCE (s, 0);
	  SET_BALANCE (r, a);
	} else {
	  SET_BALANCE (s, 0);
	  SET_BALANCE (r, 0);
	}
	SET_BALANCE (p, 0);
      }
      /* finishing touch */
      if (s == t->right) {
	t->right = p;
      } else {
	t->left = p;
      }
      p->parent = t;
    }
  }
  return 0;
}

int
get_item_by_index (avl_tree * tree,
		   unsigned long index,
		   void ** value_address)
{
  avl_node * p = tree->root->right;
  unsigned long m = index + 1;
  while (1) {
    if (!p) {
      return -1;
    }
    if (m < GET_RANK(p)) {
      p = p->left;
    } else if (m > GET_RANK(p)) {
      m = m - GET_RANK(p);
      p = p->right;
    } else {
      *value_address = p->key;
      return 0;
    }
  }
}
		   
int
get_item_by_key (avl_tree * tree,
		 void * key,
		 void **value_address)
{
  avl_node * x = tree->root->right;
  while (1) {
    int compare_result = tree->compare_fun (tree->compare_arg, key, x->key);
    if (compare_result < 0) {
      if (x->left) {
	x = x->left;
      } else {
	return -1;
      }
    } else if (compare_result > 0) {
      if (x->right) {
	x = x->right;
      } else {
	return -1;
      }
    } else {
      *value_address = x->key;
      return 0;
    }
  }
}

int
remove_by_key (avl_tree * tree,
	       void * key,
	       avl_free_key_fun_type free_key_fun)
{
  avl_node *x, *y, *p, *q, *r, *top, *x_child;
  int shortened_side, shorter;
  
  x = tree->root->right;
  while (1) {
    int compare_result = tree->compare_fun (tree->compare_arg, key, x->key);
    if (compare_result < 0) {
      /* move left
       * We will be deleting from the left, adjust this node's
       * rank accordingly
       */
      SET_RANK (x, (GET_RANK(x) - 1));
      if (x->left) {
	x = x->left;
      } else {
	/* Oops! now we have to undo the rank changes
	 * all the way up the tree
	 */
	while (x != tree->root->right) {
	  if (x->parent->left == x) {
	    SET_RANK(x->parent, (GET_RANK (x->parent) + 1));
	  }
	  x = x->parent;
	}
	return -1;		/* key not in tree */
      }
    } else if (compare_result > 0) {
      /* move right */
      if (x->right) {
	x = x->right;
      } else {
	while (x != tree->root->right) {
	  if (x->parent->left == x) {
	    SET_RANK(x->parent, (GET_RANK (x->parent) + 1));
	  }
	  x = x->parent;
	}
	return -1;		/* key not in tree */
      }
    } else {
      break;
    }
  }

  if (x->left && x->right) {
    void * temp_key;

    /* The complicated case.
     * reduce this to the simple case where we are deleting
     * a node with at most one child.
     */
    
    /* find the immediate predecessor <y> */
    y = x->left;
    while (y->right) {
      y = y->right;
    }
    /* swap <x> with <y> */
    temp_key = x->key;
    x->key = y->key;
    y->key = temp_key;
    /* we know <x>'s left subtree lost a node because that's
     * where we took it from
     */
    SET_RANK (x, (GET_RANK (x) - 1));
    x = y;
  }
  /* now <x> has at most one child
   * scoot this child into the place of <x>
   */
  if (x->left) {
    x_child = x->left;
    x_child->parent = x->parent;
  } else if (x->right) {
    x_child = x->right;
    x_child->parent = x->parent;
  } else {
    x_child = NULL;
  }

  /* now tell <x>'s parent that a grandchild became a child */
  if (x == x->parent->left) {
    x->parent->left = x_child;
    shortened_side = -1;
  } else {
    x->parent->right = x_child;
    shortened_side = +1;
  }

  /*
   * the height of the subtree <x>
   * has now been shortened.  climb back up
   * the tree, rotating when necessary to adjust
   * for the change.
   */
  shorter = 1;
  p = x->parent;
  
  /* return the key and node to storage */
  free_key_fun (x->key);
  free (x);

  while (shorter && p->parent) {
    
    /* case 1: height unchanged */
    if (GET_BALANCE(p) == 0) {
      if (shortened_side == -1) {
	/* we removed a left child, the tree is now heavier
	 * on the right
	 */
	SET_BALANCE (p, +1);
      } else {
	/* we removed a right child, the tree is now heavier
	 * on the left
	 */
	SET_BALANCE (p, -1);
      }
      shorter = 0;
      
    } else if (GET_BALANCE (p) == shortened_side) {
      /* case 2: taller subtree shortened, height reduced */
      SET_BALANCE (p, 0);
    } else {
      /* case 3: shorter subtree shortened */
      top = p->parent;
      /* set <q> to the taller of the two subtrees of <p> */
      if (shortened_side == 1) {
	q = p->left;
      } else {
	q = p->right;
      }
      if (GET_BALANCE (q) == 0) {
	/* case 3a: height unchanged */
	if (shortened_side == -1) {
	  /* single rotate left */
	  q->parent = p->parent;
	  p->right = q->left;
	  if (q->left) {
	    q->left->parent = p;
	  }
	  q->left = p;
	  p->parent = q;
	  SET_RANK (q, (GET_RANK (q) + GET_RANK (p)));
	} else {
	  /* single rotate right */
	  q->parent = p->parent;
	  p->left = q->right;
	  if (q->right) {
	    q->right->parent = p;
	  }
	  q->right = p;
	  p->parent = q;
	  SET_RANK (p, (GET_RANK (p) - GET_RANK (q)));
	}
	shorter = 0;
	SET_BALANCE (q, shortened_side);
	SET_BALANCE (p, (- shortened_side));
      } else if (GET_BALANCE (q) == GET_BALANCE (p)) {
	/* case 3b: height reduced */
	if (shortened_side == -1) {
	  /* single rotate left */
	  q->parent = p->parent;
	  p->right = q->left;
	  if (q->left) {
	    q->left->parent = p;
	  }
	  q->left = p;
	  p->parent = q;
	  SET_RANK (q, (GET_RANK (q) + GET_RANK (p)));
	} else {
	  /* single rotate right */
	  q->parent = p->parent;
	  p->left = q->right;
	  if (q->right) {
	    q->right->parent = p;
	  }
	  q->right = p;
	  p->parent = q;
	  SET_RANK (p, (GET_RANK (p) - GET_RANK (q)));
	}
	shorter = 1;
	SET_BALANCE (q, 0);
	SET_BALANCE (p, 0);
      } else {
	/* case 3c: height reduced, balance factors opposite */
	if (shortened_side == 1) {
	  /* double rotate right */
	  /* first, a left rotation around q */
	  r = q->right;
	  r->parent = p->parent;
	  q->right = r->left;
	  if (r->left) {
	    r->left->parent = q;
	  }
	  r->left = q;
	  q->parent = r;
	  /* now, a right rotation around p */
	  p->left = r->right;
	  if (r->right) {
	    r->right->parent = p;
	  }
	  r->right = p;
	  p->parent = r;
	  SET_RANK (r, (GET_RANK (r) + GET_RANK (q)));
	  SET_RANK (p, (GET_RANK (p) - GET_RANK (r)));
	} else {
	  /* double rotate left */
	  /* first, a right rotation around q */
	  r = q->left;
	  r->parent = p->parent;
	  q->left = r->right;
	  if (r->right) {
	    r->right->parent = q;
	  }
	  r->right = q;
	  q->parent = r;
	  /* now a left rotation around p */
	  p->right = r->left;
	  if (r->left) {
	    r->left->parent = p;
	  }
	  r->left = p;
	  p->parent = r;
	  SET_RANK (q, (GET_RANK (q) - GET_RANK (r)));
	  SET_RANK (r, (GET_RANK (r) + GET_RANK (p)));	    
	}
	if (GET_BALANCE (r) == shortened_side) {
	  SET_BALANCE (q, (- shortened_side));
	  SET_BALANCE (p, 0);
	} else if (GET_BALANCE (r) == (- shortened_side)) {
	  SET_BALANCE (q, 0);
	  SET_BALANCE (p, shortened_side);
	} else {
	  SET_BALANCE (q, 0);
	  SET_BALANCE (p, 0);
	}
	SET_BALANCE (r, 0);
	q = r;
      }
      /* a rotation has caused <q> (or <r> in case 3c) to become
       * the root.  let <p>'s former parent know this.
       */
      if (top->left == p) {
	top->left = q;
      } else {
	top->right = q;
      }
      /* end case 3 */
      p = q;
    }
    x = p;
    p = x->parent;
    /* shortened_side tells us which side we came up from */
    if (x == p->left) {
      shortened_side = -1;
    } else {
      shortened_side = +1;
    }
  } /* end while(shorter) */
  /* when we're all done, we're one shorter */
  tree->length = tree->length - 1;
  return (0);
}

int
iterate_inorder_helper (avl_node * node,
			avl_iter_fun_type iter_fun,
			void * iter_arg)
{
  int result;
  if (node->left) {
    result = iterate_inorder_helper (node->left, iter_fun, iter_arg);
    if (result != 0) {
      return result;
    }
  }
  result = (iter_fun (node->key, iter_arg));
  if (result != 0) {
    return result;
  }
  if (node->right) {
    result = iterate_inorder_helper (node->right, iter_fun, iter_arg);
    if (result != 0) {
      return result;
    }
  }
  return 0;
}

int
iterate_inorder (avl_tree * tree,
		 avl_iter_fun_type iter_fun,
		 void * iter_arg)
{
  int result;

  if (tree->length) {
    result = iterate_inorder_helper (tree->root->right, iter_fun, iter_arg);
    return (result);
  } else {
    return 0;
  }
}

avl_node *
get_predecessor (avl_node * node)
{
  if (node->left) {
    node = node->left;
    while (node->right) {
      node = node->right;
    }
    return node;
  } else {
    avl_node * child = node;
    while (node->parent) {
      node = node->parent;
      if (child == node->right) {
	return node;
      }
      child = node;
    }
    return node;
  }
}

avl_node *
get_successor (avl_node * node)
{
  if (node->right) {
    node = node->right;
    while (node->left) {
      node = node->left;
    }
    return node;
  } else {
    avl_node * child = node;
    while (node->parent) {
      node = node->parent;
      if (child == node->left) {
	return node;
      }
      child = node;
    }
    return node;
  }
}

/* iterate a function over a range of indices, using get_predecessor */

int
iterate_index_range (avl_tree * tree,
		     avl_iter_index_fun_type iter_fun,
		     unsigned long low,
		     unsigned long high,
		     void * iter_arg)
{
  unsigned long m;
  unsigned long num_left;
  avl_node * node;

  if (high > tree->length) {
    return -1;
  }
  num_left = (high - low);
  /* find the <high-1>th node */
  m = high;
  node = tree->root->right;
  while (1) {
    if (m < GET_RANK (node)) {
      node = node->left;
    } else if (m > GET_RANK (node)) {
      m = m - GET_RANK (node);
      node = node->right;
    } else {
      break;
    }
  }
  /* call <iter_fun> on <node>, <get_pred(node)>, ... */
  while (num_left) {
    num_left = num_left - 1;
    if (iter_fun (num_left, node->key, iter_arg) != 0) {
      return -1;
    }
    node = get_predecessor (node);
  }
  return 0;
}

/* If <key> is present in the tree, return that key's node, and set <*index>
 * appropriately.  If not, return NULL, and set <*index> to the position
 * representing the closest preceding value.
 */

avl_node *
get_index_by_key (avl_tree * tree,
		  void * key,
		  unsigned long * index)
{
  avl_node * x = tree->root->right;
  unsigned long m = GET_RANK (x);

  while (1) {
    int compare_result = tree->compare_fun (tree->compare_arg, key, x->key);
    if (compare_result < 0) {
      if (x->left) {
	m = m - GET_RANK(x);
	x = x->left;
	m = m + GET_RANK(x);
      } else {
	*index = m - 2;
	return NULL;
      }
    } else if (compare_result > 0) {
      if (x->right) {
	x = x->right;
	m = m + GET_RANK(x);
      } else {
	*index = m - 1;
	return NULL;
      }
    } else {
      *index = m - 1;
      return x;
    }
  }
}

/* return the (low index, high index) pair that spans the given key */

int
get_span_by_key (avl_tree * tree,
		 void * key,
		 unsigned long * low,
		 unsigned long * high)
{
  unsigned long m, i, j;
  avl_node * node;

  node = get_index_by_key (tree, key, &m);

  /* did we find an exact match?
   * if so, we have to search left and right
   * to find the span, since we know nothing about
   * the arrangement of like keys.
   */
  if (node) {
    avl_node * left, * right;
    /* search left */
    left = get_predecessor (node);
    i = m;
    while ((i > 0) && (tree->compare_fun (tree->compare_arg, key, left->key) == 0)) {
      left = get_predecessor (left);
      i = i - 1;
    }
    /* search right */
    right = get_successor (node);
    j = m;
    while ((j <= tree->length) && (tree->compare_fun (tree->compare_arg, key, right->key) == 0)) {
      right = get_successor (right);
      j = j + 1;
    }
    *low = i;
    *high = j + 1;
    return 0;
  } else {
    *low = *high = m;
  }
  return 0;
}

/* return the (low index, high index) pair that spans the given key */

int
get_span_by_two_keys (avl_tree * tree,
		      void * low_key,
		      void * high_key,
		      unsigned long * low,
		      unsigned long * high)
{
  unsigned long i, j;
  avl_node * low_node, * high_node;
  int order;

  /* we may need to swap them */
  order = tree->compare_fun (tree->compare_arg, low_key, high_key);
  if (order > 0) {
    void * temp = low_key;
    low_key = high_key;
    high_key = temp;
  }

  low_node = get_index_by_key (tree, low_key, &i);
  high_node = get_index_by_key (tree, high_key, &j);

  if (low_node) {
    avl_node * left;
    /* search left */
    left = get_predecessor (low_node);
    while ((i > 0) && (tree->compare_fun (tree->compare_arg, low_key, left->key) == 0)) {
      left = get_predecessor (left);
      i = i - 1;
    }
  }
  if (high_node) {
    avl_node * right;
    /* search right */
    right = get_successor (high_node);
    while ((j <= tree->length) && (tree->compare_fun (tree->compare_arg, high_key, right->key) == 0)) {
      right = get_successor (right);
      j = j + 1;
    }
  } else {
    j = j + 1;
  }

  *low = i;
  *high = j;
  return 0;
}

		   
int
get_item_by_key_most (avl_tree * tree,
		      void * key,
		      void **value_address)
{
  avl_node * x = tree->root->right;
  *value_address = NULL;

  while (1) {
    int compare_result = tree->compare_fun (tree->compare_arg, key, x->key);

    if (compare_result == 0) {
      *value_address = x->key;
      return 0;
    } else if (compare_result < 0) {
      /* the given key is less than the current key */
      if (x->left) {
	x = x->left;
      } else {
	if (*value_address) 
	  return 0;
	else
	  return -1;
      }
    } else {
      /* the given key is more than the current key */
      /* save this value, it might end up being the right one! */
      *value_address = x->key;
      if (x->right) {
	/* there is a bigger entry */
	x = x->right;
      } else {
	if (*value_address) 
	  return 0;
	else
	  return -1;
      }
    }
  }
}

int
get_item_by_key_least (avl_tree * tree,
		       void * key,
		       void **value_address)
{
  avl_node * x = tree->root->right;
  *value_address = NULL;

  while (1) {
    int compare_result = tree->compare_fun (tree->compare_arg, key, x->key);
    if (compare_result == 0) {
      *value_address = x->key;
      return 0;  /* exact match */
    } else if (compare_result < 0) {
      /* the given key is less than the current key */
      /* save this value, it might end up being the right one! */
      *value_address = x->key;
      if (x->left) {
	x = x->left;
      } else {
	if (*value_address)  /* we have found a valid entry */
	  return 0; 
	else
	  return -1;
      }
    } else {
      if (x->right) {
	/* there is a bigger entry */
	x = x->right;
      } else {
	if (*value_address)  /* we have found a valid entry */
	  return 0; 
	else
	  return -1;
      }
    }
  }
}

#define MAX(X, Y)  ((X) > (Y) ? (X) : (Y))

long
verify_balance (avl_node * node)
{
  if (!node) {
    return 0;
  } else {
    long lh = verify_balance (node->left);
    long rh = verify_balance (node->right);
    if ((rh - lh) != GET_BALANCE(node)) {
      fprintf (stderr, "invalid balance at node %d\n", (int) node->key);
      exit(1);
    }
    if (((lh - rh) > 1) || ((lh - rh) < -1)) {
      fprintf (stderr, "unbalanced at node %d\n", (int) node->key);
      exit(1);
    }
    return (1 + MAX (lh, rh));
  }
}
    
void
verify_parent (avl_node * node, avl_node * parent)
{
  if (node->parent != parent) {
    fprintf (stderr, "invalid parent at node %d\n", (int) node->key);
    exit(1);
  }
  if (node->left) {
    verify_parent (node->left, node);
  }
  if (node->right) {
    verify_parent (node->right, node);
  }
}

long
verify_rank (avl_node * node)
{
  if (!node) {
    return 0;
  } else {
    unsigned long num_left=0, num_right=0;
    if (node->left) {
      num_left = verify_rank (node->left);
    }
    if (node->right) {
      num_right = verify_rank (node->right);
    }
    if (GET_RANK (node) != num_left + 1) {
      fprintf (stderr, "invalid rank at node %d\n", (int) node->key);
      exit (1);
    }
    return (num_left + num_right + 1);
  }
}

/* sanity-check the tree */

int
verify (avl_tree * tree)
{
  if (tree->length) {
    verify_balance (tree->root->right);
    verify_parent  (tree->root->right, tree->root);
    verify_rank    (tree->root->right);
  }
  return (0);
}

/*
 * These structures are accumulated on the stack during print_tree
 * and are used to keep track of the width and direction of each
 * branch in the history of a particular line <node>.
 */ 

typedef struct _link_node {
  struct _link_node	* parent;
  char			direction;
  int			width;
} link_node;  

char balance_chars[3] = {'\\', '-', '/'};

int
default_key_printer (char * buffer, void * key)
{
  return sprintf (buffer, "%p", key);
}  

/*
 * When traveling the family tree, a change in direction
 * indicates when to print a connector.  This is kinda crazy,
 * we use the stack to build a linked list, and then travel
 * it backwards using recursion.
 */

void
print_connectors (link_node * link)
{
  if (link->parent) {
    print_connectors (link->parent);
  }
  if (link->parent && (link->parent->direction != link->direction) && (link->parent->parent)) {
    int i;
    fprintf (stdout, "|");
    for (i=0; i < (link->width - 1); i++) {
      fprintf (stdout, " ");
    }
  } else {
    int i;
    for (i=0; i < (link->width); i++) {
      fprintf (stdout, " ");
    }
  }
}

/*
 * The <key_printer> function writes a representation of the
 * key into <buffer> (which is conveniently fixed in size to add
 * the spice of danger).  It should return the size of the
 * representation.
 */

void
print_node (avl_key_printer_fun_type key_printer,
	    avl_node * node,
	    link_node * link)
{
  char buffer[256];
  unsigned int width;
  width = key_printer (buffer, node->key);

  if (node->right) {
    link_node here = {link, 1, width+11};
    print_node (key_printer, node->right, &here);
  }
  print_connectors (link);
  fprintf (stdout, "+-[%c %s %03d]",
	   balance_chars[GET_BALANCE(node)+1],
	   buffer,
	   (int)GET_RANK(node));
  if (node->left || node->right) {
    fprintf (stdout, "-|\n");
  } else {
    fprintf (stdout, "\n");
  }
  if (node->left) {
    link_node here = {link, -1, width+11};
    print_node (key_printer, node->left, &here);
  } 
}  

void
print_tree (avl_tree * tree, avl_key_printer_fun_type key_printer)
{
  link_node top = {NULL, 0, 0};
  if (!key_printer) {
    key_printer = default_key_printer;
  }
  if (tree->length) {
    print_node (key_printer, tree->root->right, &top);
  } else {
    fprintf (stdout, "<empty tree>\n");
  }  
}
