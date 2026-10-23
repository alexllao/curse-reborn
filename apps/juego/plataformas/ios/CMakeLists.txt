cmake_minimum_required(VERSION 3.24)
project(CurseRebornIOS LANGUAGES C CXX OBJC)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
  message(FATAL_ERROR "Usa el preparador iOS en un Mac con Xcode, no el destino de escritorio.")
endif()
get_filename_component(RAIZ_JUEGO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(CURSE_IOS_GENERADOS "" CACHE PATH "Recursos y catálogos preparados localmente")
set(CURSE_IOS_EQUIPO "" CACHE STRING "Equipo de firma de Xcode, opcional")
if(NOT EXISTS "${CURSE_IOS_GENERADOS}/catalogo_habitaciones.hpp" OR
   NOT EXISTS "${CURSE_IOS_GENERADOS}/datos_originales/CURSE/CURSE.EXE")
  message(FATAL_ERROR "Faltan recursos locales: ejecuta pnpm ios desde la raíz del repositorio.")
endif()

# Compilar SDL para el SDK elegido; no reutilizar una biblioteca macOS o Windows en iPad.
include(FetchContent)
file(READ "${RAIZ_JUEGO}/herramientas/dependencias_sdl.json" DEPENDENCIAS)
string(JSON SDL_URL GET "${DEPENDENCIAS}" fuentes url)
string(JSON SDL_SHA GET "${DEPENDENCIAS}" fuentes sha256)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3 URL "${SDL_URL}" URL_HASH "SHA256=${SDL_SHA}"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(SDL3)
FetchContent_GetProperties(SDL3 SOURCE_DIR SDL_FUENTES)

# La misma lista SDL y el mismo manifiesto de motor que en escritorio.
include("${RAIZ_JUEGO}/plataformas/sdl/fuentes.cmake")
file(READ "${RAIZ_JUEGO}/fuentes_motor.json" MANIFIESTO)
string(JSON CANTIDAD LENGTH "${MANIFIESTO}")
math(EXPR ULTIMA "${CANTIDAD} - 1")
foreach(I RANGE ${ULTIMA})
  string(JSON FUENTE GET "${MANIFIESTO}" ${I})
  list(APPEND FUENTES_MOTOR "${RAIZ_JUEGO}/${FUENTE}")
endforeach()
add_executable(curse-reborn MACOSX_BUNDLE ${FUENTES_SDL} ${FUENTES_MOTOR} fondos.cpp)
target_compile_features(curse-reborn PRIVATE cxx_std_17)
target_include_directories(curse-reborn PRIVATE "${CURSE_IOS_GENERADOS}")
file(READ "${RAIZ_JUEGO}/package.json" PAQUETE)
string(JSON VERSION GET "${PAQUETE}" version)
target_compile_definitions(curse-reborn PRIVATE CURSE_REBORN_VERSION="${VERSION}" CURSE_REBORN_SIN_MT32=1)
target_link_libraries(curse-reborn PRIVATE SDL3::SDL3-static "-framework ImageIO" "-framework CoreGraphics")
target_compile_options(curse-reborn PRIVATE -Wall -Wextra)

# Xcode incorpora los recursos en el bundle antes de firmarlo; nunca escribir dentro de la app.
file(GLOB_RECURSE RECURSOS CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE "${CURSE_IOS_GENERADOS}/*")
foreach(RECURSO IN LISTS RECURSOS)
  if(RECURSO MATCHES "\\.hpp$" OR RECURSO MATCHES "/preparacion\\.json$" OR RECURSO MATCHES "/Assets\\.xcassets/")
    continue()
  endif()
  file(RELATIVE_PATH RELATIVO "${CURSE_IOS_GENERADOS}" "${RECURSO}")
  get_filename_component(CARPETA "${RELATIVO}" DIRECTORY)
  set_source_files_properties("${RECURSO}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${CARPETA}")
  target_sources(curse-reborn PRIVATE "${RECURSO}")
endforeach()
target_sources(curse-reborn PRIVATE "${CURSE_IOS_GENERADOS}/Assets.xcassets")
set_source_files_properties("${CURSE_IOS_GENERADOS}/Assets.xcassets" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
configure_file("${SDL_FUENTES}/LICENSE.txt" "${CMAKE_CURRENT_BINARY_DIR}/LICENCIA_SDL3.txt" COPYONLY)
target_sources(curse-reborn PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/LICENCIA_SDL3.txt")
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/LICENCIA_SDL3.txt" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_target_properties(curse-reborn PROPERTIES
  OUTPUT_NAME CurseReborn
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
  MACOSX_BUNDLE_GUI_IDENTIFIER "com.alexllao.curse-reborn"
  MACOSX_BUNDLE_BUNDLE_NAME "Curse of Enchantia: Reborn"
  MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}"
  MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}"
  XCODE_GENERATE_SCHEME TRUE
  XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.alexllao.curse-reborn"
  XCODE_ATTRIBUTE_CODE_SIGN_STYLE Automatic
  XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
  XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "2")
if(CURSE_IOS_EQUIPO)
  set_target_properties(curse-reborn PROPERTIES XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${CURSE_IOS_EQUIPO}")
endif()
