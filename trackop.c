/* $Id$ */
/*
 * Copyright (c) 2003-2005 Alexandre Ratchov
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

/*
 * implements high level track operations
 */

#include "dbg.h"
#include "trackop.h"
#include "track.h"
#include "default.h"
#include "frame.h"

	/*
	 * set the current position to the
	 * following frame. if no frame is available
	 * the current position is 'o->eot'
	 */

unsigned
track_framefind(struct track_s *o, struct seqptr_s *p) {
	unsigned tics;
	tics = 0;
	for (;;) {
		tics += track_ticlast(o, p);
		if ((*p->pos)->ev.cmd == EV_NON ||
		    !EV_ISNOTE(&(*p->pos)->ev)) {
		    	break;
		}
		track_evnext(o, p);
	}
	return tics;
}

	/*
	 * same as track_frameget, but 
	 * if the same type of frame is found several times
	 * in the same tic, only the latest one is kept.
	 */

void
track_frameuniq(struct track_s *o, struct seqptr_s *p, struct track_s *frame) {	
	struct seqptr_s op, fp;
	unsigned delta;
	struct ev_s ev;	
	
	track_frameget(o, p, frame);
	op = *p;
	track_rew(frame, &fp);
	if (!track_evavail(frame, &fp)) {
		return;
	}
	track_evget(frame, &fp, &ev);
	for (;;) {
		delta = track_framefind(o, &op);
		if (delta != 0 || !track_evavail(o, &op)) {
			break;
		}
		if (ev_sameclass(&ev,  &(*op.pos)->ev)) {
			track_frameget(o, &op, frame);
		} else {
			track_evnext(o, &op);
		}
	}
}


void
track_framecp(struct track_s *s, struct track_s *d) {
	struct seqptr_s sp, dp;
	struct ev_s ev;
	unsigned tics;
	track_rew(s, &sp);
	track_rew(d, &dp);
	for (;;) {
		tics = track_ticlast(s, &sp);
		track_seekblank(d, &dp, tics);
		if (!track_evavail(s, &sp)) {
			break;
		}
		track_evget(s, &sp, &ev);
		track_evput(d, &dp, &ev);
	}
}

	/*
	 * retrun 1 if all avents match the givent event range
	 * and zero otherwise
	 */

unsigned
track_framematch(struct track_s *s, struct evspec_s *e) {
	struct seqptr_s sp;
	track_rew(s, &sp);
	for (;;) {
		if (!track_seqevavail(s, &sp)) {
			break;
		}
		if (!evspec_matchev(e, &(*sp.pos)->ev)) {
			return 0;
		}
		track_seqevnext(s, &sp);
	}
	return 1;
}


void
track_frametransp(struct track_s *o, int halftones) {
	struct seqptr_s op;
	
	track_rew(o, &op);	
	for (;;) {
		if (!track_seqevavail(o, &op)) {
			break;
		}
		if (EV_ISNOTE(&(*op.pos)->ev)) {
			(*op.pos)->ev.data.voice.b0 += halftones;
			(*op.pos)->ev.data.voice.b0 &= 0x7f;
		}
		track_seqevnext(o, &op);
	}
}


	/*
	 * suppress orphaned NOTEOFFs and NOTEONs and nested notes
	 * if the same frame is found twice on the same tic, 
	 * only the latest is kept.
	 */

void
track_opcheck(struct track_s *o) {
	struct track_s temp, frame;
	struct seqptr_s op, tp, fp;
	unsigned delta;
	
	track_init(&temp);
	track_init(&frame);
	
	track_rew(o, &op);
	track_rew(&temp, &tp);
	track_rew(&frame, &fp);

	for (;;) {
		delta = track_framefind(o, &op);
		track_seekblank(&temp, &tp, delta);
		
		if (!track_evavail(o, &op)) {
			break;
		}
		track_frameuniq(o, &op, &frame);
		track_frameput(&temp, &tp, &frame);
	}
	
	/*
	dbg_puts("track_check: the following events were dropped\n");
	track_dump(o);
	*/
	track_clear(o, &op);
	track_frameput(o, &op, &temp);

	track_done(&temp);
	track_done(&frame);
}

	/*
	 * quantise and (suppress orphaned NOTEOFFs and NOTEONs)
	 * arguments
	 *	first - the number of the curpos tic
	 *	len - the number of tic to quantise
	 *	quantum - number of tics to round to note-on positions
	 *	rate - 0 =  dont quantise, 100 = full quantisation
	 */

