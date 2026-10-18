/** Valida los marcos propios y prepara el catálogo C++ y sus imágenes junto a la aplicación. */
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
  const origen = join(raiz, 'assets', 'original', 'marcos');
  const carpeta = join(destino, 'assets', 'original', 'marcos');
  mkdirSync(carpeta, { recursive: true });
  const origenDecorados = join(raiz, 'assets', 'original', 'decorados');
  const destinoDecorados = join(destino, 'assets', 'original', 'decorados');
  mkdirSync(destinoDecorados, { recursive: true });
  const decorados = JSON.parse(
    readFileSync(join(origenDecorados, 'catalogo.json'), 'utf8'),
  ).decorados;
  const definiciones = [];
  const escenasDecoradas = new Set();
  for (const decorado of decorados) {
    const { escena, archivo, resolucion, reservaOriginal, vanoImagen } = decorado;
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
    for (const [rect, ancho, alto] of [
      [reservaOriginal, 320, 200],
      [vanoImagen, ...resolucion],
    ])
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
      `{${escena},${JSON.stringify(`assets/original/decorados/${archivo}`)},{${reservaOriginal.join(',')}},{${vanoImagen.join(',')}},${resolucion.join(',')},${decorado.ajustarSilueta === true},${decorado.muestreoArea === true},${solape}}`,
    );
    copyFileSync(join(origenDecorados, archivo), join(destinoDecorados, archivo));
  }
  writeFileSync(
    join(destino, 'catalogo_decorados.hpp'),
    `// Generado desde assets/original/decorados; no editar.\n#pragma once\ninline constexpr DefinicionDecorado catalogo_decorados[] = {${definiciones.join(',\n')}};\n`,
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
      `{${JSON.stringify(id)},${JSON.stringify(parametros.label ?? id)},${JSON.stringify(`assets/original/marcos/${png}`)},${izquierda},${derecha},${solape.toFixed(4)}f,${parametros.fit === 'cover'},${parametros.mirrorRight === true}}`,
    );
    identificadores.push(id);
    copyFileSync(join(origen, png), join(carpeta, png));
    copyFileSync(join(origen, archivo), join(carpeta, archivo));
  }
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
  const cabecera = `// Generado desde assets/original/marcos; no editar.\n#pragma once\ninline constexpr DefinicionMarco catalogo_marcos[] = {${marcos.join(',\n')}};\ninline constexpr AsignacionMarco asignaciones_marcos[] = {${asignaciones.join(',\n')}};\ninline constexpr const char *marco_predeterminado = ${JSON.stringify(catalogo.predeterminado)};\n`;
  writeFileSync(join(destino, 'catalogo_marcos.hpp'), cabecera);
  copyFileSync(join(origen, 'catalogo.json'), join(carpeta, 'catalogo.json'));
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '..');
  if (!process.argv[2])
    throw new Error('Indica la carpeta de compilación para preparar los marcos.');
  prepararMarcos(raiz, resolve(process.argv[2]));
}
