#pragma once

// Adaptador del YM3812 independiente del núcleo OPL3, sin exponer dependencias al motor
// reconstruido.
class SintesisOpl2 {
public:
  // Mantiene el estado externo oculto hasta que se inicializa la reproducción.
  SintesisOpl2() = default;
  // Libera el núcleo YM3812 al cerrar el propietario del audio.
  ~SintesisOpl2();
  // El estado del chip tiene un único propietario y no se copia entre sesiones.
  SintesisOpl2(const SintesisOpl2 &) = delete;
  // Evita compartir por accidente el estado y los contadores de dos salidas.
  SintesisOpl2 &operator=(const SintesisOpl2 &) = delete;
  // Reinicia chip y conversión de frecuencia antes de recibir los registros originales.
  bool reiniciar();
  // Entrega los registros de ocho bits al YM3812, sin modificar sus instrumentos.
  void escribir(unsigned short registro, unsigned char valor);
  // Convierte una muestra al reloj de salida de 48 kHz conservando la fase entre bloques.
  short generar();

private:
  struct Estado;
  Estado *estado = nullptr;
};
