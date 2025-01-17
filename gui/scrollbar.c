/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/scrollbar.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/gui_math.h>

#define SBPOS(sb,x,y) (((sb)->type == AG_SCROLLBAR_HORIZ) ? (x) : (y))
#define SBLEN(sb)     (((sb)->type == AG_SCROLLBAR_HORIZ) ? WIDTH(sb) : HEIGHT(sb))

AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type, Uint flags)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar));
	AG_ObjectInit(sb, &agScrollbarClass);
	sb->type = type;
	sb->flags |= flags;

	if (flags & AG_SCROLLBAR_HFILL) { AG_ExpandHoriz(sb); }
	if (flags & AG_SCROLLBAR_VFILL) { AG_ExpandVert(sb); }
	if (flags & AG_SCROLLBAR_NOAUTOHIDE) { sb->flags &= ~(AG_SCROLLBAR_AUTOHIDE); }

	AG_ObjectAttach(parent, sb);
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewHoriz(void *parent, Uint flags)
{
	return AG_ScrollbarNew(parent, AG_SCROLLBAR_HORIZ, flags);
}

AG_Scrollbar *
AG_ScrollbarNewVert(void *parent, Uint flags)
{
	return AG_ScrollbarNew(parent, AG_SCROLLBAR_VERT, flags);
}

/* Configure an initial length for the size requisition. */
void
AG_ScrollbarSizeHint(AG_Scrollbar *sb, int len)
{
	AG_ObjectLock(sb);
	sb->lenPre = len;
	AG_ObjectUnlock(sb);
}

/* Set/retrieve scrolling control length */
void
AG_ScrollbarSetControlLength(AG_Scrollbar *sb, int bsize)
{
	AG_ObjectLock(sb);
	sb->wBar = (bsize > 10 || bsize == -1) ? bsize : 10;
	sb->length = (sb->type == AG_SCROLLBAR_VERT) ? AGWIDGET(sb)->h :
	                                               AGWIDGET(sb)->w;
	sb->length -= sb->width*2;
	sb->length -= sb->wBar;
	AG_ObjectUnlock(sb);
}

/* Set scrollbar width in pixels. */
void
AG_ScrollbarSetWidth(AG_Scrollbar *sb, int width)
{
	AG_ObjectLock(sb);
	sb->width = width;
	AG_ObjectUnlock(sb);
}

/* Return the current width of a scrollbar in pixels. */
int
AG_ScrollbarWidth(AG_Scrollbar *sb)
{
	int w;

	AG_ObjectLock(sb);
	w = sb->width;
	AG_ObjectUnlock(sb);
	return (w);
}

/* Set an alternate handler for UP/LEFT button click. */
void
AG_ScrollbarSetIncFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	if (fn != NULL) {
		sb->buttonIncFn = AG_SetEvent(sb, NULL, fn, NULL);
		AG_EVENT_GET_ARGS(sb->buttonIncFn, fmt);
	} else {
		sb->buttonIncFn = NULL;
	}
	AG_ObjectUnlock(sb);
}

/* Set an alternate handler for DOWN/RIGHT button click. */
void
AG_ScrollbarSetDecFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	sb->buttonDecFn = AG_SetEvent(sb, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(sb->buttonDecFn, fmt);
	AG_ObjectUnlock(sb);
}

/*
 * Return the the current position and size (in pixels) of the scrollbar
 * control. Returns 0 on success, or -1 if the values are outside the range.
 */
