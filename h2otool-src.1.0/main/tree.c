#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "tree.h"

struct node_t *allocnode(enum nodetype_t type, const char *name) {
	struct node_t *node = (struct node_t *)calloc(1, sizeof(struct node_t));
	node->type = type;
	node->name = strdup(name);
	return node;
}

struct node_t *allocdir(const char *name) {
	struct node_t *node = allocnode(NODETYPE_DIRECTORY, name);
	return node;
}

struct node_t *allocfile(const char *name, struct entry_t *entry) {
	struct node_t *node = allocnode(NODETYPE_FILE, name);
	node->file = entry;
	return node;
}

void freenode(struct node_t *node) {
	
	struct node_t *n = node->firstchild;
	while (n != NULL) {
		freenode(n);
		n = n->nextsibling;
	}
	if (node->name != NULL)
		free(node->name);
	if (node->file != NULL)
		free(node->file);
	free(node);
}

void attachnode(struct node_t *parent, struct node_t *child)  {

	child->parent = parent;

	struct node_t *node = parent;

	if (node->firstchild == NULL) {
		// This is its first child
		node->firstchild = child;
	} else {
		// Iterate the previous childs and add a link to the last one
		node = node->firstchild;
		while (node->nextsibling != NULL) {
			node = node->nextsibling;
		}
		node->nextsibling = child;
	}
}

void printpath(struct node_t *node) {
	if (node->parent == NULL)
		return;
	if (node->parent->parent != NULL) {
		printpath(node->parent);
		printf("\\");
	}
	printf("%s", node->name);
}

static int _pathlength(struct node_t *node, int len) {
	if (node->parent == NULL)
		return len;
	if (node->parent->parent != NULL) {
		len = _pathlength(node->parent, len);
		len++; // separator
	}
	return len + strlen(node->name);
}

int pathlength(struct node_t *node) {
	return _pathlength(node, 0);
}

static void _getpathname(struct node_t *node, char *s) {
	if (node->parent == NULL)
		return;
	if (node->parent->parent != NULL) {
		_getpathname(node->parent, s);
		strcat(s, "\\");
	}
	strcat(s, node->name);
}

char *getpathname(struct node_t *node) {
	int len = pathlength(node);
	char *s = (char*)calloc(1, len + 1);
	_getpathname(node, s);
	return s;
}
