import { copyFileSync, mkdirSync } from 'node:fs';
import { join } from 'node:path';
import { rutaAssets } from './rutas_proyecto.mjs';

// Solo recursos redistribuibles de presentación; los efectos DOS usan sus bancos originales.
export function prepararSonidosPC(raiz, destino) {
  const carpeta = join(destino, 'assets', 'lanzador', 'sonidos_pc');
  mkdirSync(carpeta, { recursive: true });
  for (const archivo of ['arranque.wav', 'boton.wav', 'ORIGEN.txt'])
    copyFileSync(rutaAssets(raiz, 'lanzador', 'sonidos_pc', archivo), join(carpeta, archivo));
}