#undef GET_EXTENT_PX
#define GET_EXTENT_PX(TYPE) {						\
	if (vis > 0) {							\
		if ((max - min) > 0) {					\
			extentPx = sb->length -				\
			    (int)(vis * sb->length / (max - min));	\
		} else {						\
			extentPx = 0;					\
		}							\
	} else {							\
		extentPx = (sb->wBar == -1) ? 0 : (sb->length - sb->wBar); \
	}								\
}
#define GET_PX_COORDS(TYPE) {						\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	TYPE vis = *(TYPE *)pVis;					\
	int extentPx, divPx;						\
									\
	if (min >= (max - vis)) {					\
		goto fail;						\
	}								\
	GET_EXTENT_PX(TYPE);						\
	divPx = (int)(max - vis - min);					\
	if (divPx < 1) { goto fail; }					\
	*x = (int)(((*(TYPE *)pVal - min) * extentPx) / divPx);		\
	*len = (vis > 0) ? (vis * sb->length / (max - min)) :			\
	                    (sb->wBar == -1) ? sb->length : sb->wBar;	\
}
static __inline__ int
GetPxCoords(AG_Scrollbar *_Nonnull sb, int *_Nonnull x, int *_Nonnull len)
{
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	*x = 0;
	*len = 0;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		GET_PX_COORDS(int);	break;
	case AG_VARIABLE_UINT:		GET_PX_COORDS(Uint);	break;
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:		GET_PX_COORDS(float);	break;
	case AG_VARIABLE_DOUBLE:	GET_PX_COORDS(double);	break;
# ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	GET_PX_COORDS(long double);	break;
# endif
#endif
	case AG_VARIABLE_UINT8:		GET_PX_COORDS(Uint8);	break;
	case AG_VARIABLE_SINT8:		GET_PX_COORDS(Sint8);	break;
	case AG_VARIABLE_UINT16:	GET_PX_COORDS(Uint16);	break;
	case AG_VARIABLE_SINT16:	GET_PX_COORDS(Sint16);	break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:	GET_PX_COORDS(Uint32);	break;
	case AG_VARIABLE_SINT32:	GET_PX_COORDS(Sint32);	break;
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	GET_PX_COORDS(Uint64);	break;
	case AG_VARIABLE_SINT64:	GET_PX_COORDS(Sint64);	break;
#endif
	default:						break;
	} 

	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (0);
fail:
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (-1);
}
#undef GET_PX_COORDS

/*
 * Map specified pixel coordinates to a value.
 */
#define MAP_PX_COORDS(TYPE) {						\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	TYPE vis = *(TYPE *)pVis;					\
	int extentPx;							\
									\
	if (*(TYPE *)pMax > *(TYPE *)pVis) {				\
		GET_EXTENT_PX(TYPE);					\
		if (x <= 0) {						\
			*(TYPE *)pVal = min;				\
		} else if (x >= (int)extentPx) {			\
			*(TYPE *)pVal = MAX(min, (max - vis));		\
		} else {						\
			*(TYPE *)pVal = min + x*(max-vis-min)/extentPx;	\
			if (*(TYPE *)pVal < min) { *(TYPE *)pVal = min;	} \
			if (*(TYPE *)pVal > max) { *(TYPE *)pVal = max;	} \
		}							\
	}								\
}
static __inline__ void
SeekToPxCoords(AG_Scrollbar *_Nonnull sb, int x)
{
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		MAP_PX_COORDS(int);		break;
	case AG_VARIABLE_UINT:		MAP_PX_COORDS(Uint);		break;
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:		MAP_PX_COORDS(float);		break;
	case AG_VARIABLE_DOUBLE:	MAP_PX_COORDS(double);		break;
# ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	MAP_PX_COORDS(long double);	break;
# endif
#endif
	case AG_VARIABLE_UINT8:		MAP_PX_COORDS(Uint8);		break;
	case AG_VARIABLE_SINT8:		MAP_PX_COORDS(Sint8);		break;
	case AG_VARIABLE_UINT16:	MAP_PX_COORDS(Uint16);		break;
	case AG_VARIABLE_SINT16:	MAP_PX_COORDS(Sint16);		break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:	MAP_PX_COORDS(Uint32);		break;
	case AG_VARIABLE_SINT32:	MAP_PX_COORDS(Sint32);		break;
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	MAP_PX_COORDS(Uint64);		break;
	case AG_VARIABLE_SINT64:	MAP_PX_COORDS(Sint64);		break;
#endif
	default:							break;
	} 

	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sb);
}
#undef MAP_PX_COORDS

/*
 * Type-independent increment/decrement operation.
 */
