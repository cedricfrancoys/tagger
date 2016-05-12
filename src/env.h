/* env.h - interface for checking and setting program environment.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/


#ifndef ENV_H
#define ENV_H 1

/* We'll use a local-defined value for constant FILENAME_MAX
 (which might not be defined on some environement)
*/
#if defined( FILENAME_MAX )
#undef FILENAME_MAX
#endif

/* Force FILENAME_MAX macro to a local value for compatibility purpose.
 FILENAME_MAX serves as system limit for the length of a string holding a filename with full path.
 (Though 1024 chars long is a quite long path, this might be adjusted depending on user's specific needs.)
*/
#define FILENAME_MAX 1024


/* Convert a path string according to current environment.
*/
char* fix_path(char* path);

char* absolute_path(char* filename);

char* get_relative_path(char* reference_path, char* absolute_path);

/* Obtain the path of a file relatively to install dir */
char* relative_path(char* filename);

char* get_path(char* filename);

char* get_install_dir();

int check_env();

int setup_env();

#endif