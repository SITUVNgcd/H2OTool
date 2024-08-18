#include <stdio.h>
#include <stdlib.h>

void cmd_help() {
	puts("Usage : h2otool <command> [archive] [directory]");
	puts("");
	puts("Where command is one of: info, list, extract, build or help");
	puts("");
	puts("Examples :");
	puts("");
	puts("To display info about an archive type:");
	puts("    h2otool info <archive>");
	puts("");
	puts("To list the contents of an archive type:");
	puts("    h2otool list <archive>");
	puts("");
	puts("To extract the contents of an archive type:");
	puts("    h2otool extract <archive> <target directory>");
	puts("");
	puts("To build an archive from the contents of a directory type:");
	puts("    h2otool extract <archive> <source directory>");
	puts("");
}