#undef INCREMENT
#define INCREMENT(TYPE)	{						\
	if (*(TYPE *)pMax > *(TYPE *)pVis) {				\
		if ((*(TYPE *)pVal + *(TYPE *)pInc) >			\
		    (*(TYPE *)pMax - (*(TYPE *)pVis))) {		\
			*(TYPE *)pVal = (*(TYPE *)pMax) - (*(TYPE *)pVis); \
			rv = 1;						\
		} else { 						\
			*(TYPE *)pVal += *(TYPE *)pInc;			\
		}							\
	}								\
}
#undef DECREMENT
#define DECREMENT(TYPE) {						\
	if (*(TYPE *)pMax > *(TYPE *)pVis) {				\
		if (*(TYPE *)pVal < *(TYPE *)pMin + *(TYPE *)pInc) {	\
			*(TYPE *)pVal = *(TYPE *)pMin;			\
			rv = 1;						\
		} else { 						\
			*(TYPE *)pVal -= *(TYPE *)pInc;			\
		}							\
	}								\
}
static int
Increment(AG_Scrollbar *_Nonnull sb)
{
	AG_Variable *bVal, *bMin, *bMax, *bInc, *bVis;
	void *pVal, *pMin, *pMax, *pInc, *pVis;
	int rv = 0;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bInc = AG_GetVariable(sb, "inc", &pInc);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		INCREMENT(int);		break;
	case AG_VARIABLE_UINT:		INCREMENT(Uint);	break;
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:		INCREMENT(float);	break;
	case AG_VARIABLE_DOUBLE:	INCREMENT(double);	break;
# ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	INCREMENT(long double);	break;
# endif
#endif
	case AG_VARIABLE_UINT8:		INCREMENT(Uint8);	break;
	case AG_VARIABLE_SINT8:		INCREMENT(Sint8);	break;
	case AG_VARIABLE_UINT16:	INCREMENT(Uint16);	break;
	case AG_VARIABLE_SINT16:	INCREMENT(Sint16);	break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:	INCREMENT(Uint32);	break;
	case AG_VARIABLE_SINT32:	INCREMENT(Sint32);	break;
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	INCREMENT(Uint64);	break;
	case AG_VARIABLE_SINT64:	INCREMENT(Sint64);	break;
#endif
	default:						break;
	} 

	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVal);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bInc);
	AG_UnlockVariable(bVis);
	AG_Redraw(sb);
	return (rv);
}
static int
Decrement(AG_Scrollbar *_Nonnull sb)
{
	AG_Variable *bVal, *bMin, *bMax, *bInc, *bVis;
	void *pVal, *pMin, *pMax, *pInc, *pVis;
	int rv = 0;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bInc = AG_GetVariable(sb, "inc", &pInc);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		DECREMENT(int);		break;
	case AG_VARIABLE_UINT:		DECREMENT(Uint);	break;
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:		DECREMENT(float);	break;
	case AG_VARIABLE_DOUBLE:	DECREMENT(double);	break;
# ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	DECREMENT(long double);	break;
# endif
#endif
	case AG_VARIABLE_UINT8:		DECREMENT(Uint8);	break;
	case AG_VARIABLE_SINT8:		DECREMENT(Sint8);	break;
	case AG_VARIABLE_UINT16:	DECREMENT(Uint16);	break;
	case AG_VARIABLE_SINT16:	DECREMENT(Sint16);	break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32:	DECREMENT(Uint32);	break;
	case AG_VARIABLE_SINT32:	DECREMENT(Sint32);	break;
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	DECREMENT(Uint64);	break;
	case AG_VARIABLE_SINT64:	DECREMENT(Sint64);	break;
#endif
	default:						break;
	}

	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVal);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bInc);
	AG_UnlockVariable(bVis);
	AG_Redraw(sb);
	return (rv);
}
#undef INCREMENT
#undef DECREMENT

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();

#ifdef AG_TIMERS
	AG_DelTimer(sb, &sb->moveTo);
