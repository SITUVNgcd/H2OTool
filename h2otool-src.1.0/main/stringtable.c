#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../support/debug.h"
#include "types.h"
#include "format.h"
#include "chunk.h"
#include "stringtable.h"
#include "../support/crc32.h"

// Allocates a stringtable with room for a fixed number of strings
struct stringtable_t *allocstringtable(int count) {
	struct stringtable_t *table = (struct stringtable_t*)malloc(sizeof(struct stringtable_t));
	table->count = count;
	table->strings = (char**)calloc(count, sizeof(char*));
	return table;
}

// Frees a string table and all its strings
void freestringtable(struct stringtable_t *table) {
	int i;
	for (i = 0; i < table->count; i++) {
		if (table->strings[i] != NULL)
			free(table->strings[i]);
	}
	free(table->strings);
	free(table);
}

// Fills in a string table by reading character data from its storage format
static void parsestringsdata(struct stringtable_t *table, uint16 *data) {
	int i;
	for (i = 0; i < table->count; i++) {

		uint16 *runner = data;

		// find the length of this string

		int n = 0;
		while (*runner++) {
			n++;
		}
		runner++;

		// allocate enough room for it

		table->strings[i] = malloc(n + 1);
		
		// fill it in
		runner = data;

		int x = 0;
		while (n--) {
			table->strings[i][x++] = *runner++;
		}
		table->strings[i][x++] = *runner++;

		data = runner;
	}
}

// Reads a stringtable from a file
struct stringtable_t *readstringtable(FILE *f) {

	char *data;
	
	readchunk(f, &data);
	
	struct stringsheader_t *strhdr = (struct stringsheader_t*)data;

	struct stringtable_t *table = allocstringtable(strhdr->count);
	
	parsestringsdata(table, (uint16*)(data + sizeof(struct stringsheader_t)));

	free(data);
	
	int i;
	for (i = 0; i < table->count; i++) {
		dprintf("%3d %s\n", i, table->strings[i]);
	}

	// skip redundant chunk
	skipchunk(f);

	return table;
}

// Transforms a string table to its storage format and fills in a chunkheader
static void *buildstringsdata(struct stringtable_t *table, struct chunkheader_t *chunk) {

	int i;
	int size = 0;
	
	// calculate total length of all strings
	for (i = 0; i < table->count; i++) {
		size += strlen(table->strings[i]) + 1;
	}

	// calculate byte size of strings data
	size = size * 2 + sizeof(struct stringsheader_t);

	// allocate data block and fill it in
	unsigned char *buffer = (unsigned char*)malloc(size);
	
	struct stringsheader_t *strhdr = (struct stringsheader_t *)buffer;
	strhdr->count = table->count;
	strhdr->length = size;
	
	uint16 *dest = (uint16*)(buffer + sizeof(struct stringsheader_t));
	
	for (i = 0; i < table->count; i++) {
		char *src = table->strings[i];
		while(1) {
			uint16 d = *src++;
			*dest++ = d;
			if (d == 0)
				break;
		}
	}

	// fill in the chunkheader
	chunk->size_raw = size;
	chunk->size_compressed = chunk->size_raw;
	chunk->checksum = ~crc32(buffer, size, CRC32_INITIAL_VALUE);
	
	return buffer;
}

// Transforms string table to its storage representation and write it to the file
void writestringtable(FILE *f, struct stringtable_t *table) {
	struct chunkheader_t chunk;
	void *data = buildstringsdata(table, &chunk);
	writechunk(f, &chunk, data);
	writechunk(f, &chunk, data);
	free(data);
}
