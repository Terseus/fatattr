/**
 * Copyright 2013 David Caro Martinez
 *
 * This file is part of fatattr.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "dosfs.h"
#include "bool.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>

#ifndef DIR_ENTRY_SIZE
#define DIR_ENTRY_SIZE 256
#endif
#ifndef REAL_DIR_ENTRY_SIZE
#define REAL_DIR_ENTRY_SIZE 1025
#endif
#define ERRMSG_MAX 1025


enum {
    ENOERR = 0,
    EALLOC
};

enum {
    FLAG_VERBOSE = 0x01,
    FLAG_RECURSIVE = 0x02,
    FLAG_HELP = 0x04,
    FLAG_VERSION = 0x08
};

struct programArgs {
	char **fileList;
	size_t fileListSize;
	uint32_t attrsToAdd;
	uint32_t attrsToRemove;
	unsigned int flags;
};

static char errmsg[ERRMSG_MAX] = {0};

/**
 * Returns a descriptive message associated with an error code.
 * This is only used for the errors that happens purely in this source file,
 * like a memory alloc error.
 */
const char *mainGetError(int err);
/**
 * Print FAT attributes in a readable format.
 */
void printAttrs(uint32_t attrs);
/**
 * Prints the program name, version and copyright/license notices.
 */
void showVersion(void);
/**
 * Prints the same as showVersion along with the program help.
 */
void showHelp(void);
/**
 * Appends a file to the file list of 'args'.
 * Returns 0 on success, !0 if an error happens.
 */
int appendFileToList(struct programArgs *args, char *file);
/**
 * Print a file's attributes with the configuration saved in 'args'.
 * If 'processDir' != 0 and 'file' is a directory, process the files inside it.
 * Returns 0 on success, !0 if an error happens.
 */
int processPrintAttributes(const struct programArgs *const args,
                           char *file,
                           int processDir);
/**
 * Internal function, sub of processPrintAttributes.
 * Returns 0 on success, !0 if an error happens.
 */
int processPrintAttributesFd(const struct programArgs *const args,
                             char *file,
                             int fd,
                             int processDir);
/**
 * Modify the attributes of a file based on the configuration saved in 'args'.
 * If 'processDir' != 0 and 'file' is a directory, process the files inside it.
 * Returns 0 on success, !0 if an error happens.
 */
int processModifyAttributes(const struct programArgs *const args,
                            char *file,
                            int processDir);
/**
 * Internal function, sub of processModifyAttributes.
 * Returns 0 on success, !0 if an error happens.
 */
int processModifyAttributesFd(const struct programArgs *const args,
                              char *file,
                              int fd,
                              int processDir);
/**
 * Process the program's arguments and saved the readed values in 'result'.
 * Returns 0 on success, !0 if an error happens.
 */
int processArgs(int argc, char **argv, struct programArgs *result);



const char *mainGetError(int err)
{
	switch (err) {
	case ENOERR:
		snprintf(errmsg, ERRMSG_MAX,
		         "No error occurred");
		break;
	case EALLOC:
		snprintf(errmsg, ERRMSG_MAX,
		         "Error allocating memory: %s",
		         strerror(errno));
		break;
	default:
		snprintf(errmsg, ERRMSG_MAX,
		         "Unknown error");
	}
	return errmsg;
}

void printAttrs(uint32_t attrs)
{
	printf("%c%c%c%c%c%c",
	       DOSFS_HAS_ATTR_RO(attrs) ? 'R' : '-',
	       DOSFS_HAS_ATTR_HIDDEN(attrs) ? 'H' : '-',
	       DOSFS_HAS_ATTR_SYS(attrs) ? 'S' : '-',
	       DOSFS_HAS_ATTR_ARCH(attrs) ? 'A' : '-',
	       DOSFS_HAS_ATTR_DIR(attrs) ? 'D' : '-',
	       DOSFS_HAS_ATTR_VOLUME(attrs) ? 'V' : '-'
	      );
}

