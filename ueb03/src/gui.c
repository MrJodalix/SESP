/**
 * Graphisches Nutzerinterface für die Software.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "gui.h"

#include <string.h>
#include <sesp/nuklear.h>

#include "window.h"
#include "input.h"

 ////////////////////////////////// KONSTANTEN //////////////////////////////////

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define STATS_WIDTH (80)
#define STATS_HEIGHT (30)

// Definitionen der Fenster IDs
#define GUI_WINDOW_HELP "window_help"
#define GUI_WINDOW_MENU "window_menu"
#define GUI_WINDOW_STATS "window_stats"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Datentyp für die GUI Daten
struct GuiData {
	struct nk_glfw glfw;
	struct nk_context* nk;
};
typedef struct GuiData GuiData;



/////////////////////////////// LOKALE CALLBACKS ///////////////////////////////

/**
 * Callback Funktion, die aufgerufen wird, wenn eine Taste gedrückt wurde.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param codepoint 32bit Zeichencode.
 */
static void callback_glfwChar(GLFWwindow* win, unsigned int codepoint)
{
	ProgContext* ctx = (ProgContext*)glfwGetWindowUserPointer(win);
	nk_glfw3_char_callback(&ctx->gui->glfw, codepoint);
}

/**
 * Callback Funktion, die aufgerufen wird, wenn das Maus-Scollrad gedreht wird.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param xoff die Veränderung in X-Richtung (z.B. bei Touchpads).
 * @param yoff die Veränderung in Y-Richtung (z.B. klassisches Mausrad).
 */
static void callback_glfwScroll(GLFWwindow* win, double xoff, double yoff)
{
	ProgContext* ctx = (ProgContext*)glfwGetWindowUserPointer(win);

	// Prüfen, ob aktuell die GUI aktiv ist.
	if (nk_item_is_any_active(ctx->gui->nk))
	{
		// Wenn ja, Event an die GUI weiterleiten.
		nk_gflw3_scroll_callback(&ctx->gui->glfw, xoff, yoff);
	}
	else
	{
		// Wenn nicht, Event an das Input-Modul weiterleiten.
		input_scroll(ctx, xoff, yoff);
	}
}

/**
 * Callback Funktion, die aufgerufen wird, wenn eine Maustaste gedrückt wurde.
 * Die Events werden direkt an Nuklear weitergegeben.
 *
 * @param win GLFW Fensterhandle.
 * @param button die gedrückte Taste.
 * @param action die Aktion, die das Event ausgelöst hat (z.B. loslassen).
 * @param mods aktivierte Modifikatoren.
 */
static void callback_glfwMouseButton(GLFWwindow* win, int button,
	int action, int mods)
{
	ProgContext* ctx = (ProgContext*)glfwGetWindowUserPointer(win);

	// Prüfen, ob aktuell die GUI aktiv ist.
	if (nk_item_is_any_active(ctx->gui->nk))
	{
		// Wenn ja, Event an die GUI weiterleiten.
		nk_glfw3_mouse_button_callback(
			&ctx->gui->glfw, win,
			button, action, mods
		);
	}
	else
	{
		// Wenn nicht, Event an das Input-Modul weiterleiten.
		input_mouseAction(ctx, button, action, mods);
	}
}

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Hilfswidget um einen 3D Vektor anzupassen.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param val der einstellbare Vektor
 */
static void gui_widgetVec3(struct nk_context* nk, vec3 val)
{
	struct nk_rect nkTrans;
	nkTrans.x = val[0];
	nkTrans.y = val[1];
	nkTrans.w = val[2];

	// Danach zeigen wir die Auswahlfelder an.
	nk_layout_row_dynamic(nk, 25, 1);
	nk_property_float(nk, "#X:", -100.0f, &nkTrans.x, 100.0f, 0.005f, 0.001f);
	nk_property_float(nk, "#Y:", -100.0f, &nkTrans.y, 100.0f, 0.005f, 0.001f);
	nk_property_float(nk, "#Z:", -100.0f, &nkTrans.w, 100.0f, 0.005f, 0.001f);

	val[0] = nkTrans.x;
	val[1] = nkTrans.y;
	val[2] = nkTrans.w;
}

/**
 * Hilfswidget um einen 3D Vektor anzupassen.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param val der einstellbare Vektor
 */
