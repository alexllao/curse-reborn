#include "secuencias.hpp"
#include "../estado/segmento.hpp"

namespace motor {
namespace {

void desactivar(unsigned char *datos, Palabra posicion) {
  datos[posicion] = 0;
  constexpr Palabra campos[] = {0x10, 0x15, 0x1a};
  for (Palabra campo : campos) {
    guardar(datos, static_cast<Palabra>(posicion + campo), 0);
  }
}

} // namespace

EstadoSecuencias avanzar_secuencias_entidad(unsigned char *datos, EstadoAleatorio &estado,
                                            const RecursosSecuencias &recursos,
                                            unsigned short posicion) {
  const auto direccion = [posicion](int campo) { return static_cast<Palabra>(posicion + campo); };
  if (datos[posicion] == 0)
    return EstadoSecuencias::correcto;
  for (int canal = 0; canal < 3; ++canal) {
    const auto base = direccion(0x10 + canal * 5);
    auto referencia = palabra(datos, base);
    if (referencia == 0)
      continue;
    auto &indice = datos[static_cast<Palabra>(base + 2)];
    auto &contador = datos[static_cast<Palabra>(base + 3)];
    auto &intervalo = datos[static_cast<Palabra>(base + 4)];
    contador = static_cast<unsigned char>(contador - 1);
    // OR BX,BX despeja CF antes de DEC: JBE aquí solo avanza si el contador llega a cero.
    if (contador != 0)
      continue;
    contador = intervalo;
    if (canal != 0)
      guardar(datos, 0x9972, 0);
    const auto tomar = [&]() {
      const auto valor = datos[static_cast<Palabra>(referencia + indice)];
      indice = static_cast<unsigned char>(indice + 1);
      return valor;
    };
    bool terminado = false;
    // Límite de protección de la API: el original puede quedar en un ciclo de órdenes.
    for (unsigned int orden = 0; orden < 4096 && !terminado; ++orden) {
      auto valor = tomar();
      if (canal != 0 && valor == 0x80) {
        guardar(datos, 0x9972, static_cast<Palabra>(palabra(datos, 0x9972) + 100));
        continue;
      }
      if (valor == 0x7f) {
        indice = 0;
        continue;
      }
      if (valor == 0x7e) {
        referencia = palabra(datos, static_cast<Palabra>(referencia + indice));
        guardar(datos, base, referencia);
        indice = 0;
        continue;
      }
      if (valor == 0x7d) {
        desactivar(datos, posicion);
        return EstadoSecuencias::correcto;
      }
      if (valor == 0x7c) {
        guardar(datos, base, 0);
        terminado = true;
        continue;
      }
      if (valor == 0x7b) {
        contador = static_cast<unsigned char>(tomar() * intervalo);
        terminado = true;
        continue;
      }
      if (valor == 0x7a) {
        intervalo = tomar();
        contador = intervalo;
        continue;
      }
      if (valor == 0x79) {
        indice = static_cast<unsigned char>(indice - 2);
        continue;
      }
      if (valor == 0x78) {
        Palabra azar = 0;
        if (!generar_aleatorio(estado, tomar(), azar))
          return EstadoSecuencias::entrada_invalida;
        const auto espera = static_cast<unsigned char>(azar + tomar());
        for (int restante = canal; restante < 3; ++restante)
          datos[direccion(0x13 + restante * 5)] = espera;
        return EstadoSecuencias::correcto;
      }
      if (canal == 0) {
        bool marcador_transicion = false;
        if (valor == 0xff || valor == 0xfe) {
          datos[posicion] = valor == 0xff ? static_cast<unsigned char>(datos[posicion] & 0xfd)
                                          : static_cast<unsigned char>(datos[posicion] | 2);
          terminado = true;
          continue;
        }
        if (valor == 0xfd) {
          datos[0x3594] = datos[0x3595] = datos[0x3596] = 1;
          continue;
        }
        if (valor == 0xfc) {
          datos[0x3595] = 1;
          continue;
        }
        if (valor == 0xfb) {
          const auto identificador = datos[static_cast<Palabra>(referencia + indice)];
          if (!recursos.sonido ||
              !recursos.sonido(recursos.contexto, datos, posicion, identificador)) {
            return EstadoSecuencias::sonido_pendiente;
          }
          indice = static_cast<unsigned char>(indice + 1);
          continue;
        }
        if (valor == 0xfa || valor == 0xf9) {
          datos[0x0def] = valor == 0xfa ? 1 : 0;
          continue;
        }
        if (valor == 0xf8) {
          const auto modelo = tomar();
          datos[0x358c] = modelo;
          contador = intervalo = modelo == 0xff ? 1 : 2;
          datos[direccion(0x19)] = datos[direccion(0x1e)] = intervalo;
          Palabra recurso = recursos.personaje;
          if (modelo == 0xfe)
            recurso = recursos.escena;
          else if (modelo != 0xff) {
            if (datos[0x9974] != modelo) {
              datos[0x9974] = modelo;
              const auto nombre = palabra(datos, static_cast<Palabra>(0x0918 + modelo * 2));
              if (!recursos.cargar_modelo ||
                  !recursos.cargar_modelo(recursos.contexto, datos, nombre, 64000)) {
                return EstadoSecuencias::recurso_pendiente;
              }
            }
            recurso = recursos.modelo;
          }
          guardar(datos, direccion(0x1f), recurso);
          continue;
        }
        if (valor == 0xf7) {
          bool elegido = false;
          for (unsigned int intento = 0; intento < 4096; ++intento) {
            Palabra azar = 0;
            if (!generar_aleatorio(estado, tomar(), azar))
              return EstadoSecuencias::entrada_invalida;
            valor =
                static_cast<unsigned char>(azar + datos[static_cast<Palabra>(referencia + indice)]);
            indice = static_cast<unsigned char>(indice - 1);
            if (valor != datos[direccion(1)]) {
              indice = static_cast<unsigned char>(indice + 2);
              elegido = true;
              break;
            }
          }
          if (!elegido)
            return EstadoSecuencias::entrada_invalida;
        } else if (valor == 0xf6) {
          valor = tomar();
          datos[0x9975] = valor;
          marcador_transicion = true;
        }
        datos[direccion(1)] = valor;
        Palabra ancho = 0, alto = 0;
        if (!recursos.dimensiones ||
            !recursos.dimensiones(recursos.contexto, palabra(datos, direccion(0x1f)), valor & 0x7f,
                                  ancho, alto)) {
          // F6 reutiliza la salida como imagen transitoria; 0A5C carga la escena sin dibujarla.
          // El EXE puede leer fuera del banco; la API acotada normaliza esa cabecera a cero.
          if (!marcador_transicion)
            return EstadoSecuencias::entrada_invalida;
          ancho = alto = 0;
        }
        guardar(datos, direccion(6), ancho);
        guardar(datos, direccion(8), alto);
      } else {
        const int delta = valor < 128 ? valor : static_cast<int>(valor) - 256;
        const auto adicional = palabra(datos, 0x9972);
        const auto cambio =
            static_cast<Palabra>(delta + (delta < 0 ? -static_cast<int>(adicional) : adicional));
        const auto campo = direccion(canal == 1 ? 2 : 4);
        const auto coordenada = static_cast<Palabra>(palabra(datos, campo) + cambio);
        guardar(datos, campo, coordenada);
        if (canal == 2 && con_signo(coordenada) >= 300 &&
            con_signo(static_cast<Palabra>(coordenada + palabra(datos, direccion(8)))) <= -100) {
          datos[posicion] = 0;
        }
      }
      terminado = true;
    }
    if (!terminado)
      return EstadoSecuencias::entrada_invalida;
  }
  return EstadoSecuencias::correcto;
}

void retirar_entidad_fuera_de_limites(unsigned char *datos, unsigned short posicion) {
  if (datos[posicion] == 0)
    return;
  const auto x = palabra(datos, static_cast<Palabra>(posicion + 2));
  const auto y = palabra(datos, static_cast<Palabra>(posicion + 4));
  if (con_signo(x) >= 384 ||
      con_signo(static_cast<Palabra>(x + palabra(datos, static_cast<Palabra>(posicion + 6)))) <
          -64 ||
      con_signo(y) >= 250 ||
      con_signo(static_cast<Palabra>(y + palabra(datos, static_cast<Palabra>(posicion + 8)))) <
          -50) {
    desactivar(datos, posicion);
  }
}

EstadoSecuencias avanzar_secuencias(unsigned char *datos, EstadoAleatorio &estado,
                                    const RecursosSecuencias &recursos) {
  const auto reservadas = palabra(datos, 0x3588);
  if (reservadas >= 70)
    return EstadoSecuencias::entrada_invalida;
  for (int i = 0; i < 70; ++i) {
    const auto resultado =
        avanzar_secuencias_entidad(datos, estado, recursos, static_cast<Palabra>(0x2c0a + i * 33));
    if (resultado != EstadoSecuencias::correcto)
      return resultado;
  }
  for (unsigned int i = reservadas; i < 70; ++i) {
    retirar_entidad_fuera_de_limites(datos, static_cast<Palabra>(0x2c0a + i * 33));
  }
  return EstadoSecuencias::correcto;
}

} // namespace motor
