#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "../support/debug.h" 

int main(int argc, const char **argv) {
	
	dprintf("! this exececutable is compiled in debug mode\n");

	if (argc < 2) {
		puts("h2otool: missing command");
		puts("Try 'h2otool help' for more information.");
		return EXIT_SUCCESS;
	}
	
	puts("");
	puts("H2OTOOL - h2o archive utility v1.0 - sigget 2007");
	puts("-----------------------------------------------------");
	puts("");

	const char *cmd = argv[1];
	
	if (strcmp(cmd, "help") == 0) {
		cmd_help();
	} else if (strcmp(cmd, "info") == 0) {
		if (argc < 3) {
			puts("h2otool: missing archive");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *archive = argv[2];
		cmd_info(archive);
	} else if (strcmp(cmd, "debug") == 0) {
		if (argc < 3) {
			puts("h2otool: missing archive");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *archive = argv[2];
		cmd_debug(archive);
	} else if (strcmp(cmd, "list") == 0) {
		if (argc < 3) {
			puts("h2otool: missing archive");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *archive = argv[2];
		cmd_list(archive);
	} else if (strcmp(cmd, "extract") == 0) {
		if (argc < 3) {
			puts("h2otool: missing archive");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *archive = argv[2];
		if (argc < 4) {
			puts("h2otool: missing directory");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *directory = argv[3];
		cmd_extract(archive, directory);
	} else if (strcmp(cmd, "build") == 0) {
		if (argc < 3) {
			puts("h2otool: missing archive");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *archive = argv[2];
		if (argc < 4) {
			puts("h2otool: missing directory");
			puts("Try 'h2otool help' for more information.");
			return EXIT_SUCCESS;
		}
		const char *directory = argv[3];
		cmd_build(archive, directory);
	} else {
		puts("unknown command");
		puts("Try 'h2otool help' for more information.");
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}