#endif
	if (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC && sb->buttonDecFn != NULL) {
		AG_PostEventByPtr(NULL, sb, sb->buttonDecFn, "%i", 0);
	}
	if (sb->curBtn == AG_SCROLLBAR_BUTTON_INC && sb->buttonIncFn != NULL) {
		AG_PostEventByPtr(NULL, sb, sb->buttonIncFn, "%i", 0);
	}

	if (sb->curBtn != AG_SCROLLBAR_BUTTON_NONE) {
		sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
		sb->xOffs = 0;
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-end", NULL);
	AG_Redraw(sb);
}

#ifdef AG_TIMERS
/* Timer for scrolling controlled by buttons (mouse spin setting). */
static Uint32
MoveButtonsTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int dir = AG_INT(1);
	int rv;

	if (dir == -1) {
		rv = Decrement(sb);
	} else {
		rv = Increment(sb);
	}
	if (sb->xSeek != -1) {
		int pos, len;
		
		if (GetPxCoords(sb, &pos, &len) == -1 ||
		    ((dir == -1 && sb->xSeek >= pos) ||
		     (dir == +1 && sb->xSeek <= pos+len))) {
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xSeek = -1;
			if (dir == +1) { sb->xOffs = len; }
			return (0);
		}
	}
	return (rv != 1) ? agMouseScrollIval : 0;
}

/* Timer for scrolling controlled by keyboard (keyrepeat setting). */
static Uint32
MoveKbdTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int dir = AG_INT(1);
	int rv;

	if (dir == -1) {
		rv = Decrement(sb);
	} else {
		rv = Increment(sb);
	}
	return (rv != 1) ? agKbdRepeat : 0;
}
#endif /* AG_TIMERS */

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = SBPOS(sb, AG_INT(2), AG_INT(3)) - sb->width;
	int totsize = SBLEN(sb);

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	if (!AG_WidgetIsFocused(sb)) {
		AG_WidgetFocus(sb);
	}
	if (x < 0) {						/* Decrement */
		sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
		if (sb->buttonDecFn != NULL) {
			AG_PostEventByPtr(NULL, sb, sb->buttonDecFn, "%i", 1);
		} else {
			if (Decrement(sb) != 1) {
				sb->xSeek = -1;
#ifdef AG_TIMERS
				AG_AddTimer(sb, &sb->moveTo, agMouseScrollDelay,
				    MoveButtonsTimeout, "%i", -1);
#endif
			}
		}
	} else if (x > totsize - (sb->width << 1)) {		/* Increment */
		sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
		if (sb->buttonIncFn != NULL) {
			AG_PostEventByPtr(NULL, sb, sb->buttonIncFn, "%i", 1);
		} else {
			if (Increment(sb) != 1) {
				sb->xSeek = -1;
#ifdef AG_TIMERS
				AG_AddTimer(sb, &sb->moveTo, agMouseScrollDelay,
				    MoveButtonsTimeout, "%i", +1);
#endif
			}
		}
	} else {
		int pos, len;

		if (GetPxCoords(sb, &pos, &len) == -1) {	/* No range */
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xOffs = x;
		} else if (x >= pos && x <= pos+len) {
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xOffs = (x - pos);
		} else {
			if (x < pos) {
				sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
				if (Decrement(sb) != 1) {
					sb->xSeek = x;
#ifdef AG_TIMERS
					AG_AddTimer(sb, &sb->moveTo, agMouseScrollDelay,
					    MoveButtonsTimeout, "%i,", -1);
#endif
				}
			} else {
				sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
				if (Increment(sb) != 1) {
					sb->xSeek = x;
#ifdef AG_TIMERS
					AG_AddTimer(sb, &sb->moveTo, agMouseScrollDelay,
					    MoveButtonsTimeout, "%i", +1);
#endif
				}
			}
		}
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-begin", NULL);
	AG_Redraw(sb);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int mx = AG_INT(1);
	int my = AG_INT(2);
	int x = SBPOS(sb,mx,my) - sb->width;
	enum ag_scrollbar_button mouseOverBtn;

	if (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) {
		SeekToPxCoords(sb, x - sb->xOffs);
	} else if (AG_WidgetRelativeArea(sb, mx,my)) {
		if (x < 0) {
			mouseOverBtn = AG_SCROLLBAR_BUTTON_DEC;
		} else if (x > SBLEN(sb) - (sb->width << 1)) {
			mouseOverBtn = AG_SCROLLBAR_BUTTON_INC;
		} else {
			int pos, len;
	
			if (GetPxCoords(sb, &pos, &len) == -1 || /* No range */
			    (x >= pos && x <= pos+len)) {
				mouseOverBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			} else {
				mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
				if (sb->xSeek != -1)
					sb->xSeek = x;
			}
		}
		if (mouseOverBtn != sb->mouseOverBtn) {
			sb->mouseOverBtn = mouseOverBtn;
			AG_Redraw(sb);
		}
	} else {
		if (sb->mouseOverBtn != AG_SCROLLBAR_BUTTON_NONE) {
			sb->mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
			AG_Redraw(sb);
		}
	}

}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		if (Decrement(sb) != 1) {
#ifdef AG_TIMERS
			AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
			    MoveKbdTimeout, "%i", -1);
