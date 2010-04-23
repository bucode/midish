/*
 * Copyright (c) 2003-2007 Alexandre Ratchov <alex@caoua.org>
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

#ifndef MIDISH_MUX_H
#define MIDISH_MUX_H

#define MUX_STARTWAIT		0	/* waiting for a start event */
#define MUX_START 		1	/* just got a start */
#define MUX_FIRST		2	/* just got the first tic */
#define MUX_NEXT		3	/* just got the next tic */
#define MUX_STOP		4	/* nothing to do */

#define MUX_LINESIZE		1024

struct ev;
struct sysex;

/*
 * modules are chained as follows: mux -> norm -> filt -> song -> output
 * each module calls call-backs of the next module of the chain. In
 * theory we should use function pointers to "connect" modules to
 * each other...
 *
 * But since connection between various modules are hardcoded and not
 * user configurable, we don't use function pointers and other
 * over-engineered stuff. We just call the following call-backs. We
 * save 3 layers of indirection
 */
struct song;
extern struct song *usong;
extern unsigned mux_isopen;

void song_startcb(struct song *);
void song_stopcb(struct song *);
void song_movecb(struct song *);
void song_evcb(struct song *, struct ev *);
void song_sysexcb(struct song *, struct sysex *);
unsigned song_gotocb(struct song *, unsigned);

struct norm;
void norm_evcb(struct ev *);

struct filt;
void filt_evcb(struct filt *, struct ev *);

/*
 * public functions usable in the rest of the code to send/receive
 * events and to manipulate the clock
 */
void mux_open(void);
void mux_close(void);
void mux_run(void);
void mux_sleep(unsigned);
void mux_flush(void);
void mux_shut(void);
void mux_putev(struct ev *);
void mux_sendraw(unsigned, unsigned char *, unsigned);
unsigned mux_getphase(void);
struct sysex *mux_getsysex(void);
void mux_chgtempo(unsigned long);
void mux_chgticrate(unsigned);
void mux_startreq(void);
void mux_stopreq(void);
void mux_gotoreq(unsigned);
int mux_mdep_wait(void); /* XXX: hide this prototype */

/*
 * call-backs called by midi device drivers
 */
void mux_timercb(unsigned long);
void mux_startcb(void);
void mux_stopcb(void);
void mux_ticcb(void);
void mux_ackcb(unsigned);
void mux_evcb(unsigned, struct ev *);
void mux_sysexcb(unsigned, struct sysex *);
void mux_errorcb(unsigned);

void mux_mtcstart(unsigned);
void mux_mtctick(unsigned);
void mux_mtcstop(void);

#endif /* MIDISH_MUX_H */
