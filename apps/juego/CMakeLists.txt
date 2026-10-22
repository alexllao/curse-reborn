cmake_minimum_required(VERSION 3.21)
project(CurseReborn LANGUAGES CXX)

get_filename_component(RAIZ_REPOSITORIO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(RAIZ_ASSETS "${RAIZ_REPOSITORIO}/packages/assets")
find_package(SDL3 3.4.18 CONFIG REQUIRED)
# Mantener la ubicación del ejecutable compatible con los comandos existentes.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Preparar únicamente recursos propios; los archivos comerciales siguen siendo locales.
find_program(NODE_EXECUTABLE node REQUIRED)
file(GLOB_RECURSE FUENTES_HABITACIONES CONFIGURE_DEPENDS "${RAIZ_ASSETS}/lanzador/habitaciones/*")
file(GLOB_RECURSE FUENTES_MARCOS CONFIGURE_DEPENDS "${RAIZ_ASSETS}/marcos/*")
file(GLOB FUENTES_DECORADOS CONFIGURE_DEPENDS "${RAIZ_ASSETS}/original/decorados/*")
file(GLOB FUENTES_PRESENTACION CONFIGURE_DEPENDS "${RAIZ_ASSETS}/lanzador/presentacion/*")
file(GLOB FUENTES_SONIDOS_PC CONFIGURE_DEPENDS "${RAIZ_ASSETS}/lanzador/sonidos_pc/*")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/catalogo_presentacion.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_marcos.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_decorados.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_habitaciones.hpp"
  COMMAND "${NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_marcos.mjs" "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS ${FUENTES_PRESENTACION} "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_presentacion.mjs" ${FUENTES_HABITACIONES} "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_habitaciones.mjs" ${FUENTES_MARCOS} ${FUENTES_DECORADOS} ${FUENTES_SONIDOS_PC} "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_sonidos_pc.mjs" "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_marcos.mjs"
  VERBATIM)

# Compartir la lista de fuentes con las herramientas Node de compilación y comparación.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/fuentes_motor.json" MANIFIESTO_MOTOR)
string(JSON CANTIDAD_FUENTES LENGTH "${MANIFIESTO_MOTOR}")
math(EXPR ULTIMA_FUENTE "${CANTIDAD_FUENTES} - 1")
foreach(INDICE RANGE ${ULTIMA_FUENTE})
  string(JSON FUENTE GET "${MANIFIESTO_MOTOR}" ${INDICE})
  list(APPEND FUENTES_MOTOR "${FUENTE}")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fuentes_motor.json")
add_executable(curse-reborn plataformas/sdl/interfaz/contador_fps.cpp plataformas/sdl/video/filtros.cpp plataformas/sdl/principal.cpp plataformas/sdl/mazmorra.cpp plataformas/sdl/introduccion.cpp plataformas/sdl/audio/audio.cpp plataformas/sdl/audio/musica_adlib.cpp plataformas/sdl/audio/musica_mt32.cpp plataformas/sdl/audio/opl.cpp plataformas/sdl/audio/opl2.cpp plataformas/sdl/audio/ymfm_opl.cpp plataformas/sdl/audio/ymfm_adpcm.cpp plataformas/sdl/audio/ymfm_pcm.cpp ${FUENTES_MOTOR})
target_compile_features(curse-reborn PRIVATE cxx_std_17)
target_sources(curse-reborn PRIVATE plataformas/sdl/audio/musica_remaster.cpp plataformas/sdl/interfaz/json.cpp)
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND "${NODE_EXECUTABLE}"
  "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/audio/preparar_musica.mjs" "$<TARGET_FILE_DIR:curse-reborn>")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PAQUETE_JUEGO)
string(JSON VERSION_JUEGO GET "${PAQUETE_JUEGO}" version)
target_compile_definitions(curse-reborn PRIVATE CURSE_REBORN_VERSION="${VERSION_JUEGO}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/package.json")
target_sources(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/catalogo_presentacion.hpp" plataformas/sdl/interfaz/partidas.cpp plataformas/sdl/tiempo_juego.cpp plataformas/sdl/video/marcos.cpp plataformas/sdl/lanzador/habitacion.cpp "${CMAKE_CURRENT_BINARY_DIR}/catalogo_habitaciones.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_marcos.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_decorados.hpp")
target_include_directories(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_BINARY_DIR OR CMAKE_CONFIGURATION_TYPES)
  add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_BINARY_DIR}/assets/lanzador/habitaciones"
    "$<TARGET_FILE_DIR:curse-reborn>/assets/lanzador/habitaciones")
