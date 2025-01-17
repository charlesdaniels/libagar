/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/rg/animview.h>
#include <agar/rg/icons.h>

RG_Animview *
RG_AnimviewNew(void *parent)
{
	RG_Animview *av;

	av = Malloc(sizeof(RG_Animview));
	AG_ObjectInit(av, &rgAnimviewClass);
	AG_ObjectAttach(parent, av);
	return (av);
}

/* Timer for updating the current frame during playback. */
static Uint32
PlayUpdateTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_SELF();

	if (av->anim->nframes == 0)
		return (0);

	AG_Redraw(av);

	if (++av->frame < av->anim->nframes) {
		RG_AnimFrame *fr = &av->anim->frames[av->frame];

		return (fr->delay > 0 ? fr->delay*100/av->speed : 1);
	} else {
		RG_AnimFrame *fr = &av->anim->frames[0];

		av->frame = 0;
		return (fr->delay > 0 ? fr->delay*100/av->speed: 1);
	}
}

static void
Play(AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_PTR(1);

	AG_AddTimer(av, &av->timer, 1, PlayUpdateTimeout, NULL);
	AG_WidgetDisable(av->btns.play);
	AG_WidgetEnable(av->btns.pause);
	AG_WidgetEnable(av->btns.stop);
}

static void
Pause(AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_PTR(1);
	
	AG_DelTimer(av, &av->timer);
	AG_WidgetEnable(av->btns.play);
	AG_WidgetDisable(av->btns.pause);
	AG_WidgetDisable(av->btns.stop);
}

static void
Stop(AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_PTR(1);
	
	AG_DelTimer(av, &av->timer);
	av->frame = 0;
	
	AG_WidgetEnable(av->btns.play);
	AG_WidgetDisable(av->btns.pause);
	AG_WidgetDisable(av->btns.stop);
}

static void
SetSpeed(AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_PTR(1);
	Uint factor = AG_INT(2);

	av->speed = factor;
}

static void
PopupMenu(RG_Animview *_Nonnull av)
{
	AG_PopupMenu *pm;
	AG_MenuItem *mSpd;

	if (av->pm != NULL) {
		AG_PopupShow(av->pm);
		return;
	}
	if ((av->pm = pm = AG_PopupNew(av)) == NULL)
		return;

	AG_MenuAction(pm->root, _("Play"),  rgIconPlay.s,	Play,"%p", av);
	AG_MenuAction(pm->root, _("Pause"), rgIconPause.s,	Pause,"%p", av);
	AG_MenuAction(pm->root, _("Stop"),  rgIconStop.s,	Stop,"%p", av);

	AG_MenuSeparator(pm->root);

	mSpd = AG_MenuNode(pm->root, _("Playback speed"), NULL);
	{
		AG_MenuAction(mSpd, "4x",  NULL, SetSpeed,"%p,%i", av, 400);
		AG_MenuAction(mSpd, "3x",  NULL, SetSpeed,"%p,%i", av, 300);
		AG_MenuAction(mSpd, "2x",  NULL, SetSpeed,"%p,%i", av, 200);
		AG_MenuAction(mSpd, "1x",  NULL, SetSpeed,"%p,%i", av, 100);
		AG_MenuAction(mSpd, "1/2", NULL, SetSpeed,"%p,%i", av, 50);
		AG_MenuAction(mSpd, "1/3", NULL, SetSpeed,"%p,%i", av, 33);
		AG_MenuAction(mSpd, "1/4", NULL, SetSpeed,"%p,%i", av, 25);
	}
	AG_PopupShow(pm);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	RG_Animview *av = AG_SELF();
	int button = AG_INT(1);
/*	int x = AG_INT(2); */
	int y = AG_INT(3);

	if (button == AG_MOUSE_RIGHT &&
	    y < WIDGET(av)->h - WIDGET(av->btns.play)->h)
		PopupMenu(av);
}

