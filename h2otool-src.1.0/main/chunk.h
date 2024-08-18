#ifndef CHUNK_H_
#define CHUNK_H_

void readchunk(FILE *f, char **output);

void dumpchunk(FILE *src, FILE *dst, uint32 expected_checksum);

void skipchunk(FILE *f);

void writechunk(FILE *f, struct chunkheader_t *c, void *data);

int transfer(FILE *src, FILE *dst, struct entry_t *entry);

int dump(FILE *src, FILE *dst, int len, uint32 expected_checksum);

#endif /*CHUNK_H_*/