endif()
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${RAIZ_ASSETS}/marcos"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/marcos")
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${RAIZ_ASSETS}/original/decorados"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/original/decorados")
target_link_libraries(curse-reborn PRIVATE SDL3::SDL3)
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${RAIZ_ASSETS}/lanzador/icono"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/lanzador/icono")
if(WIN32)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/icono.rc" "1 ICON \"${RAIZ_ASSETS}/lanzador/icono/curse-reborn.ico\"\n")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    get_filename_component(CARPETA_COMPILADOR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    find_program(COMPILADOR_RECURSOS_LLVM llvm-rc HINTS "${CARPETA_COMPILADOR}" REQUIRED)
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/icono.res"
      COMMAND "${COMPILADOR_RECURSOS_LLVM}" /FO "${CMAKE_CURRENT_BINARY_DIR}/icono.res" "${CMAKE_CURRENT_BINARY_DIR}/icono.rc"
      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/icono.rc" "${RAIZ_ASSETS}/lanzador/icono/curse-reborn.ico"
      VERBATIM)
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/icono.res" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
    target_sources(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/icono.res")
  else()
    enable_language(RC)
    target_sources(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/icono.rc")
    set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/icono.rc" APPEND PROPERTY OBJECT_DEPENDS
      "${RAIZ_ASSETS}/lanzador/icono/curse-reborn.ico")
  endif()
endif()
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${RAIZ_ASSETS}/lanzador/sonidos_pc"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/lanzador/sonidos_pc")
if(MSVC)
  target_compile_options(curse-reborn PRIVATE /W4 /WX /utf-8)
else()
  target_compile_options(curse-reborn PRIVATE -Wall -Wextra -Werror)
endif()
if(WIN32)
  add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:curse-reborn>)
endif()

# Acompañar cada compilación con la licencia del sintetizador externo, sin modificar su fuente.
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/terceros/nuked_opl3/LICENSE"
  "$<TARGET_FILE_DIR:curse-reborn>/LICENCIA_NUKED_OPL3.txt")

find_path(MT32EMU_INCLUDE_DIR mt32emu/mt32emu.h
  HINTS "${CMAKE_CURRENT_SOURCE_DIR}/trabajo/dependencias/mt32emu/include" REQUIRED)
target_include_directories(curse-reborn PRIVATE "${MT32EMU_INCLUDE_DIR}")

# La carga es dinámica; conservar un nombre estable y permitir reemplazar la biblioteca.
if(WIN32)
  set(NOMBRE_MT32EMU mt32emu.dll)
  set(CANDIDATOS_MT32EMU mt32emu-2.dll mt32emu.dll)
elseif(APPLE)
  set(NOMBRE_MT32EMU libmt32emu.dylib)
  set(CANDIDATOS_MT32EMU libmt32emu.dylib)
else()
  set(NOMBRE_MT32EMU libmt32emu.so)
  set(CANDIDATOS_MT32EMU libmt32emu.so)
endif()
find_file(MT32EMU_BIBLIOTECA NAMES ${CANDIDATOS_MT32EMU}
  HINTS "${CMAKE_CURRENT_SOURCE_DIR}/trabajo/dependencias/mt32emu/bin"
        "${CMAKE_CURRENT_SOURCE_DIR}/trabajo/dependencias/mt32emu/lib")
if(MT32EMU_BIBLIOTECA)
  add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${MT32EMU_BIBLIOTECA}" "$<TARGET_FILE_DIR:curse-reborn>/${NOMBRE_MT32EMU}")
endif()
set(CARPETA_LICENCIA_MT32EMU "${CMAKE_CURRENT_SOURCE_DIR}/trabajo/dependencias/mt32emu/share/doc/munt/libmt32emu")
foreach(LICENCIA COPYING.LESSER.txt COPYING.txt)
  if(EXISTS "${CARPETA_LICENCIA_MT32EMU}/${LICENCIA}")
    add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CARPETA_LICENCIA_MT32EMU}/${LICENCIA}" "$<TARGET_FILE_DIR:curse-reborn>/MT32EMU_${LICENCIA}")
  endif()
endforeach()

# Mantener la atribución BSD del núcleo YM3812 también en compilaciones fuera de Windows.
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_CURRENT_SOURCE_DIR}/terceros/ymfm/LICENSE"
  "$<TARGET_FILE_DIR:curse-reborn>/LICENCIA_YMFM.txt")

# Las habitaciones interactivas son independientes de los marcos decorativos.
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
 "${RAIZ_ASSETS}/lanzador/habitaciones"
 "$<TARGET_FILE_DIR:curse-reborn>/assets/lanzador/habitaciones")

# El preparador copia los banners registrados y el logo usado por el menú del port.
if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL CMAKE_BINARY_DIR OR CMAKE_CONFIGURATION_TYPES)
  add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_BINARY_DIR}/assets/lanzador/presentacion"
    "$<TARGET_FILE_DIR:curse-reborn>/assets/lanzador/presentacion")
endif()
