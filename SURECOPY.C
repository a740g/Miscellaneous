#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <limits.h>

int main(int argc, char *argv[]) {
	FILE *fSource;
	FILE *fDestin;
	long i;
	long fSize;
	unsigned short c = 0;
	int dataByte;

	if (argc < 3) {
		puts(
			"\nSURECOPY: Smart file dupicator.\n"
			"\nCopyright (c) Samuel Gomes, 1998-2001.\n"
			"All rights reserved.\n"
			"\nUsage: SURECOPY [source] [destination]\n"
			"\nNote: Both [source] and [destination] must be file names."
			);
		return EXIT_FAILURE;
	}

	if ((fSource = fopen(argv[1], "rb")) == NULL) {
		puts("\nFailed to open source file!");
		return EXIT_FAILURE;
	}

	if ((fDestin = fopen(argv[2], "wb")) == NULL) {
		puts("\nFailed to open destination file!");
		fclose(fSource);
		return EXIT_FAILURE;
	}

	if ((fSize = filelength(fileno(fSource))) < 1) {
		puts("\nCannot copy empty file!");
		fclose(fSource);
		fclose(fDestin);
		return EXIT_FAILURE;
	}

	printf("\nCopying \"%s\" to \"%s\" ...\n", argv[1], argv[2]);

	for (i = 0; i < fSize; i++) {
		fseek(fSource, i, SEEK_SET);
		dataByte = getc(fSource);

		if (dataByte != EOF) {
			fseek(fDestin, i, SEEK_SET);
			putc(dataByte, fDestin);
		}

		if ((++c) == USHRT_MAX) {
			printf("\r%ld bytes copied.", i);
		}
	}

	fclose(fSource);
	fclose(fDestin);

	puts("\nFinished copying.");

	return EXIT_SUCCESS;
}
