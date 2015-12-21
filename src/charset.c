/* charset.c - interface for charset detection and conversion.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

/*

 Notes:
    Under POSIX environment, setlocale(LC_TYPE, NULL) returns a string 
     following XPG syntax (i.e.: language[_territory[.codeset]])

    Under Win32 console/cmd.exe,     
    * stdin, stdout, stderr use a MS-DOS code-page (OEM 8-bit charset, ex. : CP437, CP850)
     (@see complete list of DOS OEM code-pages : https://en.wikipedia.org/wiki/Code_page#IBM_PC_.28OEM.29_code_pages)
    * arguments from command line are coded in Win32 ANSI (Win32 UI charset, ex. : CP1250, CP1252)    
     (@see complete list of Windows ANSI code-pages : https://en.wikipedia.org/wiki/Code_page#Windows_.28ANSI.29_code_pages)
    As console is part of the UI environment, setlocale always returns the ANSI code-page (ex. 1252) 
    To get the code-page that cmd.exe is using for I/O (ex. CP470), we need to call the system command 'chcp'
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <iconv.h>

#include "xalloc.h"
#include "charset.h"



/* Retrieve the current charset using setlocale function.
 Return value is a string holding the name of the current charset.
 On error, function returns a NULL pointer. 
*/
char* get_charset() {
    // read environment locale for LC_CTYPE category
    setlocale(LC_CTYPE, "");
    char* locale = setlocale(LC_CTYPE, NULL);    
    // return codeset (last block of chars preceeded by a dot)
    return strrchr(locale, '.')+1;
}

char* get_input_charset() {
    static char result[128];
    char* charset = get_charset();
    if(strcmp(OS_ENV, "WIN32") == 0) {
        sprintf(result, "CP%s", charset);
    }
    else {
        sprintf(result, "%s", charset);
    }
    return result;
}

char* get_output_charset() {
    static char result[128];
    char* charset = get_charset();

    if(strcmp(OS_ENV, "WIN32") == 0) {
        char* tmp = (char*) xmalloc(1024);
		char* temp_dir = getenv("TEMP");
		char* chcp_path = (char*) xmalloc(strlen(temp_dir)+strlen("\\chcp.out")+1);
        FILE* fp ;
		sprintf(chcp_path, "%s\\chcp.out", temp_dir);
        // retrieve current code-page by using 'chcp' system command 
        fp = fopen(chcp_path, "r");
        if(fp == NULL) {
			sprintf(tmp, "chcp > %s", chcp_path);
            if(system(tmp) != 0) return NULL;
            fp = fopen(chcp_path, "r");
        }
        if(fp == NULL) return NULL;
        fgets(tmp, 1024, fp);
        // remove the newline char
        tmp[strlen(tmp)-1] = 0;
        // retrieve codeset (last block of chars preceeded by a space)
        charset = strrchr(tmp, ' ')+1;
        sprintf(result, "CP%s", charset);
        fclose(fp);
		free(chcp_path);
        free(tmp);
    }
    else {
        sprintf(result, "%s", charset);
    }
    return result;
}


/* Determine the length of a UTF-8 coded string.
 As UTF-8 is a variable length multi-byte encoding, this function is useful 
 when converting to a fixed length single/multi-byte encoding.
 */
int utf8len(char *s) {
    int len = 0;
    if(s != NULL) {
        for(int i = 0; s[i]; ++i) {
            // count all bytes but continuation ones
            if ((s[i] & 0xc0) != 0x80) ++len;     
        }
    }
    return len;
}

char* strtoutf8(char* cs_from, char* str) {
    char* result = NULL;
    
    if(str == NULL) {
        return result;
    }

    if(cs_from == NULL) {
        return result;
    }
        
    size_t len_in = strlen(str)+1;    

    if(strcmp(cs_from, "UTF-8") == 0) {
        result = (char*) xmalloc(len_in);
        strcpy(result, str);
    }
    else {        
        iconv_t cd = iconv_open("UTF-8", cs_from);
        if (cd == (iconv_t) -1) {
            // Something went wrong.
        }
        else {
            // output size might be up to four times bigger than input
            size_t len_out = len_in*4;
            result = (char*) xzalloc(len_out);        
            char *p_in = str, *p_out = result;        
            size_t nconv = iconv (cd, &p_in, &len_in, &p_out, &len_out);
            if (nconv == (size_t) -1) {
                // Not everything went right. It might be an unfinished byte sequence 
                // at the end of the buffer, or an even more serious problem. 
                //if (errno == EINVAL) { }
                free(result);
                result = NULL;
            }        
            iconv_close(cd);
        }
    }
    return result;
}

char* utf8tostr(char* cs_to, char* str) {
    char* result = NULL;
    
    if(str == NULL) {
        return result;
    }

    if(cs_to == NULL) {
        return result;
    }
    size_t len_in = utf8len(str)+1;    
    if(strcmp(cs_to, "UTF-8") == 0) {
        result = (char*) xmalloc(len_in);
        strcpy(result, str);
    }
    else {
        iconv_t cd = iconv_open(cs_to, "UTF-8");
        if (cd == (iconv_t) -1) {
            // Something went wrong.
        }
        else {
            // output size will always be less or equal than input
            size_t len_out = len_in;
            result = (char*) xzalloc(len_out);
            char *p_in = str, *p_out = result;        
            size_t nconv = iconv (cd, &p_in, &len_in, &p_out, &len_out);
            if (nconv == (size_t) -1) {
                // Not everything went right. It might be an unfinished byte sequence 
                // at the end of the buffer, or an even more serious problem. 
                free(result);
                result = NULL;
            }        
            iconv_close(cd);
        }
    }
    return result;
}

int output(FILE* stream, char* str) {
    // get the environment current charset (in which we have to convert any output)
    char* cs_to = get_output_charset();
    // convert internal string to match output charset
    char* output = utf8tostr(cs_to, str);
    if(output == NULL) {
        fprintf(stderr, "conversion from 'UTF-8' to '%s' failed\n", cs_to);
        return 0;
    }
    fprintf(stream, "%s", output);
    free(output);
    return 1;
}