void
track_opquantise(struct track_s *o, struct seqptr_s *p, 
    unsigned first, unsigned len, unsigned quantum, unsigned rate) {
	struct track_s ctls, frame;
	struct seqptr_s op, cp, fp;
	unsigned tic, delta;
	unsigned remaind;
	int ofs;
	
	if (rate > 100) {
		dbg_puts("track_quantise: rate > 100\n");
	}

	len += first;
	track_init(&ctls);
	track_init(&frame);
	op = *p;
	track_rew(&ctls, &cp);
	track_rew(&frame, &fp);
	delta = ofs = 0;
	tic = first;
	
	
	/* first, move all non-quantizable frames to &ctls */	
	for (;;) {
		delta = track_framefind(o, &op);
		tic += delta;
		if (!track_evavail(o, &op) || tic >= len) {
			break; 	
		
		} 
		track_seekblank(&ctls, &cp, delta);
		if ((*op.pos)->ev.cmd != EV_NON) {
			track_evlast(&ctls, &cp);
			track_frameget(o, &op, &frame);
			track_frameput(&ctls, &cp, &frame);
		} else {
			track_evnext(o, &op);
		}
	}
	op = *p;
	track_rew(&ctls, &cp);	
	delta = ofs = 0;	
	tic = first;
	
	/* now we can start */

	for (;;) {
		delta = track_framefind(o, &op);
		tic += delta;
		delta -= ofs;
		ofs = 0;
		
		if (!track_evavail(o, &op) || tic >= len) {	/* no more notes? */
			break;
		} 
			
		if (quantum != 0) {
			remaind = tic % quantum;
		} else {
			remaind = 0;
		}
		if (remaind < quantum / 2) {
			ofs = - ((remaind * rate + 99) / 100);
		} else {
			ofs = ((quantum - remaind) * rate + 99) / 100;
		}
		if (ofs < 0 && delta < (unsigned)(-ofs)) { 
			dbg_puts("track_opquantise: delta < ofs\n");
			dbg_panic();
		}
		track_seekblank(&ctls, &cp, delta + ofs);
		track_evlast(&ctls, &cp);
		track_frameget(o, &op, &frame);
		track_frameput(&ctls, &cp, &frame);
	}
	op = *p;
	track_frameput(o, &op, &ctls);
	track_opcheck(o);
	track_done(&ctls);
}

	/*
	 * extract all frames from a track
	 * begging at the current position during
	 * 'len' tics.
	 */

void
track_opextract(struct track_s *o, struct seqptr_s *p, 
    unsigned len, struct track_s *targ, struct evspec_s *es) {
	struct track_s frame;
	struct seqptr_s op, tp;
	unsigned delta, tic;
	
	tic = 0;
	op = *p;
	track_init(&frame);
	track_clear(targ, &tp);
	
	for (;;) {
		delta = track_framefind(o, &op);
		track_seekblank(targ, &tp, delta);
		track_evlast(targ, &tp);
		tic += delta;

		if (!track_evavail(o, &op) || tic >= len) {
			break;
		}
		
		if (evspec_matchev(es, &(*op.pos)->ev)) {
			track_frameget(o, &op, &frame);
			track_frameput(targ, &tp, &frame);
		} else {
			track_evnext(o, &op);
		}
	}	
	track_done(&frame);
}


	/* 
	 * cut a piece of the track (events and blank space)
	 */

void
track_opcut(struct track_s *o, unsigned start, unsigned len) {
	struct track_s frame, temp;
	struct seqptr_s op, fp, tp;
	unsigned delta, tic;
	
	track_init(&frame);
	track_init(&temp);
	
	tic = 0;
	track_rew(o, &op);
	track_rew(&frame, &fp);
	track_rew(&temp, &tp);
	
	for (;;) {
		delta = track_ticskipmax(o, &op, start - tic);
		track_seekblank(&temp, &tp, delta);
				
		tic += delta;
		if (tic == start) {
			break;
		}
		if (!track_evavail(o, &op)) {
			goto end;
		}
		track_frameget(o, &op, &frame);
		track_framecut(&frame, tic, start, len);
		track_frameput(&temp, &tp, &frame);
	}
	
	for (;;) {
		len -= track_ticdelmax(o, &op, len);
		if (len == 0) {
			break;
		}
		if (!track_evavail(o, &op)) {
			goto end;
		}
		track_frameget(o, &op, &frame);
		track_framecut(&frame, tic, start, len);
		track_frameput(&temp, &tp, &frame);
	}
	
	for (;;) {
		delta = track_ticlast(o, &op);
		track_seekblank(&temp, &tp, delta);
		
		tic += delta;
		if (!track_evavail(o, &op)) {
			goto end;
		}
		track_frameget(o, &op, &frame);
		track_frameput(&temp, &tp, &frame);
	}
	
end:
	track_rew(o, &op);
	track_frameput(o, &op, &temp);
	track_done(&frame);
	track_done(&temp);
}

	/* 
	 * blank a piece of the track (remove events but not blank space)
	 */
	 
