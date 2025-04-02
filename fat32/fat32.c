/*
 * Personal License
 *
 * Author: Sujal More
 * Project: fat32 File System Shell
 *
 * This code is provided solely for educational and
 * personal use. Unauthorized copying, distribution,
 * or commercial use of this code, in whole or in
 * part, without the explicit permission of the author
 * is strictly prohibited.
 *
 * For permissions or inquiries, please contact:
 * sujalm7002@gmail.com
 *
 * © 2025 Sujal More. All rights reserved.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>

#include <stdint.h>

// Values need to be read form the fat32.img
typedef struct
{
    char BS_OEMName[8];
    int16_t BPB_BytsPerSec;
    int8_t BPB_SecPerClus;
    int16_t BPB_RsvdSecCnt;
    int8_t BPB_NumFATs;
    int16_t BPB_RootEntCnt;
    char BS_VolLab[11];
    int32_t BPB_FATSz32;
    int16_t BPB_ExtFlags;

    int32_t BPB_RootClus;
    int16_t BPB_FSInfo;

    int32_t RootDirSectors;
    int32_t FirstDataSector;
    int32_t FirstSectorofCluster;
} fileSys;

// Directory Attribute Information
struct __attribute__((packed)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

// Global Entries

// Instance of fileSys as thw hole program might need the data
// Better to keep it global and reset as the file closes
fileSys fsInfo;

// Globat stats variables
// If True a System is already open
int fsIsOpen = 0;

// Directory stuct that contains all the directories
// Made global as, cd, stat, etc. need access to all of the directories in the FS
struct DirectoryEntry dir[16];

int32_t currentCluster;
int32_t cDcurrentCluster;
int32_t previousCluster;
int cdCalled = 0;

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 100

// Function Definitions
void errorFun(int lineNo);
void resetFileSysInfo();
void resetDirectoryEntrydir();
void printStatAttributes(uint8_t DIR_Attr);
void exitQuit(char **token, int token_count);
void otherCmds(char **token, int token_count, FILE *openFS);
FILE *openCmd(char **token, int token_count);
int LBAToOffset(int32_t sector);
int16_t NextLB(uint32_t sector, FILE *openFS);
uint32_t findFreeCluster(FILE *openFS);

// Error handling & Memory arranger functions
void errorFun(int lineNo)
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    printf("LINE NO: %d\n", lineNo);
}

void resetFileSysInfo()
{
    // Set all bytes of fsInfo to 0
    memset(&fsInfo, 0, sizeof(fsInfo));
}

void resetDirectoryEntrydir()
{
    // Set all bytes of dir  to 0
    memset(&dir, 0, sizeof(dir));
}

// Printing Functions
void printStatAttributes(uint8_t DIR_Attr)
{
    if (DIR_Attr & 0x01)
    {
        printf("Atrribute: 0x%x Type: Read-only\n", DIR_Attr);
    }

    if (DIR_Attr & 0x10)
    {
        printf("Atrribute: 0x%x Type: Directory\n", DIR_Attr);
    }

    if (DIR_Attr & 0x20)
    {
        printf("Atrribute: 0x%x Type: Archive\n", DIR_Attr);
    }
}

// Command Functions
void exitQuit(char **token, int token_count)
{
    // token_count >= 2 means there is something else as well along with the command
    if (strcmp(token[0], "exit") == 0)
    {
        if (token_count >= 2)
        {
            errorFun(73);
        }
        else
        {
            exit(0);
        }
    }
    // token_count >= 2 means there is something else as well along with the command
    if (strcmp(token[0], "quit") == 0)
    {
        if (token_count >= 2)
        {
            errorFun(85);
        }
        else
        {
            exit(0);
        }
    }
}

void otherCmds(char **token, int token_count, FILE *openFS)
{
    // info command
    if (strcmp(token[0], "info") == 0)
    {
        // Declare a sturct of type fileSys to get the fields
        // Used a struct to allow reuse of fields and no funky data

        printf("BPB_BytsPerSec:   	%d\t%x\n", fsInfo.BPB_BytsPerSec, fsInfo.BPB_BytsPerSec);
        printf("BPB_SecPerClus:  	  %d\t%x\n", fsInfo.BPB_SecPerClus, fsInfo.BPB_SecPerClus);
        printf("BPB_RsvdSecCnt:  	  %d\t%x\n", fsInfo.BPB_RsvdSecCnt, fsInfo.BPB_RsvdSecCnt);
        printf("BPB_NumFATs   :  	  %d\t%x\n", fsInfo.BPB_NumFATs, fsInfo.BPB_NumFATs);
        printf("BPB_FATSz32   :  	  %d\t%x\n", fsInfo.BPB_FATSz32, fsInfo.BPB_FATSz32);
        printf("BPB_ExtFlags  :  	  %d\t%x\n", fsInfo.BPB_ExtFlags, fsInfo.BPB_ExtFlags);
        printf("BPB_RootClus  :  	  %d\t%x\n", fsInfo.BPB_RootClus, fsInfo.BPB_RootClus);
        printf("BPB_FSInfo    :  	  %d\t%x\n", fsInfo.BPB_FSInfo, fsInfo.BPB_FSInfo);
    }
    // stat command
    else if (strcmp(token[0], "stat") == 0)
    {
        if (token_count < 2)
        {
            errorFun(228);
            printf("Error: File not provided. Example: stat foo.txt\n");
        }
        else
        {
            // Convert input into FOO TXT
            char *filename = token[1];

            char expanded_name[12];
            memset(expanded_name, ' ', 12);

            char *token = strtok(filename, ".");

            strncpy(expanded_name, token, strlen(token));

            token = strtok(NULL, ".");

            if (token)
            {
                strncpy((char *)(expanded_name + 8), token, strlen(token));
            }

            expanded_name[11] = '\0';

            int i;
            for (i = 0; i < 11; i++)
            {
                expanded_name[i] = toupper(expanded_name[i]);
            }

            int fileFound = 0;

            for (int i = 0; i < 16; i++)
            {
                if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                {
                    printStatAttributes(dir[i].DIR_Attr);

                    // The high and low are 16 bits each
                    // high is moved into the upper 16 bits of the 32-bit integer
                    // Bitwise | combine them together
                    uint32_t startingClus = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
                    printf("Starting Cluster: %u\n", startingClus);

                    fileFound = 1;
                }
            }

            if (!fileFound)
            {
                printf("Error: File not found\n");
            }
        }
    } // stat command
    // ls command
    else if (strcmp(token[0], "ls") == 0)
    {
        // ls .. command
        if (token_count == 2 && strcmp(token[1], "..") == 0)
        {
            int32_t lsTarggetClus;
            if (cdCalled == 0)
            {
                printf("ERROR: No parent directory found, already at root directory\n");
            }
            else if (cdCalled == 1)
            {
                if (cDcurrentCluster < fsInfo.BPB_RootClus)
                {
                    lsTarggetClus = fsInfo.BPB_RootClus;
                }
                else
                {
                    lsTarggetClus = cDcurrentCluster;
                }
            }

            struct DirectoryEntry dir[16];
            int found = 0;

            // Read directory entries from the current directory
            int offset = LBAToOffset(lsTarggetClus);
            fseek(openFS, offset, SEEK_SET);
            fread(&dir, sizeof(struct DirectoryEntry), 16, openFS);

            for (int j = 0; j < 16; j++)
            {
                if (dir[j].DIR_Name[0] == 0x00)
                {
                    // End of directory entries
                    break;
                }
                if (dir[j].DIR_Name[0] == 0xE5)
                {
                    // Deleted entry
                    continue;
                }
                if ((dir[j].DIR_Attr & ATTR_DIRECTORY) && !(dir[j].DIR_Attr & ATTR_HIDDEN))
                {
                    char formattedName[12];
                    memset(formattedName, 0, 12);
                    strncpy(formattedName, dir[j].DIR_Name, 11);

                    // Trim trailing spaces
                    for (int k = 0; k < 11; k++)
                    {
                        if (formattedName[k] == ' ')
                        {
                            formattedName[k] = '\0';
                            break;
                        }
                    }

                    // Check for the ".." directory entry and set targetCluster to the parent directory
                    if (strcmp(formattedName, "..") == 0)
                    {
                        lsTarggetClus = (dir[j].DIR_FirstClusterHigh << 16) | dir[j].DIR_FirstClusterLow;
                        // Debug
                        // printf("Found a parent directory for ls .. %s\n", dir[j].DIR_Name );
                        found = 1;
                        // cdCalled = 1;
                        break;
                    }
                }
            }

            if (!found)
            {
                printf("Error: Parent directory not found for ls ..\n");
                return;
            }

            // Check this beacuse FOLDERA .. takes us to cluster 0
            // which is past the root cluster
            if (lsTarggetClus < fsInfo.BPB_RootClus)
            {
                lsTarggetClus = fsInfo.BPB_RootClus;
            }

            // Read all the Directories as well as their repective information
            int dirOffset = LBAToOffset(lsTarggetClus);

            fseek(openFS, dirOffset, SEEK_SET);

            for (int i = 0; i < 16; i++)
            {
                // Read valid directory entries
                fread(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);
                dir[i].DIR_Name[12] = '\0';
            }
            for (int i = 0; i < 16; i++)
            {
                // Print only the needed ones

                // Skip empty or deleted entries
                if (dir[i].DIR_Name[0] == 0x00 || dir[i].DIR_Name[0] == 0xE5)
                {
                    continue;
                }

                // Skip system volume entries
                if (dir[i].DIR_Attr == 0x08)
                {
                    continue;
                }

                if (dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
                {
                    dir[i].DIR_Name[12] = '\0';
                    printf("%s\n", dir[i].DIR_Name);
                }
            }

        } // ls ..
        // ls and ls .
        else // if(strcmp(token[0], "ls") == 0)
        {
            if (cdCalled == 0)
            {
                for (int i = 0; i < 16; i++)
                {
                    // Print only the needed ones

                    // Skip empty or deleted entries
                    if (dir[i].DIR_Name[0] == 0x00 || dir[i].DIR_Name[0] == 0xE5 || dir[i].DIR_Name[0] == (char)0xE5)
                    {
                        continue;
                    }

                    // Skip system volume entries
                    if (dir[i].DIR_Attr == 0x08)
                    {
                        continue;
                    }

                    if (dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
                    {
                        dir[i].DIR_Name[12] = '\0';
                        printf("%s\n", dir[i].DIR_Name);
                    }
                }
            }
            else if (cdCalled == 1)
            {
                // Read all the Directories as well as their repective information
                int dirOffset = LBAToOffset(cDcurrentCluster);

                fseek(openFS, dirOffset, SEEK_SET);

                for (int i = 0; i < 16; i++)
                {
                    // Read valid directory entries
                    fread(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);
                    dir[i].DIR_Name[12] = '\0';
                }
                for (int i = 0; i < 16; i++)
                {
                    // Print only the needed ones

                    // Skip empty or deleted entries
                    if (dir[i].DIR_Name[0] == 0x00 || dir[i].DIR_Name[0] == 0xE5 || dir[i].DIR_Name[0] == (char)0xE5)
                    {
                        continue;
                    }

                    // Skip system volume entries
                    if (dir[i].DIR_Attr == 0x08)
                    {
                        continue;
                    }

                    if (dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
                    {
                        dir[i].DIR_Name[12] = '\0';
                        printf("%s\n", dir[i].DIR_Name);
                    }
                }
            }
        }
    } // ls command
    // get command
    else if (strcmp(token[0], "get") == 0)
    {
        if (token_count < 2 || token_count > 3)
        {
            errorFun(336);
            printf("Error: File not provided or excess files provided. Example: get <filename> or <filename>\n");
        }
        // PART 1: get <filename>
        else if (token_count == 2)
        {
            // Convert input into FOO TXT
            char *filename = token[1];
            char *outputFileName = strdup(token[1]);

            char expanded_name[12];
            memset(expanded_name, ' ', 12);

            char *token = strtok(filename, ".");

            strncpy(expanded_name, token, strlen(token));

            token = strtok(NULL, ".");

            if (token)
            {
                strncpy((char *)(expanded_name + 8), token, strlen(token));
            }

            expanded_name[11] = '\0';

            int i;
            for (i = 0; i < 11; i++)
            {
                expanded_name[i] = toupper(expanded_name[i]);
                // printf("%c", expanded_name[i]);
            }

            int fileFound = 0;

            for (int i = 0; i < 16; i++)
            {
                if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                {
                    // Debug
                    // printf("get <filename> file found\n");

                    // The high and low are 16 bits each
                    // high is moved into the upper 16 bits of the 32-bit integer
                    // Bitwise | combine them together
                    uint32_t startingClus = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
                    printf("Starting Cluster: %u\n", startingClus);

                    int32_t fileSize = dir[i].DIR_FileSize;

                    FILE *outputFile = fopen(outputFileName, "w");

                    while (fileSize >= 512)
                    {
                        int offset = LBAToOffset(startingClus);

                        fseek(openFS, offset, SEEK_SET);

                        char buffer[512];

                        // Get the min between filesize & sizeof(buffer)
                        // This handles the cases is fileze is greater we go into look if the buffer is greater
                        // it is directly copied
                        fread(buffer, 512, 1, openFS);
                        fwrite(buffer, 512, 1, outputFile);

                        // Decrease remaining file size
                        fileSize -= 512;
                        startingClus = NextLB(startingClus, openFS);
                    }

                    if (fileSize > 0)
                    {
                        int offset2 = LBAToOffset(startingClus);

                        fseek(openFS, offset2, SEEK_SET);

                        char buffer2[fileSize];

                        // Get the min between filesize & sizeof(buffer)
                        // This handles the cases is fileze is greater we go into look if the buffer is greater
                        // it is directly copied
                        fread(buffer2, fileSize, 1, openFS);
                        fwrite(buffer2, fileSize, 1, outputFile);
                    }

                    fclose(outputFile);

                    fileFound = 1;
                }
            }

            if (!fileFound)
            {
                printf("Error: File not found\n");
            }
        }
        // PART 2: get <filename> <new filename>
        else if (token_count == 3)
        {
            // Convert input into FOO TXT
            char *filename = token[1];
            char *outputFileName = strdup(token[2]);

            char expanded_name[12];
            memset(expanded_name, ' ', 12);

            char *token = strtok(filename, ".");

            strncpy(expanded_name, token, strlen(token));

            token = strtok(NULL, ".");

            if (token)
            {
                strncpy((char *)(expanded_name + 8), token, strlen(token));
            }

            expanded_name[11] = '\0';

            int i;
            for (i = 0; i < 11; i++)
            {
                expanded_name[i] = toupper(expanded_name[i]);
            }

            int fileFound = 0;

            for (int i = 0; i < 16; i++)
            {
                if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                {
                    // Debug
                    // printf("get <filename> file found\n");

                    // The high and low are 16 bits each
                    // high is moved into the upper 16 bits of the 32-bit integer
                    // Bitwise | combine them together
                    uint32_t startingClus = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
                    printf("Starting Cluster: %u\n", startingClus);

                    int32_t fileSize = dir[i].DIR_FileSize;

                    FILE *outputFile = fopen(outputFileName, "w");

                    while (fileSize >= 512)
                    {
                        int offset = LBAToOffset(startingClus);

                        fseek(openFS, offset, SEEK_SET);

                        char buffer[512];

                        // Get the min between filesize & sizeof(buffer)
                        // This handles the cases is fileze is greater we go into look if the buffer is greater
                        // it is directly copied
                        fread(buffer, 512, 1, openFS);
                        fwrite(buffer, 512, 1, outputFile);

                        // Decrease remaining file size
                        fileSize -= 512;
                        startingClus = NextLB(startingClus, openFS);
                    }

                    if (fileSize > 0)
                    {
                        int offset2 = LBAToOffset(startingClus);

                        fseek(openFS, offset2, SEEK_SET);

                        char buffer2[fileSize];

                        // Get the min between filesize & sizeof(buffer)
                        // This handles the cases is fileze is greater we go into look if the buffer is greater
                        // it is directly copied
                        fread(buffer2, fileSize, 1, openFS);
                        fwrite(buffer2, fileSize, 1, outputFile);
                    }

                    fclose(outputFile);

                    fileFound = 1;
                }
            }

            if (!fileFound)
            {
                printf("Error: File not found\n");
            }

        } // PART2: get <filename> <new filename>
    } // get command
    // put command
    else if (strcmp(token[0], "put") == 0)
    {
        if (token_count < 2 || token_count > 3)
        {
            errorFun(555);
            printf("Error: File not provided or excess files provided. Example: put <filename> or put <filename> <new filename>\n");
        }
        // PART 1:put <filename>
        else if (token_count == 2)
        {
            char *filetoWriteName = strdup(token[1]);
            FILE *fileToWrite = fopen(filetoWriteName, "r+");

            if (!fileToWrite)
            {
                printf("Error: Falied to open %s file to write to FAT32", filetoWriteName);
            }

            // Get the file size
            fseek(fileToWrite, 0, SEEK_END);
            uint32_t fileSize = ftell(fileToWrite);
            fseek(fileToWrite, 0, SEEK_SET);

            // Look for a free cluster
            uint32_t startingClus = findFreeCluster(openFS);

            if (startingClus == 0)
            {
                printf("Error: No free clusters in FAT32 Img\n");
                fclose(fileToWrite);
            }

            uint32_t currentCluster = startingClus;
            uint32_t nextClus;
            // uint8_t buffer[512];
            //  int bytesRead;
            //  uint32_t previousCluster = 0;

            while (fileSize >= 512)
            {
                int offset = LBAToOffset(currentCluster);

                fseek(openFS, offset, SEEK_SET);

                char buffer[512];

                // Get the min between filesize & sizeof(buffer)
                // This handles the cases is fileze is greater we go into look if the buffer is greater
                // it is directly copied
                fread(buffer, 512, 1, fileToWrite);
                fwrite(buffer, 512, 1, openFS);

                // Decrease remaining file size
                fileSize -= 512;

                nextClus = findFreeCluster(openFS);
                if (nextClus == (uint32_t)-1)
                {
                    printf("Error: Not enough free clusters for file data to write\n");
                    fclose(fileToWrite);
                }

                // Update the FAT to point to the next cluster
                fseek(openFS, (fsInfo.BPB_RsvdSecCnt * fsInfo.BPB_BytsPerSec) + (currentCluster * 4), SEEK_SET);
                fwrite(&nextClus, 4, 1, openFS);

                // Move to the next cluster in the chain
                currentCluster = nextClus;

                // startingClus = NextLB(startingClus, openFS);
            }

            if (fileSize > 0)
            {
                int offset2 = LBAToOffset(currentCluster);

                fseek(openFS, offset2, SEEK_SET);

                char buffer2[fileSize];

                // Get the min between filesize & sizeof(buffer)
                // This handles the cases is fileze is greater we go into look if the buffer is greater
                // it is directly copied
                fread(buffer2, fileSize, 1, fileToWrite);
                fwrite(buffer2, fileSize, 1, openFS);

                // Mark the end of the file by setting the last FAT entry to 0x0FFFFFFF
                uint32_t endCluster = 0x0FFFFFFF;
                fseek(openFS, (fsInfo.BPB_RsvdSecCnt * fsInfo.BPB_BytsPerSec) + (currentCluster * 4), SEEK_SET);
                fwrite(&endCluster, 4, 1, openFS);
            }

            fclose(fileToWrite);

            // fileFound = 1;
        }
        // PART 2:put <filename> <new filename>
        else if (token_count == 3)
        {
            char *filetoWriteName = strdup(token[1]);
            // char *newFileName = strdup(token[2]);
            FILE *fileToWrite = fopen(filetoWriteName, "r+");

            if (!fileToWrite)
            {
                printf("Error: Falied to open %s file to write to FAT32", filetoWriteName);
            }

            // Get the file size
            fseek(fileToWrite, 0, SEEK_END);
            uint32_t fileSize = ftell(fileToWrite);
            fseek(fileToWrite, 0, SEEK_SET);

            // Look for a free cluster
            uint32_t startingClus = findFreeCluster(openFS);

            if (startingClus == -1)
            {
                printf("Error: No free clusters in FAT32 Img\n");
                fclose(fileToWrite);
            }

            uint32_t currentCluster = startingClus;
            uint32_t nextClus;
            // uint8_t buffer[512];
            //  int bytesRead;
            //  uint32_t previousCluster = 0;

            while (fileSize >= 512)
            {
                int offset = LBAToOffset(currentCluster);

                fseek(openFS, offset, SEEK_SET);

                char buffer[512];

                // Get the min between filesize & sizeof(buffer)
                // This handles the cases is fileze is greater we go into look if the buffer is greater
                // it is directly copied
                fread(buffer, 512, 1, fileToWrite);
                fwrite(buffer, 512, 1, openFS);

                // Decrease remaining file size
                fileSize -= 512;

                nextClus = findFreeCluster(openFS);
                if (nextClus == (uint32_t)-1)
                {
                    printf("Error: Not enough free clusters for file data to write\n");
                    fclose(fileToWrite);
                }

                // Update the FAT to point to the next cluster
                fseek(openFS, (fsInfo.BPB_RsvdSecCnt * fsInfo.BPB_BytsPerSec) + (currentCluster * 4), SEEK_SET);
                fwrite(&nextClus, 4, 1, openFS);

                // Move to the next cluster in the chain
                currentCluster = nextClus;

                // startingClus = NextLB(startingClus, openFS);
            }

            if (fileSize > 0)
            {
                int offset2 = LBAToOffset(currentCluster);

                fseek(openFS, offset2, SEEK_SET);

                char buffer2[fileSize];

                // Get the min between filesize & sizeof(buffer)
                // This handles the cases is fileze is greater we go into look if the buffer is greater
                // it is directly copied
                fread(buffer2, fileSize, 1, fileToWrite);
                fwrite(buffer2, fileSize, 1, openFS);

                // Mark the end of the file by setting the last FAT entry to 0x0FFFFFFF
                uint32_t endCluster = 0x0FFFFFFF;
                fseek(openFS, (fsInfo.BPB_RsvdSecCnt * fsInfo.BPB_BytsPerSec) + (currentCluster * 4), SEEK_SET);
                fwrite(&endCluster, 4, 1, openFS);
            }

            fclose(fileToWrite);
        }

    } // PART 1: put <filename>
    // cd command
    else if (strcmp(token[0], "cd") == 0)
    {
        if (token_count < 2)
        {
            errorFun(789);
            printf("Error: File not provided for cd\n");
        }
        else if (token_count == 2)
        {
            int32_t targetCluster;
            if (cdCalled == 0)
            {
                targetCluster = fsInfo.BPB_RootClus;
            }
            else if (cdCalled == 1)
            {
                if (cDcurrentCluster < fsInfo.BPB_RootClus)
                {
                    targetCluster = fsInfo.BPB_RootClus;
                }
                else
                {
                    targetCluster = cDcurrentCluster;
                }
            }

            if (strcmp(token[1], "/") == 0)
            {
                targetCluster = fsInfo.BPB_RootClus;
                if (token_count == 2)
                {
                    cDcurrentCluster = targetCluster;
                    // printf("Changed to root directory\n");
                    return;
                }
            }
            // If "cd ..", move to parent directory
            else if (token_count == 2 && strcmp(token[1], "..") == 0)
            {
                struct DirectoryEntry dir[16];
                int found = 0;

                // Read directory entries from the current directory
                int offset = LBAToOffset(targetCluster);
                fseek(openFS, offset, SEEK_SET);
                fread(&dir, sizeof(struct DirectoryEntry), 16, openFS);

                for (int j = 0; j < 16; j++)
                {
                    if (dir[j].DIR_Name[0] == 0x00)
                    {
                        // End of directory entries
                        break;
                    }
                    if (dir[j].DIR_Name[0] == 0xE5)
                    {
                        // Deleted entry
                        continue;
                    }
                    if ((dir[j].DIR_Attr & ATTR_DIRECTORY) && !(dir[j].DIR_Attr & ATTR_HIDDEN))
                    {
                        char formattedName[12];
                        memset(formattedName, 0, 12);
                        strncpy(formattedName, dir[j].DIR_Name, 11);

                        // Trim trailing spaces
                        for (int k = 0; k < 11; k++)
                        {
                            if (formattedName[k] == ' ')
                            {
                                formattedName[k] = '\0';
                                break;
                            }
                        }

                        // Check for the ".." directory entry and set targetCluster to the parent directory
                        if (strcmp(formattedName, "..") == 0)
                        {
                            targetCluster = (dir[j].DIR_FirstClusterHigh << 16) | dir[j].DIR_FirstClusterLow;
                            // printf("Trying to chenge to parent directory %s\n", dir[j].DIR_Name );
                            found = 1;
                            cdCalled = 1;
                            break;
                        }
                    }
                }

                if (!found)
                {
                    printf("Error: Parent directory not found\n");
                    return;
                }

                cDcurrentCluster = targetCluster;
                // Check this beacuse FOLDERA .. takes us to cluster 0
                // which is past the root cluster
                if (cDcurrentCluster < fsInfo.BPB_RootClus)
                {
                    cDcurrentCluster = fsInfo.BPB_RootClus;
                }
                // Debug
                //  printf("Changed to parent directory \n");
                return;
            }
            else
            {
                // //Convert input into FOO TXT
                char *dirName = strdup(token[1]);

                // Convert to uppercase
                for (int i = 0; dirName[i] != '\0'; i++)
                {
                    dirName[i] = toupper((unsigned char)dirName[i]);
                }

                for (int i = 1; i < token_count; i++)
                {
                    struct DirectoryEntry dir[16];
                    int found = 0;

                    // Read directory entries from the target cluster
                    int offset = LBAToOffset(targetCluster);
                    fseek(openFS, offset, SEEK_SET);
                    fread(&dir, sizeof(struct DirectoryEntry), 16, openFS);

                    for (int j = 0; j < 16; j++)
                    {
                        if (dir[j].DIR_Name[0] == 0x00)
                        {
                            // End of directory entries
                            break;
                        }
                        if (dir[j].DIR_Name[0] == 0xE5)
                        {
                            // Deleted entry
                            continue;
                        }
                        // Check if a Directory and not hidden
                        if ((dir[j].DIR_Attr & ATTR_DIRECTORY) && !(dir[j].DIR_Attr & ATTR_HIDDEN))
                        {
                            char formattedName[12];
                            memset(formattedName, 0, 12);
                            strncpy(formattedName, dir[j].DIR_Name, 11);

                            // Trim trailing spaces
                            for (int k = 0; k < 11; k++)
                            {
                                if (formattedName[k] == ' ')
                                {
                                    formattedName[k] = '\0';
                                    break;
                                }
                            }

                            // Check if the current token matches the directory name
                            if (strcmp(formattedName, dirName) == 0)
                            {
                                // Update targetCluster to the directory's starting cluster
                                targetCluster = (dir[j].DIR_FirstClusterHigh << 16) | dir[j].DIR_FirstClusterLow;
                                found = 1;
                                cdCalled = 1;
                                break;
                            }
                        }
                    }

                    if (!found)
                    {
                        printf("Error: Directory %s not found\n", token[i]);
                        return;
                    }
                }

                // Update the current cluster if the path traversal is successful
                cDcurrentCluster = targetCluster;
                // Debug
                //  printf("Changed directory to ");
                //  for (int i = 1; i < token_count; i++)
                //  {
                //    printf("/%s", token[i]);
                //  }
                //  printf("\n");
            }
        }

    } // cd command
    // read command
    else if (strcmp(token[0], "read") == 0)
    {
        if (token_count < 4 || token_count > 5)
        {
            printf("ERROR: Invalid use of read. Example: read <filename> <position> <number of bytes> <OPTION>\n");
        }
        else if (token_count == 4 || token_count == 5)
        {

            char *readFileName = strdup(token[1]);
            int position = atoi(token[2]);
            int bytesToRead = atoi(token[3]);
            char *option = NULL;

            if (token_count == 5)
            {
                option = strdup(token[4]);
            }

            // Convert input into FOO TXT
            char expanded_name[12];
            memset(expanded_name, ' ', 12);

            char *token = strtok(readFileName, ".");

            strncpy(expanded_name, token, strlen(token));

            token = strtok(NULL, ".");

            if (token)
            {
                strncpy((char *)(expanded_name + 8), token, strlen(token));
            }

            expanded_name[11] = '\0';

            int i;
            for (i = 0; i < 11; i++)
            {
                expanded_name[i] = toupper(expanded_name[i]);
                // printf("%c", expanded_name[i]);
            }

            // In a diff directory than root directory
            if (cdCalled == 1)
            {
                struct DirectoryEntry dir[16];
                int fileFound = 0;

                int offset = LBAToOffset(cDcurrentCluster);

                fseek(openFS, offset, SEEK_SET);
                fread(&dir, sizeof(struct DirectoryEntry), 16, openFS);

                for (int i = 0; i < 16; i++)
                {
                    if (dir[i].DIR_Name[0] == 0x00)
                    {
                        break;
                    }
                    if (dir[i].DIR_Name[0] == 0xE5)
                    {
                        continue;
                    }

                    char formattedName[12];
                    memset(formattedName, 0, 12);
                    strncpy(formattedName, dir[i].DIR_Name, 11);

                    // Trim trailing spaces
                    for (int j = 0; j < 11; j++)
                    {
                        if (formattedName[j] == ' ')
                        {
                            formattedName[j] = '\0';
                            break;
                        }
                    }

                    if (strcmp(formattedName, expanded_name) == 0)
                    {
                        // Debug
                        //  printf("read <filename> file found\n");

                        // The high and low are 16 bits each
                        // high is moved into the upper 16 bits of the 32-bit integer
                        // Bitwise | combine them together
                        uint32_t startingClus = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
                        printf("Starting Cluster: %u\n", startingClus);

                        int32_t fileSize = dir[i].DIR_FileSize;

                        // Check we are not excedding fileAize limit
                        if (position >= fileSize)
                        {
                            printf("Error: Position exceeds file size.\n");
                            return;
                        }

                        // Gets the start position of the file
                        int offset = LBAToOffset(startingClus);

                        // fseek() to start + postion we want to go
                        fseek(openFS, offset + position, SEEK_SET);

                        uint8_t buffer[bytesToRead];

                        fread(buffer, bytesToRead, 1, openFS);

                        for (int k = 0; k < bytesToRead; k++)
                        {
                            if (option == NULL || strcmp(option, "-hex") == 0)
                            {
                                // Print as hexadecimal
                                printf("0x%02X ", buffer[k]);
                            }
                            else if (strcmp(option, "-ascii") == 0)
                            {
                                // Print as ASCII character
                                printf("%c", buffer[k] >= 32 && buffer[k] <= 126 ? buffer[k] : '.');
                            }
                            else if (strcmp(option, "-dec") == 0)
                            {
                                // Print as decimal integer
                                printf("%d ", buffer[k]);
                            }
                            else
                            {
                                printf("Error: Invalid option provided.\n");
                                return;
                            }
                        }
                        printf("\n");
                        return;

                        fileFound = 1;
                    }
                }

                if (!fileFound)
                {
                    printf("Error: File not found\n");
                }

            } // cd == 1
            // We are in root directory
            else if (cdCalled == 0)
            {
                int fileFound = 0;
                for (int i = 0; i < 16; i++)
                {
                    if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                    {
                        // Debug
                        //  printf("read <filename> file found\n");

                        // The high and low are 16 bits each
                        // high is moved into the upper 16 bits of the 32-bit integer
                        // Bitwise | combine them together
                        uint32_t startingClus = (dir[i].DIR_FirstClusterHigh << 16) | dir[i].DIR_FirstClusterLow;
                        printf("Starting Cluster: %u\n", startingClus);

                        int32_t fileSize = dir[i].DIR_FileSize;

                        // Check we are not excedding fileAize limit
                        if (position >= fileSize)
                        {
                            printf("Error: Position exceeds file size.\n");
                            return;
                        }

                        // Gets the start position of the file
                        int offset = LBAToOffset(startingClus);

                        // fseek() to start + postion we want to go
                        fseek(openFS, offset + position, SEEK_SET);

                        uint8_t buffer[bytesToRead];

                        fread(buffer, bytesToRead, 1, openFS);

                        for (int k = 0; k < bytesToRead; k++)
                        {
                            if (option == NULL || strcmp(option, "-hex") == 0)
                            {
                                // Print as hexadecimal
                                printf("0x%02X ", buffer[k]);
                            }
                            else if (strcmp(option, "-ascii") == 0)
                            {
                                // Print as ASCII character
                                printf("%c", buffer[k] >= 32 && buffer[k] <= 126 ? buffer[k] : '.');
                            }
                            else if (strcmp(option, "-dec") == 0)
                            {
                                // Print as decimal integer
                                printf("%d ", buffer[k]);
                            }
                            else
                            {
                                printf("Error: Invalid option provided.\n");
                                return;
                            }
                        }
                        printf("\n");
                        return;

                        fileFound = 1;
                    }
                }

                if (!fileFound)
                {
                    printf("Error: File not found\n");
                }
            } // cd == 0

        } // read with and without options
    } // read command
    // del command
    else if (strcmp(token[0], "del") == 0)
    {
        if (token_count != 2)
        {
            printf("ERROR: Invalid command for del, no filename provided. Example: del <filename>");
        }
        else if (token_count == 2)
        {
            // Convert input into FOO TXT
            char *delFilename = strdup(token[1]);
            // char *outputFileName = strdup(token[1]);

            char expanded_name[12];
            memset(expanded_name, ' ', 12);

            char *token = strtok(delFilename, ".");

            strncpy(expanded_name, token, strlen(token));

            token = strtok(NULL, ".");

            if (token)
            {
                strncpy((char *)(expanded_name + 8), token, strlen(token));
            }

            expanded_name[11] = '\0';

            int i;
            for (i = 0; i < 11; i++)
            {
                expanded_name[i] = toupper(expanded_name[i]);
                // printf("%c", expanded_name[i]);
            }

            // Already in root directory, just search the global dir[16]
            if (cdCalled == 0)
            {
                int fileFound = 0;

                for (int i = 0; i < 16; i++)
                {
                    if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                    {
                        // Debug
                        //  printf("del <filename> file found\n");
                        fileFound = 1;

                        if (dir[i].DIR_Name[0] == 0xE5)
                        {
                            printf("Error: File already deleted\n");
                            break;
                        }
                        else
                        {
                            dir[i].DIR_Name[0] = 0xE5;

                            int offset = LBAToOffset(cDcurrentCluster);
                            fseek(openFS, offset, SEEK_SET);
                            uint8_t buffer[sizeof(struct DirectoryEntry)];
                            // struct DirectoryEntry buffer;
                            memcpy(buffer, &dir[i], sizeof(struct DirectoryEntry));

                            fwrite(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);
                            fflush(openFS);

                            break;
                        }
                    }
                }
                if (!fileFound)
                {
                    printf("Error: File not found\n");
                }

            } // cd == 0
            // Not in root somewhere else
            else if (cdCalled == 1)
            {
                struct DirectoryEntry dir[16];
                int fileFound = 0;

                int dirOffset = LBAToOffset(cDcurrentCluster);

                fseek(openFS, dirOffset, SEEK_SET);

                fread(&dir, sizeof(struct DirectoryEntry), 16, openFS);

                for (int i = 0; i < 16; i++)
                {
                    if (strncmp(expanded_name, dir[i].DIR_Name, 11) == 0)
                    {
                        // Debug
                        //  printf("del <filename> file found\n");

                        if (dir[i].DIR_Name[0] == 0xE5)
                        {
                            printf("Error: File already deleted\n");
                            break;
                        }
                        else
                        {
                            dir[i].DIR_Name[0] = 0xE5;

                            int offset = LBAToOffset(cDcurrentCluster);
                            fseek(openFS, offset, SEEK_SET);
                            uint8_t buffer[sizeof(struct DirectoryEntry)];
                            // struct DirectoryEntry buffer;
                            memcpy(buffer, &dir[i], sizeof(struct DirectoryEntry));

                            fwrite(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);
                            fflush(openFS);

                            break;
                        }
                    }
                }
                if (!fileFound)
                {
                    printf("Error: File not found\n");
                }
            } // cd == 1
        }
    } // del command
    // undel command
    else if (strcmp(token[0], "undel") == 0)
    {
        if (token_count != 2)
        {
            printf("ERROR: Invalid command for del, no filename provided. Example: undel <filename>");
        }
        else if (token_count == 2)
        {
            if (cdCalled == 0)
            {
                // Convert input into FOO TXT
                char *delFilename = strdup(token[1]);
                int fileFound = 0;
                // char *outputFileName = strdup(token[1]);

                char expanded_name[12];
                memset(expanded_name, ' ', 12);

                char *token = strtok(delFilename, ".");

                strncpy(expanded_name, token, strlen(token));

                token = strtok(NULL, ".");

                if (token)
                {
                    strncpy((char *)(expanded_name + 8), token, strlen(token));
                }

                expanded_name[11] = '\0';

                int i;
                for (i = 0; i < 11; i++)
                {
                    expanded_name[i] = toupper(expanded_name[i]);
                    // printf("%c", expanded_name[i]);
                }

                for (int i = 0; i < 16; i++)
                {
                    // Check if the entry is marked as deleted (first byte is 0xE5)
                    if (dir[i].DIR_Name[0] == (char)0xE5) //|| dir[i].DIR_Name[0] == 0xE5)
                    {
                        char formattedName[12];
                        memset(formattedName, 0, 12);
                        strncpy(formattedName, dir[i].DIR_Name, 11);
                        formattedName[0] = expanded_name[0];

                        // Compare the formatted name to the given filename (excluding first character)
                        if (strncmp(formattedName + 1, expanded_name + 1, 10) == 0)
                        {
                            // Restore the first byte to its original value
                            dir[i].DIR_Name[0] = expanded_name[0];

                            int offset = LBAToOffset(fsInfo.BPB_RootClus) + (i * sizeof(struct DirectoryEntry));
                            fseek(openFS, offset, SEEK_SET);
                            fwrite(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);

                            // printf("File %s recovered successfully.\n", expanded_name);
                            fileFound = 1;
                            break;
                        }
                    }
                }
                if (!fileFound)
                {
                    printf("Error: File not found\n");
                }
            }
        }
        // cd == 0
    } // undel command
    // Debug
    else if (strcmp(token[0], "aa") == 0)
    {
        for (int i = 0; i < 16; i++)
        {
            if (dir[i].DIR_Attr == 0x01 || dir[i].DIR_Attr == 0x10 || dir[i].DIR_Attr == 0x20)
            {
                printf("%s\tSize: %u\n", dir[i].DIR_Name, dir[i].DIR_FileSize);
            }
        }
    }
}

FILE *openCmd(char **token, int token_count)
{

    // Check is the File Name has multiple spaces, if so print errror and go back to main
    int i = 0;
    int spacepresent = 0;

    for (i = 1; i < token_count; i++)
    {
        if (token[i] == NULL)
        {
            // printf("The FileName has spaces, please input a valid name. Example: fat32.img\n");
            spacepresent = 1;
            break;
        }
    }

    // Validate there is a File Provided and open is not by itself
    if (token_count < 2)
    {
        errorFun(123);
        printf("Error: File system image not provided\n");
        return NULL;
    }
    // Checkpoint to Validate File Name for spaces
    else if (spacepresent == 1)
    {
        printf("The FileName has spaces, please input a valid name. Example: fat32.img\n");
        return NULL;
    }
    else
    {

        // Check before opening if File System is already open
        if (fsIsOpen == 1)
        {
            printf("Error: File system image already open\n");
            return NULL;
        }
        else
        {
            FILE *openFS = fopen(token[1], "r+");

            if (openFS == NULL)
            {
                printf("Error: File system image not found or Invalid name. Example: fat32.img\n\n");
                return openFS;
            }

            if (openFS != NULL)
            {
                // File system is open
                fsIsOpen = 1;

                // Read data as soon as opened to make sure that every command has access
                // to calculate addresses to fseek() to.

                // For info command
                fseek(openFS, 11, SEEK_SET);
                fread(&fsInfo.BPB_BytsPerSec, 2, 1, openFS);

                fseek(openFS, 13, SEEK_SET);
                fread(&fsInfo.BPB_SecPerClus, 1, 1, openFS);

                fseek(openFS, 14, SEEK_SET);
                fread(&fsInfo.BPB_RsvdSecCnt, 2, 1, openFS);

                fseek(openFS, 16, SEEK_SET);
                fread(&fsInfo.BPB_NumFATs, 1, 1, openFS);

                fseek(openFS, 36, SEEK_SET);
                fread(&fsInfo.BPB_FATSz32, 4, 1, openFS);

                fseek(openFS, 40, SEEK_SET);
                fread(&fsInfo.BPB_ExtFlags, 2, 1, openFS);

                fseek(openFS, 44, SEEK_SET);
                fread(&fsInfo.BPB_RootClus, 4, 1, openFS);

                fseek(openFS, 48, SEEK_SET);
                fread(&fsInfo.BPB_FSInfo, 2, 1, openFS);

                // Other attributes to fill the struct
                fseek(openFS, 3, SEEK_SET);
                fread(&fsInfo.BS_OEMName, 8, 1, openFS);

                fseek(openFS, 17, SEEK_SET);
                fread(&fsInfo.BPB_RootEntCnt, 2, 1, openFS);

                fseek(openFS, 71, SEEK_SET);
                fread(&fsInfo.BS_VolLab, 11, 1, openFS);

                // Read all the Directories as well as their repective information
                int dirOffset = LBAToOffset(fsInfo.BPB_RootClus);

                fseek(openFS, dirOffset, SEEK_SET);

                for (int i = 0; i < 16; i++)
                {
                    // Read valid directory entries
                    fread(&dir[i], sizeof(struct DirectoryEntry), 1, openFS);
                    dir[i].DIR_Name[12] = '\0';
                }

                // Debug
                //  printf("Successfully opened %s\n", token[1]);
                return openFS;
            }
        }
        // reading
    }
    // open command
    // This should not fire but if it does some error
    return NULL;
}

// Directory Attribute Functions
// To convert a logical block address to an offset
int LBAToOffset(int32_t sector)
{
    return ((sector - 2) * fsInfo.BPB_BytsPerSec) + (fsInfo.BPB_BytsPerSec * fsInfo.BPB_RsvdSecCnt) + (fsInfo.BPB_NumFATs * fsInfo.BPB_FATSz32 * fsInfo.BPB_BytsPerSec);
}

// Function to get the next logical block address
int16_t NextLB(uint32_t sector, FILE *openFS)
{
    uint32_t FATAddress = (fsInfo.BPB_BytsPerSec * fsInfo.BPB_RsvdSecCnt) + (sector * 4);
    int16_t val;
    fseek(openFS, FATAddress, SEEK_SET);
    fread(&val, 2, 1, openFS);
    return val;
}

// Find the free cluster for put command
uint32_t findFreeCluster(FILE *openFS)
{
    // Calculate the start address of the FAT table (FAT #1)
    uint32_t FATStart = fsInfo.BPB_RsvdSecCnt * fsInfo.BPB_BytsPerSec;

    // Total FAT size (all FAT tables, not just the first one)
    uint32_t FATSize = fsInfo.BPB_NumFATs * fsInfo.BPB_FATSz32 * fsInfo.BPB_BytsPerSec;

    // The number of clusters in the FAT32 system (each entry represents one cluster)
    uint32_t clustersInFAT = FATSize / 32; // 4 bytes per cluster entry

    uint32_t cluster;
    int32_t FATValue;

    // Iterate through the FAT entries to find a free cluster (value 0x00000000 means free)
    for (cluster = 0; cluster < clustersInFAT; cluster++)
    {
        // Seek to the FAT entry for the current cluster
        fseek(openFS, FATStart + (cluster * 4), SEEK_SET);
        fread(&FATValue, 4, 1, openFS); // Read 4 bytes (32-bit entry)

        // Check if the entry is free (-1 means the cluster is free)
        if (FATValue == -1)
        {
            // Debug
            //  printf("Cluster : %u\n", cluster);
            return cluster; // Return the index of the first free cluster found
        }
    }

    // No free clusters found, return -1 to indicate the file system is full
    return (uint32_t)-1;
}

int main(int argc, char *argv[])
{
    ////////////////////////  MFS> PROMPT /////////////////////////

    // Tokenize the input and run interactive mode
    char *cmdStr = (char *)malloc(MAX_COMMAND_SIZE);

    while (1)
    {
        // Print out the msh prompt
        printf("mfs> ");

        // Read the command from the commandi line.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something.
        while (!fgets(cmdStr, MAX_COMMAND_SIZE, stdin))
            ;

        // Skip leading spaces/tabs
        cmdStr += strspn(cmdStr, WHITESPACE);

        // If the command is empty after trimming, continue to next iteration
        if (strlen(cmdStr) == 0)
        {
            continue;
        }
        else
        {
            int len = strlen(cmdStr);
            for (int i = len - 1; i >= 0; i--)
            {
                if (cmdStr[i] == '\0')
                {
                    break;
                }
                else if ((cmdStr[i] == ' ') || (cmdStr[i] == '\t') || (cmdStr[i] == '\n'))
                {
                    cmdStr[i] = '\0';
                }
                else
                {
                    break;
                }
            }
        }

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *argument_pointer;

        char *working_string = strdup(cmdStr);

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end

        char *head_ptr = working_string;

        // Tokenize the input with whitespace used as the delimiter
        while (((argument_pointer = strsep(&working_string, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(argument_pointer, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }

            token_count++;
        }

        FILE *openFS;

        // Check if it's a built-in command or other command
        // Call appropriate function
        if (token[0] != NULL)
        {
            // if(token_count > 30)
            // {
            //   printf("Too long input must be withing 30 characters")
            // }
            if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0)
            {
                exitQuit(token, token_count);
            }
            else // if(strcmp(token[0], "open") == 0 || strcmp(token[0], "close") == 0)
            {
                if (strcmp(token[0], "open") == 0)
                {
                    openFS = openCmd(token, token_count);
                }
                else if (strcmp(token[0], "close") == 0)
                {
                    if (fsIsOpen != 1)
                    {
                        printf("Error: File system not open.\n");
                    }
                    else
                    {
                        fclose(openFS);
                        openFS = NULL;
                        fsIsOpen = 0;
                        // Reset all read in data as everything is read as soon as we open the FS
                        resetFileSysInfo();
                        resetDirectoryEntrydir();
                        // Debug
                        //  printf("File Successfully Closed\n");
                    }
                }
                else
                {
                    // Make sure that a File System is open before executing any other commands
                    // Also handles the case after close() is called; No other command other than
                    // openn, exit and quit allowed
                    if (fsIsOpen == 1)
                    {
                        otherCmds(token, token_count, openFS);
                    }
                    else
                    {
                        printf("Error: File system image must be opened first.\n");
                    }
                }
            }
        }

        free(head_ptr);

    } // mfs prompt while loop while(1)

    return 0;
}
