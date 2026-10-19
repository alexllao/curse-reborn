/** Rutas de código y recursos del monorepositorio; los datos y resultados locales siguen en raíz. */
import { join } from 'node:path';

// Sitúa las fuentes y dependencias del port sin depender del directorio desde el que se invoque pnpm.
export function rutaJuego(raiz, ...partes) {
  return join(raiz, 'apps', 'juego', ...partes);
}

// Sitúa los recursos propios compartidos; la distribución mantiene su carpeta relativa assets/.
export function rutaAssets(raiz, ...partes) {
  return join(raiz, 'packages', 'assets', ...partes);
}