static void gui_widgetVec3PointLight(struct nk_context* nk, vec3 val)
{
	struct nk_rect nkTrans;
	nkTrans.x = val[0];
	nkTrans.y = val[1];
	nkTrans.w = val[2];

	// Danach zeigen wir die Auswahlfelder an.
	nk_layout_row_dynamic(nk, 25, 1);
	nk_property_float(nk, "#Distanz:", -100.0f, &nkTrans.x, 100.0f, 1.0f, 0.1f);
	nk_property_float(nk, "#Mittelpunkt X:", -100.0f, &nkTrans.y, 100.0f, 1.0f, 0.1f);
	nk_property_float(nk, "#Mittelpunkt Z:", -100.0f, &nkTrans.w, 100.0f, 1.0f, 0.1f);

	val[0] = nkTrans.x;
	val[1] = nkTrans.y;
	val[2] = nkTrans.w;
}

/**
 * Hilfswidget um einen Colorpicker anzuzeigen, der die Farbe
 * als vec4 speichert.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param col die einstellbare Farbe
 */
static void gui_widgetColor(struct nk_context* nk, const char* name, vec4 col)
{
	// Zuerst muss zwischen den Datentypen vec4 und
	// nk_colorf konvertiert werden.
	struct nk_colorf nkColor;
	nkColor.r = col[0];
	nkColor.g = col[1];
	nkColor.b = col[2];
	nkColor.a = col[3];

	// Dann wird der Titel angezeigt.
	nk_layout_row_dynamic(nk, 20, 1);
	nk_label(nk, name, NK_TEXT_LEFT);

	// Danach zeigen wir den Colorpicker selbst an.
	nk_layout_row_dynamic(nk, 25, 1);
	if (nk_combo_begin_color(nk, nk_rgb_cf(nkColor),
		nk_vec2(nk_widget_width(nk), 400)))
	{
		nk_layout_row_dynamic(nk, 120, 1);
		nkColor = nk_color_picker(nk, nkColor, NK_RGBA);
		nk_layout_row_dynamic(nk, 25, 1);
		nkColor.r = nk_propertyf(nk, "#R:", 0, nkColor.r, 1.0f, 0.01f, 0.005f);
		nkColor.g = nk_propertyf(nk, "#G:", 0, nkColor.g, 1.0f, 0.01f, 0.005f);
		nkColor.b = nk_propertyf(nk, "#B:", 0, nkColor.b, 1.0f, 0.01f, 0.005f);
		nkColor.a = nk_propertyf(nk, "#A:", 0, nkColor.a, 1.0f, 0.01f, 0.005f);
		nk_combo_end(nk);
	}

	// Zum Schluss wird wieder zurück konvertiert.
	col[0] = nkColor.r;
	col[1] = nkColor.g;
	col[2] = nkColor.b;
	col[3] = nkColor.a;
}

/**
 * Hilfswidget um die Parameter des Lichts anzuzeigen, der die Farbe
 * als vec4 speichert.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param col die einstellbare Farbe
 */
static void gui_widgetLightComp(struct nk_context* nk, const char* name, vec4 col)
{
	struct nk_colorf nkLight;
	nkLight.r = col[0];
	nkLight.g = col[1];
	nkLight.b = col[2];
	nkLight.a = col[3];

	nk_layout_row_dynamic(nk, 20, 1);
	nk_label(nk, name, NK_TEXT_LEFT);

	nk_layout_row_dynamic(nk, 25, 1);
	if (nk_combo_begin_color(nk, nk_rgb_cf(nkLight),
		nk_vec2(nk_widget_width(nk), 400)))
	{
		nk_layout_row_dynamic(nk, 25, 1);
		nkLight.r = nk_propertyf(nk, "#Ambient:", 0, nkLight.r, 2.0f, 0.001f, 0.0005f);
		nkLight.g = nk_propertyf(nk, "#Spekular:", 0, nkLight.g, 2.0f, 0.001f, 0.005f);
		nkLight.b = nk_propertyf(nk, "#Diffuse:", 0, nkLight.b, 2.0f, 0.001f, 0.005f);
		nk_combo_end(nk);
	}

	col[0] = nkLight.r;
	col[1] = nkLight.g;
	col[2] = nkLight.b;
	col[3] = nkLight.a;
}

/**
 * Hilfswidget um die Parameter des Lichts anzuzeigen, der die Farbe
 * als vec4 speichert.
 *
 * @param nk der Nuklear Kontext
 * @param name der anzuzeigende Name
 * @param col die einstellbare Farbe
 */
