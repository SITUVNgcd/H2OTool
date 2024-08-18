#ifndef COMMANDS_H_
#define COMMANDS_H_

void cmd_help();
void cmd_info(const char *filename);
void cmd_debug(const char *filename);
void cmd_list(const char *filename);
void cmd_extract(const char *filename, const char *targetdirectory);
void cmd_build(const char *filename, const char *sourcedirectory);

#endif /*COMMANDS_H_*/
