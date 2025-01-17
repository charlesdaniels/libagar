/*
 * Copyright (c) 2001-2018 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Properties dialog for AG_Object.
 */

#include <agar/core/core.h>

#include <agar/gui/window.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/tlist.h>
#include <agar/gui/textbox.h>
#include <agar/gui/notebook.h>
#include <agar/gui/separator.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/iconmgr.h>

#include <agar/dev/dev.h>

const AG_FlagDescr devObjectFlags[] = {
	{ AG_OBJECT_DEBUG,		N_("Debugging"),		  1 },
	{ AG_OBJECT_READONLY,		N_("Read-only"),		  1 },
	{ AG_OBJECT_INDESTRUCTIBLE,	N_("Indestructible"),		  1 },
	{ AG_OBJECT_NON_PERSISTENT,	N_("Non-persistent"),		  1 },
	{ AG_OBJECT_PRESERVE_DEPS,	N_("Preserve null dependencies"), 1 },
	{ AG_OBJECT_REMAIN_DATA,	N_("Keep data resident"),	  1 },
	{ AG_OBJECT_RESIDENT,		N_("Data part is resident"),	  0 },
	{ AG_OBJECT_STATIC,		N_("Statically allocated"),	  0 },
	{ AG_OBJECT_REOPEN_ONLOAD,	N_("Recreate UI on load"),	  0 },
	{ AG_OBJECT_NAME_ONATTACH,	N_("Generate name upon attach"),  0 },
	{ 0,				"",				  0 }
};

static void
PollDeps(AG_Event *_Nonnull event)
{
	char path[AG_OBJECT_PATH_MAX];
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_ObjectDep *dep;

	AG_TlistClear(tl);
	AG_LockVFS(ob);
	TAILQ_FOREACH(dep, &ob->deps, deps) {
		char label[AG_TLIST_LABEL_MAX];
	
		if (dep->obj != NULL) {
			AG_ObjectCopyName(dep->obj, path, sizeof(path));
		} else {
			Strlcpy(path, "(NULL)", sizeof(path));
		}
		if (dep->count == AG_OBJECT_DEP_MAX) {
			Snprintf(label, sizeof(label), "%s (wired)", path);
		} else {
			Snprintf(label, sizeof(label), "%s (%u)", path,
			    (Uint)dep->count);
		}
		AG_TlistAddPtr(tl, NULL, label, dep);
	}
	AG_UnlockVFS(ob);
	AG_TlistRestore(tl);
}

static void
PollVariables(AG_Event *_Nonnull event)
{
	char val[64];
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_Variable *V;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(V, &ob->vars, vars) {
		AG_TlistItem *ti;

		AG_LockVariable(V);
		AG_PrintVariable(val, sizeof(val), V);
		ti = AG_TlistAdd(tl, NULL, "%s = %s", V->name, val);
		ti->p1 = V;
		AG_UnlockVariable(V);
	}
	AG_TlistRestore(tl);
}

static void
PollEvents(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *ob = AG_PTR(1);
	AG_Event *ev;
	
	AG_TlistClear(tl);
	TAILQ_FOREACH(ev, &ob->events, events) {
		char args[AG_TLIST_LABEL_MAX], arg[64];
		int i;

		args[0] = '(';
		args[1] = '\0';
		for (i = 1; i < ev->argc; i++) {
			AG_Variable *V = &ev->argv[i];

			AG_LockVariable(V);
			if (V->name[0] != '\0') {
				Strlcat(args, V->name, sizeof(args));
				Strlcat(args, "=", sizeof(args));
			}
			AG_PrintVariable(arg, sizeof(arg), &ev->argv[i]);
			AG_UnlockVariable(V);

			Strlcat(args, arg, sizeof(args));
			if (i < ev->argc-1)
				Strlcat(args, ", ", sizeof(args));
		}
		Strlcat(args, ")", sizeof(args));

		AG_TlistAdd(tl, NULL, "%s%s%s %s", ev->name,
		    (ev->flags & AG_EVENT_ASYNC) ? " <async>" : "",
		    (ev->flags & AG_EVENT_PROPAGATE) ? " <propagate>" : "",
		    args);

	}
	AG_TlistRestore(tl);
}

static void
RenameObject(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_SELF();
	AG_Object *ob = AG_PTR(1);

	if (AG_ObjectPageIn(ob) == 0) {
		AG_ObjectUnlinkDatafiles(ob);
		AG_TextboxCopyString(tb, ob->name, sizeof(ob->name));
		AG_ObjectPageOut(ob);
	}
	AG_PostEvent(NULL, ob, "renamed", NULL);
}

void *
DEV_ObjectEdit(void *p)
{
	AG_Object *ob = p;
	AG_Window *win;
	AG_Textbox *tbox;
	AG_Notebook *nb;
	AG_NotebookTab *ntab;
	AG_Tlist *tl;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Object %s"), ob->name);
	AG_WindowSetPosition(win, AG_WINDOW_UPPER_RIGHT, 1);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL);
	ntab = AG_NotebookAdd(nb, _("Infos"), AG_BOX_VERT);
	{
		tbox = AG_TextboxNewS(ntab, AG_TEXTBOX_HFILL, _("Name: "));
		AG_TextboxPrintf(tbox, ob->name);
		AG_WidgetFocus(tbox);
		AG_SetEvent(tbox, "textbox-return", RenameObject, "%p", ob);
		
		AG_SeparatorNew(ntab, AG_SEPARATOR_HORIZ);
	
		AG_LabelNew(ntab, 0, _("Class: %s"), ob->cls->hier);
		AG_CheckboxSetFromFlags(ntab, 0, &ob->flags, devObjectFlags);
#if 0
		AG_LabelNewPolledMT(ntab, AG_LABEL_HFILL, &agLinkageLock,
		    _("Parent: %[obj]"), &ob->parent);
#endif
		AG_LabelNew(ntab, 0, _("Save prefix: %s"),
		    ob->save_pfx != NULL ? ob->save_pfx : AG_PATHSEP);
	}

	ntab = AG_NotebookAdd(nb, _("Deps"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_TlistSizeHint(tl, "XXXXXXXXXXXX", 6);
		AG_SetEvent(tl, "tlist-poll", PollDeps, "%p", ob);
	}
	
	ntab = AG_NotebookAdd(nb, _("Events"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollEvents, "%p", ob);
	}
	
	ntab = AG_NotebookAdd(nb, _("Variables"), AG_BOX_VERT);
	{
		tl = AG_TlistNew(ntab, AG_TLIST_POLL|AG_TLIST_EXPAND);
		AG_SetEvent(tl, "tlist-poll", PollVariables, "%p", ob);
	}
	return (win);
}
