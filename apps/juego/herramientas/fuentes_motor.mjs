/** Manifiesto compartido por la biblioteca de comparación, SDL y CMake. */
import { readFileSync } from 'node:fs';
import { rutaJuego } from './rutas_proyecto.mjs';

// Resuelve las fuentes propias de la lista explícita, sin incluir archivos de trabajo ni terceros.
export function obtenerFuentesMotor(raiz) {
  return JSON.parse(readFileSync(rutaJuego(raiz, 'fuentes_motor.json'), 'utf8')).map((ruta) =>
    rutaJuego(raiz, ruta),
  );
}
