fatattr
=======

Small linux application to see or modify MSDOS attributes in a FAT file system.
Uses the ioctl calls of the linux kernel header msdos_fs.h to access and modify the MSDOS attributes.


Requirements
============

- SCons
- Clang
- Linux kernel :)


Installation
============
Type `scons` to build the program, the executable will be in `bin/fatattr`.
Type `scons -h` to see the build options available.


Usage
=====
Command syntax:
`fatattr [options] FILE1 ...`

Accepted options:
	+R: Sets the read-only attribute.
	-R: Remove the read-only attribute.
	+A: Sets the archive attribute.
	-A: Remove the archive attribute.
	+S: Sets the system attribute.
	-S: Remove the system attribute.
	+H: Sets the hidden attribute.
	-H: Remove the system attribute.
	+D: Sets the directory attribute (warning! see below).
	-D: Remove the directory attribute (warning! see below).
	+V: Sets the volume label attribute (warning! see below).
	-V: Remove the volume label attribute (warning! see below).
	--recursive: If FILE is a directory, process it recursively.
	--verbose: Verbose attribute changes.
	--help: Show this help.
	--version: Show only the program name, version and credits.
	--: Forces all arguments past this one to be interpreted as files.
If no attribute change is specified, the program prints the file(s) attributes.
Do NOT use the +D, -D, +V and -V options if you don't know EXACTLY what you are doing.
