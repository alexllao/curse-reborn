import assert from 'node:assert/strict';
import { test } from 'node:test';
import { mkdtempSync, writeFileSync, symlinkSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, basename } from 'node:path';
import sharp from 'sharp';
import {
  huella,
  rutaDentro,
  listarArchivos,
  verificarPaquete,
} from '../../herramientas/distribucion/archivos.mjs';
import { optimizarPng } from '../../herramientas/distribucion/optimizar_assets.mjs';
import { analizarOpciones } from '../../herramientas/distribucion/opciones.mjs';
import { seleccionarHabitaciones } from '../../herramientas/gestionar_habitaciones.mjs';

// El valor inicial del JSON y una elección explícita deben producir un catálogo coherente sin tocar el maestro.
test('Habitaciones: inicial, alternativa y todas conservan el catálogo fuente', () => {
  const catalogo = {
    version: 1,
    inicial: '02-buhardilla',
    habitaciones: [{ id: '01-misterio' }, { id: '02-buhardilla' }, { id: '03-lucasarts' }],
  };
  const antes = JSON.stringify(catalogo);
  const inicial = seleccionarHabitaciones(catalogo, analizarOpciones([]).habitacion);
  assert.equal(inicial.inicial, '02-buhardilla');
  assert.deepEqual(
    inicial.habitaciones.map((h) => h.id),
    ['02-buhardilla'],
  );
  const otra = seleccionarHabitaciones(catalogo, '03-lucasarts');
  assert.equal(otra.inicial, '03-lucasarts');
  assert.deepEqual(
    otra.habitaciones.map((h) => h.id),
    ['03-lucasarts'],
  );
  assert.deepEqual(seleccionarHabitaciones(catalogo, 'todas'), catalogo);
  assert.throws(() => seleccionarHabitaciones(catalogo, 'inexistente'), /desconocida/);
  assert.throws(
    () => seleccionarHabitaciones({ ...catalogo, inicial: 'inexistente' }, 'inicial'),
    /inválidos/,
  );
  assert.equal(JSON.stringify(catalogo), antes);
});

// Valida combinaciones y valores ausentes antes de lanzar una compilación costosa.
test('Parámetros del builder: selección explícita y errores', () => {
  assert.deepEqual(analizarOpciones(['--con-datos', '--habitacion', 'todas', '--sin-perdida']), {
    conDatos: true,
    perfil: 'sin-perdida',
    habitacion: 'todas',
  });
  assert.equal(analizarOpciones(['--habitacion', '01-misterio']).habitacion, '01-misterio');
  for (const a of [
    ['--habitacion'],
    ['--habitacion', '--con-datos'],
    ['--habitacion', '../fuera'],
    ['--habitacion', 'todas', '--habitacion', 'inicial'],
    ['--otro'],
  ])
    assert.throws(() => analizarOpciones(a));
});

// Retira solo la carpeta de prueba recién creada, verificando que siga dentro del temporal.
function temporal(t, prefijo) {
  const carpeta = mkdtempSync(join(tmpdir(), prefijo));
  t.after(() => rmSync(rutaDentro(tmpdir(), basename(carpeta)), { recursive: true, force: true }));
  return carpeta;
}

// Protege las transparencias y los colores de los marcos sin modificar el búfer fuente.
test('PNG con alfa: mismos píxeles y dimensiones incluso en perfil equilibrado', async () => {
  const original = await sharp({
    create: { width: 32, height: 24, channels: 4, background: { r: 12, g: 37, b: 98, alpha: 0.4 } },
  })
    .png()
    .toBuffer();
  const copia = Buffer.from(original);
  const r = await optimizarPng(original, 'equilibrado', true);
  assert.ok(original.equals(copia));
  assert.equal(r.ancho, 32);
  assert.equal(r.alto, 24);
  const a = await sharp(original).ensureAlpha().raw().toBuffer();
  const b = await sharp(r.datos).ensureAlpha().raw().toBuffer();
  assert.ok(a.equals(b));
  assert.ok(r.datos.length <= original.length);
});

// El perfil fotográfico puede reducir colores, pero no debe reducir tamaño ni añadir alfa.
test('PNG opaco: los dos perfiles conservan el espacio de coordenadas', async () => {
  const pixeles = Buffer.alloc(128 * 128 * 3);
  for (let i = 0; i < pixeles.length; ++i) pixeles[i] = (i * 13 + Math.floor(i / 127)) % 256;
  const original = await sharp(pixeles, { raw: { width: 128, height: 128, channels: 3 } })
    .png({ compressionLevel: 0 })
    .toBuffer();
  for (const perfil of ['equilibrado', 'sin-perdida']) {
    const r = await optimizarPng(original, perfil, true);
    const m = await sharp(r.datos).metadata();
    assert.equal(m.width, 128);
    assert.equal(m.height, 128);
    assert.equal(m.hasAlpha, false);
    assert.ok(r.datos.length < original.length);
    if (perfil === 'sin-perdida')
      assert.ok((await sharp(r.datos).raw().toBuffer()).equals(pixeles));
  }
  await assert.rejects(optimizarPng(original, 'desconocido'));
});

// Rechaza escapes de directorio y detecta cambios, ficheros extra o partidas en un paquete.
test('Manifiesto: integridad, privacidad y límites de rutas', (t) => {
  const carpeta = temporal(t, 'curse-builder-');
  assert.throws(() => rutaDentro(carpeta, '../fuera'));
  assert.throws(() => rutaDentro(carpeta, carpeta));
  writeFileSync(join(carpeta, 'dato.txt'), 'abc');
  const manifiesto = {
    version: '0.9.0',
    archivos: [{ ruta: 'dato.txt', bytes: 3, sha256: huella(Buffer.from('abc')) }],
  };
  writeFileSync(join(carpeta, 'paquete.json'), JSON.stringify(manifiesto));
  assert.equal(verificarPaquete(carpeta).version, '0.9.0');
  writeFileSync(join(carpeta, 'dato.txt'), 'abd');
  assert.throws(() => verificarPaquete(carpeta), /alterado/);
  writeFileSync(join(carpeta, 'dato.txt'), 'abc');
  writeFileSync(join(carpeta, 'CURSE.SAV'), 'privado');
  assert.throws(() => verificarPaquete(carpeta), /ajenos/);
  manifiesto.archivos.push({ ruta: 'CURSE.SAV', bytes: 7, sha256: huella(Buffer.from('privado')) });
  writeFileSync(join(carpeta, 'paquete.json'), JSON.stringify(manifiesto));
  assert.throws(() => verificarPaquete(carpeta), /privado/);
});

// Un enlace a un directorio ajeno nunca debe convertirse en datos del ZIP.
test('El inventario rechaza enlaces de directorio', (t) => {
  const carpeta = temporal(t, 'curse-enlace-');
  const ajena = temporal(t, 'curse-ajena-');
  symlinkSync(ajena, join(carpeta, 'enlace'), process.platform === 'win32' ? 'junction' : 'dir');
  assert.throws(() => listarArchivos(carpeta), /enlaces/);
});