void
track_opblank(struct track_s *o, unsigned start, unsigned len, 
   struct evspec_s *es) {
	struct track_s frame, temp;
	struct seqptr_s op, fp, tp;
	unsigned delta, tic;
	
	track_init(&frame);
	track_init(&temp);
	
	tic = 0;
	track_rew(o, &op);
	track_rew(&frame, &fp);
	track_rew(&temp, &tp);
	
	for (;;) {
		delta = track_ticlast(o, &op);
		track_seekblank(&temp, &tp, delta);
				
		tic += delta;
		if (!track_evavail(o, &op)) {
			break;
		}
		if (evspec_matchev(es, &(*op.pos)->ev)) {
			track_frameget(o, &op, &frame);
			track_frameblank(&frame, tic, start, len);
			track_frameput(&temp, &tp, &frame);
		} else {
			track_evnext(o, &op);
		}
	}

	track_rew(o, &op);
	track_frameput(o, &op, &temp);
	track_done(&frame);
	track_done(&temp);
	/*track_opcheck(o);*/
}

	/*
	 * insert blank space at the given position
	 */

void
track_opinsert(struct track_s *o, struct seqptr_s *p, unsigned len) {
	struct track_s frame, temp;
	struct seqptr_s op, tp;
	unsigned delta, tic;
	
	track_init(&temp);
	track_init(&frame);

	tic = 0;
	op = *p;
	track_rew(&temp, &tp);
	track_seekblank(&temp, &tp, len);
		
	for (;;) {
		delta = track_framefind(o, &op);
		track_seekblank(&temp, &tp, delta);
		track_evlast(&temp, &tp);
		tic += delta;

		if (!track_evavail(o, &op)) {
			break;
		}
	
		track_frameget(o, &op, &frame);
		track_frameput(&temp, &tp, &frame);
	}
	op = *p;
	track_frameput(o, &op, &temp);
	track_done(&frame);
	track_done(&temp);
	track_opcheck(o);
}


void
track_optransp(struct track_s *o, struct seqptr_s *p, unsigned len, 
    int halftones, struct evspec_s *es) {
	struct track_s frame, temp;
	struct seqptr_s op, tp;
	unsigned delta, tic;
	
	track_init(&temp);
	track_init(&frame);

	tic = 0;
	op = *p;
	track_rew(&temp, &tp);
		
	for (;;) {
		delta = track_framefind(o, &op);
		track_seekblank(&temp, &tp, delta);
		track_evlast(&temp, &tp);
		tic += delta;

		if (!track_evavail(o, &op)) {
			break;
		}
	
		if (evspec_matchev(es, &(*op.pos)->ev)) {
			track_frameget(o, &op, &frame);
			track_frametransp(&frame, halftones);
			track_frameput(&temp, &tp, &frame);
		} else {
			track_evnext(o, &op);
		}
	}
	op = *p;
	track_frameput(o, &op, &temp);
	track_done(&frame);
	track_done(&temp);
	track_opcheck(o);
}

	/*
	 * set the chan (dev/midichan pair) of
	 * all voice events
	 */

void
track_opsetchan(struct track_s *o, unsigned dev, unsigned ch) {
	struct seqptr_s op;
	track_rew(o, &op);
	for (;;) {
		if (!track_seqevavail(o, &op)) {
			break;
		}
		if (EV_ISVOICE(&(*op.pos)->ev)) {
			(*op.pos)->ev.data.voice.dev = dev;
			(*op.pos)->ev.data.voice.ch = ch;
		}
		track_seqevnext(o, &op);
	}
}


	/*
	 * takes a measure and returns the corresponding tic
	 */


unsigned
track_opfindtic(struct track_s *o, unsigned m0) {
	struct seqptr_s op;
	unsigned m, dm, tpb, bpm, pos, delta;
	struct ev_s ev;
	
	/* default to 24 tics per beat, 4 beats per measure */
	tpb = DEFAULT_TPB;
	bpm = DEFAULT_BPM;
	
	/* go to the beggining */
	track_rew(o, &op);
	m = 0;
	delta = pos = 0;
	
	/* seek the wanted measure */
	for (;;) {
		delta += track_ticlast(o, &op);
		dm = delta / (bpm * tpb);
		if (dm >= m0 - m) {		/* dont seek beyound m0 */
			dm = m0 - m;
		}
		m     += dm;
		delta -= dm * (bpm * tpb);
		pos   += dm * (bpm * tpb);
		if (m == m0) {
			break;
		}
		/* check for premature end of track */
		if (!track_evavail(o, &op)) {
			/* complete as we had blank space */
			pos += (m0 - m) * bpm * tpb;
			m = m0;
			break;
		}
		/* scan for timesign changes */
		while (track_evavail(o, &op)) {
			track_evget(o, &op, &ev);
			if (ev.cmd == EV_TIMESIG) {
				if (delta != 0) {
					/* 
					 * XXX: may happen with bogus midi 
					 * files 
					 */
					dbg_puts("track_opfindtic: EV_TIMESIG in a measure\n");
					dbg_panic();
				}
				bpm = ev.data.sign.beats;
				tpb = ev.data.sign.tics;
			}
		}
	}
	return pos;
}

	/*
	 * determine the tempo and the
	 * time signature at a given position (in tics)
	 * (the whole tic is scanned)
	 */

