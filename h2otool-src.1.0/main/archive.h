#ifndef STRUCTURE_H_
#define STRUCTURE_H_

struct archivestat_t {
	uint32 numfiles;
	uint32 numdirectories;
	uint32 size_raw;
	uint32 size_compressed;
};

struct archive_t {
	struct node_t *root;
};

void buildarchivestat(struct archivestat_t *stat, struct node_t *node);

struct archive_t *readarchive(FILE *f);

void writearchive(FILE *f, struct archive_t *s);

#endif /*STRUCTURE_H_*/
