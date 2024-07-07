/**
 * Modul für das Verarbeiten und Laden von 3D Szenen.
 * Eine Szene ist dabei ein serialisiertes Datenobjekt, das festlegt:
 * -> welche 3D Modelle angezeigt werden (nur eins zur Zeit)
 * -> welche Skybox verwendet werden soll (nicht implementiert)
 * -> welche Lichter (inklusiver Eigenschaften dieser) in der Szene sind
 * -> ggf. weitere Eigenschaften
 * 
 * Szenen werden dabei aus JSON Dateien geladen.
 * 
 * Copyright (C) 2020, FH Wedel
 * Autor: Nicolas Hollmann
 */

#include "scene.h"

#include <string.h>
#include <sesp/json.h>

#include "utils.h"

////////////////////////////// LOKALE DATENTYPEN ///////////////////////////////

// Struktur, die beim Parsen der JSON Hilft.
struct SceneParsingState {
    bool ok;
    char* model;
    Scene* scene;
};
typedef struct SceneParsingState SceneParsingState;

////////////////////////////// LOKALE FUNKTIONEN ///////////////////////////////

/**
 * Liest den Szenennamen aus der JSON Datei ein.
 * 
 * @param nameVal der JSON String in dem der Name steht
 * @param scene die Szene, die den Namen bekommen soll
 */
static void scene_parseSceneName(struct json_value_s* nameVal, Scene* scene)
{
    struct json_string_s* nameStr = json_value_as_string(nameVal);
    if (nameStr)
    {
        printf("[JSON] Found scene name: %s\n", nameStr->string);
        scene->name = malloc(nameStr->string_size + 1);
        memcpy(scene->name, nameStr->string, nameStr->string_size + 1);
    }
    else
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Name needs to be a string!\n"
        );
    }
}

/**
 * Diese Funktion ließt den Modellnamen aus der JSON Datei.
 * 
 * @param modelVal der JSON String mit dem Modellnamen
 * @param st der Parsing Status
 * @return true bei erfolg, false wenn der Eintrag korrupt war
 */
static bool scene_parseModelName(struct json_value_s* modelVal, 
                                 SceneParsingState* st)
{
    struct json_string_s* modelStr = json_value_as_string(modelVal);
    if (modelStr)
    {
        printf("[JSON] Found scene model: %s\n", modelStr->string);
        st->model = malloc(modelStr->string_size + 1);
        memcpy(st->model, modelStr->string, modelStr->string_size + 1);
    }
    else
    {
        fprintf(
            stderr, 
            "[JSON] Error: Modelname needs to be a string!\n"
        );
        return false;
    }

    return true;
}

/**
 * Diese Funktion liest einen Vec3 aus einer JSON Datei aus.
 * 
 * @param v das JSON Vec3 Objekt
 * @param keys die char-Schlüssel nach denen gesucht werden soll, z.B. "xyz"
 * @param out der Output Vektor
 */
static bool scene_parseVec3(struct json_value_s* v, const char* keys, vec3 out)
{
    glm_vec3_zero(out);

    struct json_object_s* vObj = json_value_as_object(v);
    if (!vObj)
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Found non-object vector!\n"
        );
        return false;
    }

    struct json_object_element_s* elem = vObj->start;
    while (elem)
    {
        int key = elem->name->string_size > 0 ? elem->name->string[0] : 0;   
        char* keyIdx = strchr(keys, key);

        if (keyIdx)
        {
            struct json_number_s* number = json_value_as_number(elem->value);
            if (number)
            {
                ptrdiff_t offset = keyIdx - keys;
                out[offset] = (float) atof(number->number);
            }
            else
            {
                fprintf(
                    stderr, 
                    "[JSON] Warning: Vector field is not a number!\n"
                );
            }
        }
        else
        {
            fprintf(stderr, "[JSON] Warning: Unkown vector field!\n");
        }
        
        elem = elem->next;
    }

    return true;
}

/**
 * Liest ein Richtungslicht aus der JSON Datei ein.
 * Wenn das Licht komplett geladen werden konnte wird es direkt der Szene
 * hinzugefügt.
 * 
 * @param lightVal das JSON Licht Objekt
 * @param scene die Szene der das Licht hinzugefügt werden soll
 */
static void scene_parseDirLight(struct json_value_s* lightVal, Scene* scene)
{
    struct json_object_s* lightObj = json_value_as_object(lightVal);
    if (!lightObj)
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Found non-object dirlight!\n"
        );
        return;
    }

    vec3 dir;
    vec3 color;
    bool dirSet = false;
    bool colorSet = false;

    struct json_object_element_s* dirLightElem = lightObj->start;
    while (dirLightElem)
    {
        if (strcmp(dirLightElem->name->string, "dir") == 0)
        {
            dirSet = scene_parseVec3(dirLightElem->value, "xyz", dir);                         
        }
        else if (strcmp(dirLightElem->name->string, "color") == 0)
        {
            colorSet = scene_parseVec3(dirLightElem->value, "rgb", color);    
        }

        dirLightElem = dirLightElem->next;
    }

    if (!dirSet || !colorSet)
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Missing property in dirlight!\n"
        );
        return;
    }

    DirLight* light = light_createDirLight(dir, color);
    scene_addDirLight(scene, light);
}

