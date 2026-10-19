cmake_minimum_required(VERSION 3.21)
project(CurseReborn LANGUAGES CXX)

find_package(SDL3 3.4.18 CONFIG REQUIRED)

# Preparar únicamente recursos propios; los archivos comerciales siguen siendo locales.
find_program(NODE_EXECUTABLE node REQUIRED)
file(GLOB_RECURSE FUENTES_HABITACIONES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/habitaciones/*")
file(GLOB FUENTES_MARCOS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/marcos/*")
file(GLOB FUENTES_DECORADOS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/decorados/*")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/catalogo_marcos.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_decorados.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_habitaciones.hpp"
  COMMAND "${NODE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_marcos.mjs" "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS ${FUENTES_HABITACIONES} "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_habitaciones.mjs" ${FUENTES_MARCOS} ${FUENTES_DECORADOS} "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/gestionar_marcos.mjs"
  VERBATIM)

# Compartir la lista de fuentes con las herramientas Node de compilación y comparación.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/herramientas/fuentes_motor.json" MANIFIESTO_MOTOR)
string(JSON CANTIDAD_FUENTES LENGTH "${MANIFIESTO_MOTOR}")
math(EXPR ULTIMA_FUENTE "${CANTIDAD_FUENTES} - 1")
foreach(INDICE RANGE ${ULTIMA_FUENTE})
  string(JSON FUENTE GET "${MANIFIESTO_MOTOR}" ${INDICE})
  list(APPEND FUENTES_MOTOR "${FUENTE}")
endforeach()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS herramientas/fuentes_motor.json)
add_executable(curse-reborn plataformas/sdl/contador_fps.cpp plataformas/sdl/filtros.cpp plataformas/sdl/principal.cpp plataformas/sdl/mazmorra.cpp plataformas/sdl/introduccion.cpp plataformas/sdl/audio.cpp plataformas/sdl/musica_adlib.cpp plataformas/sdl/musica_mt32.cpp plataformas/sdl/opl.cpp plataformas/sdl/opl2.cpp plataformas/sdl/ymfm_opl.cpp plataformas/sdl/ymfm_adpcm.cpp plataformas/sdl/ymfm_pcm.cpp ${FUENTES_MOTOR})
target_compile_features(curse-reborn PRIVATE cxx_std_17)
target_sources(curse-reborn PRIVATE plataformas/sdl/partidas.cpp plataformas/sdl/tiempo_juego.cpp plataformas/sdl/marcos.cpp plataformas/sdl/habitacion.cpp "${CMAKE_CURRENT_BINARY_DIR}/catalogo_habitaciones.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_marcos.hpp" "${CMAKE_CURRENT_BINARY_DIR}/catalogo_decorados.hpp")
target_include_directories(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/marcos"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/original/marcos")
add_custom_command(TARGET curse-reborn POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/decorados"
  "$<TARGET_FILE_DIR:curse-reborn>/assets/original/decorados")
target_link_libraries(curse-reborn PRIVATE SDL3::SDL3)
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
 "${CMAKE_CURRENT_SOURCE_DIR}/assets/original/habitaciones"
 "$<TARGET_FILE_DIR:curse-reborn>/assets/original/habitaciones")
