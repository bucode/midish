/* $Id: lex.h,v 1.8 2006/02/14 12:21:40 alex Exp $ */
/*
 * Copyright (c) 2003-2006 Alexandre Ratchov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *
 * 	- Redistributions of source code must retain the above
 * 	  copyright notice, this list of conditions and the
 * 	  following disclaimer.
 *
 * 	- Redistributions in binary form must reproduce the above
 * 	  copyright notice, this list of conditions and the
 * 	  following disclaimer in the documentation and/or other
 * 	  materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MIDISH_LEX_H
#define MIDISH_LEX_H

#define IDENT_MAXLEN	30
#define STRING_MAXLEN	1024
#define TOK_MAXLEN	STRING_MAXLEN

enum SYM_ID {
	TOK_EOF = 0, TOK_ASSIGN, 
	TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PCT,
	TOK_LSHIFT, TOK_RSHIFT, TOK_BITAND, TOK_BITOR, TOK_BITXOR, TOK_TILDE,
	TOK_EQ, TOK_NEQ, TOK_GE, TOK_GT, TOK_LE, TOK_LT, 
	TOK_EXCLAM, TOK_AND, TOK_OR,
	TOK_LPAR, TOK_RPAR, TOK_LBRACE, TOK_RBRACE, TOK_LBRACKET, TOK_RBRACKET,
	TOK_COMMA, TOK_DOT, TOK_SEMICOLON, TOK_COLON, 
	TOK_AT, TOK_DOLLAR, TOK_ENDLINE,
	/* keywords */
	TOK_IF, TOK_ELSE, TOK_WHILE, TOK_DO, TOK_FOR, TOK_IN,
	TOK_PROC, TOK_LET, TOK_RETURN, TOK_NIL,
	/* data */
	TOK_IDENT, TOK_NUM, TOK_STRING,
};

struct tokdef_s {
	unsigned id;				/* token id */
	char *str;				/* corresponding string */
};

struct lex_s {
	/*struct tokdef_s *op, *kw;*/
	unsigned id;
	char strval[TOK_MAXLEN + 1];
	unsigned long longval;	
	/* input */
	struct textin_s *in;
	/* used by ungetchar */
	int lookchar;
	/* for error reporting */
	unsigned line, col;
};

unsigned lex_init(struct lex_s *, char *);
void     lex_done(struct lex_s *);
unsigned lex_scan(struct lex_s *);
void	 lex_dbg(struct lex_s *);

unsigned lex_getchar(struct lex_s *o, int *c);
void	 lex_ungetchar(struct lex_s *o, int c);
void	 lex_err(struct lex_s *o, char *msg);
void	 lex_recover(struct lex_s *o, char *msg);
unsigned lex_str2long(struct lex_s *o, unsigned base);

#endif /* MIDISH_LEX_H */
