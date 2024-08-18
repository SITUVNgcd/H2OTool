#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"
#include "chunk.h"

static int extractdirectory(FILE *src, struct node_t *directory) {
	
	struct node_t *node = directory->firstchild;
	while (node != NULL) {
		if (node->type == NODETYPE_FILE) {

			fseek(src, node->file->offset, SEEK_SET);
			FILE *dst = fopen(node->name, "wb");
			
			if (dst == NULL) {
				printf("Failed to create file for '%s'\n", node->name);
				return -1;
			}
			
			if (node->file->entrytype == ENTRYTYPE_COMPRESSED) {
				printf("decompressing file: %s\n", node->name);
				dumpchunk(src, dst, node->file->checksum);
			} else 	if (node->file->entrytype == ENTRYTYPE_RAW) {
				printf("extracting file: %s\n", node->name);
				dump(src, dst, node->file->size_raw, node->file->checksum);
			}

			fclose(dst);

		} else if (node->type == NODETYPE_DIRECTORY) {
			printf("creating directory %s\n", node->name);

			mkdir(node->name);
			chdir(node->name);

			if (extractdirectory(src, node) < 0)
				return -1;
			
			chdir("..");
		}
		node = node->nextsibling;
	}
	return 0;
}

void cmd_extract(const char *filename, const char *targetdirectory) {
	
	FILE* f = fopen(filename, "rb");

	if (f == NULL) {
		puts("Could not open archive");
		return;
	}

	struct archive_t *archive = readarchive(f);

	if (archive == NULL)
		return;

	// make sure targetdirectory exist
	mkdir(targetdirectory);

	// enter it
	chdir(targetdirectory);

	// traverse and extract
	extractdirectory(f, archive->root);

	fclose(f);
}