void showVersion()
{
	printf("FAT attributes utility, version %s\n", VERSION_STRING);
	printf("Copyright 2013 David Caro Martinez\n"
	       "\n"
	       "This software comes with ABSOLUTELY NO WARRANTY.\n"
	       "This is free software, and you are welcome to redistribute it \n"
	       "under certain conditions. See the GNU General Public License \n"
	       "for details.\n"
	      );
}

void showHelp()
{
	showVersion();
	printf("Usage: %s [options] FILE ...\n", PROGRAM_NAME);
	printf("Accepted options:\n"
	       "\t+R: Sets the read-only attribute.\n"
	       "\t-R: Remove the read-only attribute.\n"
	       "\t+A: Sets the archive attribute.\n"
	       "\t-A: Remove the archive attribute.\n"
	       "\t+S: Sets the system attribute.\n"
	       "\t-S: Remove the system attribute.\n"
	       "\t+H: Sets the hidden attribute.\n"
	       "\t-H: Remove the system attribute.\n"
	       "\t+D: Sets the directory attribute (warning! see below).\n"
	       "\t-D: Remove the directory attribute (warning! see below).\n"
	       "\t+V: Sets the volume label attribute (warning! see below).\n"
	       "\t-V: Remove the volume label attribute (warning! see below).\n"
	       "\t--recursive: If FILE is a directory, process it recursively.\n"
	       "\t--verbose: Verbose attribute changes.\n"
	       "\t--help: Show this help.\n"
	       "\t--version: Show only the program name, version and credits.\n"
	       "\t--: Forces all arguments past this one to be interpreted as "
	       "files.\n"
	       "If no attribute change is specified, the program prints the "
	       "file's attributes.\n"
	       "Do NOT use the +D, -D, +V and -V options if you don't know "
	       "EXACTLY what you are doing.\n"
	      );
}

int appendFileToList(struct programArgs *args, char *file)
{
	args->fileList = realloc(args->fileList,
	                         sizeof(char *) * (++(args->fileListSize)));
	if (args->fileList == NULL) {
		return EALLOC;
	}
	args->fileList[args->fileListSize - 1] = file;
	return ENOERR;
}

int processPrintAttributes(const struct programArgs *const args,
                           char *file,
                           int processDir)
{
	int fd = 0;
	int dosfsErrno = dosfsOpen(file, &fd);
	if (dosfsErrno) {
		return dosfsErrno;
	}
	dosfsErrno = processPrintAttributesFd(args, file, fd, processDir);
	dosfsClose(fd);
	return dosfsErrno;
}

int processPrintAttributesFd(const struct programArgs *const args,
                             char *file,
                             int fd,
                             int processDir)
{
	uint32_t fileAttrs = 0;
	int dosfsErrno = dosfsGetAttributes(fd, &fileAttrs);
	if (dosfsErrno) {
		return dosfsErrno;
	}
	printAttrs(fileAttrs);
	printf("  %s\n", file);
	if (DOSFS_HAS_ATTR_DIR(fileAttrs) && processDir) {
		char dirEntry[DIR_ENTRY_SIZE] = {0};
		char realDirEntry[REAL_DIR_ENTRY_SIZE] = {0};
		while (!(dosfsErrno = dosfsReadDir(fd, dirEntry, DIR_ENTRY_SIZE)) &&
		        strlen(dirEntry) > 0) {
			snprintf(realDirEntry, REAL_DIR_ENTRY_SIZE,
			         "%s/%s",
			         file, dirEntry);
			int recursive = (args->flags & FLAG_RECURSIVE) &&
			                strcmp(dirEntry, ".") != 0 &&
			                strcmp(dirEntry, "..") != 0;
			dosfsErrno = processPrintAttributes(args, realDirEntry, recursive);
			if (dosfsErrno) {
				fprintf(stderr, "Error processing file '%s': %s\n",
				        realDirEntry, dosfsGetError(dosfsErrno));
			}
		}
	}
	return ENOERR;
}

