/*	Public domain	*/

#ifndef _AGAR_SG_SG_PALETTE_VIEW_H_
#define _AGAR_SG_SG_PALETTE_VIEW_H_
#include <agar/sg/begin.h>

typedef struct sg_palette_view {
	struct sg_view _inherit;

	Uint flags;
#define SG_PALETTE_VIEW_HFILL  0x001
#define SG_PALETTE_VIEW_VFILL  0x002
#define SG_PALETTE_VIEW_EXPAND (SG_PALETTE_VIEW_HFILL | SG_PALETTE_VIEW_VFILL)

	SG_Palette *pal;		/* Bound Palette object */
	SG *sg;				/* Visualization scene */
} SG_PaletteView;

__BEGIN_DECLS
extern AG_WidgetClass sgPaletteViewClass;

SG_PaletteView *SG_PaletteViewNew(void *, SG_Palette *, Uint);
__END_DECLS

#include <agar/sg/close.h>
#endif /* _AGAR_SG_SG_PALETTE_VIEW_H_ */
