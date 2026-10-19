/** Rutas del monorepositorio: fuentes y trabajo del port en juego, recursos compartidos en assets. */
import { join } from 'node:path';
import { existsSync, lstatSync, renameSync, readdirSync, rmdirSync, mkdirSync } from 'node:fs';

// Al actualizar una copia anterior, conserva las dependencias y resultados ignorados por Git.
export function migrarTrabajoLocal(raiz) {
  const trasladar = (anterior, destino) => {
    if (!existsSync(anterior) || existsSync(destino)) return false;
    if (!lstatSync(anterior).isDirectory() || lstatSync(anterior).isSymbolicLink())
      throw new Error('La carpeta local que se traslada debe ser un directorio, no un enlace.');
    renameSync(anterior, destino);
    return true;
  };
  const anterior = join(raiz, 'trabajo');
  const destino = rutaJuego(raiz, 'trabajo');
  if (trasladar(anterior, destino))
    console.log('Trabajo local trasladado a apps/juego/trabajo; se conservan sus archivos.');
  const datosAnteriores = join(raiz, 'datos_originales');
  if (existsSync(datosAnteriores)) {
    if (!lstatSync(datosAnteriores).isDirectory() || lstatSync(datosAnteriores).isSymbolicLink())
      throw new Error('datos_originales debe ser un directorio local, no un enlace.');
    mkdirSync(rutaJuego(raiz, 'datos_originales'), { recursive: true });
  }
  for (const carpeta of ['CURSE', 'ROMS']) {
    if (trasladar(join(datosAnteriores, carpeta), rutaJuego(raiz, 'datos_originales', carpeta)))
      console.log(`Datos locales trasladados a apps/juego/datos_originales/${carpeta}.`);
  }
  // Solo elimina el directorio anterior si ya está vacío; no mezcla ni borra otros archivos.
  if (
    existsSync(datosAnteriores) &&
    !lstatSync(datosAnteriores).isSymbolicLink() &&
    lstatSync(datosAnteriores).isDirectory() &&
    readdirSync(datosAnteriores).length === 0
  )
    rmdirSync(datosAnteriores);
}

// Sitúa las fuentes y dependencias del port sin depender del directorio desde el que se invoque pnpm.
export function rutaJuego(raiz, ...partes) {
  return join(raiz, 'apps', 'juego', ...partes);
}

// Sitúa los recursos propios compartidos; la distribución mantiene su carpeta relativa assets/.
export function rutaAssets(raiz, ...partes) {
  return join(raiz, 'packages', 'assets', ...partes);
}
