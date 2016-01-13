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
* *description*: Create one or more new tags
* *syntax*: tagger create TAG [TAG2 [TAG3 [...]]]
* *output*: n tag(s) successfully created, m tag(s) ignored
* *errors*: tag T already exists
* *examples*: 
<pre>
tagger create mp3 music/soundtracks
</pre>

#### delete ####
* *description*: Delete one or more tags (all relations will be lost)
* *syntax*: tagger delete TAG [TAG2 [TAG3 [...]]]
* *output*: n tag(s) successfully deleted, m tag(s) ignored
* *errors*: tag T not found
* *examples*: 
<pre>
tagger delete mp3
tagger delete "music/*"
</pre>

#### rename ####
* *description*: Rename an existing tag
* *syntax*: tagger rename [TAG_OLDNAME] [TAG_NEWNAME]
* *output*: tag successfully renamed
* *errors*: tag T not found
* *examples*: 
<pre>
tagger rename mp3 music/mp3	
</pre>

#### merge ####
* *description*: Merge two tags (relations of each tag will be applied to both)
* *syntax*: tagger merge TAG1 TAG2
* *examples*: 
<pre>
tagger merge umsic music
</pre>


#### clone ####
* *description*: Create a new tag by copying all existing relations from another
* *syntax*: tagger clone TAG1 TAG2
* *examples*: 
<pre>
tagger clone music sounds
</pre>	


#### tag ####
* *description*: Add(+) or removes(-) tag(s) to/from one or more files
* *note*: paramters order does not matter, any argument preceeded by + or - is considered as a tag name.
* *note*: if a given tag does not exist yet, it is automatcaly created (a message will be displayed if the --debug option is set)
* *syntax*: tagger [tag] +|-TAG1 [[+|-TAG2] [...]] FILE1 [FILE2 [...]]
* *example*: 
<pre>
tagger tag +"good ideas" notes.txt
tagger +movies *.mp4
tagger +music/mp3 +samples -movies test.mp3
</pre>	
	

#### files ####
* *description*: List files matching the given criterias
* *note*: if no criteria is specified, outputs all tagged files
* *syntax*: tagger files TAG1|"QUERY" [TAG2|"QUERY" [TAG3|"QUERY" [...]]]
* *query syntax*: tagger files [!]TAG1 [&| [!]TAG2 [&| [!]TAG3] [...]]
* *examples*: 
<pre>
tagger files "music/*"
tagger files "music & !mp3"
</pre>.  


**About queries**: 
* Special chars for queries are: 
 * parentheses, '(' and ')' : for grouping
 * ampercent, '&' : logical AND operator (binary)
 * pipe, '|' : logical OR operator (binary)
 * exclamation point, '!' : logical NOT operator (unary)
* A tag containing spaces should be escaped with double quotes 
 * ex.: tagger files "my music"
* A tag containing reserved chars inside a query should be escaped with curly brackets
 * ex.: tagger files "notes & {thoughts & ideas}"

Final result is built step by step by taking each argument and applying logical OR operator between them. If one of the argument has query format, it is processed as such.
<pre>"a & !b" c</pre> 
is equivalent to: 
<pre>"(a & !b) | c"</pre>


#### tags ####
* *description*: List all tags related to one or more files (no file means all tags)
* *note*: if no file is specified, outputs all existing tags
* *syntax*: tagger tags [FILE1 [FILE2 [...]]]
* *examples*: 
<pre>
tag tags
tag tags /home/ced/music/buddy_holly.mp3
tag tags *.mp3  
</pre>	
	



## Installation ##


### Download binary ###
1. Download the binaries from github.com
 https://github.com/cedricfrancoys/tagger/releases/latest
 * Linux 32-bit version: tagger.o (compiled/tested under Ubuntu)
 * Windows 32-bit version: tagger.exe (compiled/tested under Windows XP) 

2. Copy the executable into the directory of your choice.  

  
**Note**: program needs read/write access to user's home directory for storing/retrieving data.

### Download sources ###
Alternatively, you can download source files and compile the project with gcc.  
See "compiling" section below.

## Compiling ##
Download the full project source from github.com either from master repository or, at your opinion, from the latest release page.
 * https://github.com/cedricfrancoys/tagger/archive/master.zip
 * https://github.com/cedricfrancoys/tagger/releases/latest


Sources use standard C librairies (C99) + GNU C library (glibc).  

**Note**: makefile stores compiled objects in an additional ./obj directory (which is not part of the project).
So, in order to use the makefile, use 'mkdir obj' first.

### Linux ###

Use 'make' command in your root project directory to generate executable.

<pre>
cp makefile.lin makefile
make
./tagger.o
</pre>

**Note**: compiling under Linux has been tested with gcc and libc6 (under Ubuntu 32-bit).
### Windows ###

Use 'make.exe' command in your root project directory to generate executable.

<pre>
copy makefile.win makefile
make
tagger
</pre>

**Note**: Compiling under win32 has been tested with MinGW32 and libgw32c-0.4 along with GnuWin32 tools.
 * http://www.mingw.org/download/installer
 * http://gnuwin32.sourceforge.net/

## Author ##

Feel free to send any comments, patches, flowers and suggestions to: Cédric Françoys <cedricfrancoys@gmail.com>