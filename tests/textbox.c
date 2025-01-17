/*	Public domain	*/
/*
 * This application demonstrates different uses for the Textbox widget.
 */

#include "agartest.h"

#include <agar/core/snprintf.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char bufferShd[60];	/* Shared text buffer */
char bufferExcl[60];	/* Exclusive text buffer */


static void
SetDisable(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	if (flag) {
		AG_WidgetDisable(textbox);
	} else {
		AG_WidgetEnable(textbox);
	}
}

static void
SetWordWrap(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	int flag = AG_INT(2);

	AG_TextboxSetWordWrap(textbox, flag);
}

static void
SetString(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);

	AG_TextboxPrintf(textbox, "Formatted string");
	AG_TextboxSetString(textbox, "Set string");
}

static void
DupString(AG_Event *event)
{
	AG_Textbox *textbox = AG_PTR(1);
	char *s;

	if ((s = AG_TextboxDupString(textbox)) != NULL) {
		AG_TextMsg(AG_MSG_INFO, "Duplicated string:\n\"%s\"\n", s);
		Free(s);
	} else {
		AG_TextMsgS(AG_MSG_INFO, "Failed");
	}
}

static void
CopyString(AG_Event *event)
{
	char tinybuf[10];
	AG_Textbox *textbox = AG_PTR(1);

	AG_TextboxCopyString(textbox, tinybuf, sizeof(tinybuf));
	AG_TextMsg(AG_MSG_INFO, "String truncated to fit %d-byte buffer:\n"
	                        "\"%s\"\n", (int)sizeof(tinybuf), tinybuf);
}

static void
DebugStuff(AG_Box *win, AG_Textbox *textbox)
{
	AG_Box *hBox;
	AG_Editable *ed = textbox->ed;

	AG_SeparatorNewHoriz(win);

#ifdef AG_ENABLE_STRING
	AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &AGOBJECT(ed)->pvt.lock,
	    "Cursor at: %i", &ed->pos);
	AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &AGOBJECT(ed)->pvt.lock,
	    "Selection: %i", &ed->sel);
	AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &AGOBJECT(ed)->pvt.lock,
	    "x: %i", &ed->x);
#endif
	AG_SeparatorNewHoriz(win);

	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);

	AG_CheckboxNewFlag(win, 0, "Multiline",           &ed->flags, AG_EDITABLE_MULTILINE);
	AG_CheckboxNewFlag(win, 0, "Blink",               &ed->flags, AG_EDITABLE_BLINK_ON);
	AG_CheckboxNewFlag(win, 0, "Read-only",	          &ed->flags, AG_EDITABLE_READONLY);
	AG_CheckboxNewFlag(win, 0, "Password input",      &ed->flags, AG_EDITABLE_PASSWORD);
	AG_CheckboxNewFlag(win, 0, "Force integer input", &ed->flags, AG_EDITABLE_INT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Force float input",   &ed->flags, AG_EDITABLE_FLT_ONLY);
	AG_CheckboxNewFlag(win, 0, "Keep cursor visible", &ed->flags, AG_EDITABLE_KEEPVISCURSOR);
	AG_CheckboxNewFlag(win, 0, "No emacs keys",       &ed->flags, AG_EDITABLE_NOEMACS);
	AG_CheckboxNewFlag(win, 0, "No latin1 keys",      &ed->flags, AG_EDITABLE_NOLATIN1);

	AG_SeparatorNewHoriz(win);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	{
		AG_ButtonNewFn(hBox, 0, "Set string", SetString, "%p", textbox);
		AG_ButtonNewFn(hBox, 0, "Dup string", DupString, "%p", textbox);
		AG_ButtonNewFn(hBox, 0, "Copy string", CopyString, "%p", textbox);
	}
}

static void
SingleLineExample(AG_Event *event)
{
	const char *text = "The Quick Brown Fox Jumps Over The Lazy Dog";
	AG_Window *winParent = AG_PTR(1);
	AG_Window *win;
	AG_Box *hBox, *vBox;
	AG_Textbox *textbox;
#ifdef AG_UNICODE
	AG_Text *txt;
#endif
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "textbox: Single-line Example");
	hBox = AG_BoxNewHoriz(win, AG_BOX_EXPAND);

	/*
	 * Create two single-line Textbox widgets bound to the same
	 * fixed-size buffer.
	 */
	vBox = AG_BoxNewVert(hBox, AG_BOX_VFILL);
	{
		AG_Strlcpy(bufferShd, text, sizeof(bufferShd));

		textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL,
		    "Fixed C buffer (shared): ");
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(textbox, bufferShd, sizeof(bufferShd));
#else
		AG_TextboxBindASCII(textbox, bufferShd, sizeof(bufferShd));
