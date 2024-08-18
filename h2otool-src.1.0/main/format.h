#ifndef FORMAT_H_
#define FORMAT_H_

#define FORMAT_VERSION 6 

#define MAGIC_HEADER_UNKNOWN1 0
#define MAGIC_HEADER_UNKNOWN2 4317964

struct header_t {
	uint32 version;
	uint32 numfiles;
	uint32 unknown1; // always 0
	uint32 unknown2; // always 0x0CE34100 / 4317964
	uint32 size_compressed; // includes size of chunkheader_t for compressed entries
	uint32 filler1; // the rest of the 64 bit word lives here
	uint32 size_raw;
	uint32 filler2; // the rest of the 64 bit word lives here
} __attribute__((__packed__));

#define MAGIC_ENTRY_UNKNOWN1 0
#define MAGIC_ENTRY_UNKNOWN5 0

enum entrytype_t {
	ENTRYTYPE_RAW = 0,
	ENTRYTYPE_COMPRESSED = 1
};

struct entry_t {
	uint32 entrytype;
	uint32 dir_entry;
	uint32 file_entry;
	uint32 size_raw;
	uint32 size_compressed;
	uint32 unknown4; // always 0x7CFC1200
	uint32 offset;
	uint32 unknown5; // always 0
	uint32 checksum; // only a guess
	uint32 unknown6; // always 0x90189200
} __attribute__((__packed__));

struct chunkheader_t {
	uint32 size_compressed;
	uint32 size_raw;
	uint32 checksum;
} __attribute__((__packed__));

struct stringsheader_t {
	uint32 count;
	uint32 length; // size of the strings block, incl. this header
} __attribute__((__packed__));

#endif /*FORMAT_H_*/
