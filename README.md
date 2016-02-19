# Tagger

## What is Tagger? ##
Tagger is a cross-platform lightweight command-line tool for tagging files and directories. 
It is written in C and released under the GNU Public License version 3. 

Its main features are:
* cross-platform (Windows, Linux, Mac OS, Unix)
* no alteration of the (un)tagged files
* no tag naming restrictions
* no tag limit
* no dependencies
* no SQL (filesystem DB)
* allows tagging, untagging and recover
* instant backup (data stored in 1 directory)
* portable encoding (UTF-8 encoded data)
* easy import / export 
	
Notes: 
* For a Graphical User Interface for this tool, see the [Tagger-UI](https://github.com/cedricfrancoys/tagger-ui) project, which also allows to maintain tag consistency when tagged files are renamed, moved or deleted.


* If you're interested in file tagging tools, here is a summary of other open source projects:
  1. [TMSU](http://tmsu.org/): allows to browse among files by using a query-based virtual FS, full functionnalities on Linux only.
  2. [dantalian](https://github.com/darkfeline/dantalian): cross platform, requires Python3+, creates hidden files in each tagged directory.
  3. [tagxfs](http://tagxfs.sourceforge.net/): Linux only, uses a virtual FS to browse among tags.
  4. [tagspaces](http://www.tagspaces.org/) : cross platform, GUI only, alter filenames in order to store tagging data.
  5. [dfym](https://github.com/alvatar/dfym): Linux only, requires SQLite, no queries (search by tagname).
  6. [tagsistant](http://www.tagsistant.net/): Linux only, uses FS commands to manage tags (mkdir, cp, ls), limited querying possibilities.
  7. [OYEPA](http://pages.stern.nyu.edu/~marriaga/software/oyepa/) : cross platform, requires Python2.5+, GUI only, needs directories to be watched before taggging, alter filenames in order to store tagging data.


## How to use it? ##
Commands allow to set and modify relations between two kind of elements: tags and filesystem nodes (hereafter referred to as 'files').
All relations are of M:N type (many-to-many): one tag might be applied on several files and one file might be tagged by several tags.

User can specify which kind of element to work with by using --tags or --files option.
 
### USAGE ###
<pre>
tagger [OPTION] OPERATION [PARAMETERS]
</pre>

### OPTIONS ###
<pre>
  --tags        (default) Set mode to apply operation on 'tag' elements
  --files       Set mode to apply operation on 'file' elements
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
* *description*: Delete one or more element from database (all relations will be removed as well)
* *syntax*: tagger [--_mode_] delete ELEM [ELEM2 [ELEM3 [...]]]
* *output*: n tag(s) successfully deleted, m tag(s) ignored
* *errors*: tag T not found
* *note*: in case of specifying --files as operation mode, filesystem remains untouched (only DB is affected)
* *examples*: 
<pre>
tagger --tags delete mp3
tagger --tags delete "music/*"
</pre>


#### recover ####
* *description*: Tries to recover a previously deleted element
* *syntax*: tagger [--_mode_] recover ELEM_OLDNAME
* *output*: 1 tag successfully recovered.
* *examples*: 
<pre>
tagger --tags recover mp3
tagger --files recover "/home/ced/music/buddy_holly.mp3"
</pre>


#### rename ####
* *description*: Rename an existing tag
* *syntax*: tagger [--_mode_] rename [ELEM_OLDNAME] [ELEM_NEWNAME]
* *output*: tag successfully renamed
* *errors*: tag T not found
* *examples*: 
<pre>
tagger rename mp3 music/mp3	
</pre>


#### merge ####
* *description*: Merge two tags (relations of each tag will be applied to both)
* *syntax*: tagger [--_mode_] merge ELEM1 ELEM2
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


#### list ####
* *description*: Show all elements in database for specified mode
* *syntax*: tagger [--_mode_] list
* *examples*: 
<pre>
tagger --files list
tagger --tags list
tagger list
</pre>


#### query ####
* *description*: Retrieve all elements matching given criteria. It might be either a list of elements, a query, or a mix of both
* *syntax*: tagger [--_mode_] TAG1|"QUERY" [TAG2|"QUERY" [TAG3|"QUERY" [...]]]
* *query syntax*: tagger  [!]TAG1 [&| [!]TAG2 [&| [!]TAG3] [...]]
* Special chars for queries are: 
 * '(',')' (parentheses): for grouping logical operations
 * '&' (ampercent): logical AND operator (binary)
 * '|' (pipe): logical OR operator (binary)
 * '!' (exclamation point): logical NOT operator (unary)
 * '{', '}' (curly brackets): for escaping names containing spaces
* A tag containing spaces should be escaped with double quotes 
 * ex.: tagger --files "my music"
* A tag containing reserved chars inside a query should be escaped with curly brackets
 * ex.: tagger --files "notes & {thoughts & ideas}"
* *output*: No tag currently applied on given file(s). / No file currently tagged with given tag(s).
* *examples*: 
<pre>
tagger --files "music/*"
tagger --files "music & !mp3"
tagger --tags sound.mp3
tagger --tags /home/ced/music/buddy_holly.mp3
</pre>

Final result is built step by step by taking each argument and applying logical OR operator between them. If one of the argument has query format, it is processed as such.
<pre>"a & !b" c</pre> 
is equivalent to: 
<pre>"(a & !b) | c"</pre>
	

#### files ####
* *description*: List all tagged files
* *syntax*: tagger files 
* *note*: this is a shorthand notation for tagger --files list
* *examples*: 
<pre>
tagger files
</pre>


#### tags ####
* *description*: List all existing tags
* *syntax*: tagger tags
* *note*: this is a shorthand notation for tagger --tags list
* *examples*: 
<pre>
tag tags
</pre>	
	



## Installation ##


### Download binary ###
1. Download the binaries from github.com
 https://github.com/cedricfrancoys/tagger/releases/latest
 * Windows 32-bit version: tagger.exe 
 * Linux 32-bit version: tagger.o 
 
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

Feel free to send any comments, patches, suggestions or flowers to Cédric Françoys <cedricfrancoys@gmail.com>