int processModifyAttributes(const struct programArgs *const args,
                            char *file,
                            int processDir)
{
	int fd = 0;
	int dosfsErrno = dosfsOpen(file, &fd);
	if (dosfsErrno) {
		return dosfsErrno;
	}
	dosfsErrno = processModifyAttributesFd(args, file, fd, processDir);
	dosfsClose(fd);
	return dosfsErrno;
}

int processModifyAttributesFd(const struct programArgs *const args,
                              char *file,
                              int fd,
                              int processDir)
{
	uint32_t fileAttrs = 0;
	int dosfsErrno = dosfsGetAttributes(fd, &fileAttrs);
	if (dosfsErrno) {
		return dosfsErrno;
	}
	if (args->attrsToAdd != 0 &&
	        (args->attrsToAdd & fileAttrs) != args->attrsToAdd) {
		dosfsErrno = dosfsAddAttributes(fd, args->attrsToAdd);
		if (dosfsErrno) {
			return dosfsErrno;
		}
	}
	if (args->attrsToRemove != 0 &&
	        (args->attrsToRemove & fileAttrs) != 0) {
		dosfsErrno = dosfsRemoveAttributes(fd, args->attrsToRemove);
		if (dosfsErrno) {
			return dosfsErrno;
		}
	}
	uint32_t newAttrs = 0;
	dosfsErrno = dosfsGetAttributes(fd, &newAttrs);
	if (dosfsErrno) {
		return dosfsErrno;
	}
	if (args->flags & FLAG_VERBOSE) {
		printAttrs(fileAttrs);
		printf(" => ");
		printAttrs(newAttrs);
		printf("  %s\n", file);
	}
	if (DOSFS_HAS_ATTR_DIR(newAttrs) && processDir) {
		char dirEntry[DIR_ENTRY_SIZE] = {0};
		char realDirEntry[REAL_DIR_ENTRY_SIZE] = {0};
		while (!(dosfsErrno = dosfsReadDir(fd, dirEntry, DIR_ENTRY_SIZE)) &&
		        strlen(dirEntry) > 0) {
			snprintf(realDirEntry, REAL_DIR_ENTRY_SIZE,
			         "%s/%s",
			         file, dirEntry);
			int recursive = (args->flags & FLAG_RECURSIVE) &&
			                strcmp(dirEntry, ".") != 0 &&
			                strcmp(dirEntry, "..") != 0;
			dosfsErrno = processModifyAttributes(args, realDirEntry, recursive);
			if (dosfsErrno) {
				fprintf(stderr, "Error processing file '%s': %s\n",
				        realDirEntry, dosfsGetError(dosfsErrno));
			}
		}
	}
	return ENOERR;
}