#endif
		}
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		if (Increment(sb) != 1) {
#ifdef AG_TIMERS
			AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
			    MoveKbdTimeout, "%i", +1);
#endif
		}
		break;
	}
}

#ifdef AG_TIMERS
static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	int keysym = AG_INT(1);
	
	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		AG_DelTimer(sb, &sb->moveTo);
		break;
	}
}

#if 0
/* Timer for AUTOHIDE visibility test. */
static Uint32
AutoHideTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	AG_Window *pwin;
	int rv, x, len;

	if ((pwin = WIDGET(sb)->window) == NULL ||
	    !pwin->visible) {
		return (to->ival);
	}

	rv = GetPxCoords(sb, &x, &len);
	if (rv == -1 || len == sb->length) {
		if (AG_WidgetVisible(sb))
			AG_WidgetHide(sb);
	} else {
		if (!AG_WidgetVisible(sb)) {
			AG_WidgetShow(sb);
			AG_Redraw(sb);
		}
	}
	return (to->ival);
}
#endif

static void
OnFocusLoss(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();

	AG_DelTimer(sb, &sb->moveTo);
}
#endif /* AG_TIMERS */

#undef SET_DEF
#define SET_DEF(fn,dmin,dmax,dinc) { 					\
	if (!AG_Defined(sb, "min")) { fn(sb, "min", dmin); }		\
	if (!AG_Defined(sb, "max")) { fn(sb, "max", dmax); }		\
	if (!AG_Defined(sb, "inc")) { fn(sb, "inc", dinc); }		\
	if (!AG_Defined(sb, "visible")) { fn(sb, "visible", 0); }	\
}
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	AG_Variable *V;

	if ((V = AG_AccessVariable(sb, "value")) == NULL) {
		V = AG_BindInt(sb, "value", &sb->value);
	}
	switch (AG_VARIABLE_TYPE(V)) {
#ifdef HAVE_FLOAT
	case AG_VARIABLE_FLOAT:	 SET_DEF(AG_SetFloat, 0.0f, 1.0f, 0.1f); break;
	case AG_VARIABLE_DOUBLE: SET_DEF(AG_SetDouble, 0.0, 1.0, 0.1); break;
# ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE: SET_DEF(AG_SetLongDouble, 0.0l, 1.0l, 0.1l); break;
# endif
#endif
	case AG_VARIABLE_INT:    SET_DEF(AG_SetInt, 0, AG_INT_MAX-1, 1); break;
	case AG_VARIABLE_UINT:   SET_DEF(AG_SetUint, 0U, AG_UINT_MAX-1, 1U); break;
	case AG_VARIABLE_UINT8:  SET_DEF(AG_SetUint8, 0U, 0xffU, 1U); break;
	case AG_VARIABLE_SINT8:  SET_DEF(AG_SetSint8, 0, 0x7f, 1); break;
	case AG_VARIABLE_UINT16: SET_DEF(AG_SetUint16, 0U, 0xffffU, 1U); break;
	case AG_VARIABLE_SINT16: SET_DEF(AG_SetSint16, 0, 0x7fff, 1); break;
#if AG_MODEL != AG_SMALL
	case AG_VARIABLE_UINT32: SET_DEF(AG_SetUint32, 0UL, 0xffffffffUL, 1UL); break;
	case AG_VARIABLE_SINT32: SET_DEF(AG_SetSint32, 0L, 0x7fffffffL, 1L); break;
#endif
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64: SET_DEF(AG_SetUint64, 0ULL, 0xffffffffffffffffULL, 1ULL); break;
	case AG_VARIABLE_SINT64: SET_DEF(AG_SetSint64, 0LL, 0x7fffffffffffffffLL, 1LL); break;
#endif
	default: break;
	}
	AG_UnlockVariable(V);

	if ((sb->flags & AG_SCROLLBAR_EXCL) == 0) {
		/* Trigger redraw upon external changes to the bindings. */
		AG_RedrawOnChange(sb, 500, "value");
		AG_RedrawOnChange(sb, 500, "min");
		AG_RedrawOnChange(sb, 500, "max");
		AG_RedrawOnChange(sb, 500, "visible");
	}
