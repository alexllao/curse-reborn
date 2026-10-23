import {
  copyFileSync,
  mkdirSync,
  readFileSync,
  renameSync,
  writeFileSync,
  unlinkSync,
  existsSync,
} from 'node:fs';
import { dirname, join } from 'node:path';
import { prepararMarcos } from '../gestionar_marcos.mjs';
import { prepararHabitaciones } from '../gestionar_habitaciones.mjs';
import { prepararSonidosPC } from '../gestionar_sonidos_pc.mjs';
import { prepararPresentacion } from '../gestionar_presentacion.mjs';
import { prepararMusica } from '../audio/preparar_musica.mjs';
import { huella, listarArchivos, rutaDentro } from '../distribucion/archivos.mjs';
import { optimizarImagen } from '../distribucion/optimizar_assets.mjs';
import { prepararIconoIOS } from './icono.mjs';

// Comparte los preparadores del escritorio; los recursos comerciales solo proceden de la copia local.
export async function prepararRecursosIOS(raiz, destino, opciones) {
  const datos = opciones.datos;
  if (!existsSync(join(datos, 'CURSE.EXE')) || !existsSync(join(datos, 'DATA')))
    throw new Error('Copia CURSE.EXE y DATA en ' + datos + ' o indica --datos <carpeta CURSE>.');
  const ejecutable = readFileSync(join(datos, 'CURSE.EXE'));
  const mapa = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/documentacion/mapa_ejecutable.json')),
  );
  if (huella(ejecutable) !== mapa.sha256)
    throw new Error('CURSE.EXE no coincide con la versión del port.');
  const archivosDatos = listarArchivos(join(datos, 'DATA')).filter((r) =>
    /\.(DAT|PAL|MAP|SPR|BRD|SFX|RS[1-5])$/i.test(r),
  );
  if (!archivosDatos.length) throw new Error('La carpeta DATA está vacía.');
  mkdirSync(destino, { recursive: true });
  prepararMarcos(raiz, destino);
  const inicial = JSON.parse(readFileSync(join(raiz, 'apps/juego/configuracion/inicial.json')));
  prepararHabitaciones(raiz, destino, inicial.habitacion || 'inicial');
  prepararSonidosPC(raiz, destino);
  prepararPresentacion(raiz, destino);
  const canciones = prepararMusica(raiz, destino, opciones.musica !== 'wav');
  for (const [origen, ruta] of [
    [join(datos, 'CURSE.EXE'), 'datos_originales/CURSE/CURSE.EXE'],
    ...archivosDatos.map((r) => [
      rutaDentro(join(datos, 'DATA'), r),
      'datos_originales/CURSE/DATA/' + r,
    ]),
    [join(raiz, 'apps/juego/terceros/ymfm/LICENSE'), 'LICENCIA_YMFM.txt'],
    [join(raiz, 'apps/juego/terceros/nuked_opl3/LICENSE'), 'LICENCIA_NUKED_OPL3.txt'],
    [
      join(raiz, 'packages/assets/lanzador/icono/curse-reborn.png'),
      'assets/lanzador/icono/curse-reborn.png',
    ],
  ]) {
    const copia = rutaDentro(destino, ruta);
    mkdirSync(dirname(copia), { recursive: true });
    copyFileSync(origen, copia);
  }
  const maestros = JSON.parse(readFileSync(join(raiz, 'packages/assets/huellas.json'))).imagenes;
  const imagenes = [];
  for (const ruta of listarArchivos(join(destino, 'assets')).filter((r) => r.endsWith('.png'))) {
    const archivo = join(destino, 'assets', ruta);
    const original = readFileSync(archivo);
    if (maestros[ruta] !== huella(original)) throw new Error('Maestro gráfico modificado: ' + ruta);
    const fondo =
      ruta.startsWith('lanzador/habitaciones/') ||
      (ruta.startsWith('lanzador/presentacion/') && !ruta.endsWith('-logo.png')) ||
      (ruta.startsWith('marcos/cielo_encantado/') && !ruta.includes('/overlay/'));
    const resultado = await optimizarImagen(original, opciones.imagenes, fondo);
    const copia = archivo.slice(0, -4) + resultado.extension;
    // Archivo nuevo y renombrado solo dentro de esta preparación, nunca sobre el maestro.
    writeFileSync(copia + '.preparando', resultado.datos);
    renameSync(copia + '.preparando', copia);
    if (copia !== archivo) unlinkSync(archivo);
    imagenes.push({
      maestro: 'assets/' + ruta,
      ruta: 'assets/' + ruta.slice(0, -4) + resultado.extension,
      antes: original.length,
      despues: resultado.datos.length,
      metodo: resultado.metodo,
      sha256_maestro: huella(original),
      sha256: huella(resultado.datos),
    });
  }
  for (const imagen of imagenes.filter((i) => i.maestro.startsWith('assets/marcos/'))) {
    const ruta = join(destino, imagen.maestro + '.json');
    if (!existsSync(ruta)) continue;
    const meta = JSON.parse(readFileSync(ruta));
    if (meta.sourceSha256) meta.sourceSha256 = imagen.sha256;
    writeFileSync(ruta, JSON.stringify(meta, null, 2) + '\n');
  }
  const icono = await prepararIconoIOS(
    join(raiz, 'packages/assets/lanzador/icono/curse-reborn.png'),
    destino,
  );
  const informe = { imagenes, canciones, icono, archivos_datos: archivosDatos.length };
  writeFileSync(join(destino, 'preparacion.json'), JSON.stringify(informe, null, 2) + '\n');
  return informe;
}
