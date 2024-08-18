#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "format.h"
#include "chunk.h"
#include "../support/blast.h"
#include "../support/crc32.h"
#include "../support/debug.h"

#define BATCH_SIZE 65536

struct blastprogress_t {
	uint32 inbytesavail;
	uint32 inbytesread;
	uint8 *inbuffer;
	FILE *infile;
	
	uint8 *buffer;
	uint32 bufsize;
	uint32 checksum;
	
	uint32 outbyteswritten;
	uint8 *outbuffer;
	FILE *outfile;
};

static unsigned fileblastin(void *how, unsigned char **buf) {
	struct blastprogress_t *p = (struct blastprogress_t*)how;
	uint32 n = p->inbytesavail - p->inbytesread;
	if (n > p->bufsize)
		n = p->bufsize;
	n = fread(p->buffer, 1, n, p->infile);
	p->inbytesread += n;
	*buf = p->buffer;		
	return n;
}

static int fileblastout(void *how, unsigned char *buf, unsigned len) {
	struct blastprogress_t *p = (struct blastprogress_t*)how;
	int n = fwrite(buf, 1, len, p->outfile);
	p->outbyteswritten += n;
	p->checksum = crc32(buf, len, p->checksum);
	return 0;
}

static unsigned memblastin(void* how, unsigned char** buf) {
	struct blastprogress_t *p = (struct blastprogress_t*)how;
	*buf = p->inbuffer;
	p->inbytesread += p->inbytesavail;
	return p->inbytesavail;
}

static int memblastout(void* how, unsigned char* buf, unsigned len) {
	struct blastprogress_t *p = (struct blastprogress_t*)how;
	memcpy(p->outbuffer + p->outbyteswritten, buf, (size_t)len);
	p->outbyteswritten += len;
	p->checksum = crc32(buf, len, p->checksum);
	return 0;
}

static void runblast(struct blastprogress_t *p, blast_in infun, blast_out outfun, uint32 expected_checksum) {
	blast(infun, (void*)p, outfun, (void*)p);
	p->checksum = ~p->checksum;
	if (p->checksum != expected_checksum) {
		dprintf("cheksum didnt match\n");
	} else {
		dprintf("checksum ok\n");
	}
}

static void runblast_mem2mem(void* in, uint32 insize, void* out, uint32 expected_checksum) {
	struct blastprogress_t p;
	p.inbytesavail = insize;
	p.inbytesread = 0;
	p.inbuffer = in;
	p.checksum = CRC32_INITIAL_VALUE;
	p.outbyteswritten = 0;
	p.outbuffer = out;
	runblast(&p, memblastin, memblastout, expected_checksum);
}

static void runblast_file2mem(FILE* infile, uint32 insize, void* out, uint32 expected_checksum) {
	struct blastprogress_t p;
	p.inbytesavail = insize;
	p.inbytesread = 0;
	p.infile = infile;
	p.bufsize = BATCH_SIZE;
	p.buffer = (uint8*)malloc(p.bufsize);
	p.checksum = CRC32_INITIAL_VALUE;
	p.outbyteswritten = 0;
	p.outbuffer = out;
	runblast(&p, fileblastin, memblastout, expected_checksum);
	free(p.buffer);
}

static void runblast_file2file(FILE* infile, uint32 insize, FILE* outfile, uint32 expected_checksum) {
	struct blastprogress_t p;
	p.inbytesavail = insize;
	p.inbytesread = 0;
	p.infile = infile;
	p.bufsize = BATCH_SIZE;
	p.buffer = (uint8*)malloc(p.bufsize);
	p.checksum = CRC32_INITIAL_VALUE;
	p.outbyteswritten = 0;
	p.outfile = outfile;
	runblast(&p, fileblastin, fileblastout, expected_checksum);
	free(p.buffer);
}

// Reads a chunk from the file, and decompresses it if it has to 
void readchunk(FILE *f, char **output) {
	struct chunkheader_t c;
	fread(&c, 1, sizeof(struct chunkheader_t), f);
	char* bufout = malloc(c.size_raw);
	if (c.size_compressed == c.size_raw) {
		fread(bufout, 1, c.size_raw, f);
		if (~crc32(bufout, c.size_raw, CRC32_INITIAL_VALUE) != c.checksum) {
			dprintf("checksum didnt match\n");
		} else {
			dprintf("checksum ok\n");
		}
	} else {
		runblast_file2mem(f, c.size_compressed, bufout, c.checksum);
	}
	*output = bufout;
}

// Reads a chunk from a file and then transfers its contents to another file
void dumpchunk(FILE *src, FILE *dst, uint32 expected_checksum) {
	struct chunkheader_t c;
	fread(&c, 1, sizeof(struct chunkheader_t), src);
	if (c.size_compressed == c.size_raw) {
		dump(src, dst, c.size_raw, expected_checksum);
	} else {
		runblast_file2file(src, c.size_compressed, dst, expected_checksum);
	}
}

// Reads a chunk from the file and then seeks past it in the file
void skipchunk(FILE *f) {
	struct chunkheader_t c;
	fread(&c,1,sizeof(struct chunkheader_t),f);
	fseek(f, c.size_compressed, SEEK_CUR);
}

// Writes a chunk to disk, a chunk header and a block of data
void writechunk(FILE *f, struct chunkheader_t *c, void *data) {
	fwrite(c, 1, sizeof(struct chunkheader_t), f);
	fwrite(data, 1, c->size_compressed, f);
}

// Copies the contents from one file to another and updates an entry with offset and crc32 checksum
int transfer(FILE *src, FILE *dst, struct entry_t *entry) {
	uint32 checksum = CRC32_INITIAL_VALUE;
	unsigned char *buf = (unsigned char*)malloc(BATCH_SIZE);
	
	entry->offset = ftell(dst);

	while (1) {
		int batch = BATCH_SIZE;
		int read = fread(buf, 1, batch, src);
		if (ferror(src))
			return -1;
		if (read == 0)
			break;
		int written = fwrite(buf, 1, read, dst);
		if (written != read)
			return 1;
		checksum = crc32(buf, read, checksum);
	}

	free(buf);	

	entry->checksum = ~checksum;
	return 0;
}

// Copies len bytes from one file to another
int dump(FILE *src, FILE *dst, int len, uint32 expected_checksum) {
	unsigned char *buf = (unsigned char*)malloc(BATCH_SIZE);
	uint32 checksum = CRC32_INITIAL_VALUE;

	while (len > 0) {
		int batch = len;
		if (batch > BATCH_SIZE)
			batch = BATCH_SIZE;
		int read = fread(buf, 1, batch, src);
		if (read != batch)
			return -1;
		int written = fwrite(buf, 1, read, dst);
		if (written != read)
			return 1;
		len -= written;
		checksum = crc32(buf, read, checksum);
	}

	free(buf);
	checksum = ~checksum;
	if (checksum != expected_checksum) {
		dprintf("checksum didnt match\n");
		return 2;
	} else {
		dprintf("checksum ok\n");
	}
	return 0;
}
