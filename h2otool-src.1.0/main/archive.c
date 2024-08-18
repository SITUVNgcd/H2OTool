#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"
#include "chunk.h"
#include "stringtable.h"
#include "../support/debug.h"

void buildarchivestat(struct archivestat_t *stat, struct node_t *directory) {
	struct node_t *node = directory->firstchild;
	while (node != NULL) {
		if (node->type == NODETYPE_FILE) {
			stat->numfiles++;
			if (node->file->entrytype == ENTRYTYPE_COMPRESSED) {
				stat->size_compressed += node->file->size_compressed + sizeof(struct chunkheader_t);
				stat->size_raw += node->file->size_raw;
			} else {
				stat->size_compressed += node->file->size_compressed;
				stat->size_raw += node->file->size_raw;
			}
		} else if (node->type == NODETYPE_DIRECTORY) {
			stat->numdirectories++;
			buildarchivestat(stat, node);
		}
		node = node->nextsibling;
	}
}

// Reads an archive from a file
struct archive_t *readarchive(FILE *f) {
	int i;

	dprintf("begin readarchive\n");

	// scan to header
	char ch = 0;
	while (!feof(f)) {
		ch = fgetc(f);
		if (ch == 0x1A)
			break;
	}
	
	if (ch != 0x1A) {
		puts("Could not locate header");
		return NULL;
	}

	dprintf("found header\n");

	// read header
	struct header_t hdr;
	fread(&hdr, 1, sizeof(struct header_t), f);
	dprintf("read header\n");
	dprintf("Version: %d\n", hdr.version);
	dprintf("Files: %d\n", hdr.numfiles);
	dprintf("Size: %d\n", hdr.size_raw);
	dprintf("Size compressed: %d\n", hdr.size_compressed);

	if (hdr.version != FORMAT_VERSION) {
		puts("The archive is in an unsupported version of the format");
		return NULL;
	}

	// read entries
	struct entry_t *entries = (struct entry_t *)malloc(hdr.numfiles * sizeof(struct entry_t));
	fread(entries, 1, hdr.numfiles * sizeof(struct entry_t), f);
	fseek(f, hdr.numfiles * sizeof(struct entry_t), SEEK_CUR);
	dprintf("read entries\n");

	// directory name block
	struct stringtable_t *dirnames = readstringtable(f);
	dprintf("read dirnames\n");
	
	// file name block
	struct stringtable_t *filenames = readstringtable(f);
	dprintf("read filenames\n");

	// read directory tree map
	uint32 numdirectories;
	fread(&numdirectories, 1, sizeof(uint32), f);
	fseek(f, sizeof(uint32), SEEK_CUR);

	int32 *map = (int32*)malloc(numdirectories * sizeof(int32));
	fread(map, 1, numdirectories * sizeof(int32), f);
	fseek(f, numdirectories * sizeof(int32), SEEK_CUR);
	dprintf("read dirmappings\n");

	struct node_t *root = allocdir("<root>");

	// allocate and build an array of directory nodes
	struct node_t **dirs = (struct node_t **)malloc(numdirectories * sizeof(struct node_t *));
	for (i = 0; i < dirnames->count; i++) {
		char *name = dirnames->strings[i];
		if (strchr(name, '\\') != NULL)
			name = strrchr(name, '\\') + 1;
		dirs[i] = allocdir(name);
	}
	dprintf("allocated directory nodes\n");

	// attach directory nodes to each other using the array
	for (i = 0; i < numdirectories; i++) {
		struct node_t *parent;

		if (map[i] == -1) {
			parent = root;
		} else {
			parent = dirs[map[i]];
		}
		attachnode(parent, dirs[i]);
	}
	dprintf("attached nodes\n");
	
	uint32 sc = 0;

