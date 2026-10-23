# Una única lista para Windows e iOS; el motor conserva su manifiesto JSON compartido.
set(FUENTES_SDL)
foreach(FUENTE
    principal.cpp mazmorra.cpp introduccion.cpp tiempo_juego.cpp
    interfaz/contador_fps.cpp interfaz/json.cpp interfaz/partidas.cpp
    video/filtros.cpp video/marcos.cpp lanzador/habitacion.cpp
    audio/audio.cpp audio/musica_adlib.cpp audio/musica_mt32.cpp audio/musica_remaster.cpp
    audio/opl.cpp audio/opl2.cpp audio/ymfm_opl.cpp audio/ymfm_adpcm.cpp audio/ymfm_pcm.cpp)
  list(APPEND FUENTES_SDL "${CMAKE_CURRENT_LIST_DIR}/${FUENTE}")
endforeach()
