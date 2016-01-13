/* eval.c - interface for interpreting search queries.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "eval.h"
#include "xalloc.h"
#include "list.h"
#include "elem.h"
#include "error.h"

/* Check if given string matches query syntax or if it is a single tag name 
*/
int is_query(char* str) {
	char* ptr = str;
    int bracket_count = 0;
    if(*ptr == '{') {
        ++ptr;
        bracket_count = 1;
    }
	while(*ptr) {
        if(*ptr == '}') bracket_count = 0;
        else if(!bracket_count) {
            if( is_operator(*ptr) || is_parenth(*ptr) ) {
                return 1;
            }
        }
		++ptr;
	}
	return 0;
}

int is_operator(char c) {	
	return (c == '!' || c == '&' || c == '|');
}

int is_parenth(char c) {
	return (c == '(' || c == ')');
}

int op_preced(const char c) {
    switch(c)    {
        case '|':
            return 3;
        case '&':
            return 2;
        case '!':
            return 1;
    }
    return 0;
}

int op_left_assoc(const char c) {
    switch(c)    {
        // left to right
        case '|': case '&':
            return 1;
        // right to left
        case '!':
            return 0;
    }
    return 0;
}


char* operand_end(char* str) {
	char* ptr = str;
    int bracket_count = 0;
    if(*ptr == '{') {
        ++ptr;
        bracket_count = 1;
    }
	while(*ptr) {
        if(*ptr == '}') bracket_count = 0;
        else if(!bracket_count) {
            if( is_operator(*ptr) || 
                is_parenth(*ptr)  ||
                (*ptr == ' ' && (is_operator(*(ptr+1)) || is_parenth(*(ptr+1)))) )
                break;
        }
		++ptr;
	}
	return ptr;
}


/* Convert query expression into postfix notation (RPN) (op1;op2;operator)
 This function implements the Shunting-Yard algorithm wich preserves 
 operands order and set operators order according to their priorities.
 
 Operands are replaced by a 'x' char in the output buffer (and not copied)
 Note : list of forbidden chars inside operands: (,),&,|,!
 To deal with operand containing operator chars, user must escape it with '{' and '}'
*/
int postfix_convert(char *input, char *output) {
    char *inptr = input;
    char *outptr = output;

    char stack[128];    // operator stack
    int sl = 0;         // stack length
    char sc;            // used for record stack element

    int so_i = 0;       // output stack index
    
    while(*inptr)   {
        // read one token from the input stream
        char c = *inptr;
        if(c != ' ') {
            // if the token is an identifier, add it to the output queue
            if(!is_operator(c) && c != '(' && c != ')')  {
                // update outptr
                *outptr = 'x';
				++outptr;                
                // find end pos of current operand
                char* end = operand_end(inptr);
                // set inptr according to further increment (at the end of current loop)
				inptr = end-1;                
            } 
            // If the token is an operator, op1, then:
            else if(is_operator(c)){
                while(sl > 0)    {
                    sc = stack[sl - 1];
                    if(is_operator(sc) &&
                        ((op_left_assoc(c) && (op_preced(c) >= op_preced(sc))) ||
                           (op_preced(c) > op_preced(sc))))   {
                        // Pop op2 off the stack, onto the output queue;
                        *outptr = sc;
                        ++outptr;
                        sl--;
                    }
                    else   {
                        break;
                    }
                }
                // push op1 onto the stack.
                stack[sl] = c;
                ++sl;
            }
            // If the token is a left parenthesis, then push it onto the stack.
            else if(c == '(')   {
                stack[sl] = c;
                ++sl;
            }
            // If the token is a right parenthesis:
            else if(c == ')')    {
                int pe = 0;
                // Until the token at the top of the stack is a left parenthesis,
                // pop operators off the stack onto the output queue
                while(sl > 0)     {
                    sc = stack[sl - 1];
                    if(sc == '(')    {
                        pe = 1;
                        break;
                    }
                    else  {
                        *outptr = sc;
                        ++outptr;
                        sl--;
                    }
                }
                // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
                if(!pe)  {
                    printf("Error: parentheses mismatch\n");
                    return 0;
                }
                // Pop the left parenthesis from the stack, but not onto the output queue.
                sl--;
                // If the token at the top of the stack is a function token, pop it onto the output queue.
                if(sl > 0)   {
                    sc = stack[sl - 1];
                }
            }
        }
        ++inptr;
    }
    // no more token    
    while(sl > 0)  {
        // read remaining operators in the stack
        sc = stack[sl - 1];
        if(sc == '(' || sc == ')')   {
            printf("Error: parentheses mismatched\n");
            return 0;
        }
        *outptr = sc;
        ++outptr;
        --sl;
    }
    // close output sring 
    *outptr = 0; 
    return 1;
}


