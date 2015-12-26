# Tagger

## What is Tagger? ##
Tagger is a cross-platform lightweight command-line tool for tagging files and directories.

Its main features are:
* cross-platform (Windows, Linux, Mac OS, Unix)
* no alteration of the tagged files
* no tag naming restrictions
* no tag limit
* no dependencies
* no SQL (filesystem DB)
* instant backup (data stored in 1 directory)
* portable encoding (UTF-8 encoded data)
* easy import / export 


_Tagger program is written in C and released under the GNU Public License version 3._

	
Note: if you plan to work exclusively on Linux, consider as well the excellent [TMSU](http://http://tmsu.org/) project.

## How to use it? ##

### USAGE ###
<pre>
tagger [OPTION] OPERATION [PARAMETERS]
</pre>

### OPTIONS ###
<pre>
  --quiet       Suppress all normal output
  --debug       Output program trace and internal errors
  --help        Display this help text and exit
  --version     Display version information and exit
</pre>

### OPERATIONS ###

  
#### create ####
* **description**: Create one or more new tags
* **syntax**: tagger create TAG [TAG2 [TAG3 [...]]]
* **output**: n tag(s) successfully created, m tag(s) ignored
* **errors**: tag T already exists
* **examples**: 
<pre>
tagger create mp3	
</pre>

#### delete ####
* **description**: Delete one or more tags (all relations will be lost)
* **syntax**: tagger delete TAG [TAG2 [TAG3 [...]]]
* **output**: n tag(s) successfully deleted, m tag(s) ignored
* **errors**: tag T not found
* **examples**: 
<pre>
tagger delete mp3
tagger delete music/*
</pre>

#### rename ####
* **description**: Rename an existing tag
* **syntax**: tagger rename [TAG_OLDNAME] [TAG_NEWNAME]
* **output**: tag successfully renamed
* **errors**: tag T not found
* **examples**: 
<pre>
tagger rename mp3 music/mp3	
</pre>

#### merge ####
* **description**: Merge two tags (relations of each tag will be applied to both)
* **syntax**: tagger merge TAG1 TAG2
* **examples**: 
<pre>
tagger merge umsic music
</pre>


#### clone ####
* **description**: Create a new tag by copying all existin relations from another
* **syntax**: tagger clone TAG1 TAG2
* **examples**: 
<pre>
tagger clone blah.mp3 blahblah.mp3
</pre>	


#### tag ####
* **description**: Add(+) or removes(-) tag(s) to/from one or more files
* **note**: paramters order does not matter, any argument preceeded by + or - is considered as file
* **note**: if any given tag name does not exist yet, it is automatcaly created (a message will be displayed if the --debug option is set)
* **syntax**: tagger [tag] +|-TAG1 [[+|-TAG2] [...]] FILE1 [FILE2 [...]]
* **example**: 
<pre>
tagger +movie *.mp4
tagger +music +mp3 -movie test.mp3
</pre>	
	

#### files ####
* **description**: List files matching the given criterias
* **note**: if no query is specified, outputs all tagged files
* **syntax**: tagger files [QUERY]
* **query syntax**: "[!]TAG1 [[&|] [!]TAG2 [[&|] [!]TAG3]]"
* **examples**: 
<pre>
tagger files music/*
tagger files "music & !mp3"
</pre>


#### tags ####
* **description**: List all tags related to one or more files (no file means all tags)
* **note**: if no file is specified, outputs all existing tags
* **syntaxe**: tagger tags [FILE1 [FILE2 [...]]]
* **examples**: 
<pre>
tag tags
tag tags /home/ced/music/buddy_holly.mp3
tag tags *.mp3  
</pre>	
	



## Installation ##
1. Download the project from github.com

2. Copy the executable into the directory of your choice.
...Linux 32-bit version: bin/tagger.o (compiled/tested under Ubuntu)
...Windows 32-bit version: bin/tagger.exe (compiled/tested under Windows XP)

If you need to compile the source, refer to compilation section.
Note: program will need read/write access to home directory for storing/retrieving data.

## Compiling ##

Sources use standard C librairies (C99) + GNU C library (glibc).


### Linux ###

Note: compiling under Linux has been tested with gcc and libc6 (under Ubuntu 32-bit).

Use makefile in your root project directory to generate executable.

<pre>
cp makefile.lin makefile
make
../bin/tagger.o
</pre>

### Windows ###

Note: Compiling under win32 has been tested with TDM-GCC-32/MinGW32 (<http://www.mingw.org/download/installer>) and libgw32c-0.4 along with GnuWin32 tools (<http://gnuwin32.sourceforge.net/>).

Use/adapt the makefile in your root project directory to generate executable.

<pre>
copy makefile.win makefile
make
bin\tagger.exe
</pre>

## Author ##

Send any other comments, patches, flowers and suggestions to: Cédric Françoys <cedricfrancoys@gmail.com>