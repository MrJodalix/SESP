###############################################################################
# SESP CMake Konfiguration
#
# Copyright (C) 2023, FH Wedel
# Autor: Nicolas Hollmann, Philipp Munz
###############################################################################

# Minimum CMake Version
cmake_minimum_required(VERSION 3.13)

# Projektname
project("ueb03" LANGUAGES C CXX VERSION 1.0.0)

############################# Absicherungen ###################################

# In-source builds deaktivieren
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       Please create a separate directory for build files.
")
endif()

# Release voreinstellen, wenn keine Auswahl getroffen wurde
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

################################ Includes #####################################

# FetchContent hilft dabei, Bibliotheken bei Bedarf herunterzuladen
include(FetchContent)

############################## Einstellungen ##################################

# Wir verwenden C99 für alle Übungen
set(C_STANDARD 99)

# Netzlaufwerk im MRZ mit Abhängigkeiten
set(NETWORK_DRIVE_PATH "P:/Bibliotheken")

# Das ZERO_CHECK Target deaktivieren
set(CMAKE_SUPPRESS_REGENERATION true)

# FetchContent soll Ausgaben zum Prozess machen
set(FETCHCONTENT_QUIET NO)

# Das aktuelle Projekt in Visual Studio als Startprojekt festlegen
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Aktiviert die Verwendung von Ordnern in IDEs, die dies unterstützen.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################# Funktionen ##################################

# Funktion um die Abhängigkeiten entweder von einem Netzlaufwerk oder 
# von GitHub herunterzuladen
function(fetch_dependency DEPENDENCY_NAME GITHUB_URL LOCAL_FILE_PATH)
    set(FULL_LOCAL_FILE_PATH "${NETWORK_DRIVE_PATH}/${LOCAL_FILE_PATH}")
	message(STATUS "Searching for ${FULL_LOCAL_FILE_PATH}")

    # Test, ob Datei im Netzwerk vorhanden ist
    if(EXISTS "${FULL_LOCAL_FILE_PATH}")
        message(STATUS "Found the ${DEPENDENCY_NAME} file on the network drive.")
		set(FINAL_URL "file://${FULL_LOCAL_FILE_PATH}")
    else()
        message(STATUS "${DEPENDENCY_NAME} file not found on the network drive. Fetching from GitHub.")
		set(FINAL_URL "${GITHUB_URL}")
	endif()
	
	message(STATUS "Fetching from ${FINAL_URL}")

    FetchContent_Declare(
        ${DEPENDENCY_NAME}
		DOWNLOAD_EXTRACT_TIMESTAMP true 
        URL ${FINAL_URL}         
    )

    FetchContent_GetProperties(${DEPENDENCY_NAME})
    if(NOT ${DEPENDENCY_NAME}_POPULATED)
        FetchContent_Populate(${DEPENDENCY_NAME})
        add_subdirectory(${${DEPENDENCY_NAME}_SOURCE_DIR} ${${DEPENDENCY_NAME}_BINARY_DIR})
    endif()
    
endfunction()

################################# OpenGL ######################################

# OpenGL muss auf dem System vorhanden sein
find_package(OpenGL REQUIRED)

################################## GLFW #######################################

# Unbenötigte Features deaktivieren
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# GLFW als Abhängigkeit anlegen
fetch_dependency(
    glfw
    https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip 
    glfw-3.3.8.zip
)

################################## CGLM #######################################

# Unbenötigte Features deaktivieren
set(CGLM_SHARED OFF CACHE BOOL "" FORCE)
set(CGLM_STATIC ON CACHE BOOL "" FORCE)
set(CGLM_USE_TEST OFF CACHE BOOL "" FORCE)
set(CGLM_USE_C99 ON CACHE BOOL "" FORCE)

# CGLM als Abhängigkeit anlegen
fetch_dependency(
    cglm
    https://github.com/recp/cglm/archive/v0.9.1.zip
    cglm-0.9.1.zip
)

################################# ASSIMP ######################################

# Unbenötigte Features deaktivieren
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# Assimp ist sehr groß und unterstützt sehr viele Formate, die
# wir in dieser Übung garnicht benötigen. Außerdem stehen auch
# Exporter für viele Formate zur Verfügung.
# Um Speicher zu sparen und die Übersetzungszeit zu verkürzen,
# werden hier alle Exporter deaktivert. Des weiteren werden nur
# einzelne Importer wieder aktiviert.
# Eine Übersicht über alle untersützten Formate findet sich hier:
# https://github.com/assimp/assimp/blob/master/doc/Fileformats.md
# Die hier aufgelisteten Schalter werden hier definiert:
# https://github.com/assimp/assimp/blob/master/code/CMakeLists.txt
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)

# ASSIMP als Abhängigkeit anlegen
fetch_dependency(
    assimp
    https://github.com/assimp/assimp/archive/v5.0.1.zip
    assimp-5.0.1.zip
)

################################ Projekt ######################################

# Alle Quellcode-Dateien und Header im src-Verzeichnis finden:
file(GLOB src_files
    "src/*.h"
    "src/*.c"
)

# Zusätzliche Quellcode-Dateien, die zwar mit dem Projekt kompiliert werden,
# jedoch nicht direkt Teil des Projektes sind. Zum Beispiel die Implementierungen
# der Single Header Libraries.
set(extra_src
    3rdparty/glad/glad.c
    3rdparty/misc/implementations.c
)

