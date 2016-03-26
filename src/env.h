/* env.h - interface for checking and setting program environment.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/


#ifndef ENV_H
#define ENV_H 1

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