/* eval.h - interface for interpreting search queries.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/
#include "list.h"

int is_operator(char c);
int is_parenth(char c);	
int op_preced(const char c);
int op_left_assoc(const char c);

int is_query(char* str);
int posix_convert(char *input, char *output);
LIST* eval(char* query);
