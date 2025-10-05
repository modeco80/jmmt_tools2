---
author:
- Lily Tsuru
date: 'October 5, 2025'
title: jmpak(1)
---

# NAME

jmpak - extract, list JMMT package files.

# SYNOPSIS

**jmpak** *SUBCOMMAND* [*OPTIONS*...]

**jmpak** **e** **-s|--stdout** *PACKFILE* *FILENAME*

**jmpak** **x** **-d|--directory** *PACKFILE*

**jmpak** **l** *PACKFILE*

**jmpak** **L**

# DESCRIPTION

jmpak is a tool which allows extraction and listing of JMMT .pak files.

jmpak uses either the environment variable "JMMT_FS_PATH" or the current filesystem directory 
as the root path of the JMMT filesystem for it to work with.

# COMMAND SYNTAX

## EXTRACT FILE ('e')

**-s** is optional, and tells extraction to output the file to the standard output stream.

## EXTRACT DIRECTORY ('x')

**-d** is optional, and will default to the basename of the packfile name.

*PACKFILE* must be specified as a non-positional argument. It is the path of the packfile to extract.

## LIST PACKFILE ('l')

*PACKFILE* must be specified as a non-positional argument. It is the path of the packfile to list information and files from.

## LIST FILESYSTEM PACKFILES ('L')

No arguments are specified to this command.
