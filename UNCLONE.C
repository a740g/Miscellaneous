/****************************************************************************\
*                                                                            *
* Zero byte and clone file cleaner. Version 2.1                              *
* Copyright (c) Samuel Gomes (Blade), 2000-2003                              *
* mailto: blade_go@hotmail.com                                               *
*                                                                            *
\****************************************************************************/

#include <stdlib.h>
#include <fcntl.h>
#include <direct.h>
#include <string.h>
#include <dos.h>
#include <stdio.h>
#include <conio.h>

#define FALSE 0
#define TRUE 1
#define COMPARE_BUFFER_SIZE 8192    /* 8kb; tune this to your hearts content! */

/* Linked list node of file information type */
typedef struct llNodeFileInfo {
   struct llNodeFileInfo *previous;
   struct find_t *fileInfo;
   struct llNodeFileInfo *next;
} llNodeFileInfo_t;

/* Linked list for file info linked list node type */
typedef struct {
   llNodeFileInfo_t *first;
   llNodeFileInfo_t *last;
   unsigned length;
} llFileInfo_t;

int automatic = FALSE;
llFileInfo_t *fileList = NULL;      /* llFileInfo_t list holder */


/* Initializes the file list */
int initFileList(void) {
    fileList = (llFileInfo_t *)malloc(sizeof(llFileInfo_t));
    if (fileList) {
        fileList->first = fileList->last = NULL;
        fileList->length = 0;
    }
    return (fileList != NULL);  /* return true if mem alloc passed */
}

/* Adds file info into a new node */
int addFileInfo(struct find_t *fileInf) {
    llNodeFileInfo_t *This;

    This = (llNodeFileInfo_t *)malloc(sizeof(llNodeFileInfo_t));
    if (!This) return FALSE;    /* return false if failed to alloc mem */

    This->previous = fileList->last;
    This->next = NULL;
    if (fileList->length)
        fileList->last->next = This;
    else
        fileList->first = This;
    fileList->last = This;
    fileList->length++;
	This->fileInfo = (struct find_t *)malloc(sizeof(struct find_t));
    if (!This->fileInfo) return FALSE;
	memcpy(This->fileInfo, fileInf, sizeof(struct find_t));

    return TRUE;
}

/* Deletes the specified node */
void deleteFileInfo(llNodeFileInfo_t *node) {
    if (!node->previous)
        fileList->first = node->next;
    else
        node->previous->next = node->next;

    if (!node->next)
        fileList->last = node->previous;
    else
        node->next->previous = node->previous;

    free(node->fileInfo);   /* delete associated data */
    free(node);

    fileList->length--;
}

/* Disposes the file list */
void doneFileList(void) {
    /* Delete all nodes */
    while (fileList->length) {
        deleteFileInfo(fileList->last);
    }
    free(fileList);
}

/* Compares two files in the given two nodes */
int fileEquals(llNodeFileInfo_t *file1, llNodeFileInfo_t *file2) {
    char buf1[COMPARE_BUFFER_SIZE], buf2[COMPARE_BUFFER_SIZE];
    FILE *handle1, *handle2;

    /* Return false if files are unequal in size */
	if (file1->fileInfo->size != file2->fileInfo->size) return FALSE;
    /* Return true if both are zero in size */
	if (file1->fileInfo->size == 0 && file2->fileInfo->size == 0) return TRUE;

    /* Open both files */
	handle1 = fopen(file1->fileInfo->name, "rb");
	handle2 = fopen(file2->fileInfo->name, "rb");

    /* Return false if failed to open files */
    if (handle1 == NULL || handle2 == NULL) {
        fclose(handle1);
        fclose(handle2);
        return FALSE;
    }

    /* Clear the buffers */
    memset(buf1, 0, COMPARE_BUFFER_SIZE);
    memset(buf2, 0, COMPARE_BUFFER_SIZE);

    /* Compare the data */
    do {
        fread(buf1, 1, COMPARE_BUFFER_SIZE, handle1);
        fread(buf2, 1, COMPARE_BUFFER_SIZE, handle2);
        if (memcmp(buf1, buf2, COMPARE_BUFFER_SIZE) != 0) {
            fclose(handle1);
            fclose(handle2);
            return FALSE;
        }
    } while (!feof(handle1) || !feof(handle2));

    fclose(handle1);
    fclose(handle2);
    return TRUE;
}

/* Build file list */
int makeFileList(char *findFileSpec) {
	struct find_t fileInf;
    int minReqFilesOK = FALSE, complete;

    initFileList();

	if (_dos_findfirst(findFileSpec, _A_NORMAL, &fileInf) != 0) {
        doneFileList();
        return FALSE;
    }

    addFileInfo(&fileInf);

    do {
		complete = _dos_findnext(&fileInf);
        if (!complete) {
            minReqFilesOK = TRUE;
            addFileInfo(&fileInf);
        }
    } while (!complete);

    if (!minReqFilesOK) {
        doneFileList();
        return FALSE;
    }

    return TRUE;
}

