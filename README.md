libspk
======

What?
-----
Well, I don't know. You've got to figure that one out for yourself.
If you don't want to, here's a quick overview:

This is a simple library for reading an even simpler package format and here's a rundown of its features:

 - Low resource usage (hopefully)
 - Allows for multiple instances
 - Each instance has multiple virtual file handles
 - No dynamic memory management
 - Virtual file interface mimics standard C fopen, fclose, fread, fseek and ftell functions
 - Pure C
 - Requires only standard C libraries and headers (stdio.h, stdint.h and string.h)
 - Should be fairly portable
 - Bug free.
 - Ha, almost got you there, haven't I?
 - Virtually untested
 - Open Source!?
 - Flat packages. None of that hierarchy rubbish.
 - Intended to be read-only

Why?
----
The low resource usage and static memory management make this library attractive in embedded fields.
One example is the development of PSX(PlayStation 1) games, since ISO9660 as implemented in the PSX BIOS (without extensions) only allows for a few files per folder and is very restrictive concerning the file names (8.3, uppercase only, ...).

This library allows for up to 255 files in one package, each having a filename with a maximum length that you can specify at compile time. (Warning: Even though you could turn the maximum file name size up to almost insane amounts, you should keep in mind that not only would every singe FAT entry and the header be of that size, so would the static cache be.)

The amount of concurrent virtual files can also be increased, but has similar consequences (without the package file size part).

The (hopefully) reasonable default values are 64 bytes for a FAT entry (giving effectively 61 characters for the file name) and 16 concurrent files.

Another restriction is the maximum amount of files(255). Increasing this would require slight modifications to the header (a second byte for the file size), but would also up the memory usage by an insane amount. (We're talking about the size of a single LUT entry * max. amount of files, since libspk doesn't use any dynamic memory allocation functions.)

How?
----
Usage should be pretty straight forward.
There are a few steps you should follow:

 1. Create a state struct (e.g. struct spkState state;)
 2. Initialize (clear) the struct to make sure all values are at a known state (spkInit(&state);)
 3. Open a package from the real file system (spkOpenArchive(&state, "package.spk");)
 4. Open a file from the package (struct spkFileHandle\* test = spkFopen(&state, "filename");)
 5. From here you use the spk equivalents of the usual file access (e.g. spkFread(somewhere, 1, 10, test);)
 6. Closing a virtual file is just as easy (spkFclose(test);)
 7. Closing the package isn't any harder (spkCloseArchive(state);)

Do I even wa-
-------------
Yes, you do.
