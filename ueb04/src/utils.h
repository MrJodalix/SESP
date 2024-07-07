/**
 * Allgemeine Hilfsfunktionen, die überall nützlich sein
 * können.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

////////////////////////////////// KONSTANTEN //////////////////////////////////

// Wenn kein Pfad zu den Ressourcen gesetzt ist, gehen wir davon aus,
// das sie in diesem Verzeichnis liegen.
#ifndef RESOURCE_PATH
    #define RESOURCE_PATH "./res/"
#endif

//////////////////////////////////// MAKROS ////////////////////////////////////

// Gibt den Pfad zu einer Ressource im Ressourcenverzeichnis zurück.
// Der Pfad muss dabei als Literal übergeben werden.
// Wenn der Pfad dynamisch zur Laufzeit ermittelt werden muss,
// sollte die Funktion utils_getResourcePath verwendet werden.
#define UTILS_CONST_RES(res) (RESOURCE_PATH res)

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

/**
 * Hilfsfunktion zum bestimmen eines Pfades im Ressourcenverzeichnis.
 * Der String-Parameter darf nicht mit einem '/' anfangen!
 * Zurückgegeben wird ein neuer, zusammengesetzter Pfad. Dieser muss
 * später von Hand mit free freigegeben werden.
 * Wird für path NULL verwendet, wird nur der Pfad zum Ressourcenordner
 * zurückgegeben. Auch dieser muss mit free wieder freigegeben werden.
 * 
 * @param path ein Pfad im Ressourcenverzeichnis oder NULL
 * @return ein neuer String mit dem zusammengesetzten Pfad
 */
char* utils_getResourcePath(const char* path);

/**
 * Hilfsfunktion zum Einlesen einer Datei in den Arbeitsspeicher.
 * Der zurückgegebene String muss mit free wieder gelöscht werden.
 * 
 * @param filename der Dateiname der einzulesenden Datei
 * @return der Inhalt der eingelesenen Datei
 */
char* utils_readFile(const char* filename);

/**
 * Prüft, ob ein Suffix am Ende eines Stringes zu finden ist.
 * Diese Funktion kann zum Beispiel genutzt werden, um Dateiendungen
 * abzufragen: utils_hasSuffix(filename, ".png");
 * Ein leeres Suffix ist nie Teil des Subjektes.
 * 
 * @param subject der String, bei dem das Suffix gesucht werden soll
 * @param suffix das zu suchende Suffix
 * @return true, wenn das Suffix vorhanden ist, false wenn nicht
 */
bool utils_hasSuffix(const char* subject, const char* suffix);

/**
 * Gibt den Ordnerpfad einer Datei zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt. Auch ".." Verzeichnisse bleiben erhalten.
 * 
 * Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 * 
 * @param filepath der Dateipfad, aus dem der Ordner extrahiert werden soll
 * @return der Pfad des Ordners
 */
char* utils_getDirectory(const char* filepath);

/**
 * Gibt den Dateinamen in einem Pfad zurück. Dabei handelt es sich um eine
 * reine String-Bearbeitung, es findet keine Validierung auf Dateisystem-
 * Ebene statt.
 * 
 * Der zurückgegebene String muss selbstständig wieder freigegeben werden.
 * 
 * @param filepath der Dateipfad, aus dem der Dateiname extrahiert werden soll
 * @return der Dateiname
 */
char* utils_getFilename(const char* filepath);

/**
 * Gibt den Größeren von zwei Integern zurück.
 * Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:
 * Das Makro MAX(a. b) ((a) > (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) > (y++) ? (x++) : (y++))
 * Demnach würde der jeweils größere Wert zweimal erhöht werden. Solche Probleme
 * können auch in anderer Weise auftreten, weshalb eine Funktion hier sicherer
 * ist.
 * 
 * @param a der erste Integer.
 * @param b der zweite Integer.
 * @return der gößere Wert.
 */
int utils_maxInt(int a, int b);

/**
 * Gibt den Kleineren von zwei Integern zurück.
 * Diese Funktionalität wurde als Funktion und nicht als Makro umgesetzt,
 * da ein Makro schnell zu Fehlern führen kann:
 * Das Makro MIN(a. b) ((a) < (b) ? (a) : (b)) führt bei a = x++ und b = y++
 * zu folgender Ersetzung: ((x++) < (y++) ? (x++) : (y++))
 * Demnach würde der jeweils kleinere Wert zweimal erhöht werden. Solche 
 * Probleme können auch in anderer Weise auftreten, weshalb eine Funktion hier 
 * sicherer ist.
 * 
 * @param a der erste Integer.
 * @param b der zweite Integer.
 * @return der kleinere Wert.
 */
int utils_minInt(int a, int b);

#endif // UTILS_H