# Alle Dateien im Shader-Verzeichnis werden auch zum Projekt hinzugefügt.
# Nur dadurch ist es möglich, dass die Shader direkt in Visual Studio
# aufgelistet werden.
file(GLOB_RECURSE shaders
    "res/shader/*"
)

# Projekt anlegen
add_executable(${PROJECT_NAME} ${src_files} ${extra_src} ${shaders})

# Include Verzeichnis zum Projekt hinzufügen
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Bibliotheken zum Projekt hinzufügen
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} glfw cglm assimp)

if(UNIX AND NOT APPLE)
    # Unter Linux muss die Mathebibliothek extra gelinkt werden, wenn Funktionen
    # aus math.h genutzt werden sollen.
    target_link_libraries(${PROJECT_NAME} m)
endif()

# Den Namen des Programms festlegen
target_compile_definitions(${PROJECT_NAME} PRIVATE PROGRAM_NAME="${PROJECT_NAME}")

# Bestimmen, ob 64- oder 32-Bit-System
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(sys_arch_bits "64bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(sys_arch_bits "32bit")
else()
    # Unkown
    set(sys_arch_bits "??bit")
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE ARCH_BITS="${sys_arch_bits}")

# Das Buildverzeichnis zur Ausgabe des Programmes nutzen, kein Unterverzeichnis anlegen.
set_target_properties(${PROJECT_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_BINARY_DIR}"
)

########################### Visual Studio Filter ##############################

# Targets der Dependencies in Ordnern organisieren.
# Dadurch wird das Projekt etwas übersichtlicher.
# Zuerst brauchen wir eine Liste aller Dependencies die versteckt werden sollen:
set(dependency_targets
    glfw
    cglm
    assimp
    UpdateAssimpLibsDebugSymbolsAndDLLs
    uninstall
    zlib
    zlibstatic
    IrrXML
)

# Danach gehen wir über die Liste und verstecken sie in einem Unterordner:
foreach(_dependency IN ITEMS ${dependency_targets})
    if(TARGET ${_dependency})
        set_target_properties(
            ${_dependency}
            PROPERTIES FOLDER "Dependencies"
        )
    endif()
endforeach()

# Die folgende Schleife liest für alle Shader die Pfade aus und sortiert sie in
# einem Visual Studio Filter. Dadurch wieder das Projekt dort aufgeräumter.
foreach(_shader IN ITEMS ${shaders})
    get_filename_component(_source_path "${_shader}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}/res/shader" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("Shader\\${_group_path}" FILES "${_shader}")
endforeach()

# Dieser Befehl sort dafür, dass die extra Quellcodes nicht unter Sources
# in einer eigenen Kategorie angezeigt werden.
source_group("3rdParty" FILES ${extra_src})

############################## Compilerflags ##################################

if(MSVC)
    # Flags unter Windows mit dem MSVC:
    # /W4: Höchstes Warnungslevel einstellen
    # /WX: Alle Warnungen als Fehler betrachten, aber:
    # /wd4996: Warnung 4996 (veraltete Systemfunktionen) deaktivieren
    # /wd4204: Warnung 4204 (nicht-konstante struct Initialisierung) deaktivieren
    # /wd4127: Warnung 4127 (konstanter Vergleich) deaktivieren
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4996 /wd4204 /wd4127)
else()
    # Flags bei allen anderen Compilern:
    # -Wall: (Fast) alle Warnungen aktivieren
    # -Wno-long-long: Warnung bezüglich der Verwendung von long-long deaktivieren
    # -Werror: Alle Warnungen als Fehler behandeln
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wno-long-long -Werror)

    if(APPLE)
        # Unter macOS gilt OpenGL als veraltet. Deshalb werden vom Compiler Warnungen erzeugt,
        # die durch -Werror zu Fehlermeldungen werden. Dies verhindert, dass das Projekt
        # übersetzt werden kann. Aus dem Grund ist es erforderlich, die Warnung für
        # veralteten Code zu deaktivieren.
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated)
    endif()
endif()

############################### Shaderpfade ###################################

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res")
    if(WIN32)
        # Auf Windows können Symlinks nur als Administrator oder im Entwicklermodus erstellt werden.
        # Deshalb berechnen wir den relativen Pfad von dem Build-Verzeichnis zum Ressourcen-Verzeichnis.
        # Dazu berechnen wir ersteinmal den absoluten Pfad zu den Ressourcen:
        get_filename_component(RESOURCE_ABS_PATH "./res" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

        # Als nächstes bestimmen wir dann den relativen Pfad von dem Build-Verzeichnis zu dem vorherigen
        # absoluten Pfad und setzen ihn als Konstante in unserem Programm:
        file(RELATIVE_PATH RESOURCE_REL_PATH "${CMAKE_CURRENT_BINARY_DIR}" "${RESOURCE_ABS_PATH}")
        target_compile_definitions(${PROJECT_NAME} PRIVATE RESOURCE_PATH="${RESOURCE_REL_PATH}/")
    else()
        # Unter Linux und macOS kann stattdessen einfach ein Symlink verwendet werden:
        add_custom_target(
            res_link ALL 
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                "${CMAKE_CURRENT_SOURCE_DIR}/res" "${CMAKE_CURRENT_BINARY_DIR}/res")
    endif()
else()
    message(STATUS "SESP: No resource directory in use.")
endif()

###############################################################################
################################## EOF ########################################
###############################################################################
