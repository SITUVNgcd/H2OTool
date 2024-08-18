#ifndef STRINGTABLE_H_
#define STRINGTABLE_H_

struct stringtable_t {
	uint32 count;
	char** strings;
};

struct stringtable_t *allocstringtable(int count);

void freestringtable(struct stringtable_t *s);

struct stringtable_t *readstringtable(FILE *f);

void writestringtable(FILE *f, struct stringtable_t *table);

#endif /*STRINGTABLE_H_*/
