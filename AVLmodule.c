/*
 * Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
 */

/* $Id: $ */

#include "Python.h"

#include "avl.h"

static PyObject *ErrorObject;

/* ----------------------------------------------------- */

/* Declarations for objects of type AVL tree */

typedef struct {
	PyObject_HEAD
	avl_tree * tree;
} avl_treeobject;

staticforward PyTypeObject Avl_treetype;


/* ---------------------------------------------------------------- */

static char avl_tree_insert__doc__[] = 
"Insert an item into the tree"
;

static PyObject *
avl_tree_insert(self, args)
	avl_treeobject *self;
	PyObject *args;
{
  PyObject * val;

  if (!PyArg_ParseTuple(args, "O", &val)) {
    return NULL;
  } else {
    if (insert_by_key (self->tree,
		       (void *) val,
		       (int(*)(void *, void *)) PyObject_Compare) != 0) {
      PyErr_SetString (ErrorObject, "error while inserting item");
      return NULL;
    } else {
      Py_INCREF(val);
      Py_INCREF(Py_None);
      return Py_None;
    }
  }
}

static char avl_tree_remove__doc__[] = 
"remove an item from the AVL tree"
;

/* remove_by_key's free_key_fun callback */

int
avl_tree_key_free_fun (void * key)
{
  Py_DECREF ((PyObject *) key);
  return 0;
}

static PyObject *
avl_tree_remove(self, args)
	avl_treeobject *self;
	PyObject *args;
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
"return the first object comparing equal to the <key> argument";

static PyObject *
avl_tree_lookup (self, args)
     avl_treeobject * self;
     PyObject * args;
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

#ifdef DEBUG_AVL
static char avl_tree_verify__doc__[] =
"Verify the internal structure of the AVL tree (testing only)";

static PyObject *
avl_tree_verify (self, args)
     avl_treeobject * self;
     PyObject * args;
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
avl_tree_print_internal_structure (self, args)
     avl_treeobject * self;
     PyObject * args;
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
avl_tree_iterate(self, args)
	avl_treeobject *self;
	PyObject *args;
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

static struct PyMethodDef avl_tree_methods[] = {
  {"insert",	avl_tree_insert,		1,	avl_tree_insert__doc__},
  {"remove",	avl_tree_remove,	1,	avl_tree_remove__doc__},
  {"iterate",	avl_tree_iterate,	1,	avl_tree_iterate__doc__},
  {"lookup",	avl_tree_lookup,	1,	avl_tree_lookup__doc__},
#ifdef DEBUG_AVL
  {"verify",	avl_tree_verify,	1,	avl_tree_verify__doc__},
  {"print_internal_structure",	avl_tree_print_internal_structure,	1,	avl_tree_print_internal_structure__doc__},
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
avl_tree_dealloc(self)
	avl_treeobject *self;
{
  free_avl_tree (self->tree, avl_tree_key_free_fun);
  PyMem_DEL(self);
}

int
avl_tree_print_helper (avl_node * node,
		       long * index,
		       FILE * fp)
{
  int result;
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
avl_tree_print(self, fp, flags)
	avl_treeobject *self;
	FILE *fp;
	int flags;
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
avl_tree_getattr(self, name)
	avl_treeobject *self;
	char *name;
{
	/* XXXX Add your own getattr code here */
	return Py_FindMethod(avl_tree_methods, (PyObject *)self, name);
}

static int
avl_tree_setattr(self, name, v)
	avl_treeobject *self;
	char *name;
	PyObject *v;
{
	/* XXXX Add your own setattr code here */
	return -1;
}

void
avl_tree_inorder_list_helper (PyObject * list,
			      long * index,
			      avl_node * node)
{
  if (node->left) {
    avl_tree_inorder_list_helper (list, index, node->left);
  }
  PyList_SetItem (list, (*index)++, node->key);
  if (node->right) {
    avl_tree_inorder_list_helper (list, index, node->right);    
  }
}

static PyObject *
avl_tree_inorder_list (avl_treeobject * self)
{
  PyObject * list;
  long index = 0;

  list = PyList_New (self->tree->length);
  if (!list) {
    return NULL;
  }
  if (self->tree->length) {
    avl_tree_inorder_list_helper (list, &index, self->tree->root->right);
  }
  return list;
}

static PyObject *
avl_tree_repr(self)
     avl_treeobject *self;
{
  PyObject *s = NULL;
  PyObject *inorder_list;
	
  inorder_list = avl_tree_inorder_list (self);
  if (!inorder_list) {
    return NULL;
  } else {
    s = (PyObject *) PyObject_Repr (inorder_list);
    Py_DECREF (inorder_list);
    if (!s) {
      return NULL;
    } else {
      return s;
    }
  }
}

/* Code to handle accessing AVL tree objects as sequence objects */

static int
avl_tree_length(self)
	avl_treeobject *self;
{
  return (int) self->tree->length;
}

static PyObject *
avl_tree_item(self, i)
	avl_treeobject *self;
	int i;
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

int
slice_callback (unsigned long index, void * key, void * arg)
{
  /* arg is a list template */
  Py_INCREF ((PyObject *) key);
  return (PyList_SetItem ((PyObject *)arg, (int) index, (PyObject *) key));
}

static PyObject *
avl_tree_slice(self, ilow, ihigh)
	avl_treeobject *self;
	int ilow, ihigh;
{
  PyObject * list;
  int result;

  /*
   * Python takes care of negative indices, but someone can still
   * ask for the -40th element of a 10-element tree
   */

  if ((ilow < 0) || (ihigh > self->tree->length)) {
    PyErr_SetString (PyExc_IndexError, "tree index out of range");
    return NULL;
  }
  if (!(list = PyList_New (ihigh - ilow))) {
    return NULL;
  }
  result = iterate_index_range (self->tree,
				slice_callback,
				(unsigned long) ilow,
				(unsigned long) ihigh,
				(void *) list);
  if (result != 0) {
    PyErr_SetString (ErrorObject, "error while accessing slice");
    return NULL;
  } else {
    return list;
  }
}

static PySequenceMethods avl_tree_as_sequence = {
	(inquiry)avl_tree_length,		/*sq_length*/
	(binaryfunc)0,				/*sq_concat*/
	(intargfunc)0,				/*sq_repeat*/
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
	(printfunc)avl_tree_print,		/*tp_print*/
	(getattrfunc)avl_tree_getattr,	/*tp_getattr*/
	(setattrfunc)avl_tree_setattr,	/*tp_setattr*/
	(cmpfunc)0,		/*tp_compare*/
	(reprfunc)avl_tree_repr,		/*tp_repr*/
	0,			/*tp_as_number*/
	&avl_tree_as_sequence,		/*tp_as_sequence*/
	0,		/*tp_as_mapping*/
	(hashfunc)0,		/*tp_hash*/
	(binaryfunc)0,		/*tp_call*/
	(reprfunc)0,		/*tp_str*/

	/* Space for future expansion */
	0L,0L,0L,0L,
	Avl_treetype__doc__ /* Documentation string */
};

/* End of code for AVL tree objects */
/* -------------------------------------------------------- */


static char avl_newavl__doc__[] =
"Create a new (and empty) tree"
;

static PyObject *
avl_newavl(self, args)
     PyObject *self;	/* Not used */
     PyObject *args;
{
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  return (PyObject *) newavl_treeobject();
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
initavl()
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