/**
 * Liest ein Punktlicht aus der JSON Datei ein.
 * Wenn das Licht komplett geladen werden konnte wird es direkt der Szene
 * hinzugefügt.
 * 
 * @param lightVal das JSON Licht Objekt
 * @param scene die Szene der das Licht hinzugefügt werden soll
 */
static void scene_parsePointLight(struct json_value_s* lightVal, Scene* scene)
{
    struct json_object_s* lightObj = json_value_as_object(lightVal);
    if (!lightObj)
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Found non-object pointlight!\n"
        );
        return;
    }

    vec3 pos;
    vec3 color;
    bool posSet = false;
    bool colorSet = false;

    struct json_object_element_s* pointLightElem = lightObj->start;
    while (pointLightElem)
    {
        if (strcmp(pointLightElem->name->string, "pos") == 0)
        {
            posSet = scene_parseVec3(pointLightElem->value, "xyz", pos);                         
        }
        else if (strcmp(pointLightElem->name->string, "color") == 0)
        {
            colorSet = scene_parseVec3(pointLightElem->value, "rgb", color);
        }

        pointLightElem = pointLightElem->next;
    }

    if (!posSet || !colorSet)
    {
        fprintf(
            stderr, 
            "[JSON] Warning: Missing property in pointlight!\n"
        );
        return;
    }

    PointLight* light = light_createPointLight(pos, color);
    scene_addPointLight(scene, light);
}

/**
 * Funktion zum Einlesen aller Lichtquellen in einem Array.
 * 
 * @param lights das Array mit allen Lichtern
 * @param scene die Szene, zu der die Lichter hinzugefügt werden sollen
 * @param dirLights bei true werden Richtungs-, sonst Punktlichter eingelesen
 */
static void scene_parseLightArray(struct json_value_s* lights, Scene* scene, 
    	                          bool dirLights)
{
    struct json_array_s* lightArr = json_value_as_array(lights);
    if (lightArr)
    {
        struct json_array_element_s* lightElem = lightArr->start;
        while (lightElem)
        {
            if (dirLights)
            {
                scene_parseDirLight(lightElem->value, scene);
            }
            else
            {
                scene_parsePointLight(lightElem->value, scene);
            }
            lightElem = lightElem->next;
        }
    }
    else
    {
        fprintf(
            stderr, 
            "[JSON] Warning: [dir/point]lights needs to be an array!\n"
        );
    }
}

/**
 * Hauptschleife für das parsen der JSON Datei.
 * Hier werden alle Root-Elemente eingelesen.
 * 
 * @param root die Wurzel des JSON Dokumentes
 * @param st der Parsing-State, der beim Parsen verändert wird
 */
static void scene_parseJson(struct json_value_s* root, SceneParsingState* st)
{
    // Wir gehen davon aus, dass die JSON Datei grundsätzlich ok ist.
    st->ok = true;

    // Als erstes laden wir das root Objekt und werfen einen Fehler,
    // wenn an der Wurzel garkein Objekt liegt.
    struct json_object_s* rootObj = json_value_as_object(root);
    if (!rootObj) {
        fprintf(stderr, "[JSON] Error: Object at root expected!\n");
        st->ok = false;
        return;
    }

    // Dann laden wir das erste Element des Objektes und gehen in einer
    // Schleife über alle Elemente.
    struct json_object_element_s* elem = rootObj->start;
    while (elem)
    {
        if (strcmp(elem->name->string, "name") == 0)
        {
            scene_parseSceneName(elem->value, st->scene);
        }
        else if (strcmp(elem->name->string, "model") == 0)
        {
            if (!scene_parseModelName(elem->value, st))
            {
                st->ok = false;
                return;
            }
        }
        else if (strcmp(elem->name->string, "dirlights") == 0)
        {
            scene_parseLightArray(elem->value, st->scene, true);
        }
        else if (strcmp(elem->name->string, "pointlights") == 0)
        {
            scene_parseLightArray(elem->value, st->scene, false);
        }
        else
        {
            fprintf(
                stderr, 
                "[JSON] Warning: Found unsupported root key: %s\n", 
                elem->name->string
            );
        }
        
        // Im nächsten Schleifendurchlauf schauen wir uns das nächste Element
        // des Root-Objektes an.
        elem = elem->next;
    }

    // Prüfen, ob überhaupt ein Modell gesetzt wurde.
    if (!st->model)
    {
        fprintf(
            stderr, 
            "[JSON] Error: No Modelname supplied!\n"
        );
        st->ok = false;
        return;
    }

    printf("[JSON] Parsing done.\n\n");
}

