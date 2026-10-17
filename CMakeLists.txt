cmake_minimum_required(VERSION 3.21)
project(CurseReborn LANGUAGES CXX)

find_package(SDL3 3.4.18 CONFIG REQUIRED)

add_executable(curse-reborn plataformas/sdl/principal.cpp
  motor/recursos/titulo.cpp motor/recursos/rnc.cpp motor/video/planos.cpp)
target_compile_features(curse-reborn PRIVATE cxx_std_17)
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
