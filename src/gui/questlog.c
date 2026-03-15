/*
 * Part of Astonia Client (c) Daniel Brockhaus. Please read license.txt.
 *
 * Questlog
 *
 * Displays the quest log and parses clicks on the quest log window.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "astonia.h"
#include "gui/gui.h"
#include "gui/gui_private.h"
#include "game/game.h"
#include "client/client.h"
#include "client/protocol.h"

static int havequest = 0;

DLL_EXPORT struct questlog *game_questlog = NULL;
int _game_questcount = 0;
DLL_EXPORT int *game_questcount = &_game_questcount;

int questonscreen[10];

int questlist[MAXQUEST], questinit = 0;

static struct questlog *questlog_data_from_ID(int qid)
{
	int n;

	for (n = 0; n < *game_questcount; n++) {
		if (game_questlog[n].ID == qid) {
			return &game_questlog[n];
		}
	}

	return NULL;
}

static unsigned short int questlog_color_from_level(const struct questlog *questdata, int qid)
{
	int cn;
	stat_t level;

	if (!questdata) {
		return graycolor;
	}
	if (qid >= 0 && qid < MAXQUEST && (quest[qid].flags & QF_DONE)) {
		return graycolor;
	}
	if (plrmn < 0 || !map[plrmn].cn) {
		return graycolor;
	}

	cn = (int)map[plrmn].cn;
	level = player[cn].level;

	if (level < questdata->minlevel) {
		return redcolor;
	}
	if (level > questdata->maxlevel) {
		return greencolor;
	}

	return graycolor;
}

static int questcmp(const void *a, const void *b)
{
	const int *va = (const int *)a;
	const int *vb = (const int *)b;
	struct questlog *qa = questlog_data_from_ID(*va);
	struct questlog *qb = questlog_data_from_ID(*vb);

	if (!qa && !qb) {
		return *va - *vb;
	}
	if (!qa) {
		return 1;
	}
	if (!qb) {
		return -1;
	}
	if (qa->minlevel != qb->minlevel) {
		return qb->minlevel - qa->minlevel;
	}
	if (qa->maxlevel != qb->maxlevel) {
		return qb->maxlevel - qa->maxlevel;
	}
	return *va - *vb;
}

int (*do_display_random)(void) = _do_display_random;

DLL_EXPORT int _do_display_random(void)
{
	int y = doty(DOT_HLP) + 15, x, n;
	unsigned int idx, bit, m;
	static short indec[10] = {0, 11, 24, 38, 43, 57, 64, 76, 83, 96};
	static short bribes[10] = {0, 15, 22, 34, 48, 54, 67, 78, 86, 93};
	static short welding[10] = {0, 18, 27, 32, 46, 52, 62, 72, 81, 98};
	static short welding2[10] = {0, 12, 25, 35, 44, 56, 65, 77, 89, 95};
	static short edge[10] = {0, 13, 26, 36, 42, 59, 66, 74, 88, 91};
	static short kindness[10] = {0, 21, 55};
	static short jobless[10] = {0, 20, 45, 61, 82, 97};
	static short security[10] = {0, 10, 29, 41, 58, 69, 75, 85, 94};

	render_text(dotx(DOT_HLP) + (10 + 204) / 2, y, whitecolor, RENDER_ALIGN_CENTER, "Random Dungeon");
	y += 24;

	render_text_fmt(dotx(DOT_HLP) + 10, y, graycolor, 0, "Continuity: %d", shrine.continuity);
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Indecisiveness: ");
	for (n = 1; n < 10; n++) {
		idx = (unsigned int)n / 32U;
		bit = 1U << ((unsigned int)n & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (indec[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", indec[n]);
			}
		}
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Bribes: ");
	for (n = 1; n < 10; n++) {
		m = (unsigned int)n + 10U;
		idx = m / 32U;
		bit = 1U << (m & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (bribes[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", bribes[n]);
			}
		}
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Welding: ");
	for (n = 1; n < 10; n++) {
		m = (unsigned int)n + 20U;
		idx = m / 32U;
		bit = 1U << (m & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (welding[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", welding[n]);
			}
		}
	}
	y += 12;

	if (sv_ver == 35) {
		x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Welding: ");
		for (n = 1; n < 10; n++) {
			m = (unsigned int)n + 72U;
			idx = m / 32U;
			bit = 1 << (m & 31);
			if (shrine.used[idx] & bit) {
				x = render_text(x, y, graycolor, 0, "- ");
			} else {
				if (welding2[n] < shrine.continuity) {
					x = render_text_fmt(x, y, graycolor, 0, "%d ", welding2[n]);
				}
			}
		}
		y += 12;
	}

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "LOE: ");
	for (n = 1; n < 10; n++) {
		m = (unsigned int)n + 30U;
		idx = m / 32U;
		bit = 1U << (m & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (edge[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", edge[n]);
			}
		}
	}
	y += 12;

	if (sv_ver == 30) {
		x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Kindness: ");
		for (n = 1; n < 3; n++) {
			m = (unsigned int)n + 40U;
			idx = m / 32U;
			bit = 1U << (m & 31U);
			if (shrine.used[idx] & bit) {
				x = render_text(x, y, graycolor, 0, "- ");
			} else {
				if (kindness[n] < shrine.continuity) {
					x = render_text_fmt(x, y, graycolor, 0, "%d ", kindness[n]);
				}
			}
		}
		y += 12;
	}

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Security: ");
	for (n = 1; n < 9; n++) {
		m = (unsigned int)n + 53U;
		idx = m / 32U;
		bit = 1U << (m & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (security[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", security[n]);
			}
		}
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Jobless: ");
	for (n = 1; n < 6; n++) {
		m = (unsigned int)n + 63U;
		idx = m / 32U;
		bit = 1U << (m & 31U);
		if (shrine.used[idx] & bit) {
			x = render_text(x, y, graycolor, 0, "- ");
		} else {
			if (jobless[n] < shrine.continuity) {
				x = render_text_fmt(x, y, graycolor, 0, "%d ", jobless[n]);
			}
		}
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Vitality: ");
	if (shrine.used[50 / 32] & (1 << (50 & 31))) {
		render_text(x, y, graycolor, 0, "- ");
	} else if (30 < shrine.continuity) {
		render_text(x, y, graycolor, 0, "30");
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Death: ");
	if (shrine.used[51 / 32] & (1 << (51 & 31))) {
		render_text(x, y, graycolor, 0, "- ");
	} else if (37 < shrine.continuity) {
		render_text(x, y, graycolor, 0, "37");
	}
	y += 12;

	x = render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, "Braveness: ");
	if (shrine.used[52 / 32] & (1 << (52 & 31))) {
		render_text(x, y, graycolor, 0, "- ");
	} else if (51 < shrine.continuity) {
		render_text(x, y, graycolor, 0, "51");
	}
	y += 12;

	y += 12;
	y = render_text_break(dotx(DOT_HLP) + 10, y, dotx(DOT_HLP) + 204, graycolor, 0,
	    "Only shrines in dungeons you have already solved (used the continuity shrine), but not yet used, are shown. "
	    "The continuity shrine shown is the first one you haven't used yet.");

	return y;
}

int do_display_questlog(int nr)
{
	int y = doty(DOT_HLP) + 15, off, n, cnt, pass, m;
	char buf[256];

	if (!questinit) {
		for (n = 0; n < *game_questcount; n++) {
			questlist[n] = game_questlog[n].ID;
		}
		qsort(questlist, (size_t)*game_questcount, sizeof(int), questcmp);
		questinit = 1;
	}

	if (!havequest) {
		cmd_getquestlog();
		havequest = 1;
	}

	for (n = 0; n < 10; n++) {
		questonscreen[n] = -1;
	}

	if (nr == 10) {
		return do_display_random();
	}

	off = (nr - 1) * 9;

	for (pass = cnt = 0; pass < 2; pass++) {
		for (m = 0; m < *game_questcount; m++) {
			struct questlog *questdata;
			unsigned short int questcolor;
			int qid = questlist[m];

			if (qid < 0 || qid >= MAXQUEST) {
				continue;
			}
			if (!(questdata = questlog_data_from_ID(qid))) {
				continue;
			}
			questcolor = questlog_color_from_level(questdata, qid);

			if ((pass == 0 && (quest[qid].flags) == QF_OPEN && quest[qid].done < 10) ||
			    (pass == 1 && (quest[qid].flags) == QF_DONE)) {
				if (cnt >= off) {
					sprintf(buf, "Quest: %s", questdata->name);
					render_text(dotx(DOT_HLP) + 10, y, questcolor, 0, buf);
					y += 10;

					sprintf(buf, "From: %s in %s.", questdata->giver, questdata->area);
					render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, buf);
					y += 10;

					sprintf(buf, "Status: %s.", (quest[qid].flags & QF_DONE) ? "Quest Complete" : "Open");
					render_text(dotx(DOT_HLP) + 10, y, graycolor, 0, buf);
					y += 10;
					y += 10;
				}
				if (cnt - off >= 0 && cnt - off < 10) {
					questonscreen[cnt - off] = qid;
				}
				cnt++;
				if (cnt >= off + 9) {
					break;
				}
			}
		}
		if (cnt >= off + 9) {
			break;
		}
	}
	if (cnt == 0) {
		y += 50;
		y = render_text(dotx(DOT_HLP) + (10 + 202) / 2, y, whitecolor, RENDER_ALIGN_CENTER, "Your questlog is empty.");
	}

	return y;
}

void quest_select(int nr)
{
	(void)nr;
}

void questlog_metadata_updated(void)
{
	questinit = 0;
}