#if 0
	if (sb->flags & AG_SCROLLBAR_AUTOHIDE)
		AG_AddTimer(sb, &sb->autoHideTo, 250, AutoHideTimeout, NULL);
#endif
}
#undef SET_DEF

#ifdef AG_TIMERS
static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	
	AG_DelTimer(sb, &sb->moveTo);
}

static void
OnDetach(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SELF();
	
	if (sb->flags & AG_SCROLLBAR_AUTOHIDE)
		AG_DelTimer(sb, &sb->autoHideTo);
}
#endif /* AG_TIMERS */

static void
Init(void *_Nonnull obj)
{
	AG_Scrollbar *sb = obj;

	WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_FOCUSABLE;

	sb->type = AG_SCROLLBAR_HORIZ;
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->flags = AG_SCROLLBAR_AUTOHIDE;
	sb->buttonIncFn = NULL;
	sb->buttonDecFn = NULL;
	sb->xOffs = 0;
	sb->xSeek = -1;
	sb->length = 0;
	sb->lenPre = 32;
	sb->wBar = agTextFontHeight >> 1;
	sb->wBarMin = 16;
	sb->width = agTextFontHeight;
	sb->hArrow = sb->width >> 1;
	sb->value = 0;
	
	AG_AddEvent(sb, "widget-shown", OnShow, NULL);
	AG_SetEvent(sb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sb, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sb, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(sb, "key-down", KeyDown, NULL);
#ifdef AG_TIMERS
	AG_InitTimer(&sb->moveTo, "move", 0);
	AG_InitTimer(&sb->autoHideTo, "autoHide", 0);
	AG_AddEvent(sb, "detached", OnDetach, NULL);
	AG_AddEvent(sb, "widget-hidden", OnHide, NULL);
	AG_SetEvent(sb, "widget-lostfocus", OnFocusLoss, NULL);
	AG_SetEvent(sb, "key-up", KeyUp, NULL);
#endif

#if 0
	AG_BindInt(sb, "width", &sb->width);
	AG_BindInt(sb, "length", &sb->length);
	AG_BindInt(sb, "wBar", &sb->wBar);
	AG_BindInt(sb, "xOffs", &sb->xOffs);
#endif
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Scrollbar *sb = obj;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->w = (sb->width << 1) + sb->lenPre;
		r->h = sb->width;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->width;
		r->h = (sb->width << 1) + sb->lenPre;
		break;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Scrollbar *sb = obj;

	sb->length = ((sb->type == AG_SCROLLBAR_VERT) ? a->h : a->w) -
	             (sb->width << 1);
	if (sb->length < 0) {
		sb->length = 0;
	}
	return (0);
}

static void
DrawText(AG_Scrollbar *_Nonnull sb)
{
	char label[32];
	AG_Driver *drv = WIDGET(sb)->drv;
	AG_Surface *txt;
	AG_Rect r;

	AG_PushTextState();
	AG_TextColor(&WCOLOR(sb,TEXT_COLOR));

	Snprintf(label, sizeof(label),
	    (sb->type == AG_SCROLLBAR_HORIZ) ?
	    "%d|%d|%d|%d" : "%d\n:%d\n%d\nv%d\n",
	    AG_GetInt(sb,"min"),
	    AG_GetInt(sb,"value"),
	    AG_GetInt(sb,"max"),
	    AG_GetInt(sb,"visible"));

	txt = AG_TextRender(label);		/* XXX inefficient */

	r.x = (WIDTH(sb)  >> 1) - (txt->w >> 1);
	r.y = (HEIGHT(sb) >> 1) - (txt->h >> 1);
	r.w = txt->w;
	r.h = txt->h;
	AG_WidgetBlit(sb, txt, r.x, r.y);
	AG_SurfaceFree(txt);

	if (AGDRIVER_CLASS(drv)->updateRegion != NULL) {
		AG_RectTranslate(&r,
		    WIDGET(sb)->rView.x1,
		    WIDGET(sb)->rView.y1);
		AGDRIVER_CLASS(drv)->updateRegion(drv, &r);
	}
	AG_PopTextState();
}

static void
DrawVertUndersize(AG_Scrollbar *_Nonnull sb)
{
	AG_Rect r;
	int w_2, s;

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(sb);
	r.h = HEIGHT(sb);
	AG_DrawBox(sb, &r, 1, &WCOLOR(sb,0));
	
	w_2 = (r.w >> 1);
	s = MIN(r.h>>2, r.w);

	AG_DrawArrowUp(sb,   w_2, s,          s, &WCOLOR(sb,SHAPE_COLOR));
	AG_DrawArrowDown(sb, w_2, (r.h>>1)+s, s, &WCOLOR(sb,SHAPE_COLOR));
}
static void
DrawVert(AG_Scrollbar *_Nonnull sb, int y, int len)
{
	AG_Rect r;
	int w = WIDTH(sb);
	int h = HEIGHT(sb);
	int mid = (w >> 1);
	int wBar = sb->width, wBar2 = (wBar<<1), wBar_2 = (wBar>>1);
	int hArrow = MIN(w, sb->hArrow);

	if (h < wBar2) {
		DrawVertUndersize(sb);
		return;
	}

	r.x = 0;
	r.y = 0;
	r.w = w;
	r.h = h;
	AG_DrawBox(sb, &r, -1, &WCOLOR(sb,0));		/* Background */
	
	/* XXX overdraw */
	
	/* Up button */
	r.h = wBar;
	AG_DrawBox(sb, &r,
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC) ? -1 : 1,
	    (sb->mouseOverBtn == AG_SCROLLBAR_BUTTON_DEC) ?
	    &WCOLOR_HOV(sb,0) : &WCOLOR(sb,0));

	AG_DrawArrowUp(sb, mid, wBar_2, hArrow,
	    &WCOLOR(sb,SHAPE_COLOR));

	/* Down button */
	r.y = h - wBar;
	r.h = wBar;
	AG_DrawBox(sb, &r,
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_INC) ? -1 : 1,
	    (sb->mouseOverBtn == AG_SCROLLBAR_BUTTON_INC) ?
	    &WCOLOR_HOV(sb,0) : &WCOLOR(sb,0));

	AG_DrawArrowDown(sb, mid, r.y + wBar_2, hArrow,
	    &WCOLOR(sb,SHAPE_COLOR));

	/* Scrollbar */
	if (len > 0) {
		r.y = wBar + y;
		r.h = MIN(len, h-wBar2);
		AG_DrawBox(sb, &r,
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    (sb->mouseOverBtn == AG_SCROLLBAR_BUTTON_SCROLL) ?
		    &WCOLOR_HOV(sb,0) : &WCOLOR(sb,0));
	} else {
		r.y = wBar;
		r.h = h-wBar2;
		AG_DrawBox(sb, &r,
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    (sb->mouseOverBtn == AG_SCROLLBAR_BUTTON_SCROLL) ?
		    &WCOLOR_HOV(sb,0) : &WCOLOR(sb,0));
	}
}
static void
DrawHorizUndersize(AG_Scrollbar *_Nonnull sb)
{
	AG_Rect r;
	int h_2, s;

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(sb);
	r.h = HEIGHT(sb);
	AG_DrawBox(sb, &r, 1, &WCOLOR(sb,0));

	h_2 = (r.h >> 1);
	s = MIN(r.w>>2, r.h);

	AG_DrawArrowLeft(sb,  s,          h_2, s, &WCOLOR(sb,SHAPE_COLOR));
	AG_DrawArrowRight(sb, (r.w>>1)+s, h_2, s, &WCOLOR(sb,SHAPE_COLOR));
}
static void
DrawHoriz(AG_Scrollbar *_Nonnull sb, int x, int len)
{
	AG_Rect r;
	int w = WIDTH(sb);
	int h = HEIGHT(sb);
	int h_2 = h >> 1;
	int wBar = sb->width, wBar2 = (wBar << 1), wBar_2 = (wBar >> 1);
	int hArrow = MIN(h, sb->hArrow);
	
	if (w < wBar2) {
		DrawHorizUndersize(sb);
		return;
	}

	r.x = 0;
	r.y = 0;
	r.w = w;
	r.h = h;
	AG_DrawBox(sb, &r, -1, &WCOLOR(sb,0));	/* Background */
	
	/* XXX overdraw */

	/* Left button */
	r.w = wBar;
	r.h = h;
	AG_DrawBox(sb, &r,
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC) ? -1 : 1,
	    &WCOLOR(sb,0));
	AG_DrawArrowLeft(sb, wBar_2, h_2, hArrow, &WCOLOR(sb,SHAPE_COLOR));

	/* Right button */
	r.x = w - wBar;
	AG_DrawBox(sb,  &r,
	    (sb->curBtn == AG_SCROLLBAR_BUTTON_INC) ? -1 : 1,
	    &WCOLOR(sb,0));
	AG_DrawArrowRight(sb, r.x + wBar_2, h_2, hArrow,
	    &WCOLOR(sb,SHAPE_COLOR));

	/* Scrollbar */
	if (len > 0) {
		r.x = wBar + x;
		r.w = MIN(len, w-wBar2);
		AG_DrawBox(sb, &r,
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    &WCOLOR(sb,0));
	} else {
		r.x = wBar;
		r.w = w - wBar2;
		AG_DrawBox(sb, &r,
		    (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) ? -1 : 1,
		    &WCOLOR(sb,0));
	}
}

