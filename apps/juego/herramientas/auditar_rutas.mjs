import { rutaJuego } from './rutas_proyecto.mjs';
/** Cruza todas las salidas BOXDET con los enlaces habilitados sin confundirlos con puzles completos. */
import { readFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
const exe = readFileSync(join(raiz, 'apps/juego/datos_originales/CURSE/CURSE.EXE'));
if (
  createHash('sha256').update(exe).digest('hex') !==
  'f2c0046e07b25f408516edae5d6a66c06107971fb95c55ed770fa52a6b4ecd8d'
)
  throw new Error('El EXE no corresponde a la versión auditada.');
const d = exe.subarray(0xf510);
const geometria = readFileSync(join(raiz, 'apps/juego/datos_originales/CURSE/DATA/BOXDET.DAT'));
const fuente = readFileSync(rutaJuego(raiz, 'plataformas/sdl/vistas.hpp'), 'utf8');
const enlaces = [
  ...fuente
    .split('salidas_integradas[] = {')[1]
    .split('};')[0]
    .matchAll(/\{(\d+),\s*(\d+)\}/g),
].map((m) => [Number(m[1]), Number(m[2])]);
const vistas = new Set(
  [
    ...fuente
      .split('vistas_desarrollo[] = {')[1]
      .split('};')[0]
      .matchAll(/\{(\d+),/g),
  ].map((m) => Number(m[1])),
);
const escenas = [];
for (let escena = 0; escena < 78; ++escena) {
  const p = geometria.readUInt16LE(escena * 2);
  const cantidad = p ? geometria.readUInt16LE(p) : 0;
  const salidas = [];
  for (let i = 0; i < cantidad; ++i) {
    const r = p + 2 + i * 10;
    if (geometria[r] !== 2) continue;
    const salida = geometria[r + 1];
    if (salidas.some((e) => e.salida === salida)) continue;
    const destino = d.readUInt16LE(0x7482 + salida * 2);
    salidas.push({
      salida,
      destino,
      enlace_habilitado: enlaces.some(([s, id]) => s === escena && id === salida),
      destino_en_catalogo: vistas.has(destino),
    });
  }
  escenas.push({ escena, en_catalogo: vistas.has(escena), salidas });
}
const pendientes = escenas
  .filter((e) => e.en_catalogo)
  .flatMap((e) =>
    e.salidas.filter((s) => !s.enlace_habilitado).map((s) => ({ origen: e.escena, ...s })),
  );
const destino = join(raiz, 'apps', 'juego', 'trabajo', 'auditoria_rutas');
mkdirSync(destino, { recursive: true });
writeFileSync(
  join(destino, 'resultado.json'),
  JSON.stringify(
    {
      escenas,
      pendientes_desde_vistas: pendientes,
      limite:
        'Inventario estático de umbrales BOXDET. No acredita condiciones, destinos indirectos, todas las interacciones ni escenas completas.',
    },
    null,
    2,
  ) + '\n',
);
for (const e of pendientes)
  console.log(`Pendiente: escena ${e.origen}, salida ${e.salida} → escena ${e.destino}.`);
console.log(
  `Auditadas las 78 tablas de geometría: ${pendientes.length} salidas pendientes desde las vistas disponibles. Informe local: ${destino}.`,
);
