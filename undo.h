/*
 * Copyright (c) 2018 Alexandre Ratchov <alex@caoua.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef MIDISH_UNDO_H
#define MIDISH_UNDO_H

#include "track.h"

struct songtrk;
struct songchan;
struct songfilt;
struct songsx;

struct undo {
	struct undo *next;
#define UNDO_TRACK	1
#define UNDO_TREN	2
#define UNDO_TDEL	3
#define UNDO_TNEW	4
	int type;
	char *func;
	char *name;
	unsigned size;
	union {
		struct undo_track {
			struct track *track;
			struct track_data data;
		} track;
		struct undo_tdel {
			struct song *song;
			struct songtrk *trk;
			struct track_data data;
		} tdel;
		struct undo_tren {
			struct songtrk *trk;
			char *name;
		} tren;
	} u;
};

void undo_pop(struct song *);
void undo_push(struct song *, struct undo *);
void undo_clear(struct song *, struct undo **);
void undo_track_save(struct song *, struct track *, char *, char *);
void undo_track_diff(struct song *);
void undo_tren_do(struct song *, struct songtrk *, char *, char *);
void undo_tdel_do(struct song *, struct songtrk *, char *);
struct songtrk *undo_tnew_do(struct song *, char *, char *);

#endif /* MIDISH_UNDO_H */