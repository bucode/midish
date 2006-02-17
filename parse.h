/* $Id: parse.h,v 1.7 2006/02/14 12:21:41 alex Exp $ */
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

#ifndef MIDISH_PARSE_H
#define MIDISH_PARSE_H

#include "lex.h"

struct node_s;

struct parse_s {
	struct lex_s lex;
	unsigned lookavail;
};

struct parse_s *parse_new(char *);
void		parse_delete(struct parse_s *);

void		parse_error(struct parse_s *, char *);

unsigned	parse_getsym(struct parse_s *);
void		parse_ungetsym(struct parse_s *);

unsigned	parse_end(struct parse_s *, struct node_s **);
unsigned	parse_call(struct parse_s *, struct node_s **);
unsigned	parse_expr(struct parse_s *, struct node_s **);
unsigned	parse_addsub(struct parse_s *, struct node_s **);
unsigned	parse_muldiv(struct parse_s *, struct node_s **);
unsigned	parse_neg(struct parse_s *, struct node_s **);
unsigned	parse_const(struct parse_s *, struct node_s **);
unsigned	parse_assign(struct parse_s *, struct node_s **);
unsigned	parse_stmt(struct parse_s *, struct node_s **);
unsigned	parse_slist(struct parse_s *, struct node_s **);
unsigned	parse_proc(struct parse_s *, struct node_s **);
unsigned	parse_line(struct parse_s *o, struct node_s **n);
unsigned	parse_prog(struct parse_s *o, struct node_s **n);

#endif /* MIDISH_PARSE_H */
