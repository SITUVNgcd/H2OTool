#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"
#include "../support/debug.h"

static void listdir(struct node_t *directory) {

	struct node_t *node = directory->firstchild;
	while (node != NULL) {
		if (node->type == NODETYPE_FILE) {
			printf("%d\t ", node->file->size_raw);
			printf("%d\t ", node->file->size_compressed);
			printpath(node);
			printf("\n");
		}
		node = node->nextsibling;
	}

	node = directory->firstchild;
	while (node != NULL) {
		listdir(node);
		node = node->nextsibling;
	}
}

void cmd_list(const char *filename) {

	FILE* f = fopen(filename, "rb");
	
	if (f == NULL) {
		puts("Could not open archive");
		return;
	}

	struct archive_t *archive = readarchive(f);

	if (archive == NULL)
		return;
	
	listdir(archive->root);
	
	struct archivestat_t stat;
	memset(&stat, 0, sizeof(stat));
	buildarchivestat(&stat, archive->root);

	printf("----\n%d files.\n", stat.numfiles);

	fclose(f);
}