/* Just what it says! */
void findAndKillZeroByteFiles(char *fileSpec) {
    llNodeFileInfo_t *tFile, *node;
    int ans;

    printf("\nSearching for zero byte files (%s)...\n", fileSpec);

    tFile = fileList->first;
    while (tFile) {
		printf("%s: ", tFile->fileInfo->name);
		if (tFile->fileInfo->size == 0) {
            printf("Zero byte file!\n");
            if (automatic) {
                ans = TRUE;
            }
            else {
                printf("Do you want to delete this file (y/n)?\7 ");
                ans = getche();
                printf("\n");
                ans = (ans == 'y' || ans == 'Y');
            }
            if (ans) {
				if (unlink(tFile->fileInfo->name) == -1) {
                    printf("Failed to delete file!\n");
                }
                else {
                    node = tFile->previous; /* cause that'll get the new node */
                    deleteFileInfo(tFile);
                    tFile = node;   /* this is the old node; got the new one */
                    printf("File deleted!\n");
                }
            }
            else {
                printf("File ignored.\n");
            }
        }
        else {
            printf("File not empty.\n");
        }
        tFile = tFile->next;
    }
}

/* Just what it says! */
void findAndKillCloneFiles(char *fileSpec) {
    llNodeFileInfo_t *tFile1, *tFile2, *node;
    int ans;

    printf("\nSearching for clone files (%s)...\n", fileSpec);

    tFile1 = fileList->first;
    while (tFile1) {
        tFile2 = tFile1->next;
        while (tFile2) {
			printf("Comparing %s and %s: ", tFile1->fileInfo->name, tFile2->fileInfo->name);
            if (fileEquals(tFile1, tFile2)) {
                printf("Clone!\n");
                if (automatic) {
					if ((tFile1->fileInfo->wr_date * 0x100L + tFile1->fileInfo->wr_time) < (tFile2->fileInfo->wr_date * 0x100L + tFile2->fileInfo->wr_time))
                        ans = '1';
                    else
                        ans = '2';
                }
                else {
					printf("Type \"1\" to delete %s, \"2\" to delete %s or \"3\" to ignore:\7 ", tFile1->fileInfo->name, tFile2->fileInfo->name);
                    ans = getche();
                    printf("\n");
                }
                if (ans == '1') {
					if (unlink(tFile1->fileInfo->name) == -1) {
                        printf("Failed to delete file!\n");
                    }
                    else {
                        node = tFile1->previous; /* cause that'll get the new node */
                        deleteFileInfo(tFile1);
                        tFile1 = node;  /* this is the old node; got the new one */
                        printf("File deleted!\n");
                    }
                }
                else if (ans == '2') {
					if (unlink(tFile2->fileInfo->name) == -1) {
                        printf("Failed to delete file!\n");
                    }
                    else {
                        node = tFile2->previous; /* cause that'll get the new node */
                        deleteFileInfo(tFile2);
                        tFile2 = node;  /* this is the old node; got the new one */
                        printf("File deleted!\n");
                    }
                }
                else {
                    printf("Files ignored.\n");
                }
            }
            else {
                printf("Different.\n");
            }
            tFile2 = tFile2->next;
        } 
        tFile1 = tFile1->next;
    }
}

/* Linked list debugging aid */
/*
void debugList(void) {

    llNodeFileInfo_t far *tFile;

    printf("\nListing all list details...\n\n");

    printf("Base list address: %p\n", fileList);
    printf("First node address: %p\n", fileList->first);
    printf("Last node address: %p\n", fileList->last);
    printf("Node length: %u\n\n", fileList->length);

    tFile = fileList->first;

    do {
        printf("Current node address: %p\n", tFile);
        printf("Current node previous address: %p\n", tFile->previous);
        printf("Current node next address: %p\n", tFile->next);
        printf("Current node file info block address: %p\n", tFile->fileInfo);
		printf("File related with current node: %s\n\n", tFile->fileInfo->name);
        tFile = tFile->next;
    } while (tFile);

}
*/

/* Program entry point */
int main(int argc, char *argv[]) {
	char fileSpec[_MAX_PATH];

	if (argc < 2 || strcmp(argv[1], "/?") == 0) {
		puts("\nZero byte and clone file cleaner. Version 2.1\n"
			"Copyright (c) Samuel Gomes (Blade), 2000-2003\n"
			"All rights reserved\n"
			"\nUsage: unclone [filespec] [/auto]\n"
			"\n[/auto] does not prompt; takes the best action\n"
			"[filespec] can be valid DOS wildcards\n"
			"\n"__FILE__" was compiled on "__DATE__" at "__TIME__"\n"
			);
		return EXIT_SUCCESS;
	}

	if (argc > 2) {
		automatic = (strcmp(strlwr(argv[2]), "/auto") == 0);
	}

	strcpy(fileSpec, argv[1]);

	if (automatic)
		printf("\nWorking in auto mode.\n");
	else
		printf("\nWorking in interactive mode.\n");

	printf("\nBuilding file list. Please wait...");
	if (!makeFileList(fileSpec)) {
		printf("failed!\n");
		return EXIT_FAILURE;
	}
	printf("done!\n");

	findAndKillZeroByteFiles(fileSpec);
	findAndKillCloneFiles(fileSpec);

    doneFileList();

    printf("\nAll done.\n");

    return EXIT_SUCCESS;
}
