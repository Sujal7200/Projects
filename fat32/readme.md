# FAT32 File System Shell

This project involves implementing a user-space shell application to interpret and interact with a FAT32 file system image. The application provides a range of commands that simulate basic file system operations such as file manipulation, directory traversal, file reading, and more. It is designed to work with an in-memory representation of a FAT32 file system, allowing the user to interact with the file system without modifying the actual image until explicitly saved. 

The goal of this project is to provide a robust, command-line interface for managing and exploring FAT32 file system images. The application does not rely on any external libraries or system calls that would mount the image or use kernel-level operations. Instead, it interacts directly with the raw file system structure using the FAT32 specification, which includes concepts like the File Allocation Table (FAT), clusters, sectors, and other low-level disk structures.

The shell simulates a subset of commonly used file system commands in a manner similar to typical shell environments, making it intuitive for users familiar with terminal interfaces.

## Features

The shell supports the following commands:

- `open <filename>`: Opens a FAT32 image file. Displays an error if the file is already open or cannot be found.
- `close`: Closes the currently opened FAT32 image. Displays an error if no file is open.
- `save`: Saves the in-memory FAT32 image to the current working directory.
- `save <new filename>`: Saves the in-memory FAT32 image to a new file in the current working directory.
- `info`: Displays information about the file system in both hexadecimal and decimal formats.
- `stat <filename>`: Displays the attributes and starting cluster number of the specified file or directory.
- `get <filename>`: Retrieves a file from the FAT32 image and places it in the current working directory.
- `get <filename> <new filename>`: Retrieves a file and saves it with a new filename.
- `put <filename>`: Adds a file from the current working directory into the FAT32 image.
- `put <filename> <new filename>`: Adds a file with a new filename into the FAT32 image.
- `cd <directory>`: Changes the current working directory.
- `ls`: Lists the contents of the current directory.
- `read <filename> <position> <number of bytes> <OPTION>`: Reads a specified number of bytes from a file starting at a given position.
- `del <filename>`: Deletes a file from the FAT32 image.
- `undel <filename>`: Undeletes a previously deleted file from the FAT32 image.
- `quit` or `exit`: Exits the program.

## Program Overview

The FAT32 File System Shell is designed to simulate real-world file operations on a FAT32 disk image without the risk of damaging or corrupting the image. The program operates as a command-line interface where users can open, manipulate, and save file system images using commands that mimic those in traditional file systems. 

The shell ensures that no operation can result in the accidental modification of the original disk image until the user explicitly requests to save the image. This is achieved by maintaining an in-memory representation of the file system that can be freely modified while keeping the original file system image safe.

By implementing this program, you will gain an understanding of the internal structure of FAT32 file systems, including the File Allocation Table (FAT), clusters, and the use of hexadecimal and decimal values to interpret and manipulate disk structures. The project challenges you to work directly with low-level file system structures and develop a robust command-line tool that can perform a wide range of operations on a FAT32 disk image.

## Requirements

- **Programming Language**: C or C++
- **System Calls**: No usage of `system()`, `fork()`, or `exec()` system calls.
- **Disk Image**: A FAT32 disk image file `fat32.img` is provided for testing.
- **File Format**: The file system image must follow the FAT32 specification (found in `fatspec.pdf`).

## File System Information
The following information about the file system will be displayed when using the info command:
- BPB_BytesPerSec
- BPB_SecPerClus
- BPB_RsvdSecCnt
- BPB_NumFATS
- BPB_FATSz32
- BPB_ExtFlags
- BPB_RootClus
- BPB_FSInfo

## License
This project is licensed under the **Personal License**. Unauthorized copying, distribution, or commercial use of this code, in whole or in part, without explicit permission from the author, is strictly prohibited. For permissions or inquiries, please contact [sujalm7200@gmail.com](mailto:sujalm7200@gmail.com).
