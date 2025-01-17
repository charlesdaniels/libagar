/*	Public domain	*/

#ifndef _AGAR_WIDGET_FIXED_H_
#define _AGAR_WIDGET_FIXED_H_

#include <agar/gui/widget.h>

#include <agar/gui/begin.h>

typedef struct ag_fixed {
	struct ag_widget wid;		/* AG_Widget -> AG_Fixed */
	Uint flags;
#define AG_FIXED_HFILL		0x01	/* Expand to fill available width */
#define AG_FIXED_VFILL		0x02	/* Expand to fill available height */
#define AG_FIXED_NO_UPDATE	0x04	/* Don't call WINDOW_UPDATE() */
#define AG_FIXED_FILLBG		0x08	/* Fill background */
#define AG_FIXED_BOX		0x10	/* Draw a box */
#define AG_FIXED_INVBOX		0x20	/* Draw a box */
#define AG_FIXED_FRAME		0x40	/* Draw a frame */
#define AG_FIXED_EXPAND		(AG_FIXED_HFILL|AG_FIXED_VFILL)
	int wPre, hPre;			/* User geometry */
} AG_Fixed;

__BEGIN_DECLS
extern AG_WidgetClass agFixedClass;

AG_Fixed *_Nonnull AG_FixedNew(void *_Nullable, Uint);

void AG_FixedSizeHint(AG_Fixed *_Nonnull, int,int);
void AG_FixedPut(AG_Fixed *_Nonnull, void *_Nonnull, int,int);
void AG_FixedDel(AG_Fixed *_Nonnull, void *_Nonnull);
void AG_FixedSize(AG_Fixed *_Nonnull, void *_Nonnull, int,int);
void AG_FixedMove(AG_Fixed *_Nonnull, void *_Nonnull, int,int);

#ifdef AG_LEGACY
#define AG_FixedPrescale(f,w,h) AG_FixedSizeHint((f),(w),(h))
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_FIXED_H_ */