static void
Draw(void *_Nonnull obj)
{
	AG_Scrollbar *sb = obj;
	int x, len;

	if (GetPxCoords(sb, &x, &len) == -1) {		/* No range */
		x = 0;
		len = sb->length;
	}
	if (sb->flags & AG_SCROLLBAR_AUTOHIDE && len == sb->length)
		return;

	/*
	 * TODO set the Draw() operation vector to DrawVert() or DrawHoriz()
	 * at initialization time and merge this code into both.
	 */
	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		DrawVert(sb, x, len);
		break;
	case AG_SCROLLBAR_HORIZ:
		DrawHoriz(sb, x, len);
		break;
	}
	if (sb->flags & AG_SCROLLBAR_TEXT)
		DrawText(sb);
}

/* Return 1 if it is useful to display the scrollbar given the current range. */
int
AG_ScrollbarVisible(AG_Scrollbar *sb)
{
	int rv, x, len;

	AG_ObjectLock(sb);
	if (!AG_Defined(sb, "value") ||
	    !AG_Defined(sb, "min") ||
	    !AG_Defined(sb, "max") ||
	    !AG_Defined(sb, "visible")) {
		AG_ObjectUnlock(sb);
		return (1);
	}
	rv = (GetPxCoords(sb, &x, &len) == -1) ? 0 : 1;
	AG_ObjectUnlock(sb);
	return (rv);
}

AG_WidgetClass agScrollbarClass = {
	{
		"Agar(Widget:Scrollbar)",
		sizeof(AG_Scrollbar),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
