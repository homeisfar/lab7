#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "objinput.h"

/* A basic program to read in files of arbitrary lengths 	*
 * It works by tokenizing the input. You presume an			*
 * initial size of 256 bytes. You then add characters		*
 * from your input stream into this string. Each time you 	*
 * stop you should generate a new token, or free and reuse 	*
 * the old token.	Heavily borrowed Dr. James Peterson		*/

#define MIN_TOKEN_SIZE 256
FILE *input;

Token *new_token(void)
{
	Token *t = (Token*) malloc(sizeof(Token));
	t->length = MIN_TOKEN_SIZE;
	t->string = (int*) calloc(t->length, 4);
	return t;
}	

void delete_token(Token *t)
{
	free(t->string);
	free(t);
}

void put_char_in_token_at(Token *t, int c, int i)
{
    if (i >= t->length)
        {
            /* need more space */
            t->length = 2 * t->length;
            t->string = (int*) realloc(t->string, t->length);
            if (t->string == NULL)
                {
                    fprintf(stderr, "Hell has frozen over!!!\n");
                    exit(-1);
                }
        }
    t->string[i] = c;
}

/*  We should be poking only in known spots (where we have n). 
    If we return EOF we are out of data. */
int poke(FILE *CDS_file){
    int c = getc(CDS_file);

    if(c == EOF)
        return -1;
    ungetc(c, CDS_file);
    return 0;
}

void get_token(FILE *CDS_file, Token *t, int size)
{
    // int c = skip_blanks(CDS_file);
    int c = 0;

    /* get the next token in the input stream */
    int i = 0;

    /* token is empty to start */
    put_char_in_token_at(t, '\0', i);
    put_char_in_token_at(t, '\0', i+1);

    /* skip spacing, look for first character */
     // if (c == EOF) return -1;


    while (i < size)
        {
            c = fgetc(CDS_file);
            put_char_in_token_at(t, c, i);
            printf("Putting char (hex) %d: %x\n", i, c);
            printf("now that string value is: %x\n", t->string[i]);
            i = i + 1;
            put_char_in_token_at(t, '\0', i);
            
        }

    /* went one too far, put it back */
    // ungetc(c, CDS_file);
}

// void get_data(FILE *CDS_file, Token *t, int size)
// {
//     char buffer[256];
//     fread(buffer, 1, size, CDS_file);
//     buffer[size] = '\0';
//     printf("yolo: %s\n", buffer);
//     int i = 0;
//     for(i < size; i++;)
//     put_char_in_token_at(t, buffer[i], i);
// }