#endif
		AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXXXXXXX");
		AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
		AG_WidgetFocus(textbox);
		DebugStuff(vBox, textbox);

		textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL,
		    "Fixed C buffer (shared): ");
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(textbox, bufferShd, sizeof(bufferShd));
#else
		AG_TextboxBindASCII(textbox, bufferShd, sizeof(bufferShd));
#endif
		DebugStuff(vBox, textbox);
	}

	AG_SeparatorNewVert(hBox);

	/*
	 * Create a single-line Textbox widget using a fixed-size buffer
	 * in exclusive mode.
	 */
	 
	vBox = AG_BoxNewVert(hBox, AG_BOX_VFILL);
	{
		AG_Strlcpy(bufferExcl, text, sizeof(bufferExcl));

		textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL|AG_TEXTBOX_EXCL,
		    "Fixed C buffer (excl): ");
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(textbox, bufferExcl, sizeof(bufferExcl));
#else
		AG_TextboxBindASCII(textbox, bufferExcl, sizeof(bufferExcl));
#endif
		AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXXXXXXX");
		AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
		AG_WidgetFocus(textbox);
		DebugStuff(vBox, textbox);
	}

#ifdef AG_UNICODE
	/* Create a single-line Textbox bound to an AG_Text object. */
	vBox = AG_BoxNewVert(hBox, AG_BOX_VFILL);
	txt = AG_TextNew(0);
	{
		AG_TextSetEntS(txt, AG_LANG_EN, "Hello");
		AG_TextSetEntS(txt, AG_LANG_FR, "Bonjour");
		AG_TextSetEntS(txt, AG_LANG_DE, "Guten Tag");
	}
	textbox = AG_TextboxNew(vBox, AG_TEXTBOX_HFILL|AG_TEXTBOX_MULTILINGUAL,
	    "AG_Text element: ");
	AG_TextboxBindText(textbox, txt);
	AG_TextboxSetLang(textbox, AG_LANG_FR);
	AG_TextboxSizeHint(textbox, "XXXXXXXXXXXXXXXXXXXXX");
	AG_TextboxSetCursorPos(textbox, -1);	/* To end of string */
	DebugStuff(vBox, textbox);
#endif

	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static void
MultiLineExample(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Window *winParent = AG_PTR(1);
	AG_Window *win;
	AG_Textbox *textbox;
	char *someText;
	FILE *f;
	AG_Size size, bufSize;
	unsigned int flags;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, "textbox: Multi-line example");

	/*
	 * Create a multiline textbox.
	 */
	flags = AG_TEXTBOX_MULTILINE|AG_TEXTBOX_CATCH_TAB|AG_TEXTBOX_EXPAND|
	        AG_TEXTBOX_EXCL;
	textbox = AG_TextboxNewS(win, flags, NULL);

	/*
	 * Load the contents of this file into a buffer. Make the buffer a
	 * bit larger so the user can try entering text.
	 */
	if (!AG_ConfigFind(AG_CONFIG_PATH_DATA, "loss.txt", path, sizeof(path)) &&
	    (f = fopen(path, "r")) != NULL) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, 0, SEEK_SET);
		bufSize = size+1024;
		someText = AG_Malloc(bufSize);
		fread(someText, size, 1, f);
		fclose(f);
		someText[size] = '\0';
	} else {
		someText = AG_Strdup("Failed to load loss.txt");
		bufSize = strlen(someText)+1;
	}

#ifdef AG_UNICODE
	/*
	 * Bind the buffer's contents to the Textbox. The size argument to
	 * AG_TextboxBindUTF8() must include space for the terminating NUL.
	 */
	AG_TextboxBindUTF8(textbox, someText, bufSize);
#else
	AG_TextboxBindASCII(textbox, someText, bufSize);
#endif

	AG_CheckboxNewFlag(win, 0, "Read-only",
	    &textbox->ed->flags, AG_EDITABLE_READONLY);
	AG_CheckboxNewFn(win, 0, "Disable input", SetDisable, "%p", textbox);
	AG_CheckboxNewFn(win, 0, "Word wrapping", SetWordWrap, "%p", textbox);
	AG_SeparatorNewHoriz(win);
	{
#ifdef AG_ENABLE_STRING
		AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Lines: %d", &textbox->ed->yMax);
		AG_LabelNewPolled(win, AG_LABEL_HFILL,
		    "Cursor position: %d", &textbox->ed->pos);
#endif
	}
	AG_WindowSetGeometryAligned(win, AG_WINDOW_MC, 540, 380);
	AG_WindowAttach(winParent, win);
	AG_WindowShow(win);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	AG_ButtonNewFn(win, 0, "Test multi-line textbox", MultiLineExample, "%p", win);
	AG_ButtonNewFn(win, 0, "Test single-line textbox", SingleLineExample, "%p", win);
	return (0);
}

const AG_TestCase textboxTest = {
	"textbox",
	N_("Test AG_Textbox(3) / AG_Editable(3) widgets"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	NULL,		/* init */
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