int processArgs(int argc, char **argv, struct programArgs *result)
{
	result->fileList = NULL;
	result->fileListSize = 0;
	result->attrsToAdd = 0;
	result->attrsToRemove = 0;
	result->flags = 0;
	int skipArgs = FALSE;
	int mainErrno = 0;
	for (int i = 1; i < argc; i++) {
		if (skipArgs || (argv[i][0] != '-' && argv[i][0] != '+')) {
			mainErrno = appendFileToList(result, argv[i]);
			if (mainErrno) {
				return mainErrno;
			}
		} else if (argv[i][0] == '-') {
			if (argv[i][1] == '\0') {
				fprintf(stderr, "Missing attribute in argument '%s'\n", argv[i]);
				exit(1);
			} else if (argv[i][1] == '-') {
				if (argv[i][2] == '\0') {
					skipArgs = TRUE;
					continue;
				} else if (strcmp(argv[i], "--recursive") == 0) {
					result->flags |= FLAG_RECURSIVE;
					continue;
				} else if (strcmp(argv[i], "--verbose") == 0) {
					result->flags |= FLAG_VERBOSE;
					continue;
				} else if (strcmp(argv[i], "--help") == 0) {
					result->flags |= FLAG_HELP;
					continue;
				} else if (strcmp(argv[i], "--version") == 0) {
					result->flags |= FLAG_VERSION;
					continue;
				} else {
					fprintf(stderr, "Invalid option '%s'\n", argv[i]);
					exit(1);
				}
			} else {
				size_t argLen = strlen(argv[i]);
				for (size_t j = 1; j < argLen; j++) {
					switch (argv[i][j]) {
					case 'R':
						result->attrsToRemove |= DOSFS_ATTR_RO;
						break;
					case 'A':
						result->attrsToRemove |= DOSFS_ATTR_ARCH;
						break;
					case 'S':
						result->attrsToRemove |= DOSFS_ATTR_SYS;
						break;
					case 'H':
						result->attrsToRemove |= DOSFS_ATTR_HIDDEN;
						break;
					case 'D':
						result->attrsToRemove |= DOSFS_ATTR_DIR;
						break;
					case 'V':
						result->attrsToRemove |= DOSFS_ATTR_VOLUME;
						break;
					default:
						fprintf(stderr, "Invalid attribute '%c' in '%s'\n",
						        argv[i][j], argv[i]);
						exit(1);
					}
				}
			}
		} else if (argv[i][0] == '+') {
			if (argv[i][1] == '\0') {
				fprintf(stderr, "Missing attribute in argument '%s'\n", argv[i]);
				exit(1);
			}
			size_t argLen = strlen(argv[i]);
			for (size_t j = 1; j < argLen; j++) {
				switch (argv[i][j]) {
				case 'R':
					result->attrsToAdd |= DOSFS_ATTR_RO;
					break;
				case 'A':
					result->attrsToAdd |= DOSFS_ATTR_ARCH;
					break;
				case 'S':
					result->attrsToAdd |= DOSFS_ATTR_SYS;
					break;
				case 'H':
					result->attrsToAdd |= DOSFS_ATTR_HIDDEN;
					break;
				case 'D':
					result->attrsToAdd |= DOSFS_ATTR_DIR;
					break;
				case 'V':
					result->attrsToAdd |= DOSFS_ATTR_VOLUME;
					break;
				default:
					fprintf(stderr, "Invalid attribute '%c' in '%s'\n",
					        argv[i][j], argv[i]);
					exit(1);
				}
			}
		}
	}
	return ENOERR;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		showHelp();
		exit(0);
	}
	struct programArgs args;
	int mainErrno = processArgs(argc, argv, &args);
	if (mainErrno) {
		fprintf(stderr, "Error processing arguments: %s\n",
		        mainGetError(mainErrno));
		exit(1);
	}
	if (args.flags & FLAG_HELP) {
		showHelp();
		exit(0);
	}
	if (args.flags & FLAG_VERSION) {
		showVersion();
		exit(0);
	}
	if (args.fileListSize == 0) {
		fprintf(stderr, "Error processing arguments: No file(s) specified\n");
		showHelp();
		exit(1);
	}
	if ((args.attrsToAdd & args.attrsToRemove) != 0) {
		fprintf(stderr,
		        "Error processing arguments: Overlapping attribute changes\n");
		exit(1);
	}
	int dosfsErrno = 0;
	if (args.attrsToRemove == 0 && args.attrsToAdd == 0) {
		for (size_t i = 0; i < args.fileListSize; i++) {
			dosfsErrno = processPrintAttributes(&args, args.fileList[i], TRUE);
			if (dosfsErrno) {
				fprintf(stderr, "Error processing file '%s': %s\n",
				        args.fileList[i], dosfsGetError(dosfsErrno));
			}
		}
	} else {
		for (size_t i = 0; i < args.fileListSize; i++) {
			dosfsErrno = processModifyAttributes(&args, args.fileList[i],
			                                     args.flags & FLAG_RECURSIVE);
			if (dosfsErrno) {
				fprintf(stderr, "Error processing file '%s': %s\n",
				        args.fileList[i], dosfsGetError(dosfsErrno));
			}
		}
	}
	free(args.fileList);
	exit(dosfsErrno);
}
