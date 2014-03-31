#include <iostream>
#include <math.h>

#include <discpp.h>

#define STEPS 200

int main(int argc, char **argv)
{
    double zmat[STEPS][STEPS];
    double fpi = 3.1415927 / 180.0;
    double stepSize = 360.0 / (STEPS - 1);
    for (int i = 0; i < STEPS; i++) {
        double x = i * stepSize;
        for (int j = 0; j < STEPS; j++) {
            double y = j * stepSize;
            zmat[i][j] = 2 * sin(x * fpi) * sin(y * fpi);
        }
    }

    Dislin g;
    g.metafl("cons"); // output driver: display
    //g.metafl("svg"); // output driver: display
    g.scrmod("revers"); // Hintergrund in weiß, Vordergrund in schwarz
    //g.sclfac(0.5); // Skalierungs-Faktor der Grafik
    g.window(100,100,640,453);
    g.disini(); // init
    g.pagera(); // äußerer Rahmen

    g.titlin("3-D Colour Plot of the Function", 2);
    g.titlin("F(X,Y) = 2 * SIN(X) * SIN(Y)", 4);

    g.name("X-axis", "x");
    g.name("Y-axis", "y");
    g.name("Z-axis", "z");

    g.intax();
    g.autres(STEPS, STEPS); // <-- bestimmt Qualitaet der Grafik!!!
    g.axspos(300, 1850);
    g.ax3len(2200, 1400, 1400);

    g.graf3(0.0, 360.0, 0.0, 90.0, 0.0, 360.0, 0.0, 90.0,
            -2.0, 2.0, -2.0, 1.0);
    g.crvmat((double *)zmat, STEPS, STEPS, 1, 1);

    g.height(50);

    g.hwfont(); // hardware fonts (sehen für cons und svg gut aus)
    g.disfin();

    return 0;
}
