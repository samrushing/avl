/* -*- Mode:C; tab-width:8 -*- */

/*
 * Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
 */

/* $Id: AVLmodule.c,v 1.8 1995/11/23 02:24:55 rushing Exp rushing $ */

/*
 * IDEA: use an 'index cache' where whenever a lookup is done
 * by index, cache the resultant node, so that if the next lookup
 * is simply <i+1>, we just get_successor() to find it.
 * Of course, invalidate this node cache whenever there's been
 * an insertion or deletion.
 */

#include "Python.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "avl.h"
#ifdef __cplusplus
}
#endif

static PyObject *ErrorObject;

/* ----------------------------------------------------- */

/* Declarations for objects of type AVL tree */

typedef struct {
	PyObject_HEAD
	avl_tree * tree;
} avl_treeobject;

staticforward PyTypeObject Avl_treetype;

#define is_avl_tree_object(v)	((v)->ob_type == &Avl_treetype)

/* forward declarations */
static avl_treeobject * newavl_treeobject(void);
static PyObject * avl_tree_slice (avl_treeobject *self, int ilow, int ihigh);
static avl_treeobject * avl_copy_avl_tree (avl_treeobject * source);


/* ---------------------------------------------------------------- */

static char avl_tree_insert__doc__[] = 
"Insert an item into the tree"
;

