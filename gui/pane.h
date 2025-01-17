/*	Public domain	*/

#ifndef _AGAR_GUI_PANE_H_
#define _AGAR_GUI_PANE_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>

#include <agar/gui/begin.h>

enum ag_pane_type {
	AG_PANE_HORIZ,
	AG_PANE_VERT
};

enum ag_pane_resize_action {
	AG_PANE_EXPAND_DIV1,		/* Expand left/upper division */
	AG_PANE_EXPAND_DIV2,		/* Expand right/lower division */
	AG_PANE_DIVIDE_EVEN,		/* Divide evenly */
	AG_PANE_DIVIDE_PCT		/* Divide per AG_PaneMoveDividerPct() */
};

typedef struct ag_pane {
	struct ag_widget wid;		/* AG_Widget -> AG_Pane */
	enum ag_pane_type type;
	Uint flags;
#define AG_PANE_HFILL		0x001
#define AG_PANE_VFILL		0x002
#define AG_PANE_DIV1FILL	0x004	/* Expand div1 (default is div2) */
#define AG_PANE_FRAME		0x008	/* Display frames for each division */
#define AG_PANE_UNMOVABLE	0x100	/* Pane is not user-movable */
#define AG_PANE_EXPAND (AG_PANE_HFILL|AG_PANE_VFILL)

#ifdef AG_LEGACY
#define AG_PANE_FORCE_DIV1FILL	0x010	/* Enforce div1 expansion */
#define AG_PANE_FORCE_DIV2FILL	0x020	/* Enforce div2 expansion */
#define AG_PANE_DIV		0x040	/* Initially divide equally */
#define AG_PANE_FORCE_DIV	0x080	/* Enforce equal division */
#endif

	AG_Box *_Nonnull div[2];	/* Sub-containers */
	int wMin[2], hMin[2];		/* Minimum geometry */
	int wReq[2], hReq[2];		/* Requisition geometry */
	int dmoving;			/* Divider being moved */
	int dx;				/* Actual divider position */
	int rx;				/* Requested divider position */
	int rxPct;			/* Requested position in % */
	int wDiv;			/* Divider width */
	AG_CursorArea *_Nullable ca;	/* Divider cursor-change area */

	enum ag_pane_resize_action resizeAction;	/* Resize action */
} AG_Pane;

__BEGIN_DECLS
extern AG_WidgetClass agPaneClass;

AG_Pane	*_Nonnull AG_PaneNew(void *_Nullable, enum ag_pane_type, Uint);
AG_Pane	*_Nonnull AG_PaneNewHoriz(void *_Nullable, Uint);
AG_Pane	*_Nonnull AG_PaneNewVert(void *_Nullable, Uint);

void AG_PaneAttachBox(AG_Pane *_Nonnull, int, AG_Box *_Nonnull);
void AG_PaneAttachBoxes(AG_Pane *_Nonnull, AG_Box *_Nonnull, AG_Box *_Nonnull);
void AG_PaneSetDividerWidth(AG_Pane *_Nonnull, int);
void AG_PaneSetDivisionMin(AG_Pane *_Nonnull, int, int,int);
void AG_PaneSetDivisionPacking(AG_Pane *_Nonnull, int, enum ag_box_type);
int  AG_PaneMoveDivider(AG_Pane *_Nonnull, int);
int  AG_PaneMoveDividerPct(AG_Pane *_Nonnull, int);
void AG_PaneResizeAction(AG_Pane *_Nonnull, enum ag_pane_resize_action);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_PANE_H_ */
