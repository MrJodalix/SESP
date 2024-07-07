/**
 * Hauptmodul des Programms.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "window.h"

////////////////////////////////// KONSTANTEN //////////////////////////////////

// Titel für das Fenster je nach Build-Type anpassen.
#ifdef NDEBUG
    #define WINDOW_TITLE PROGRAM_NAME " (Release " ARCH_BITS ")"
#else
    #define WINDOW_TITLE PROGRAM_NAME " (Debug " ARCH_BITS ")"
#endif

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Einstiegspunkt für das Programm.
 * 
 * @return EXIT_SUCCESS, wenn das Programm erfolgreich beendet wurde, 
 *         EXIT_FAILURE wenn ein Fehler aufgetreten ist (nur über exit())
 */
int main(void)
{
    // Zuerst muss das gesamte Programm initialisiert werden.
    ProgContext* ctx = window_init(WINDOW_TITLE);

    // Im zweiten Schritt wird die Hauptschleife des Programms gestartet.
    window_mainloop(ctx);

    // Zum Schluss müssen alle belegten Ressourcen wieder freigegeben werden.
    window_cleanup(ctx);

    return EXIT_SUCCESS;
}