/**
 * Hilfsfunktion zum Löschen des ParsingStates.
 * 
 * @param st der ParsingState der gelöscht werden soll.
 */
static void scene_deleteParsingState(SceneParsingState* st)
{
    if (st->model)
    {
        free(st->model);
    }
    if (st->scene)
    {
        scene_deleteScene(st->scene);
    }
}

//////////////////////////// ÖFFENTLICHE FUNKTIONEN ////////////////////////////

Scene* scene_loadScene(const char* filename)
{
    // Zuerst muss der Inhalt der Datei geladen werden.
    char* jsonContent = utils_readFile(filename);

    // Danach muss die Datei geparst werden.
    struct json_parse_result_s parseResult;
    struct json_value_s* root = json_parse_ex(
        jsonContent, 
        strlen(jsonContent),
        json_parse_flags_allow_json5,
        NULL,
        NULL, 
        &parseResult
    );

    // Bevor wir weiter machen prüfen wir, ob die Datei erfolgreich
    // eingelesen wurde, also ob die JSON Syntax korrekt ist.
    // Hier wird nicht geprüft, ob die erwarteten Strukturen vorliegen.
    if (parseResult.error != json_parse_error_none)
    {
        fprintf(
            stderr, 
            "Error on parsing JSON file in line %zu.\n", 
            parseResult.error_line_no
        );

        // Wir müssen nur den Speicher für die Datei freigeben.
        // Das root-Objekt wird null sein, aber wir gehen auf Nummer sicher:
        if (root != NULL)
        {
            fprintf(stderr, "Error: Got JSON Root in error scenario!\n");
            free(root);
        }
        free(jsonContent);

        return NULL;
    }

    // Wir legen uns ein Zustandobjekt an, das die Szene enthält und aufbaut.
    SceneParsingState state;
    memset(&state, 0, sizeof(SceneParsingState));
    state.scene = malloc(sizeof(Scene));
    memset(state.scene, 0, sizeof(Scene));

    // Dann parsen wir die gesammte JSON Datei in einer Unterfunktion.
    scene_parseJson(root, &state);

    // Wir machen nur weiter, wenn die Datei erfolgreich geparst wurde.
    Scene* scene = NULL;
    if (state.ok)
    {
        // Verzeichnis anhängen um den relativen Pfad zu korrigieren.
        char* modelPath = utils_getDirectory(filename);
        modelPath = realloc(
            modelPath, 
            strlen(modelPath) + strlen(state.model) + 1
        );
        strcat(modelPath, state.model);
        
        Model* model = model_loadModel(modelPath);
        if (model)
        {
            // Zuerst die Szene verschieben, damit sie nicht mit dem
            // ParsingState gelöscht wird.
            scene = state.scene;
            state.scene = NULL;

            // Dann das 3D Modell setzen.
            scene->model = model;
        }
    }

    // Zum Schluss muss immer der belegte Speicher wieder freigegeben werden.
    scene_deleteParsingState(&state);
    free(root);
    free(jsonContent);

    return scene;
}

Scene* scene_fromModel(const char* filename)
{
    Scene* scene = NULL;
    Model* model = model_loadModel(filename);
    if (model)
    {
        scene = malloc(sizeof(Scene));
        memset(scene, 0, sizeof(Scene));
        scene->model = model;
        scene->name = malloc(strlen(filename) + 1);
        strcpy(scene->name, filename);
    }

    return scene;
}

void scene_addDirLight(Scene* scene, DirLight* light)
{
    scene->countDirLights++;
    scene->dirLights = realloc(
        scene->dirLights, 
        sizeof(DirLight*) * scene->countDirLights
    );
    scene->dirLights[scene->countDirLights - 1] = light;
}

void scene_addPointLight(Scene* scene, PointLight* light)
{
    scene->countPointLights++;
    scene->pointLights = realloc(
        scene->pointLights, 
        sizeof(PointLight*) * scene->countPointLights
    );
    scene->pointLights[scene->countPointLights - 1] = light;
}

void scene_deleteScene(Scene* scene)
{
    // Erst den Namen löschen
    if (scene->name)
    {
        free(scene->name);
        scene->name = NULL;
    }

    // Dann das Modell löschen
    if (scene->model)
    {
        model_deleteModel(scene->model);
        scene->model = NULL;
    }

    // Dann alle Richtungslichter
    if (scene->dirLights)
    {
        for (int i = 0; i < scene->countDirLights; i++)
        {
            light_deleteDirLight(scene->dirLights[i]);
        }
        free(scene->dirLights);
        scene->dirLights = NULL;
    }

    // Dann alle Punktlichter
    if (scene->pointLights)
    {
        for (int i = 0; i < scene->countPointLights; i++)
        {
            light_deletePointLight(scene->pointLights[i]);
        }
        free(scene->pointLights);
        scene->pointLights = NULL;
    }

    // Und zum Schluss die Szene selbst
    free(scene);
}
