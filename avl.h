/*
 * Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
 */

/* $Id: avl.h,v 1.2 1995/11/16 05:53:16 rushing Exp rushing $ */

typedef struct avl_node_tag {
  void *		key;
  struct avl_node_tag *	left;
  struct avl_node_tag *	right;  
  struct avl_node_tag *	parent;
  /*
   * The lower 2 bits of <rank_and_balance> specify the balance
   * factor: 00==-1, 01==0, 02==+1.
   * The rest of the bits are used for <rank>
   */
  unsigned long		rank_and_balance;
} avl_node;

#define GET_BALANCE(n)	((int)(((n)->rank_and_balance & 3) - 1))

#define GET_RANK(n)	(((n)->rank_and_balance >> 2))

#define SET_BALANCE(n,b) \
  ((n)->rank_and_balance) = \
    (((n)->rank_and_balance & (~3)) | ((int)((b) + 1)))

#define SET_RANK(n,r) \
  ((n)->rank_and_balance) = \
    (((n)->rank_and_balance & 3) | (r << 2))

typedef struct {
  avl_node *	root;
  unsigned long	height;
  unsigned long	length;
} avl_tree;

avl_tree * new_avl_tree (void);
avl_node * new_avl_node (void * key, avl_node * parent);

void free_avl_tree (avl_tree * tree, int(*free_key_fun) (void * key));

int insert_by_key (avl_tree * ob,
		   void * key,
		   int (*compare_fun) (void * a, void * b));

int remove_by_key (avl_tree * tree,
		   void * key,
		   int (*compare_fun) (void * a, void * b),
		   int (*free_key_fun) (void * key));

int get_item_by_index (avl_tree * tree, unsigned long index, void ** value_address);

int get_item_by_key (avl_tree * tree,
		     void * key,
		     int (*compare_fun) (void * a, void * b),
		     void **value_address);

int iterate_inorder (avl_tree * tree,
		     int (*iter_fun)(void *, void *),
		     void * iter_arg);

int iterate_index_range (avl_tree * tree,
			 int (*iter_fun) (unsigned long, void *, void *),
			 unsigned long low,
			 unsigned long high,
			 void * iter_arg);

int verify (avl_tree * tree);

void print_tree (avl_tree * tree, int(*key_printer)(char *, void *));

