#ifndef TREE_H_
#define TREE_H_

enum nodetype_t {
	NODETYPE_DIRECTORY = 1,
	NODETYPE_FILE = 2
};

struct node_t {
	enum nodetype_t type;
	struct node_t *parent;
	struct node_t *firstchild;
	struct node_t *nextsibling;

	char *name;
	
	struct entry_t *file;
};


struct node_t *allocdir(const char *name);

struct node_t *allocfile(const char *name, struct entry_t *entry);

void freenode(struct node_t *node);

void attachnode(struct node_t *node, struct node_t *child);

void printpath(struct node_t *node);

int pathlength(struct node_t *node);

char *getpathname(struct node_t *node);

#endif /*TREE_H_*/
