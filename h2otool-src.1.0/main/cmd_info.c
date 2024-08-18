#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"

void cmd_info(const char *filename) {

	FILE* f = fopen(filename, "rb");

	if (f == NULL) {
		puts("Could not open archive");
		return;
	}

	struct archive_t *archive = readarchive(f);

	if (archive == NULL)
		return;

	struct archivestat_t stat;
	memset(&stat, 0, sizeof(stat));
	buildarchivestat(&stat, archive->root);

	printf("Files: %d\n", stat.numfiles);
	printf("Directories: %d\n", stat.numdirectories);
	printf("Size: %d\n", stat.size_raw);
	printf("Size compressed: %d\n", stat.size_compressed);

	fclose(f);
}
