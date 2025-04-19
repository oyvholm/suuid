<!-- README.md -->
<!-- File ID: 54fb7572-1b5b-11f0-8fac-83850402c3ce -->
# suuid

## Overview

**suuid** is a program for generating v1 UUIDs with timestamp and 
configurable MAC address. All generated UUIDs are stored as XML with 
various metadata for logging and documentation purposes.

### What is a v1 UUID?

A v1 UUID (Universally Unique Identifier) is a 128-bit identifier 
designed to be unique across time and space. It's generated using the 
current timestamp and the MAC address of the machine, ensuring no two 
UUIDs are identical, even in distributed systems. With `suuid`, these 
UUIDs include added metadata (e.g., timestamps, hostnames, and tags). 
This makes them ideal for tracking, logging, and organizing tasks across 
various applications.

An example of a valid v1 UUID is 
"`5a963834-1c76-11f0-be7b-83850402c3ce`".

### Features

- Generates RFC 4122 conforming v1 UUIDs with timestamp and MAC address
- Tags, comments, and custom metadata for UUIDs are supported
- Logs UUIDs of parent processes in the current log entry
- Portable C99 code with common Makefile for Linux and various BSD 
  variants
- Minimal dependencies, no extra C libraries needed

### Use cases

- **Tracking system events:**\
  Generate UUIDs with `suuid` for log entries to trace events across 
  servers or services, using tags to filter by category (e.g., "error" 
  or "update").

- **Backup management:**\
  Run `suuid -t backup -c "Pre-upgrade snapshot"` before a backup to 
  label and document it, simplifying recovery or auditing later. This 
  UUID can for example be stored as a label in a `.tar` or `.zip` file, 
  or added to the directory name.

- **Session-based workflows:**\
  Use `sess <command>` to start a session for a multi-step task, 
  ensuring all UUIDs generated are tied to that session's identifier. 
  For example, the command

  > `sess -c "Screen session where I write my book." screen -S book`

  starts a GNU Screen session, and all `suuid`-related activities inside 
  this session will be logged with the current and all parent session 
  UUIDs. This can be used for documenting time spent writing the book, 
  with automatic checksums of the files every time `v`(1) is terminated. 
  Because all these activities share a common session UUID, it's easy to 
  retrieve all entries related to the book.

- **File version tracking:**\
  Edit files with `v` to automatically log UUIDs and checksums, creating 
  a simple history of changes without a full version control system. If 
  all editing is done with this script, it will result in a complete log 
  of all the times this file has been edited on different computers.

  This solves problems like "I know I edited this file without 
  committing it some weeks ago, on some random computer in a temporary 
  directory somewhere".

### Status

The software is stable and has been used daily for over 16 years.

### Examples

- **`suuid`**\
  This command generates a single v1 UUID and prints it to stdout. 
  Behind the scenes, it logs metadata such as the timestamp embedded in 
  the UUID, the hostname, the current working directory, and any parent 
  process UUIDs. It's a quick way to get a unique identifier for 
  scripts, logs, or any task requiring a distinct marker.
- **`suuid -t backup -c "Backup before system upgrade"`**\
  Creates a UUID with custom metadata: a tag ("backup") and a comment. 
  This is useful for categorizing UUIDs or adding context, making them 
  easier to search or reference later.
- **`sess bash`**\
  Launches a new Bash shell with a unique session UUID stored in 
  `SESS_UUID`. Any `suuid` commands run within this shell will log the 
  session UUID, linking related operations.
- **`finduuid -u * | sortuuid`**\
  Searches all files in the current directory for UUIDs, removes 
  duplicates, and sorts them chronologically by their embedded 
  timestamps.
- **`v myfile.txt`**\
  Opens `myfile.txt` in your editor and, upon exit, logs the first or 
  last UUID found in the file along with metadata like file checksums. 
  This ties the file's content to a unique identifier, offering a 
  lightweight way to track changes or versions.
- **`conv-suuid -o sqlite --create-table *.xml | sqlite3 uuids.db`**\
  Import all XML files in the current directory into a SQLite database. 
  Postgres output is also available.

### Configuration options

Customize `suuid` with environment variables:

- **`SUUID_LOGDIR`:**\
  Set this to change where UUID logs are stored (default: `~/uuids/`). 
  Example: `export SUUID_LOGDIR=/var/log/suuid`.
- **`SUUID_HOSTNAME`:**\
  Override the default hostname stored with the UUID (useful for testing 
  or virtual environments).

### Metadata

This is an example of a log entry with the default metadata:

```xml
<suuid t="2025-04-18T17:17:40.1978380Z"
       u="0828820c-1c79-11f0-a1d1-83850402c3ce">
  <txt>Example for the man page.</txt>
  <host>hpelite2</host>
  <cwd>/home/sunny/src/git/suuid</cwd>
  <user>sunny</user>
  <tty>/dev/pts/14</tty>
  <sess desc="xterm">4abc48bc-13e9-11f0-954b-83850402c3ce</sess>
  <sess desc="screen">556aad62-13e9-11f0-b18d-83850402c3ce</sess>
</suuid>
```

