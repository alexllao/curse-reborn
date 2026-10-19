cmake_minimum_required(VERSION 3.21)
project(CurseRebornMonorepo LANGUAGES CXX)

# La compilación desde la raíz delega en el único motor nativo del monorepositorio.
add_subdirectory(apps/juego)
