
#ifndef GES_DEBUG_H
#define GES_DEBUG_H

#if 0

#define GES(info) {FILE *ges; ges = fopen("c:\\g42debug.txt", "a");\
	fprintf info ; fclose(ges);}

#define GESTRACE(name) // GES((ges, "Trace: %s, %d\n", name, GetTickCount()));
#define GESTRACESTART(name) long ges_trace = GetTickCount();// GES((ges, "Trace: %s, %d\n", name, ges_trace));
#define GESTRACEEND(name) long ges_trace_end = GetTickCount();// GES((ges, "Trace: %s, %d, %d ticks\n", name, ges_trace_end, ges_trace_end - ges_trace));

extern long GesTraceTotal, GesTraceRead, GesTraceOPal, GesTraceDither, GesTraceDraw, GesTraceOther;
extern bool GesTraceDone, GesTraceOut;

#define GESTRACEBASE long GesTraceTotal = 0, GesTraceRead = 0, GesTraceOPal = 0, GesTraceDither = 0, GesTraceDraw = 0, GesTraceOther = 0; bool GesTraceDone = false; bool GesTraceOut = false;
#define GESTRACEOUT() if (GesTraceDone && !GesTraceOut ) {GES((ges, "\nAnalysis\n\n"));\
	GES((ges, "Read              %5.3f seconds\nOptimize Palette  %5.3f seconds\n", (float)GesTraceRead / 1000.0, (float)GesTraceOPal / 1000.0)); \
	GES((ges, "Dither            %5.3f seconds\nDrawing           %5.3f seconds\n", (float)GesTraceDither / 1000.0, (float)GesTraceDraw / 1000.0)); \
	GES((ges, "Other             %5.3f seconds\nOLE/DocView       %5.3f seconds\n", (float)GesTraceOther / 1000.0, (float)(GesTraceTotal - GesTraceRead - GesTraceOPal - GesTraceDither - GesTraceDraw - GesTraceOther) / 1000.0)); \
	GES((ges, "\nTotal             %5.3f seconds\n\n", (float)GesTraceTotal / 1000.0)); \
	GesTraceRead = 0; GesTraceOPal = 0; GesTraceDither = 0; GesTraceDraw = 0; GesTraceOther = 0; GesTraceOut = true;}
#define GESTRACERESET() GESTRACEOUT(); GesTraceTotal = GetTickCount(); GesTraceDone = false; GesTraceOut = false; //GES((ges, "Reseting times\n")); //GesTraceRead = 0; GesTraceOPal = 0; GesTraceDither = 0; GesTraceDraw = 0; GesTraceOther = 0; GES((ges, "Reseting times\n"));
#define GESTRACETOTAL() GesTraceTotal = GetTickCount() - GesTraceTotal; GesTraceDone = true; //GES((ges, "Totaling time\n"));
#define GESTRACEADD(variable, amount) if (!GesTraceDone) variable += amount;
#define GESTRACESTORE(variable, amount) long variable = amount;

#else

#define GESTRACE(name)
//#define GES(info)
#define GES(info) {FILE *ges; ges = fopen("c:\\g42debug.txt", "a");\
	fprintf info ; fclose(ges);}

#define GESTRACESTART(name)
#define GESTRACEEND(name)

#define GESTRACEBASE
#define GESTRACEOUT()
#define GESTRACERESET()
#define GESTRACETOTAL()
#define GESTRACEADD(variable, amount)
#define GESTRACESTORE(variable, amount)

#endif // if 0
#endif // G42_GESDEBUG_H
