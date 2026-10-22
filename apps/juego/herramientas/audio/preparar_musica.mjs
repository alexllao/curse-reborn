import { copyFileSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { huella, rutaDentro } from '../distribucion/archivos.mjs';

// Copia maestros en desarrollo o genera MP3 nuevos únicamente dentro del paquete.
export function prepararMusica(raiz, destino, comprimida = false) {
  const config = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/configuracion/inicial.json'), 'utf8'),
  );
  if (
    config.version !== 1 ||
    !['remaster', 'mt32', 'adlib', 'opl3', 'sin-musica'].includes(config.musica?.modo)
  )
    throw new Error('Configuración inicial o modo de música inválidos.');
  const rutaCatalogo = config.musica.catalogo;
  const origen = rutaDentro(join(raiz, 'packages'), rutaCatalogo);
  const catalogo = JSON.parse(readFileSync(origen, 'utf8'));
  const carpeta = dirname(rutaDentro(destino, rutaCatalogo));
  mkdirSync(carpeta, { recursive: true });
  const pistas = [];
  for (let i = 1; i <= 5; ++i) {
    const pista = catalogo[`rs${i}`];
    if (
      !pista ||
      pista.archivo !== `rs${i}.wav` ||
      !pista.titulo ||
      !/^[0-9a-f]{64}$/.test(pista.sha256)
    )
      throw new Error(`Falta la pista RS${i} o su título.`);
    const maestro = rutaDentro(dirname(origen), pista.archivo);
    const datos = readFileSync(maestro);
    if (huella(datos) !== pista.sha256)
      throw new Error(`El maestro RS${i} cambió: revisa su SHA-256.`);
    const nombre = comprimida ? `rs${i}.mp3` : pista.archivo;
    const copia = join(carpeta, nombre);
    if (comprimida) {
      const resultado = spawnSync(
        process.env.FFMPEG || 'ffmpeg',
        [
          '-hide_banner',
          '-loglevel',
          'error',
          '-nostdin',
          '-y',
          '-i',
          maestro,
          '-map',
          '0:a:0',
          '-vn',
          '-c:a',
          'libmp3lame',
          '-q:a',
          '0',
          '-ar',
          '48000',
          '-metadata',
          `title=${pista.titulo}`,
          copia,
        ],
        { windowsHide: true, encoding: 'utf8' },
      );
      if (resultado.error || resultado.status !== 0)
        throw new Error(
          `No se pudo convertir RS${i} a MP3: ${resultado.error?.message || resultado.stderr}`,
        );
    } else copyFileSync(maestro, copia);
    const sha256 = huella(readFileSync(copia));
    pistas.push({
      banco: i,
      titulo: pista.titulo,
      maestro: pista.archivo,
      sha256_maestro: pista.sha256,
      archivo: nombre,
      sha256,
      bytes_maestro: datos.length,
      bytes: readFileSync(copia).length,
    });
    pista.archivo = nombre;
    pista.sha256_maestro = pista.sha256;
    pista.sha256 = sha256;
  }
  writeFileSync(join(carpeta, 'titulos.json'), JSON.stringify(catalogo, null, 2) + '\n');
  writeFileSync(join(destino, 'configuracion.json'), JSON.stringify(config, null, 2) + '\n');
  for (const [carpetaLicencia, archivo, salida] of [
    ['dr_mp3', 'dr_mp3.h', 'LICENCIA_DR_MP3.txt'],
    ['json11', 'json11.hpp', 'LICENCIA_JSON11.txt'],
  ])
    copyFileSync(
      join(raiz, 'apps/juego/terceros', carpetaLicencia, archivo),
      join(destino, salida),
    );
  return pistas;
}

if (
  typeof process !== 'undefined' &&
  process.argv[1] &&
  resolve(process.argv[1]) === fileURLToPath(import.meta.url)
) {
  const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../../..');
  prepararMusica(raiz, resolve(process.argv[2]), process.argv.includes('--mp3'));
}
