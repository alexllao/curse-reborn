import sharp from 'sharp';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
import { huella } from '../distribucion/archivos.mjs';

// El mismo PNG que usa el ejecutable; iOS añade su máscara de esquinas al icono opaco.
export async function prepararIconoIOS(origen, destino) {
  const maestro = readFileSync(origen);
  const catalogo = join(destino, 'Assets.xcassets');
  const carpeta = join(catalogo, 'AppIcon.appiconset');
  mkdirSync(carpeta, { recursive: true });
  const images = [];
  const escritos = new Set();
  for (const [idiom, size, scale] of [
    ['ipad', 20, 1],
    ['ipad', 20, 2],
    ['ipad', 29, 1],
    ['ipad', 29, 2],
    ['ipad', 40, 1],
    ['ipad', 40, 2],
    ['ipad', 76, 1],
    ['ipad', 76, 2],
    ['ipad', 83.5, 2],
    ['ios-marketing', 1024, 1],
  ]) {
    const pixeles = size * scale;
    const filename = `icono-${pixeles}.png`;
    if (!escritos.has(pixeles)) {
      await sharp(maestro)
        .resize(pixeles, pixeles)
        .flatten({ background: '#091624' })
        .png()
        .toFile(join(carpeta, filename));
      escritos.add(pixeles);
    }
    images.push({ idiom, size: `${size}x${size}`, scale: `${scale}x`, filename });
  }
  const info = { version: 1, author: 'xcode' };
  writeFileSync(join(catalogo, 'Contents.json'), JSON.stringify({ info }, null, 2) + '\n');
  writeFileSync(join(carpeta, 'Contents.json'), JSON.stringify({ images, info }, null, 2) + '\n');
  return { sha256_maestro: huella(maestro), variantes: images.length };
}
