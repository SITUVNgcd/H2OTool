#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"
#include "chunk.h"

// scan

static void scan(struct node_t *dirnode) {
	DIR *d = opendir(".");
	
	struct dirent *direntry;
	struct stat st;

	while ((direntry = readdir(d)) != NULL) {
		stat(direntry->d_name, &st);
		
		if (S_ISDIR(st.st_mode) && strcmp(direntry->d_name, ".") != 0 && strcmp(direntry->d_name, "..")) {
			printf("scanned dir : %s\n", direntry->d_name);
			
			struct node_t *node = allocdir(direntry->d_name);
			attachnode(dirnode, node);
			
			chdir(direntry->d_name);
			scan(node);
			chdir("..");
		} else if (S_ISREG(st.st_mode)) {
			printf("scanned file : %s\n", direntry->d_name);

			struct entry_t *entry = (struct entry *)malloc(sizeof(struct entry_t));
			entry->entrytype = ENTRYTYPE_RAW;
			entry->dir_entry = 0; // corrected later
			entry->file_entry = 0; // corrected later
			entry->size_raw = st.st_size;
			entry->size_compressed = st.st_size;
			entry->unknown4 = 0; // TODO
			entry->offset = 0; // corrected later
			entry->unknown5 = MAGIC_ENTRY_UNKNOWN5;
			entry->checksum = 0; // corrected later
			entry->unknown6 = 0; // TODO
			
			struct node_t *node = allocfile(direntry->d_name, entry);
			attachnode(dirnode, node);
		}
	}
	
	closedir(d);
}

// store

static int store(FILE *dst, struct node_t *directory) {
	
	struct node_t *node = directory->firstchild;
	while (node != NULL) {
		if (node->type == NODETYPE_FILE) {
			printf("storing: %s\n", node->name);

			FILE *source = fopen(node->name, "rb");
			
			if (source == NULL) {
				printf("Failed to open file '%s'\n", node->name);
				return -1;
			}

			transfer(source, dst, node->file);
			fclose(source);

		} else	if (node->type == NODETYPE_DIRECTORY) {
			printf("storing directory %s\n", node->name);

			chdir(node->name);
			if (store(dst, node) < 0)
				return -1;
			chdir("..");
		}
		node = node->nextsibling;
	}
	return 0;
}

void cmd_build(const char *filename, const char *sourcedirectory) {

	// create the file first while we're still in the current directory, in case sourcedirectory is a relative path 
	FILE *f = fopen(filename, "wb");

	if (f == NULL) {
		puts("Could not create file for archive");
		return;
	}

	struct archive_t *archive = (struct archive_t*)malloc(sizeof(struct archive_t));
	
	archive->root = allocdir("<root>");

	// scan - traverse directory structure and populate a tree
	chdir(sourcedirectory);
	scan(archive->root);

	// write a comment, and the archive metadata
	fputc(0x1a, f);
	writearchive(f, archive);
	
	// store - traverse tree and directory structure in parallell and write file contents, calculate crc32 in the process
	store(f, archive->root);

	// write the archive metadata again, since now the entries have updated checksums
	fseek(f, 1, SEEK_SET);
	writearchive(f, archive);
	
	// close file
	fclose(f);
}
