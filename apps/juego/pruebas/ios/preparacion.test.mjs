import assert from 'node:assert/strict';
import { test } from 'node:test';
import { mkdtempSync, readFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
import sharp from 'sharp';
import { opcionesIOS, argumentosCmakeIOS } from '../../herramientas/ios/opciones.mjs';
import { prepararIconoIOS } from '../../herramientas/ios/icono.mjs';
import { huella, rutaDentro } from '../../herramientas/distribucion/archivos.mjs';

const raiz = fileURLToPath(new URL('../../../../', import.meta.url));
test('iOS: dispositivo, simulador Intel/ARM y firma opcional separados', () => {
  const o = opcionesIOS([], raiz);
  assert.equal(o.imagenes, 'alta');
  assert.equal(o.musica, 'mp3');
  assert.equal(o.datos, resolve(raiz, 'apps/juego/datos_originales/CURSE'));
  const dispositivo = argumentosCmakeIOS(raiz, '/datos con espacios', o, 'x64');
  assert.ok(dispositivo.includes('-DCMAKE_OSX_SYSROOT=iphoneos'));
  assert.ok(dispositivo.includes('-DCMAKE_OSX_ARCHITECTURES=arm64'));
  assert.ok(dispositivo.includes('-DCURSE_IOS_GENERADOS=/datos con espacios'));
  assert.ok(!dispositivo.some((a) => a.startsWith('-DCURSE_IOS_EQUIPO=')));
  const sim = opcionesIOS(['--simulador', '--equipo', 'ABC1234567', '--sin-abrir'], raiz);
  assert.equal(sim.abrir, false);
  assert.ok(
    argumentosCmakeIOS(raiz, '/datos', sim, 'x64').includes('-DCMAKE_OSX_ARCHITECTURES=x86_64'),
  );
  assert.ok(
    argumentosCmakeIOS(raiz, '/datos', sim, 'arm64').includes('-DCMAKE_OSX_ARCHITECTURES=arm64'),
  );
  assert.ok(
    argumentosCmakeIOS(raiz, '/datos', sim).includes('-DCMAKE_OSX_SYSROOT=iphonesimulator'),
  );
  assert.ok(argumentosCmakeIOS(raiz, '/datos', sim).includes('-DCURSE_IOS_EQUIPO=ABC1234567'));
  assert.notEqual(dispositivo[3], argumentosCmakeIOS(raiz, '/datos', sim)[3]);
});
test('iOS: parámetros incompletos, ambiguos o desconocidos fallan antes de preparar', () => {
  for (const args of [
    ['--datos'],
    ['--datos', '--simulador'],
    ['--equipo', 'mal'],
    ['--imagenes', 'otra'],
    ['--musica', 'midi'],
    ['--simulador', '--simulador'],
    ['--otro'],
  ])
    assert.throws(() => opcionesIOS(args, raiz));
  assert.equal(
    opcionesIOS(['--solo-recursos', '--musica', 'wav', '--imagenes', 'sin-perdida'], raiz)
      .soloRecursos,
    true,
  );
});
test('iOS: catálogo de iconos usa el PNG del ejecutable, opaco y con todas las medidas', async (t) => {
  const destino = mkdtempSync(join(tmpdir(), 'curse-icono-ios-'));
  t.after(() => rmSync(rutaDentro(tmpdir(), basename(destino)), { recursive: true, force: true }));
  const png = join(raiz, 'packages/assets/lanzador/icono/curse-reborn.png');
  const antes = huella(readFileSync(png));
  const informe = await prepararIconoIOS(png, destino);
  assert.equal(informe.sha256_maestro, antes);
  const carpeta = join(destino, 'Assets.xcassets/AppIcon.appiconset');
  const catalogo = JSON.parse(readFileSync(join(carpeta, 'Contents.json')));
  assert.equal(catalogo.images.length, 10);
  for (const entrada of catalogo.images) {
    const m = await sharp(join(carpeta, entrada.filename)).metadata();
    const tamano = parseFloat(entrada.size) * parseFloat(entrada.scale);
    assert.equal(m.width, tamano);
    assert.equal(m.height, tamano);
    assert.equal(m.hasAlpha, false);
  }
  assert.equal(huella(readFileSync(png)), antes);
});
