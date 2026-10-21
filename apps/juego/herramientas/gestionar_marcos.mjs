import { rutaAssets } from './rutas_proyecto.mjs';
/** Valida los marcos propios y prepara el catálogo C++ y sus imágenes junto a la aplicación. */
import { prepararHabitaciones } from './gestionar_habitaciones.mjs';
import { prepararSonidosPC } from './gestionar_sonidos_pc.mjs';
import { prepararPresentacion } from './gestionar_presentacion.mjs';
import { createHash } from 'node:crypto';
import { copyFileSync, mkdirSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

// Rechaza recortes fuera del PNG y metadatos que el dibujante no puede representar.
function validarPanel(panel, ancho, alto, margen) {
  if (
    !panel ||
    !['x', 'y', 'width', 'height'].every((campo) => Number.isInteger(panel[campo])) ||
    panel.x < 0 ||
    panel.y < 0 ||
    panel.width < 1 ||
    panel.height < 1 ||
    panel.x + panel.width > ancho ||
    panel.y + panel.height > alto ||
    !Number.isInteger(margen) ||
    margen < 0 ||
    margen >= panel.width
  )
    throw new Error('Recorte o margen interior de marco inválido.');
  return `{${panel.x},${panel.y},${panel.width},${panel.height},${margen}}`;
}

// Conserva el PNG aportado y convierte únicamente sus parámetros de presentación a constantes.
export function prepararMarcos(raiz, destino) {
  const origen = rutaAssets(raiz, 'marcos');
  const carpeta = join(destino, 'assets', 'marcos');
  mkdirSync(carpeta, { recursive: true });
  const origenDecorados = rutaAssets(raiz, 'original', 'decorados');
  const destinoDecorados = join(destino, 'assets', 'original', 'decorados');
  mkdirSync(destinoDecorados, { recursive: true });
  const decorados = JSON.parse(
    readFileSync(join(origenDecorados, 'catalogo.json'), 'utf8'),
  ).decorados;
  const definiciones = [];
  const escenasDecoradas = new Set();
  for (const decorado of decorados) {
    const { escena, archivo, resolucion, reservaOriginal } = decorado;
    if (
      !Number.isInteger(escena) ||
      escena < 0 ||
      escena >= 78 ||
      escenasDecoradas.has(escena) ||
      !/^[a-z0-9_-]+\.png$/.test(archivo)
    )
      throw new Error('Asignación de decorado inválida.');
    escenasDecoradas.add(escena);
    const png = readFileSync(join(origenDecorados, archivo));
    if (
      png.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a' ||
      png.readUInt32BE(16) !== resolucion[0] ||
      png.readUInt32BE(20) !== resolucion[1]
    )
      throw new Error(`Resolución inesperada en ${archivo}.`);
    for (const [rect, ancho, alto] of [[reservaOriginal, 320, 200]])
      if (
        rect.length !== 4 ||
        !rect.every(Number.isInteger) ||
        rect[0] < 0 ||
        rect[1] < 0 ||
        rect[2] <= 0 ||
        rect[3] <= 0 ||
        rect[0] + rect[2] > ancho ||
        rect[1] + rect[3] > alto
      )
        throw new Error(`Reserva o vano inválido en ${archivo}.`);
    const solape = decorado.solape ?? 0;
    if (!Number.isInteger(solape) || solape < 0 || solape > 4)
      throw new Error(`Solape de decorado inválido en ${archivo}.`);
    definiciones.push(
      `{${escena},${JSON.stringify(`assets/original/decorados/${archivo}`)},{${reservaOriginal.join(',')}},${resolucion.join(',')},${decorado.muestreoArea === true},${solape}}`,
    );
    copyFileSync(join(origenDecorados, archivo), join(destinoDecorados, archivo));
  }
  writeFileSync(
    join(destino, 'catalogo_decorados.hpp'),
    `// Generado desde packages/assets/original/decorados; no editar.\n#pragma once\n#include <array>\ninline constexpr std::array<DefinicionDecorado, ${definiciones.length}> catalogo_decorados = {{${definiciones.join(',\n')}}};\n`,
  );
  copyFileSync(join(origenDecorados, 'catalogo.json'), join(destinoDecorados, 'catalogo.json'));
  const catalogo = JSON.parse(readFileSync(join(origen, 'catalogo.json'), 'utf8'));
  const identificadores = [];
  const marcos = [];
  for (const archivo of readdirSync(origen)
    .filter((nombre) => nombre.endsWith('.png.json'))
    .sort()) {
    const id = archivo.slice(0, -9);
    if (!/^[a-z0-9_-]+$/.test(id)) throw new Error(`Identificador de marco inválido: ${id}.`);
    const parametros = JSON.parse(readFileSync(join(origen, archivo), 'utf8'));
    const png = archivo.slice(0, -5);
    const imagen = readFileSync(join(origen, png));
    if (
      imagen.length < 33 ||
      imagen.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a' ||
      imagen.toString('ascii', 12, 16) !== 'IHDR'
    )
      throw new Error(`PNG de marco inválido: ${png}.`);
    if (
      parametros.sourceSha256 &&
      createHash('sha256').update(imagen).digest('hex') !== parametros.sourceSha256
    )
      throw new Error(`La huella del marco no coincide: ${png}.`);
    const ancho = imagen.readUInt32BE(16),
      alto = imagen.readUInt32BE(20);
    const izquierda = validarPanel(
      parametros.left,
      ancho,
      alto,
      parametros.innerPadding?.left ?? 0,
    );
    const derecha = validarPanel(
      parametros.right,
      ancho,
      alto,
      parametros.innerPadding?.right ?? 0,
    );
    if (!['cover', 'height'].includes(parametros.fit ?? 'height'))
      throw new Error(`Ajuste de marco desconocido: ${png}.`);
    const solape = parametros.innerOverlap ?? 2;
    if (typeof solape !== 'number' || !Number.isFinite(solape) || solape < 0 || solape > 4)
      throw new Error(`Solape de marco inválido: ${png}.`);
    marcos.push(
      `{${JSON.stringify(id)},${JSON.stringify(parametros.label ?? id)},${JSON.stringify(`assets/marcos/${png}`)},${izquierda},${derecha},${solape.toFixed(4)}f,${parametros.fit === 'cover'},${parametros.mirrorRight === true}}`,
    );
    identificadores.push(id);
    copyFileSync(join(origen, png), join(carpeta, png));
    copyFileSync(join(origen, archivo), join(carpeta, archivo));
  }
  // Los panoramas remaster son variantes completas; el dibujante conserva solo las bandas.
  const cielo = rutaAssets(raiz, 'marcos', 'cielo_encantado');
  const destinoCielo = join(destino, 'assets/marcos/cielo_encantado');
  mkdirSync(destinoCielo, { recursive: true });
  const panoramas = JSON.parse(readFileSync(join(cielo, 'catalogo.json'), 'utf8'));
  const overlay = join(cielo, 'overlay/moldura.png');
  const moldura = JSON.parse(readFileSync(overlay + '.json', 'utf8'));
  const imagenMoldura = readFileSync(overlay);
  if (
    imagenMoldura.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a' ||
    imagenMoldura.readUInt32BE(16) !== moldura.resolucion[0] ||
    imagenMoldura.readUInt32BE(20) !== moldura.resolucion[1] ||
    createHash('sha256').update(imagenMoldura).digest('hex') !== moldura.sourceSha256
  )
    throw new Error('Moldura transparente inválida.');
  if (
    !Array.isArray(moldura.vano) ||
    moldura.vano.length !== 4 ||
    !moldura.vano.every((n) => Number.isFinite(n) && n > 0) ||
    moldura.vano[0] + moldura.vano[2] > moldura.resolucion[0] ||
    moldura.vano[1] + moldura.vano[3] > moldura.resolucion[1] ||
    !Array.isArray(moldura.esquinas) ||
    moldura.esquinas.length !== 4 ||
    !moldura.esquinas.every((n) => Number.isFinite(n) && n > 0) ||
    moldura.esquinas[0] + moldura.esquinas[1] >= moldura.resolucion[0] ||
    moldura.esquinas[2] + moldura.esquinas[3] >= moldura.resolucion[1] ||
    !Number.isFinite(moldura.escalaPorPixel) ||
    moldura.escalaPorPixel <= 0 ||
    moldura.escalaPorPixel > 1 ||
    !Number.isInteger(moldura.solapeMaximo) ||
    moldura.solapeMaximo < 0 ||
    moldura.solapeMaximo > 2
  )
    throw new Error('Geometría de moldura inválida.');
  mkdirSync(join(destinoCielo, 'overlay'), { recursive: true });
  copyFileSync(overlay, join(destinoCielo, 'overlay/moldura.png'));
  copyFileSync(overlay + '.json', join(destinoCielo, 'overlay/moldura.png.json'));
  const luminosidad = panoramas.luminosidad ?? 1;
  if (!Number.isFinite(luminosidad) || luminosidad < 0 || luminosidad > 1)
    throw new Error('Luminosidad panorámica inválida.');
  for (const variante of panoramas.variantes) {
    if (
      !/^[a-z0-9_-]+\.png$/.test(variante.archivo) ||
      !Array.isArray(variante.proporcion) ||
      variante.proporcion.length !== 2 ||
      !variante.proporcion.every((n) => Number.isFinite(n) && n > 0)
    )
      throw new Error('Variante panorámica inválida.');
    const png = readFileSync(join(cielo, variante.archivo));
    const parametros = JSON.parse(readFileSync(join(cielo, variante.archivo + '.json'), 'utf8'));
    if (
      png.subarray(0, 8).toString('hex') !== '89504e470d0a1a0a' ||
      png.readUInt32BE(16) !== parametros.resolucion[0] ||
      png.readUInt32BE(20) !== parametros.resolucion[1] ||
      createHash('sha256').update(png).digest('hex') !== parametros.sourceSha256
    )
      throw new Error('Resolución o huella panorámica incorrecta.');
    const ruta = `assets/marcos/cielo_encantado/${variante.archivo}`;
    marcos.push(
      `{"cielo_encantado","Cielo encantado",${JSON.stringify(ruta)},{0,0,0,0,0},{0,0,0,0,0},0.0f,true,false,true,${(variante.proporcion[0] / variante.proporcion[1]).toFixed(6)}f,${Math.round(luminosidad * 255)}}`,
    );
    copyFileSync(join(cielo, variante.archivo), join(destinoCielo, variante.archivo));
    copyFileSync(
      join(cielo, variante.archivo + '.json'),
      join(destinoCielo, variante.archivo + '.json'),
    );
  }
  copyFileSync(join(cielo, 'catalogo.json'), join(destinoCielo, 'catalogo.json'));
  identificadores.push(panoramas.id);
  if (!identificadores.includes(catalogo.predeterminado))
    throw new Error('Falta el marco predeterminado del catálogo.');
  const asignaciones = [];
  for (const [pantalla, id] of Object.entries(catalogo.pantallas ?? {})) {
    if (
      !/^(core|titulo|introduccion|escena_[0-9]+)$/.test(pantalla) ||
      !identificadores.includes(id)
    )
      throw new Error(`Asignación de marco inválida: ${pantalla}.`);
    asignaciones.push(`{${JSON.stringify(pantalla)},${JSON.stringify(id)}}`);
  }
  // La entrada vacía evita matrices de tamaño cero en C++ cuando todavía no hay asignaciones.
  asignaciones.push('{"",""}');
  const cabecera = `// Generado desde packages/assets/marcos; no editar.\n#pragma once\ninline constexpr DefinicionMarco catalogo_marcos[] = {${marcos.join(',\n')}};\ninline constexpr AsignacionMarco asignaciones_marcos[] = {${asignaciones.join(',\n')}};\ninline constexpr const char *marco_predeterminado = ${JSON.stringify(catalogo.predeterminado)};\n`;
  const cabeceraMoldura = `inline constexpr const char *archivo_moldura = "assets/marcos/cielo_encantado/overlay/moldura.png";\ninline constexpr SDL_FRect vano_moldura = {${moldura.vano.join(',')}};\ninline constexpr float esquinas_moldura[] = {${moldura.esquinas.join(',')}};\ninline constexpr float escala_moldura = ${moldura.escalaPorPixel}f;\ninline constexpr float solape_moldura = ${moldura.solapeMaximo}.0f;\n`;
  writeFileSync(join(destino, 'catalogo_marcos.hpp'), cabecera + cabeceraMoldura);
  copyFileSync(join(origen, 'catalogo.json'), join(carpeta, 'catalogo.json'));
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
  if (!process.argv[2])
    throw new Error('Indica la carpeta de compilación para preparar los marcos.');
  prepararMarcos(raiz, resolve(process.argv[2]));
  prepararHabitaciones(raiz, resolve(process.argv[2]));
  prepararSonidosPC(raiz, resolve(process.argv[2]));
  prepararPresentacion(raiz, resolve(process.argv[2]));
}