static void gui_widgetLightDir(struct nk_context* nk, const char* name, vec4 col)
{
	struct nk_colorf nkLightDir;
	nkLightDir.r = col[0];
	nkLightDir.g = col[1];
	nkLightDir.b = col[2];
	nkLightDir.a = col[3];

	nk_layout_row_dynamic(nk, 20, 1);
	nk_label(nk, name, NK_TEXT_LEFT);

	nk_layout_row_dynamic(nk, 25, 1);
	if (nk_combo_begin_color(nk, nk_rgb_cf(nkLightDir),
		nk_vec2(nk_widget_width(nk), 400)))
	{
		nk_layout_row_dynamic(nk, 25, 1);
		nkLightDir.r = nk_propertyf(nk, "#X:", -10.0, nkLightDir.r, 10.0f, 0.01f, 0.005f);
		nkLightDir.g = nk_propertyf(nk, "#Y:", -10.0, nkLightDir.g, 10.0f, 0.01f, 0.005f);
		nkLightDir.b = nk_propertyf(nk, "#Z:", -10.0, nkLightDir.b, 10.0f, 0.01f, 0.005f);
		nk_combo_end(nk);
	}

	col[0] = nkLightDir.r;
	col[1] = nkLightDir.g;
	col[2] = nkLightDir.b;
	col[3] = nkLightDir.a;
}

