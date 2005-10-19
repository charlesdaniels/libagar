/*	$Csoft: agar-bench.h,v 1.3 2005/10/03 17:37:59 vedge Exp $	*/
/*	Public domain	*/

#include <agar/core/core.h>
#include <agar/core/view.h>
#include <agar/game/map/map.h>
#include <agar/gui/gui.h>

struct testfn_ops {
	char *name;
	void (*init)(void);
	void (*destroy)(void);
	void (*run)(void);
	Uint64 clksMin, clksAvg, clksMax;
};

struct test_ops {
	char *name;
	void (*edit)(AG_Window *);
	struct testfn_ops *funcs;
	u_int nfuncs;
	u_int flags;
#define TEST_SDL	0x01		/* SDL-only */
#define TEST_GL		0x02		/* OpenGL-only */
	u_int runs;
	u_int iterations;
};

extern SDL_Surface *surface, *surface64, *surface128;

void InitSurface(void);
void FreeSurface(void);
void LockView(void);
void UnlockView(void);