static PyObject *
avl_tree_insert(avl_treeobject * self, PyObject * args)
{
  PyObject * val;

  if (!PyArg_ParseTuple(args, "O", &val)) {
    return NULL;
  } else {
    Py_INCREF (val);
    if (insert_by_key (self->tree,
		       (void *) val,
		       (int(*)(void *, void *)) PyObject_Compare) != 0) {
      Py_DECREF (val);
      PyErr_SetString (ErrorObject, "error while inserting item");
      return NULL;
    } else {
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
}

static char avl_tree_remove__doc__[] = 
"Remove an item from the tree"
;

/* remove_by_key's free_key_fun callback */

int
avl_tree_key_free_fun (void * key)
{
  Py_DECREF ((PyObject *) key);
  return 0;
}

static PyObject *
avl_tree_remove (avl_treeobject *self, PyObject *args)
{
  PyObject * val;
  
  if (!PyArg_ParseTuple(args, "O", &val)) {
    return NULL;
  } else {
    Py_INCREF(val);
    if (remove_by_key (self->tree,
		       (void *) val,
		       (int(*)(void *, void *)) PyObject_Compare,
		       avl_tree_key_free_fun) != 0) {
      /* can I legally DECREF val here? */
      PyErr_SetString (ErrorObject, "error while removing item");
      return NULL;
    } else {
      Py_DECREF (val);
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
}

static char avl_tree_lookup__doc__[] =
"Return the first object comparing equal to the <key> argument";

static PyObject *
avl_tree_lookup (avl_treeobject * self, PyObject * args)
{
  PyObject * key_val;
  PyObject * return_value;
  int result;

  if (!PyArg_ParseTuple (args, "O", &key_val)) {
    return NULL;
  } else {
    Py_INCREF (key_val);
    if (self->tree->length) {
      result = get_item_by_key (self->tree,
				(void *) key_val,
				(int(*)(void *, void *)) PyObject_Compare,
				(void **) &return_value);
      if (result == 0) {
	/* success */
	Py_INCREF (return_value);
	Py_DECREF (key_val);
	return (return_value);
      } else {
	Py_DECREF (key_val);
	PyErr_SetObject (PyExc_KeyError, key_val);
	return NULL;
      }
    } else {
      Py_DECREF (key_val);
      PyErr_SetObject (PyExc_KeyError, key_val);
      return NULL;
    }
  }
}

static char avl_tree_span__doc__[] =
"t.span (key) => (low, high)\n"
"Returns a pair of indices (low, high) that span the range of <key>";

static PyObject *
avl_tree_span (avl_treeobject * self, PyObject * args)
{
  PyObject * low_key, * high_key = NULL;
  unsigned long low, high;
  int result;

  if (!PyArg_ParseTuple (args, "O|O", &low_key, &high_key)) {
    return NULL;
  } else {
    if (self->tree->length) {
      /* only one key was specified */
      if (!high_key) {
	Py_INCREF (low_key);
	result = get_span_by_key (self->tree,
				  (void *) low_key,
				  (int(*)(void *, void *)) PyObject_Compare,
				  &low,
				  &high);
	Py_DECREF (low_key);
	if (result == 0) {
	  /* success */
	  return Py_BuildValue ("(ii)", (int) low, (int) high);
	} else {
	  PyErr_SetString (ErrorObject, "error while locating key span");
	  return NULL;
	}
      } else {
	/* they specified two keys */
	Py_INCREF (low_key);
	Py_INCREF (high_key);
	result = get_span_by_two_keys (self->tree,
				       (void *) low_key,
				       (void *) high_key,
				       (int(*)(void *, void*)) PyObject_Compare,
				       &low,
				       &high);
	Py_DECREF (low_key);
	Py_DECREF (high_key);
	if (result == 0) {
	  /* success */
	  return Py_BuildValue ("(ii)", (int) low, (int) high);
	} else {
	  PyErr_SetString (ErrorObject, "error while locate key span");
	  return NULL;
	}
      }
    } else {
      return Py_BuildValue ("(ii)", 0, 0);
    }
  }
}

static char avl_tree_has_key__doc__[] =
"Does the tree contain an item comparing equal to <key>?";

static PyObject *
avl_tree_has_key (avl_treeobject * self, PyObject * args)
{
  PyObject * key_val;
  PyObject * return_value;
  int result;

  if (!PyArg_ParseTuple (args, "O", &key_val)) {
    return NULL;
  } else {
    Py_INCREF (key_val);
    if (self->tree->length) {
      result = get_item_by_key (self->tree,
				(void *) key_val,
				(int(*)(void *, void *)) PyObject_Compare,
				(void **) &return_value);
      if (result == 0) {
	/* success */
	Py_DECREF (key_val);
	return (Py_BuildValue ("i", 1));
      } else {
	Py_DECREF (key_val);
	return Py_BuildValue ("i", 0);
      }
    } else {
      Py_DECREF (key_val);
      return (Py_BuildValue ("i", 0));
    }
  }
}

#ifdef DEBUG_AVL
static char avl_tree_verify__doc__[] =
"Verify the internal structure of the AVL tree (testing only)";

static PyObject *
avl_tree_verify (avl_treeobject * self, PyObject * args)
{
  return (Py_BuildValue ("i", verify (self->tree)));
}

static char avl_tree_print_internal_structure__doc__[] =
"Print the internal structure of the AVL tree (testing only)";

int
avl_tree_key_printer (char * buffer, void * key)
{
  PyObject * repr_string;
  int length;

  repr_string = (PyObject *) PyObject_Repr ((PyObject *) key);
  if (repr_string) {
    strcpy (buffer, PyString_AsString (repr_string));
    length = PyString_Size (repr_string);
    Py_DECREF (repr_string);
    return length;
  } else {
    strcpy (buffer, "<couldn't print node>");
    return 21;
  }
}

static PyObject *
avl_tree_print_internal_structure (avl_treeobject * self, PyObject * args)
{
  print_tree (self->tree, avl_tree_key_printer);
  Py_INCREF (Py_None);
  return Py_None;
}

#endif

static char avl_tree_iterate__doc__[] = 
"Iterate <callback_function> over all the tree's items"
;

int
avl_tree_iterate_do_callback (void * item, void * callback_function)
{
  PyObject * arglist;
  PyObject * result;

  arglist = Py_BuildValue ("(O)", (PyObject *) item);
  if (!arglist) {
    return -1;
  }
  result = PyEval_CallObject ((PyObject *) callback_function, arglist);
  Py_DECREF (arglist);
  if (!result) {
    return -1;
  } else {
    Py_DECREF (result);
    return 0;
  }
}

/*
 * I don't think this is worth it - my testing shows that
 * 'for x in tree' is faster than using 'tree.iterate(callback)'  Why?
 */

static PyObject *
avl_tree_iterate(avl_treeobject *self, PyObject *args)
{
  PyObject * callback_function;
  int result;

  if (!PyArg_ParseTuple(args, "O!", &PyFunction_Type, &callback_function)) {
    return NULL;
  } else {
    result = iterate_inorder (self->tree,
			      (int (*)(void *, void *)) avl_tree_iterate_do_callback,
			      (void *) callback_function);
    if (result != 0) {
      return NULL;
    } else {
      Py_INCREF (Py_None);
      return Py_None;
    }
  }
}


/*
 * This is an inorder tree-building: with careful synchronization
 * of the divide-and-conquer recursion and the source tree iteration
 * we can ask for the next key at the exact moment when we need it.
 * neat, huh!?
 * since we know that get_successor() is O(2) (see comments in avl_tree.py)
 * this algorithm is O(slice_size).
 */

/*
 * Todo: pull the (low == high) test up one level, this will
 * remove half the function calls.
 */

int
tree_from_tree (avl_node ** node,
		avl_node * parent,
		avl_node ** address,
		unsigned int low,
		unsigned int high)
{
  unsigned int midway = ((high - low) / 2) + low;
  if (low == high) {
    *address = NULL;
    return 1;
  } else {
    PyObject * item;
    avl_node * new_node;
    int left_height, right_height;

    new_node = new_avl_node ((void *) 0, parent);
    if (!new_node) {
      return -1;
    }
    *address = new_node;
    SET_RANK (new_node, (midway-low)+1);
    left_height = tree_from_tree (node, new_node, &(new_node->left), low, midway);
    if (left_height < 0) {
      return -1;
    }

    item = (PyObject *) (*node)->key;
    Py_INCREF (item);
    new_node->key = (void *) item;
    *node = get_successor (*node);
    
    right_height = tree_from_tree (node, new_node, &(new_node->right), midway+1, high);
    if (right_height  < 0) {
      return -1;
    }
    if (left_height > right_height) {
      SET_BALANCE (new_node, -1);
      return left_height + 1;
    } else if (right_height > left_height) {
      SET_BALANCE (new_node, +1);
      return right_height + 1;
    } else {
      SET_BALANCE (new_node, 0);
      return left_height + 1;
    }
  }
}

static char avl_tree_from_tree__doc__[] = "return a slice of the tree as a new tree";

static PyObject *
avl_tree_from_tree (avl_treeobject * self,
		    PyObject * args)
{
  unsigned int low=0, high=self->tree->length - 1;

  if (!PyArg_ParseTuple (args, "|ii", &low, &high)) {
    return NULL;
  }

  return avl_tree_slice (self, low, high);
}

int
slice_callback (unsigned long index, void * key, void * arg)
{
  /* arg is a list template */
  Py_INCREF ((PyObject *) key);
  return (PyList_SetItem ((PyObject *)arg, (int) index, (PyObject *) key));
}

static char avl_tree_slice_as_list__doc__[] = "return a slice of the tree as a list";

static PyObject *
avl_tree_slice_as_list (avl_treeobject * self, PyObject * args)
{
  PyObject * list;
  unsigned int ilow, ihigh;
  int result;

  if (!PyArg_ParseTuple (args, "|ii", &ilow, &ihigh)) {
    return NULL;
  }

  if ((ilow < 0) || (ihigh > self->tree->length)) {
    PyErr_SetString (PyExc_IndexError, "tree index out of range");
    return NULL;
  }

  if (!(list = PyList_New (ihigh - ilow))) {
    return NULL;
  }
  if (ihigh - ilow) {
    result = iterate_index_range (self->tree,
				  slice_callback,
				  (unsigned long) ilow,
				  (unsigned long) ihigh,
				  (void *) list);
    if (result != 0) {
      PyErr_SetString (ErrorObject, "error while accessing slice");
      return NULL;
    }
  }
  return list;
}

static struct PyMethodDef avl_tree_methods[] = {
  {"insert",	(PyCFunction)avl_tree_insert,		1,	avl_tree_insert__doc__},
  {"remove",	(PyCFunction)avl_tree_remove,	1,	avl_tree_remove__doc__},
  {"iterate",	(PyCFunction)avl_tree_iterate,	1,	avl_tree_iterate__doc__},
  {"lookup",	(PyCFunction)avl_tree_lookup,	1,	avl_tree_lookup__doc__},
  {"has_key",	(PyCFunction)avl_tree_has_key,	1,	avl_tree_has_key__doc__},
  {"slice_as_list",	(PyCFunction)avl_tree_slice_as_list,1,	avl_tree_slice_as_list__doc__},
  {"span",	(PyCFunction)avl_tree_span,	1,	avl_tree_span__doc__},
  {"slice_as_tree",	(PyCFunction)avl_tree_from_tree,1,	avl_tree_from_tree__doc__},
#ifdef DEBUG_AVL
  {"verify",	(PyCFunction)avl_tree_verify,	1,	avl_tree_verify__doc__},
  {"print_internal_structure",	(PyCFunction)avl_tree_print_internal_structure,	1,	avl_tree_print_internal_structure__doc__},
#endif
  {NULL,		NULL}		/* sentinel */
};

/* ---------- */


static avl_treeobject *
newavl_treeobject(void)
{
  avl_treeobject *self;
	
  self = PyObject_NEW(avl_treeobject, &Avl_treetype);
  if (self == NULL) {
    return NULL;
  }
  self->tree = new_avl_tree();
  if (!self->tree) {
    PyMem_DEL (self);
    return NULL;
  }
  return self;
}

static void
avl_tree_dealloc(avl_treeobject *self)
{
  free_avl_tree (self->tree, avl_tree_key_free_fun);
  PyMem_DEL(self);
}

int
avl_tree_print_helper (avl_node * node,
		       long * index,
		       FILE * fp)
{
  if (node->left) {
    if (avl_tree_print_helper (node->left, index, fp) != 0) {
      return -1;
    }
  }
  if ((*index)) {
    fprintf (fp, ", ");
  }
  if ((PyObject_Print ((PyObject*) node->key, fp, 0)) != 0) {
    return -1;
  } else {
    (*index)++;
  }
  if (node->right) {
    if (avl_tree_print_helper (node->right, index, fp) != 0) {
      return -1;
    }
  }
  return 0;
}  

static int
avl_tree_print(avl_treeobject *self, FILE *fp, int flags)
{
  if (!self->tree->length) {
    fprintf (fp, "[]");
  } else {
    long index = 0;

    fprintf (fp, "[");
    if ((avl_tree_print_helper (self->tree->root->right, & index, fp)) != 0) {
      return -1;
    }
    fprintf (fp, "]");
  }
  return 0;
}


static PyObject *
avl_tree_getattr(avl_treeobject *self, char *name)
{
	/* XXXX Add your own getattr code here */
	return Py_FindMethod(avl_tree_methods, (PyObject *)self, name);
}

static int
avl_tree_setattr(avl_treeobject *self, char *name, PyObject *v)
{
	/* XXXX Add your own setattr code here */
	return -1;
}

static PyObject *
avl_tree_repr (avl_treeobject *self)
{
  PyObject * s;
  PyObject * comma;
  avl_node * node;
  unsigned long i;

  if (self->tree->length) {
    s = PyString_FromString ("[");
    comma = PyString_FromString (", ");
    /* find leftmost node */
    node = self->tree->root->right;
    while (node->left) {
      node = node->left;
    }
    for (i=0; i < self->tree->length; i++) {
      if (i > 0) {
	PyString_Concat (&s, comma);
      }
      PyString_ConcatAndDel (&s, PyObject_Repr ((PyObject *)node->key));
      node = get_successor (node);
    }
    Py_XDECREF (comma);
    PyString_ConcatAndDel (&s, PyString_FromString ("]"));
    return s;
  } else {
    return PyString_FromString ("[]");
  }
}

/* Code to handle accessing AVL tree objects as sequence objects */

static int
avl_tree_length (avl_treeobject *self)
{
  return (int) self->tree->length;
}

/* todo: support tree+list */

static PyObject *
avl_tree_concat (avl_treeobject *self, avl_treeobject *bb)
{
  avl_treeobject * self_copy;
  unsigned long bb_node_counter = bb->tree->length;
  
  self_copy = avl_copy_avl_tree ((avl_treeobject *) self);
  if (!self_copy) {
    return NULL;
  }

  if (bb_node_counter) {
    avl_node * bb_node;

    /* find the leftmost node of bb */
    bb_node = bb->tree->root->right;
    while (bb_node->left) {
      bb_node = bb_node->left;
    }
    
    /*
     * iterate over the items in bb, inserting
     * them into self_copy 
     */
    while (bb_node_counter--) {
      Py_INCREF ((PyObject *)bb_node->key);
      if (insert_by_key (self_copy->tree,
			 (void *) bb_node->key,
			 (int(*)(void *, void *)) PyObject_Compare) != 0) {
	avl_tree_dealloc (self_copy);
	return NULL;
      }
      bb_node = get_successor (bb_node);
    }
    self_copy->tree->length += bb->tree->length;
  }
  return (PyObject *) self_copy;
}

static PyObject *
avl_tree_repeat(avl_treeobject * self, int n)
{
  PyErr_SetString (PyExc_TypeError,"repeat operation undefined");
  return NULL;
}

static PyObject *
avl_tree_item (avl_treeobject *self, int i)
{
  void * value;
  unsigned long index;

  /* interpret a negative index normally */
  if (i < 0) {
    index = self->tree->length + i;
  } else {
    index = i;
  }

  /* range-check the index */
  if (index >= self->tree->length) {
    PyErr_SetString (PyExc_IndexError, "tree index out of range");
    return NULL;
  } else {
    /* get the python object, and store in <value> */
    if (get_item_by_index (self->tree, index, &value) != 0) {
      PyErr_SetString (ErrorObject, "error while accessing item");
      return NULL;
    } else {
      Py_INCREF ((PyObject *)value);
      return (PyObject *)value;
    }
  }
}

static PyObject *
avl_tree_slice (avl_treeobject *self, int ilow, int ihigh)
{
  avl_treeobject * new_tree;
  unsigned long m;
  avl_node * node;

  /*
   * Python takes care of negative indices, but someone can still
   * ask for the -40th element of a 10-element tree
   */

  if ((ilow < 0) || ((unsigned long) ihigh > self->tree->length)) {
    PyErr_SetString (PyExc_IndexError, "tree index out of range");
    return NULL;
  }

  new_tree = newavl_treeobject();
  if (!new_tree) {
    return NULL;
  }

  /* locate node <ilow> */
  node = self->tree->root->right;
  m = (unsigned long) ilow + 1;
  while (1) {
    if (m < GET_RANK(node)) {
      node = node->left;
    } else if (m > GET_RANK(node)) {
      m = m - GET_RANK(node);
      node = node->right;
    } else {
      break;
    }
  }

  if (tree_from_tree (&node,
		      new_tree->tree->root,
		      &(new_tree->tree->root->right),
		      0,
		      ihigh - ilow) < 0) {
    PyErr_SetString (ErrorObject, "something went amiss whilst building the tree!");
    return NULL;
  }
  new_tree->tree->length = ihigh - ilow;
  return (PyObject *) new_tree;
}

static PySequenceMethods avl_tree_as_sequence = {
	(inquiry)avl_tree_length,		/*sq_length*/
	(binaryfunc)avl_tree_concat,		/*sq_concat*/
	(intargfunc)avl_tree_repeat,		/*sq_repeat*/
	(intargfunc)avl_tree_item,		/*sq_item*/
	(intintargfunc)avl_tree_slice,		/*sq_slice*/
	(intobjargproc)0,			/*sq_ass_item*/
	(intintobjargproc)0,			/*sq_ass_slice*/
};

/* -------------------------------------------------------------- */

static char Avl_treetype__doc__[] = 
"A dual-personality object, can act like a sequence and a dictionary.  Implemented with an AVL tree"
;

static PyTypeObject Avl_treetype = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/*ob_size*/
	"AVL tree",			/*tp_name*/
	sizeof(avl_treeobject),		/*tp_basicsize*/
	0,				/*tp_itemsize*/
	/* methods */
	(destructor)avl_tree_dealloc,	/*tp_dealloc*/
	(printfunc)avl_tree_print,	/*tp_print*/
	(getattrfunc)avl_tree_getattr,	/*tp_getattr*/
	(setattrfunc)avl_tree_setattr,	/*tp_setattr*/
	(cmpfunc)0,			/*tp_compare*/
	(reprfunc)avl_tree_repr,	/*tp_repr*/
	0,				/*tp_as_number*/
	&avl_tree_as_sequence,		/*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	(hashfunc)0,		/*tp_hash*/
	(ternaryfunc)0,		/*tp_call*/
	(reprfunc)0,		/*tp_str*/

	/* Space for future expansion */
	0L,0L,0L,0L,
	Avl_treetype__doc__ /* Documentation string */
};

/* End of code for AVL tree objects */
/* -------------------------------------------------------- */

int
avl_copy_avl_node (avl_node * source_node,
		   avl_node * dest_parent,
		   avl_node ** dest_node)
{
  avl_node * new_node = new_avl_node(source_node->key, dest_parent);

  if (!new_node) {
    return -1;
  } else {
    Py_INCREF ((PyObject *) new_node->key);
    new_node->rank_and_balance = source_node->rank_and_balance;
    if (source_node->left) {
      if (avl_copy_avl_node (source_node->left,
			     new_node,
			     &(new_node->left)) != 0) {
	return -1;
      }
    } else {
      new_node->left = NULL;
    }
    if (source_node->right) {
      if (avl_copy_avl_node (source_node->right,
			     new_node,
			     &(new_node->right)) != 0) {
	return -1;
      }
    } else {
      new_node->right = NULL;
    }
    *dest_node = new_node;
    return 0;
  }
}

static avl_treeobject *
avl_copy_avl_tree (avl_treeobject * source)
{
  avl_treeobject * dest;
  
  if (!(dest = newavl_treeobject())) {
    return NULL;
  } else if (source->tree->length) {
    if (avl_copy_avl_node (source->tree->root->right,
			   dest->tree->root,
			   &(dest->tree->root->right)) != 0) {
      avl_tree_dealloc(dest);
      return NULL;
    } else {
      dest->tree->length = source->tree->length;
      return dest;
    }
  } else {
    return dest;
  }
}

/*
 * not so cleanly separated from the avl 'library',
 * since it builds the tree directly.
 */

/*
 * Todo: pull the (low == high) test up one level, this will
 * remove half the function calls.
 */

int
tree_from_list (PyObject * list,
		avl_node * parent,
		avl_node ** address,
		unsigned int low,
		unsigned int high)
{
  unsigned int midway = ((high - low) / 2) + low;
  if (low == high) {
    *address = NULL;
    return 1;
  } else {
    PyObject * item = PyList_GetItem (list, midway);
    avl_node * new_node;
    int left_height, right_height;

    new_node = new_avl_node ((void *) item, parent);
    if (!new_node) {
      return -1;
    }
    *address = new_node;
    Py_INCREF (item);
    SET_RANK (new_node, (midway-low)+1);
    left_height = tree_from_list (list, new_node, &(new_node->left), low, midway);
    if (left_height < 0) {
      return -1;
    }
    right_height = tree_from_list (list, new_node, &(new_node->right), midway+1, high);
    if (right_height  < 0) {
      return -1;
    }
    if (left_height > right_height) {
      SET_BALANCE (new_node, -1);
      return left_height + 1;
    } else if (right_height > left_height) {
      SET_BALANCE (new_node, +1);
      return right_height + 1;
    } else {
      SET_BALANCE (new_node, 0);
      return left_height + 1;
    }
  }
}

static char avl_new_avl_from_list__doc__[] =
"construct a new avl tree from a list (sorts the list as a side-effect!)";

static PyObject *
avl_new_avl_from_list (PyObject * self, /* not used */
		       PyObject * args)
{
  PyObject * list;
  avl_treeobject * tree;
  unsigned int length;

  if (!PyArg_ParseTuple (args, "O!", &PyList_Type, &list)) {
    return NULL;
  }
  /* sort the list */
  if (PyList_Sort (list) != 0) {
    return NULL;
  }

  tree = newavl_treeobject();
  if (!tree) {
    return NULL;
  }
  length = PyList_Size(list);
  if (tree_from_list (list,
		      tree->tree->root,
		      &(tree->tree->root->right),
		      0,
		      length) < 0) {
    PyErr_SetString (ErrorObject, "something went amiss whilst building the tree!");
    return NULL;
  }
  tree->tree->length = length;
  return (PyObject *) tree;
}

static char avl_newavl__doc__[] =
"With no arguments, returns a new and empty tree.\n"
"Given a list, it will return a new tree containing the elements\n"
"  of the list, and will sort the list as a side-effect\n"
"Given a tree, will return a copy of the original tree\n"
;

static PyObject *
avl_newavl(PyObject * self,	/* Not used */
	   PyObject * args)
{
  PyObject * arg = NULL;
  if (!PyArg_ParseTuple(args, "|O", &arg)) {
    return NULL;
  }
  if (!arg) {
    return (PyObject *) newavl_treeobject();
  } else if (PyList_Check(arg)) {
    return avl_new_avl_from_list ((PyObject *) NULL, args);
  } else if (is_avl_tree_object (arg)) {
    return (PyObject *) avl_copy_avl_tree ((avl_treeobject *) arg);
  } else {
    PyErr_SetString (ErrorObject, "newavl() expects a list, a tree, or no arguments");
    return NULL;
  }
}

/* List of methods defined in the module */

static struct PyMethodDef avl_methods[] = {
	{"newavl",	avl_newavl,	1,	avl_newavl__doc__},
	{NULL,		NULL}		/* sentinel */
};

/* Initialization function for the module (*must* be called initavl) */

static char avl_module_documentation[] = 
"Implements a dual-personality object (that can act like a sequence _and_ a dictionary) with AVL trees."
;

void
initavl(void)
{
	PyObject *m, *d;

	/* Create the module and add the functions */
	m = Py_InitModule4("avl", avl_methods,
		avl_module_documentation,
		(PyObject*)NULL,PYTHON_API_VERSION);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);
	ErrorObject = PyString_FromString("avl.error");
	PyDict_SetItemString(d, "error", ErrorObject);

	/* XXXX Add constants here */
	
	/* Check for errors */
	if (PyErr_Occurred())
		Py_FatalError("can't initialize module avl");
}