/**
 * Zeigt ein Hilfefenster an, in dem alle Maus- und Tastaturbefehle aufgelistet
 * werden.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderHelp(ProgContext* ctx, struct nk_context* nk)
{
	// Prüfen, ob die Hilfe überhaupt angezeigt werden soll.
	if (ctx->input->showHelp)
	{
		// Größe und Position des Fensters beim Öffnen bestimmen.
		float width = ctx->winData->realWidth * 0.25f;
		float height = ctx->winData->realHeight * 0.5f;
		float x = width * 1.5f;
		float y = height * 0.5f;

		// Fenster öffnen.
		if (nk_begin_titled(nk, GUI_WINDOW_HELP,
			"Hilfe", nk_rect(x, y, width, height),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
		{
			// Vorlage für die Darstellung der Zeilen anlegen.
			nk_layout_row_template_begin(nk, 15);
			nk_layout_row_template_push_dynamic(nk);
			nk_layout_row_template_push_static(nk, 40);
			nk_layout_row_template_end(nk);

			// Hilfsmakro zum einfachen füllen des Fensters.
#define HELP_LINE(dsc, key) { \
                nk_label(nk, (dsc), NK_TEXT_ALIGN_LEFT);\
                nk_label(nk, (key), NK_TEXT_ALIGN_RIGHT);}

			HELP_LINE("Programm beenden", "ESC");
			HELP_LINE("Hilfe umschalten", "F1");
			HELP_LINE("Fullscreen umschalten", "F2");
			HELP_LINE("Wireframe umschalten", "F3");
			HELP_LINE("Menü umschalten", "F4");
			HELP_LINE("Statistiken umschalten", "F5");
			HELP_LINE("Screenshot anfertigen", "F6");
			HELP_LINE("Kamera vorwärst", "W");
			HELP_LINE("Kamera links", "A");
			HELP_LINE("Kamera zurück", "S");
			HELP_LINE("Kamera rechts", "D");
			HELP_LINE("Kamera hoch", "E");
			HELP_LINE("Kamera runter", "Q");
			HELP_LINE("Umsehen", "LMB");
			HELP_LINE("Zoomen", "Scroll");

			// Makro wieder löschen, da es nicht mehr gebraucht wird.
#undef HELP_LINE

// Vorlage für den Schließen-Button.
			nk_layout_row_template_begin(nk, 25);
			nk_layout_row_template_push_dynamic(nk);
			nk_layout_row_template_push_static(nk, 130);
			nk_layout_row_template_end(nk);

			// Ein leeres Textfeld zum Ausrichten des Buttons.
			nk_label(nk, "", NK_TEXT_ALIGN_LEFT);

			// Button zum Schließen des Fensters.
			if (nk_button_label(nk, "Hilfe schließen"))
			{
				ctx->input->showHelp = false;
			}
		}
		nk_end(nk);
	}
}

/**
 * Zeigt ein Konfigurationsmenü an, das benutzt werden kann, um die Szene oder
 * das Rendering an sich zu beeinflussen.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderMenu(ProgContext* ctx, struct nk_context* nk)
{
	InputData* input = ctx->input;

	// Prüfen, ob das Menü überhaupt angezeigt werden soll.
	if (input->showMenu)
	{
		// Größe des Fensters beim Öffnen bestimmen.
		float height = ctx->winData->realHeight * 0.7f;

		// Fenster öffnen.
		if (nk_begin_titled(nk, GUI_WINDOW_MENU, "Szenen-Einstellungen",
			nk_rect(15, 15, 300, height),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
		{
			// Allgemeine Einstellungen anzeigen.
			if (nk_tree_push(nk, NK_TREE_TAB, "Allgemein", NK_MAXIMIZED))
			{
				nk_layout_row_dynamic(nk, 30, 2);

				if (nk_button_label(nk, "Hilfe umschalten"))
				{
					input->showHelp = !input->showHelp;
				}

				if (nk_button_label(nk,
					input->isFullscreen ? "Fenstermodus" : "Vollbild"))
				{
					input->isFullscreen = !input->isFullscreen;
					window_updateFullscreen(ctx);
				}

				if (nk_button_label(nk, "Shader reload"))
				{
					input->reloadShader = !input->reloadShader;
				}

				nk_tree_pop(nk);
			}

			// Einstellungen bezüglich der Darstellung.
			if (nk_tree_push(nk, NK_TREE_TAB, "Darstellung", NK_MINIMIZED))
			{
				// Wireframe
				nk_layout_row_dynamic(nk, 25, 1);
				nk_bool wireframe = input->showWireframe;
				if (nk_checkbox_label(nk, "Wireframe", &wireframe))
				{
					input->showWireframe = wireframe;
				}

				nk_tree_pop(nk);
			}

			// Einstellungen fuer das Licht
			if (nk_tree_push(nk, NK_TREE_TAB, "Licht Anpassung", NK_MINIMIZED))
			{
				// Lichtkomponenten
				gui_widgetLightComp(nk, "Lichtkomponenten:", input->rendering.lightComp);
				// Lichtrichtung
				gui_widgetLightDir(nk, "Lichtrichtung:", input->rendering.lightDir);
				// Lichtfarbe
				gui_widgetColor(nk, "Lichtfarbe:", input->rendering.lightColor);

				nk_tree_pop(nk);
			}

			// Einstellungen bezüglich Tesselation
			if (nk_tree_push(nk, NK_TREE_TAB, "Displacement", NK_MINIMIZED))
			{
				static nk_bool s_normalmap = nk_true;
				static nk_bool s_tesselation = nk_true;

				// Normalmapping de-/aktivieren
				if (nk_checkbox_label(nk, "Normalmapping:", &s_normalmap))
				{
					input->showNormalMap = !input->showNormalMap;
				}

				// Tesselation de-/aktivieren                
				if (nk_tree_push(nk, NK_TREE_NODE, "Tesselation", NK_MINIMIZED))
				{
					if (nk_checkbox_label(nk, "Tesselation:", &s_tesselation))
					{
						input->showTess = !input->showTess;
					}

					static float s_innerSlider = 1.0f;
					static float s_outerSlider = 1.0f;

					nk_label(nk, "Tess Inner", NK_TEXT_LEFT);
					if (nk_slider_float(nk, 1.0f, &s_innerSlider, 10.0f, 1.0f))
					{
						if (s_tesselation) {
							input->rendering.tessInner = s_innerSlider;
						}
						
					}
					nk_label(nk, "Tess Outer", NK_TEXT_LEFT);
					if (nk_slider_float(nk, 1.0f, &s_outerSlider, 10.0f, 1.0f))
					{
						if (s_tesselation) {
							input->rendering.tessOuter = s_outerSlider;
						}
					}
					nk_tree_pop(nk);
				}

				nk_tree_pop(nk);
			}

			// Einstellungen bezüglich 3D Transformationen
			if (nk_tree_push(nk, NK_TREE_TAB, "3D Transformation", NK_MINIMIZED))
			{
				if (nk_tree_push(nk, NK_TREE_NODE, "Translation", NK_MINIMIZED))
				{
					nk_tree_pop(nk);

					gui_widgetVec3(nk, input->rendering.translate);

				}
				if (nk_tree_push(nk, NK_TREE_NODE, "Rotation", NK_MINIMIZED))
				{
					nk_tree_pop(nk);

					gui_widgetVec3(nk, input->rendering.rotate);
				}
				if (nk_tree_push(nk, NK_TREE_NODE, "Skalierung", NK_MINIMIZED))
				{
					nk_tree_pop(nk);

					gui_widgetVec3(nk, input->rendering.scale);
				}
				nk_tree_pop(nk);
			}

			// Einstellungen bezüglich der Shader Auswahl
			if (nk_tree_push(nk, NK_TREE_TAB, "Shader Auswahl", NK_MINIMIZED))
			{
				enum choice { PHONG, DEBUG };

				static int s_choice = PHONG;
				s_choice = nk_option_label(nk, "Phong", s_choice == PHONG) ? PHONG : s_choice;
				s_choice = nk_option_label(nk, "Debug", s_choice == DEBUG) ? DEBUG : s_choice;

				input->shaderChoice = s_choice;
				nk_tree_pop(nk);
			}
			if (nk_tree_push(nk, NK_TREE_TAB, "PostProcess", NK_MINIMIZED))
			{
				float s_floatExposure = input->rendering.exposure;
				float s_floatGamma = input->rendering.gamma;

				nk_label(nk, "Exposure", NK_TEXT_LEFT);
				if (nk_slider_float(nk, 0.0f, &s_floatExposure, 1.0f, 0.01f))
				{
					input->rendering.exposure = s_floatExposure;
				}

				nk_label(nk, "Gamma", NK_TEXT_LEFT);
				if (nk_slider_float(nk, 0.0f, &s_floatGamma, 10.0f, 0.2f))
				{
					input->rendering.gamma = s_floatGamma;
				}

				nk_tree_pop(nk);
			}
		}
		nk_end(nk);
	}
}

/**
 * Zeigt allgemeine Zustandsinformationen über das Programm an.
 *
 * @param ctx Programmkontext.
 * @param nk Abkürzung für das GUI Handle.
 */
