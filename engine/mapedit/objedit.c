/*	$Csoft: objedit.c,v 1.2 2003/05/22 05:41:07 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <engine/engine.h>
#include <engine/typesw.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/tlist.h>
#include <engine/widget/text.h>

#include "mapedit.h"

/* Create a new object. */
static void
create_obj(int argc, union evarg *argv)
{
	char name[OBJECT_NAME_MAX];
	char type[OBJECT_TYPE_MAX];
	struct textbox *name_tb = argv[1].p;
	struct textbox *type_tb = argv[2].p;
	struct tlist *objs_tl = argv[3].p;
	struct tlist_item *it;
	void *nobj;
	int i;

	textbox_copy_string(name_tb, name, sizeof(name));
	textbox_copy_string(type_tb, type, sizeof(type));

	if (name[0] == '\0') {
		text_msg("Error", "No object name specified");
		return;
	}
	if (type[0] == '\0') {
		text_msg("Error", "No object type specified");
		return;
	}

	/* Look for a matching type. */
	for (i = 0; i < ntypesw; i++) {
		if (strcmp(typesw[i].type, type) == 0)
			break;
	}
	if (i == ntypesw) {
		text_msg("Error", "Unknown object type `%s'", type);
		return;
	}

	nobj = Malloc(typesw[i].size);
	if (typesw[i].ops->init != NULL) {
		typesw[i].ops->init(nobj, name);
	} else {
		object_init(nobj, type, name, NULL);
	}

	it = tlist_item_selected(objs_tl);
	object_attach((it != NULL) ? it->p1 : world, nobj);
}

/* Recursive function to display the object tree. */
static void
find_objs(struct tlist *tl, struct object *pob, int depth)
{
	char label[TLIST_LABEL_MAX];
	char dind[TLIST_LABEL_MAX];
	struct object *cob;
	size_t dsize = depth*4;
	struct tlist_item *it;
	int i;

	if (dsize >= sizeof(dind))
		dsize = sizeof(dind)-1;
	memset(dind, ' ', dsize);
	dind[dsize] = '\0';

	TAILQ_FOREACH(cob, &pob->childs, cobjs) {
		for (i = 0; i < ntypesw; i++) {
			if (strcmp(typesw[i].type, cob->type) == 0)
				break;
		}
#if 0
		snprintf(label, sizeof(label), "%s %s\n%s %s\n", dind,
		    cob->name,
		    dind, i == ntypesw ? cob->type : typesw[i].desc);
#else
		snprintf(label, sizeof(label), "%s %s\n", dind, cob->name);
#endif
		it = tlist_insert_item(tl, OBJECT_ICON(cob), label, cob);

		if (!TAILQ_EMPTY(&cob->childs)) {
			it->haschilds++;
			if (tlist_visible_childs(tl, it)) {
				find_objs(tl, cob, depth+1);
			}
		}
	}
}

/* Update the object tree display. */
static void
poll_objs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct object *pob = argv[1].p;

	lock_linkage();
	tlist_clear_items(tl);
	find_objs(tl, pob, 0);
	tlist_restore_selections(tl);
	unlock_linkage();
}

/* Create the object editor window. */
struct window *
objedit_window(void)
{
	struct window *win;
	struct region *reg;
	struct textbox *name_tb, *type_tb;
	struct button *create_bu;
	struct tlist *objs_tl;

	win = window_generic_new(320, 240, "mapedit-objedit");
	window_set_caption(win, "Object editor");
	event_new(win, "window-close", window_generic_hide, "%p", win);

	reg = region_new(win, REGION_HALIGN, 0, 0, 100, -1);
	{
		name_tb = textbox_new(reg, "New: ", 0, 75, -1);
		create_bu = button_new(reg, "Create", NULL, 0, 23, -1);
		button_set_padding(create_bu, 6);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		type_tb = textbox_new(reg, "Type: ", 0, 100, -1);
		textbox_printf(type_tb, "map");
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, 0);
	{
		objs_tl = tlist_new(reg, 100, 100,
		    TLIST_POLL|TLIST_MULTI|TLIST_TREE);
#if 0
		tlist_set_item_height(objs_tl, ttf_font_height(font)*2);
#endif
		event_new(objs_tl, "tlist-poll", poll_objs, "%p", world);
	}
	
	event_new(name_tb, "textbox-return", create_obj, "%p, %p, %p",
	    name_tb, type_tb, objs_tl);
	event_new(create_bu, "button-pushed", create_obj, "%p, %p, %p",
	    name_tb, type_tb, objs_tl);

	window_show(win);
	return (win);
}

