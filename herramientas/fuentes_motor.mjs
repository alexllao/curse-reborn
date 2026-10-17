/** Manifiesto compartido por la biblioteca de comparación, SDL y CMake. */
import { readFileSync } from 'node:fs';
import { join } from 'node:path';

// Resuelve las fuentes propias de la lista explícita, sin incluir archivos de trabajo ni terceros.
export function obtenerFuentesMotor(raiz) {
  return JSON.parse(readFileSync(join(raiz, 'herramientas', 'fuentes_motor.json'), 'utf8')).map(
    (ruta) => join(raiz, ruta),
  );
}
