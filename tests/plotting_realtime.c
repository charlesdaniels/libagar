/*	Public domain	*/

/*
 * This program shows a practical use for the M_Plotter(3) widget.
 * It computes an optimal "squared-sine" velocity profile, and plots
 * the derivatives.
 */

#include "agartest.h"

#include <agar/math.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>

static void
GeneratePlot(AG_Event *event)
{
	AG_Object* self = AG_PTR(0);
	M_Plotter *plt = AG_PTR(1);
	M_Plot* pl = AG_PTR(2);


	for (float i = 0 ; i < 10; i+=0.01) { 
		M_PlotReal(pl, sin(i));
		M_PlotterUpdate(plt);
	}
	M_PlotterUpdate(plt);


	/* M_Real t; */

	/* Clear the current plot data. */
	/* M_PlotClear(plVel); */
	/* M_PlotClear(plAcc); */
	/* M_PlotClear(plJerk); */

	/*
	 * Compute the data. M_PlotterUpdate() will compute the derivatives
	 * for us.
	 */
	/* ComputeSquaredSineConstants(); */
	/* for (t = 0.0; t < L; t += 1.0) { */
	/*         M_PlotReal(plVel, SquaredSineStep(t*t7/L)); */
	/*         M_PlotterUpdate(plt); */
	/* } */
}

static int
Init(void *obj)
{
	M_InitSubsystem();
	return (0);
}

static int
TestGUI(void *obj, AG_Window *win)
{
	M_Plotter *plt;
	M_Plot* pl;
	AG_Pane *pane;
	AG_Numerical *num;
	AG_Box *box;
	int i;

	plt = M_PlotterNew(win, M_PLOTTER_EXPAND);

	M_PlotterSizeHint (plt, 600, 400);

	pl = M_PlotNew(plt, M_PLOT_LINEAR);
	M_PlotSetLabel(pl, "system load");

	M_PlotSetXoffs(pl, 0);
	M_PlotSetScale(pl, 5.0, 50.0);



	AG_SetEvent(win, "window-shown", GeneratePlot, "%p,%p", plt, pl);

	AG_ButtonNewFn(win, AG_BUTTON_HFILL, "Generate",
	    GeneratePlot, "%p,%p", plt, pl);
	return (0);
}

const AG_TestCase realTimePlottingTest = {
	"plotting_realtime",
	N_("Test the M_Plotter(3) widget with real-time data"),
	"1.4.2",
	0,
	sizeof(AG_TestInstance),
	Init,
	NULL,		/* destroy */
	NULL,		/* test */
	TestGUI,
	NULL		/* bench */
};