static void
Init(void *_Nonnull obj)
{
	RG_Animview *av = obj;

	WIDGET(av)->flags |= AG_WIDGET_EXPAND;

	av->pre_w = 64;
	av->pre_h = 64;
	av->ranim.x = 0;
	av->ranim.y = 0;
	av->ranim.w = av->pre_w;
	av->ranim.h = av->pre_h;
	av->speed = 100;
	av->pm = NULL;
	
	av->btns.play = AG_ButtonNewS(av, 0, NULL);
	AG_ButtonSurfaceNODUP(av->btns.play, rgIconPlay.s);
	AG_SetEvent(av->btns.play, "button-pushed", Play, "%p", av);
	av->btns.pause = AG_ButtonNewS(av, 0, NULL);
	AG_ButtonSurfaceNODUP(av->btns.pause, rgIconPause.s);
	AG_SetEvent(av->btns.pause, "button-pushed", Pause, "%p", av);
	av->btns.stop = AG_ButtonNewS(av, 0, NULL);
	AG_ButtonSurfaceNODUP(av->btns.stop, rgIconStop.s);
	AG_SetEvent(av->btns.stop, "button-pushed", Stop, "%p", av);
	
	AG_WidgetEnable(av->btns.play);
	AG_WidgetDisable(av->btns.pause);
	AG_WidgetDisable(av->btns.stop);
	
	AG_SetEvent(av, "mouse-button-down", MouseButtonDown, NULL);
}

static void
Destroy(void *_Nonnull obj)
{
	RG_Animview *av = obj;
	
	if (av->pm != NULL)
		AG_PopupDestroy(av->pm);
}

void
RG_AnimviewSizeHint(RG_Animview *av, int w, int h)
{
	av->pre_w = w;
	av->pre_h = h;
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	RG_Animview *av = p;
	AG_SizeReq rPlay, rPause, rStop;

	AG_WidgetSizeReq(av->btns.play, &rPlay);
	AG_WidgetSizeReq(av->btns.pause, &rPause);
	AG_WidgetSizeReq(av->btns.stop, &rStop);

	r->w = AG_MAX(av->pre_w, rPlay.w+rPause.w+rStop.w);
	r->h = av->pre_h + AG_MAX3(rPlay.h, rPause.h, rStop.h);
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
{
	RG_Animview *av = p;
	AG_SizeReq rBtn;
	AG_SizeAlloc aBtn;
	int hBtn = 0;

	AG_WidgetSizeReq(av->btns.play, &rBtn);
	if (hBtn < rBtn.h) { hBtn = rBtn.h; }
	AG_WidgetSizeReq(av->btns.pause, &rBtn);
	if (hBtn < rBtn.h) { hBtn = rBtn.h; }
	AG_WidgetSizeReq(av->btns.stop, &rBtn);
	if (hBtn < rBtn.h) { hBtn = rBtn.h; }

	aBtn.x = 0;
	aBtn.y = a->h - hBtn;
	aBtn.w = a->w/3;
	aBtn.h = hBtn;
	AG_WidgetSizeAlloc(av->btns.play, &aBtn);
	aBtn.x += aBtn.w;
	AG_WidgetSizeAlloc(av->btns.pause, &aBtn);
	aBtn.x += aBtn.w;
	if ((aBtn.x + aBtn.w) < a->w) { aBtn.w++; } /* For rounding */
	AG_WidgetSizeAlloc(av->btns.stop, &aBtn);

	av->ranim.x = a->w/2 - av->ranim.w/2;
	av->ranim.y = (a->h - hBtn)/2 - av->ranim.h/2;
	return (0);
}

static void
Draw(void *_Nonnull p)
{
	RG_Animview *av = p;
	RG_AnimFrame *fr;

	if (av->anim == NULL || av->anim->nframes < 1) {
		return;
	}
	fr = &av->anim->frames[av->frame];
	if (fr->su != NULL) {
		AG_Rect r;

		r.x = 0;
		r.y = 0;
		r.w = WIDTH(av);
		r.h = HEIGHT(av);
		AG_PushClipRect(av, &r);
		AG_WidgetBlit(av, fr->su, av->ranim.x, av->ranim.y);
		AG_PopClipRect(av);
	}
}

void
RG_AnimviewSetAnimation(RG_Animview *av, RG_Anim *anim)
{
	AG_DelTimer(av, &av->timer);
	av->anim = anim;
	av->frame = 0;
	av->ranim.x = WIDGET(av)->w/2 - anim->w/2;
	av->ranim.y = (WIDGET(av)->h - WIDGET(av->btns.play)->h)/2 - anim->h/2;
	av->ranim.w = anim->w;
	av->ranim.h = anim->h;
	AG_Redraw(av);
}

AG_WidgetClass rgAnimviewClass = {
	{
		"Agar(Widget):RG(Animview)",
		sizeof(RG_Animview),
		{ 0,0 },
		Init,
		Destroy,
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
