/* input module header for pdp429 */
#ifndef _OBJINPUT_H_
#define _OBJINPUT_H_

#include <stdio.h>

struct Token
{
	char* string;
	int length;
};

typedef struct Token Token;

Token *new_token(void);
void delete_token(Token *t);
void put_char_in_token_at(Token *t, int c, int i);
int poke(FILE *CDS_file);
int skip_blanks(FILE *file);
void get_token(FILE *CDS_file, Token *t, int size);
// void get_data(FILE *CDS_file, Token *t, int size);

#endif