static void gui_renderStats(ProgContext* ctx, struct nk_context* nk)
{
	InputData* input = ctx->input;
	WindowData* win = ctx->winData;

	// Prüfen, ob das Menü überhaupt angezeigt werden soll.
	if (input->showStats)
	{
		float x = (float)win->realWidth - STATS_WIDTH;

		// Fenster öffnen.
		if (nk_begin(nk, GUI_WINDOW_STATS,
			nk_rect(x, 0, STATS_WIDTH, STATS_HEIGHT),
			NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND |
			NK_WINDOW_NO_INPUT))
		{
			// FPS Anzeigen
			nk_layout_row_dynamic(nk, 25, 1);
			char fpsString[15];
			snprintf(fpsString, 14, "FPS: %d", win->fps);
			nk_label(nk, fpsString, NK_TEXT_LEFT);
		}
		nk_end(nk);
	}
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

void gui_init(ProgContext* ctx)
{
	ctx->gui = malloc(sizeof(GuiData));
	GuiData* data = ctx->gui;

	// Der neu erzeugte Speicher muss zuerst geleert werden.
	memset(&data->glfw, 0, sizeof(struct nk_glfw));

	// Danach muss ein neuer Nuklear-GLFW3 Kontext erzeugt werden.
	data->nk = nk_glfw3_init(
		&data->glfw,
		ctx->window
	);

	// Nuklear Callbacks registrieren
	glfwSetScrollCallback(ctx->window, callback_glfwScroll);
	glfwSetCharCallback(ctx->window, callback_glfwChar);
	glfwSetMouseButtonCallback(ctx->window, callback_glfwMouseButton);

	// Als nächstes müssen wir einen neuen, leeren Font-Stash erzeugen.
	// Dadurch wird die Default-Font aktiviert.
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&data->glfw, &atlas);
	nk_glfw3_font_stash_end(&data->glfw);
}

void gui_render(ProgContext* ctx)
{
	GuiData* data = ctx->gui;

	// Wir signalisieren Nuklear, das ein neuer Frame gezeichnet wird.
	nk_glfw3_new_frame(&data->glfw);

	// Ab hier können alle unterschiedlichen GUIs aufgebaut werden
	gui_renderHelp(ctx, data->nk);
	gui_renderMenu(ctx, data->nk);
	gui_renderStats(ctx, data->nk);

	// Als letztes rendern wir die GUI
	common_pushRenderScopeSource("Nuklear GUI", GL_DEBUG_SOURCE_THIRD_PARTY);
	nk_glfw3_render(
		&data->glfw,
		NK_ANTI_ALIASING_ON,
		MAX_VERTEX_BUFFER,
		MAX_ELEMENT_BUFFER
	);
	common_popRenderScope();
}

void gui_cleanup(ProgContext* ctx)
{
	nk_glfw3_shutdown(&ctx->gui->glfw);
	free(ctx->gui);
}
