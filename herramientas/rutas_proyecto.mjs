/** Rutas del monorepositorio: fuentes y trabajo del port en juego, recursos compartidos en assets. */
import { join } from 'node:path';
import { existsSync, lstatSync, renameSync } from 'node:fs';

// Al actualizar una copia anterior, conserva las dependencias y resultados ignorados por Git.
export function migrarTrabajoLocal(raiz) {
  const anterior = join(raiz, 'trabajo');
  const destino = rutaJuego(raiz, 'trabajo');
  if (!existsSync(anterior) || existsSync(destino)) return;
  if (!lstatSync(anterior).isDirectory() || lstatSync(anterior).isSymbolicLink())
    throw new Error('La carpeta trabajo anterior debe ser un directorio local.');
  renameSync(anterior, destino);
  console.log('Trabajo local trasladado a apps/juego/trabajo; se conservan sus archivos.');
}

// Sitúa las fuentes y dependencias del port sin depender del directorio desde el que se invoque pnpm.
export function rutaJuego(raiz, ...partes) {
  return join(raiz, 'apps', 'juego', ...partes);
}

// Sitúa los recursos propios compartidos; la distribución mantiene su carpeta relativa assets/.
export function rutaAssets(raiz, ...partes) {
  return join(raiz, 'packages', 'assets', ...partes);
}
