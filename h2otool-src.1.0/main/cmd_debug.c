#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "tree.h"
#include "archive.h"

void cmd_debug(const char *filename) {
	int i, l;

	FILE* f = fopen(filename, "rb");

	// scan to header
	char ch = 0;
	while (!feof(f)) {
		ch = fgetc(f);
		if (ch == 0x1A)
			break;
	}
	
	if (ch != 0x1A) {
		puts("Could not locate header");
		return;
	}
	
	struct header_t hdr;
	l = fread(&hdr, 1, sizeof(struct header_t), f);
	if (l != sizeof(struct header_t)) {
		puts("file underrun in header");
		return;
	}

	printf("Format version: %d\n", hdr.version);
	printf("Files: %d\n", hdr.numfiles);
	printf("Size: %d\n", hdr.size_raw);
	printf("Size compressed: %d\n", hdr.size_compressed);
	printf("Unknown 1: %d\n", hdr.unknown1); 
	printf("Unknown 2: %d\n", hdr.unknown2);
	printf("Filler 1: %d\n", hdr.filler1);
	printf("Filler 2: %d\n", hdr.filler2);
	
	struct entry_t *entries = (struct entry_t*)malloc(hdr.numfiles * sizeof(struct entry_t));
	l = fread(entries, 1, hdr.numfiles * sizeof(struct entry_t), f);
	if (l != hdr.numfiles * sizeof(struct entry_t)) {
		puts("file underrun in first entries block");
		return;
	}

	struct entry_t *entries2 = (struct entry_t*)malloc(hdr.numfiles * sizeof(struct entry_t));
	l = fread(entries2, 1, hdr.numfiles * sizeof(struct entry_t), f);
	if (l != hdr.numfiles * sizeof(struct entry_t)) {
		puts("file underrun in second entries block");
		return;
	}

	if (memcmp(entries, entries2, hdr.numfiles * sizeof(struct entry_t)) != 0) {
		puts("entry blocks dont match");
		return;
	}

	for (i = 0; i < hdr.numfiles; i++) {
		struct entry_t *entry = entries + i;
		printf("E(%2d,%2d) t:%d o:%7d sr:%6d sc:%6d c:%8x /%d,%d,%d/ [%d %s\n", 
			entry->dir_entry, entry->file_entry,
			entry->entrytype, entry->offset, entry->size_raw, entry->size_compressed, entry->checksum,
			entry->unknown4, entry->unknown5, entry->unknown6,
			
			entry->offset + entry->size_compressed,
			entry->size_raw == entry->size_compressed ? "raw" : "compressed");
	}

	
	// dirnames
	struct chunkheader_t c;
	l = fread(&c, 1, sizeof(struct chunkheader_t), f);
	if (l != sizeof(struct chunkheader_t)) {
		puts("file underrun in dirnames chunk header");
		return;
	}
	
	if (c.size_compressed != c.size_raw)
		puts("dirnames are compressed");
	else
		puts("dirnames are not compressed");

	fseek(f, c.size_compressed * 2 + sizeof(struct chunkheader_t), SEEK_CUR);

	
	// filenames
	l = fread(&c, 1, sizeof(struct chunkheader_t), f);
	if (l != sizeof(struct chunkheader_t)) {
		puts("file underrun in filenames chunk header");
		return;
	}

	if (c.size_compressed != c.size_raw)
		puts("filenames are compressed");
	else
		puts("filenames are not compressed");

	fseek(f, c.size_compressed * 2 + sizeof(struct chunkheader_t), SEEK_CUR);


	// dirmappings
	uint32 dirnum, dirnum2;
	l = fread(&dirnum, 1, sizeof(uint32), f);
	if (l != sizeof(uint32)) {
		puts("file underrun in first dirnum");
		return;
	}
	
	l = fread(&dirnum2, 1, sizeof(uint32), f);
	if (l != sizeof(uint32)) {
		puts("file underrun in second dirnum");
		return;
	}
	
	if (dirnum != dirnum2) {
		puts("dirnum counts dont match");
		return;
	}
	
	printf("Directories: %d\n", dirnum);
	
	int32 *mappings = (int32*)malloc(dirnum * sizeof(int32));
	l = fread(mappings, 1, dirnum * sizeof(int32), f);
	if (l != dirnum * sizeof(int32)) {
		puts("file underrun in dirmappings");
		return;
	}

	for (i = 0; i < dirnum; i++) {
		if (mappings[i] >= (int32)dirnum)
			printf("dirmap corrupted, index: %d, maps to: %d\n", i, mappings[i]);
	}
	
	fclose(f);
}