The example is formatted for readability; all entries are stored using 1 
line to make grepping and sorting easier. Additional metadata can be 
added with the `--raw` option.

## Development

The `master` branch is considered stable; no unstable development 
happens there. Every new functionality or bug fix is created on topic 
branches which may be rebased now and then. All tests on `master` 
(executed with "make test") MUST succeed. If any test fails, it's 
considered a bug. Please report any failing tests in the issue tracker.

To ensure compatibility between versions, the program follows the 
Semantic Versioning Specification described at <http://semver.org>. 
Using the version number `X.Y.Z` as an example:

- `X` is the *major version*.
  This number is only incremented when backwards-incompatible changes 
  are introduced.
- `Y` is the *minor version*.
  Increased when new backwards-compatible features are added.
- `Z` is the *patch level*.
  Increased when new backwards-compatible bugfixes are added.

### Compiler flags for development

To avoid complications on various systems, the default build only uses 
`-Wall -O2`. Additional warning flags are enabled if any of the 
following conditions are met:

- The file `src/.devel` or `.git/.devel` exists
- The environment variable `DEVEL` is set to any value

These development flags can be explicitly disabled by setting the 
`NODEVEL` environment variable, regardless of the conditions above. The 
current `CFLAGS` can be checked with `make cflags`. For example:

- `make cflags`
- `make cflags DEVEL=1`
- `make cflags NODEVEL=true`

## `make` commands

### make / make all

Generate the `suuid` executable.

### make clean

Remove all generated files except `tags`.

### make edit

Open all files in the subtree in your favourite editor defined in 
`EDITOR`.

### make asm

Generate assembly code for all `.c` files. On many Unix-like systems, 
the assembly files are stored with a `.s` extension.

### make gcov

Generate test coverage with `gcov`(1). Should be as close to 100% as 
possible.

### make gcov-cmt / make gcov-cmt-clean

Add or remove `gcov` markers in the source code in lines that are not 
tested. Lines that are hard to test, for example full disk, full memory, 
long paths and so on, can be marked with the string `/* gncov */` to 
avoid marking them. To mark lines even when marked with gncov, set the 
GNCOV environment variable to a non-empty value. For example:

    make gcov-cmt GNCOV=1

These commands need the `gcov-cmt` script, available from 
<https://gitlab.com/oyvholm/utils/raw/master/gcov-cmt>.

### make gdb

Start gdb with main() as the default breakpoint, this is defined in 
`src/gdbrc`. Any additional gdb options can be added in `src/gdbopts`. 
An example would be "-tty /dev/\[...\]" to send the program output to 
another window.

### make install

`make install` installs `suuid` to the location defined by `PREFIX` in 
`src/Makefile`. Default location is `/usr/local`, but it can be 
installed somewhere else by specifying `PREFIX`. For example:

    make install PREFIX=~/local

### make tags

Generate `tags` file, used by Vim and other editors.

### make test

Run all tests. This command MUST NOT fail on purpose on `master`.

### make uninstall

Delete the installed version from `PREFIX`.

### make valgrind

Run all tests with Valgrind to find memory leaks and other problems. 
Should also not fail on master.

### Create HTML or PDF

`make html` creates HTML versions of all documentation in the current 
directory tree, and `make pdf` creates PDF versions. When executed in 
the `src/` directory, an HTML or PDF version of the man page is created, 
stored as `suuid.html` or `suuid.pdf`.

All `*.md` files can be converted to HTML or PDF by replacing the `.md` 
extension with `.html` or `.pdf`. For example, use `make README.html` to 
generate an HTML file from the `.md` file, or `make README.pdf` to 
create a PDF version. If the `.md` file is stored in Git, an extra 
footer with the text "Generated from *filename* revision *git-id* 
(*date*)" is added.

Uses `cmark`, available from <https://commonmark.org/>.

## Download

The main Git repository is stored at GitLab:

- URL: <https://gitlab.com/oyvholm/suuid>
- SSH clone: git@gitlab.com:oyvholm/suuid.git
- https clone: <https://gitlab.com/oyvholm/suuid.git>

## License

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or (at your 
option) any later version.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program. If not, see <http://www.gnu.org/licenses/>.

## Author

Øyvind A. Holm \<<sunny@sunbase.org>\>

## About this document

This file is written in [Commonmark](https://commonmark.org) and all 
`make` commands use `cmark`(1) to generate HTML and reformat text.

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", 
"SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this 
document are to be interpreted as described in RFC 2119.

-----

<!--
vim: set ts=2 sw=2 sts=2 tw=72 et fo=tcqw fenc=utf8 :
vim: set com=b\:#,fb\:-,fb\:*,n\:> ft=markdown :
-->
