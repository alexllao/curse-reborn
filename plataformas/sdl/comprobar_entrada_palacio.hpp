#pragma once
#include "mazmorra.hpp"

// 82/CF34/B68D–B73D: verifica error, cuatro pulsaciones reales y apertura de la puerta.
template <typename Avanzar, typename Abrir, typename Elegir, typename Objeto, typename Ir>
bool comprobar_entrada_palacio_sdl(Mazmorra &p, const std::string &opcion, Avanzar avanzar,
                                   Abrir abrir, Elegir elegir, Objeto objeto, Ir ir) {
  const auto detener = [&](const char *hito) {
    return opcion == std::string("--comprobar-final-poblacion-borde_valle_") + hito;
  };
  for (unsigned int i = 0; i < 1536 && p.escena_actual() == 44; ++i)
    if (!avanzar(1))
      return false;
  if (p.escena_actual() != 45)
    return false;
  if (detener("palacio_entrada"))
    return true;
  if (!avanzar(256))
    return false;
  if (detener("palacio_control"))
    return true;
  if (!ir(240, 145))
    return false;
  if (detener("palacio_ante_botones"))
    return true;
  if (detener("palacio_error_139"))
    return ir(234, 145) && abrir() && elegir(2) && elegir(2) && objeto(139) && avanzar(192);
  // Posiciones de los cuatro descriptores originales más los siete píxeles del manejador.
  static constexpr unsigned short posiciones[] = {269, 224, 246, 234};
  static constexpr const char *hitos[] = {"palacio_boton_136", "palacio_boton_137",
                                          "palacio_boton_138", "palacio_boton_139"};
  for (unsigned int i = 0; i < 4; ++i) {
    if (!ir(posiciones[i], 145) || !abrir() || !elegir(2) || !elegir(2) ||
        !objeto(static_cast<unsigned short>(136 + i)) || !avanzar(192))
      return false;
    if (detener(hitos[i]))
      return true;
  }
  return false;
}