void
track_optimeinfo(struct track_s *o, unsigned pos, unsigned long *usec24, unsigned *bpm, unsigned *tpb) {
	unsigned tic;
	struct ev_s ev;
	struct seqptr_s op;
	
	tic = 0;
	track_rew(o, &op);
	
	/* use default midi settings */
	*tpb = DEFAULT_TPB;
	*bpm = DEFAULT_BPM;
	*usec24 = TEMPO_TO_USEC24(DEFAULT_TEMPO, *tpb);
	
	for (;;) {
		tic += track_ticlast(o, &op);
		if (tic > pos || !track_evavail(o, &op)) {
			break;
		}
		while (track_evavail(o, &op)) {
			track_evget(o, &op, &ev);
			switch(ev.cmd) {
			case EV_TEMPO:
				*usec24 = ev.data.tempo.usec24;
				break;
			case EV_TIMESIG:
				*bpm = ev.data.sign.beats;
				*tpb = ev.data.sign.tics;
				break;
			default:
				break;
			}
		}
	}
}


	/*
	 * determine the (measure, beat, tic)
	 * corresponding to a absolut tic
	 * (the whole tic is scanned)
	 */

void
track_opgetmeasure(struct track_s *o, unsigned pos,
    unsigned *measure, unsigned *beat, unsigned *tic) {
	unsigned delta, abstic, tpb, bpm;
	struct ev_s ev;
	struct seqptr_s op;
	
	abstic = 0;
	track_rew(o, &op);
	
	/* use default midi settings */
	tpb = DEFAULT_TPB;
	bpm = DEFAULT_BPM;
	*measure = 0;
	*beat = 0;
	*tic = 0;
	
	for (;;) {
		delta = track_ticlast(o, &op);
		if (abstic + delta >= pos) {
			delta = pos - abstic;
		}
		abstic += delta;
		*tic += delta;
		*measure += *tic / (tpb * bpm);
		*tic =      *tic % (tpb * bpm);
		*beat +=    *tic / tpb;	
		*tic =      *tic % tpb;
		*measure += *beat / bpm;
		*beat =     *beat % bpm;
		if (abstic >= pos || !track_evavail(o, &op)) {
			break;
		}
		while (track_evavail(o, &op)) {
			track_evget(o, &op, &ev);
			switch(ev.cmd) {
			case EV_TIMESIG:
				bpm = ev.data.sign.beats;
				tpb = ev.data.sign.tics;
				break;
			default:
				break;
			}
		}
	}
}


void
track_opchaninfo(struct track_s *o, char *map) {
	unsigned i, ch, dev;
	struct seqptr_s p;
	
	for (i = 0; i < DEFAULT_MAXNCHANS; i++) {
		map[i] = 0;
	}
	track_rew(o, &p);
	
	while (track_seqevavail(o, &p)) {
		if (EV_ISVOICE(&(*p.pos)->ev)) {
			dev = (*p.pos)->ev.data.voice.dev;
			ch = (*p.pos)->ev.data.voice.ch;
			i = dev * 16 + ch;
			if (dev >= DEFAULT_MAXNDEVS || ch >= 16) {
				dbg_puts("track_opchaninfo: bogus dev/ch pair, stopping\n");
				break;
			}
			map[dev * 16 + ch] = 1;
		}
		track_seqevnext(o, &p);
	}
}

	/*
	 * add an event to the track (config track)
	 * in an ordered way
	 */

void
track_opconfev(struct track_s *o, struct ev_s *ev) {
	struct seqptr_s p;
	
	track_rew(o, &p);	
	while(track_seqevavail(o, &p)) {
		if (ev_sameclass(&(*p.pos)->ev, ev)) {
			(*p.pos)->ev = *ev;
			return;
		}
		track_seqevnext(o, &p);
	}

	track_rew(o, &p);	
	while(track_seqevavail(o, &p)) {
		if (ev_ordered(ev, &(*p.pos)->ev)) {
			track_evput(o, &p, ev);
			return;
		}
		track_seqevnext(o, &p);
	}
	track_evput(o, &p, ev);
}