char* next_operand(char* str, char** saveptr) {
    char *start, *end = NULL, *result = NULL;
    
    if(str) start = str;
    else start = *saveptr;
    
    for(char* ptr = start; *ptr; ptr++) {
        if(*ptr == ' ') continue;
        if(!is_operator(*ptr) && !is_parenth(*ptr))  {
            start = ptr;
            // find end pos of current operand
            end = operand_end(ptr);
            // save ptr value for next call
            *saveptr = end+1;            
            break;
        }    
    }
    if(end) {
        // remove brackets if any
        if(*start == '{') {
            start++;
            end--;
        }
        int len = end-start;
        result = start;
        result[len] = 0;
    }
    return result;
}

/* Evaluates a query string and returns the list of mathching files.
This function calls postfix_convert to convert query in RPN and then implements postfix algorithm.

Reserved chars/separators are: [space], [parentheses], [ampercent], [more], [not]
If a tagname contains reserved chars it should be escaped with brackets.
Thus '{' and '}' chars are forbidden inside tag names.
*/	
LIST* eval(char* query) {
    char *ptr, *operand;
    char postfix[256];
    LIST* stack_list[64];
    
    int stack_list_count = 0;

    // scan query to comply with postfix logical operations order, and replace tag names with a 'x'
    postfix_convert(query, postfix); 
    trace(TRACE_DEBUG, "query postfix order: '%s'", postfix);
    operand = next_operand(query, &ptr);

    // evaluate query and build the resulting list    
    for(int i = 0; postfix[i]; ++i) {
        if(postfix[i] == 'x') {
            // if current token is an operand,
            // use last extracted operand from original query string
            // and store its related list into stack_list
            LIST* new_list = (LIST*) xzalloc(sizeof(LIST));
            new_list->first = (NODE*) xzalloc(sizeof(NODE));
            
            ELEM el_tag;
            int res = elem_init(ELEM_TAG, operand, &el_tag, 0);
            if( res < 0) {
                raise_error(ERROR_ENV,
                            "%s:%d - Unexpected error occured while looking for tag '%s'",
                            __FILE__, __LINE__, operand);
            }
            else if(!res) {
                raise_error(ERROR_USAGE, "Tag '%s' does not exist.", operand);
            }
            
            if(elem_retrieve_list(&el_tag, new_list) < 0) {
                raise_error(ERROR_ENV,
                            "%s:%d - Unexpected error occured while retrieving list from file '%s'",
                            __FILE__, __LINE__, el_tag.file);
            }	            
            stack_list[stack_list_count] = new_list;
            ++stack_list_count;            
            // extract next operand
            operand = next_operand(NULL, &ptr);
        }
        if(postfix[i] == '!') {
            // if current token is 'not' operator
            // apply negation on last list stored in stack_list
            if(stack_list_count <= 0) {
                // we need at minimum one list on stack_list
                return NULL;
            }
            LIST* op_list = stack_list[stack_list_count-1];
            LIST* new_list = (LIST*) xzalloc(sizeof(LIST));
            new_list->first = (NODE*) xzalloc(sizeof(NODE));

            // retrieve all tagged files
            if( !type_retrieve_list(ELEM_FILE, new_list)) {
                raise_error(ERROR_ENV,
                            "%s:%d - Couldn't open files directory",
                            __FILE__, __LINE__);
            }
            // removes files tagged by current tag from resulting list
            list_diff(new_list, op_list);
            // replace last list with resulting list
            stack_list[stack_list_count-1] = new_list;
            list_free(op_list);
        }
        if(postfix[i] == '&') {
            // if current token is 'and' operator
            // we need at minimum two lists on stack_list
            if(stack_list_count <= 1) {
                return NULL;
            }
            LIST* list1 = stack_list[stack_list_count-2];            
            LIST* list2 = stack_list[stack_list_count-1];            
			list_intersect(list1, list2);
            list_free(list2);
            --stack_list_count;            
        }
        if(postfix[i] == '|') {
            // if current token is 'or' operator
            // we need at minimum two lists on stack_list
            if(stack_list_count <= 1) {
                return NULL;
            }
            LIST* list1 = stack_list[stack_list_count-2];            
            LIST* list2 = stack_list[stack_list_count-1];            
			list_merge(list1, list2);
            list_free(list2);
            --stack_list_count;            
        }
    }
    if(stack_list_count != 1) {
        return NULL;
    }
    return stack_list[0];
}