# tagger

Tagger is a cross-platform lightweight command-line tool for tagging files and directories
* cross-platform (Windows, Linux, Mac OS, Unix)
* no alteration of the tagged files
* no dependencies
* no tag naming restrictions
* no tag limit
* instant backup (DB stored in 1 directory)
* easy import / export
* data stored in UTF-8


Note: if you plan to work exclusively on Linux, consider as well the excellent [TMSU](http://http://tmsu.org/) project.

<pre>
USAGE: tagger [OPTION] OPERATION [PARAMETERS]

OPTIONS:
  --quiet       Suppress all normal output
  --debug       Output program trace and internal errors
  --help        Display this help text and exit
  --version     Display version information and exit

OPERATIONS:
  create        Create one or more new tags
  clone         Create a new tag by copying all existin relations from another
  delete        Delete one or more tags (all relations will be lost)
  rename        Rename an existing tag
  merge         Merge two tags (relations of each tag will be applied to both)
  tag           Add(+) or remove(-) tag(s) to/from one or more files
  files         Show files matching the given criterias
  tags          Show tags related to one or more files (no file means all tags)

Examples:
tagger create mp3 music
tagger tag +mp3 +music sound.mp3
tagger -music sound.mp3
tagger merge mp3 music
tagger tags sound.mp3
</pre>