	// attach file nodes to the directory nodes using the array
	for (i = 0; i < hdr.numfiles; i++) {
		dprintf("attaching file %d\n", i);

		struct entry_t *entry = (struct entry_t *)malloc(sizeof(struct entry_t));
		*entry = *(entries + i);
		
		sc += entry->size_compressed;
		
		char *name = filenames->strings[entry->file_entry];

		struct node_t *directory;
		if (entry->dir_entry == -1)
			directory = root;
		else
			directory = dirs[entry->dir_entry];

		struct node_t *file = allocfile(name, entry);
		dprintf("alloced file %d\n", i);

		attachnode(directory, file);
		dprintf("attached file %d\n", i);
	}
	dprintf("attached files (%d)\n", sc);

	free(dirs);

	struct archive_t *archive = (struct archive_t *)malloc(sizeof(struct archive_t));
	archive->root = root;

	dprintf("end readarchive\n");

	return archive;
}

struct progress_t {
	int curfile;
	int curdirectory;
	struct entry_t *entries;
	struct stringtable_t *dirnames;
	struct stringtable_t *filenames;
	int32 *dirmappings;
};

void collectdir(struct progress_t *progress, struct node_t *directory, int parentdirindex) {

	struct node_t *node = directory->firstchild;
	while (node != NULL) {
		if (node->type == NODETYPE_FILE) {

			int myfileindex = progress->curfile++;

			struct entry_t *entry = progress->entries + myfileindex;
			*(entry) = *(node->file);
			entry->file_entry = myfileindex;
			entry->dir_entry = parentdirindex;
			progress->filenames->strings[myfileindex] = strdup(node->name);

		} else if (node->type == NODETYPE_DIRECTORY) {

			int mydirindex = progress->curdirectory++;

			progress->dirnames->strings[mydirindex] = getpathname(node);
			progress->dirmappings[mydirindex] = parentdirindex;

			collectdir(progress, node, mydirindex);
		}
		node = node->nextsibling;
	}
}

void collect(struct progress_t *progress, struct node_t *root) {
	collectdir(progress, root, -1);
}

// Writes a archive to a file, effectively creating a new archive
void writearchive(FILE *f, struct archive_t *archive) {

	struct archivestat_t stat;
	memset(&stat, 0, sizeof(stat));
	buildarchivestat(&stat, archive->root);

	struct header_t hdr;
	hdr.version = FORMAT_VERSION;
	hdr.numfiles = stat.numfiles;
	hdr.unknown1 = MAGIC_HEADER_UNKNOWN1;
	hdr.unknown2 = MAGIC_HEADER_UNKNOWN2;
	hdr.size_compressed = stat.size_compressed;
	hdr.filler1 = 0;
	hdr.size_raw = stat.size_raw;
	hdr.filler2 = 0;

	struct progress_t progress;
	progress.curfile = 0;
	progress.curdirectory = 0;
	progress.entries = (struct entry_t*)calloc(hdr.numfiles, sizeof(struct entry_t));
	progress.dirnames = allocstringtable(stat.numdirectories);
	progress.filenames = allocstringtable(hdr.numfiles);
	progress.dirmappings = (int32*)calloc(stat.numdirectories, sizeof(int32));

	collect(&progress, archive->root);	

	// write header	
	fwrite(&hdr, 1, sizeof(struct header_t), f);

	// write entries
	fwrite(progress.entries, hdr.numfiles, sizeof(struct entry_t), f);
	fwrite(progress.entries, hdr.numfiles, sizeof(struct entry_t), f);

	// write dirnames
	writestringtable(f, progress.dirnames);

	// write filenames
	writestringtable(f, progress.filenames);

	// write dirmappings
	uint32 count = stat.numdirectories;
	fwrite(&count, 1, sizeof(uint32), f);
	fwrite(&count, 1, sizeof(uint32), f);
	fwrite(progress.dirmappings, stat.numdirectories, sizeof(int32), f);
	fwrite(progress.dirmappings, stat.numdirectories, sizeof(int32), f);

	free(progress.entries);
	freestringtable(progress.dirnames);
	freestringtable(progress.filenames);
	free(progress.entries);
}
