import { mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { randomUUID } from 'node:crypto';
import { opcionesIOS, argumentosCmakeIOS } from './opciones.mjs';
import { prepararRecursosIOS } from './recursos.mjs';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../../..');
function ejecutar(programa, argumentos) {
  const r = spawnSync(programa, argumentos, { cwd: raiz, stdio: 'inherit', shell: false });
  if (r.error || r.status !== 0)
    throw new Error(
      'No se completa ' + programa + ': ' + (r.error?.message || 'revisa el diagnóstico anterior.'),
    );
}
try {
  const opciones = opcionesIOS(process.argv.slice(2), raiz);
  if (process.platform !== 'darwin' && !opciones.soloRecursos)
    throw new Error(
      'El proyecto Xcode se genera en macOS. --solo-recursos permite comprobar la preparación aquí.',
    );
  if (!opciones.soloRecursos) {
    ejecutar('xcodebuild', ['-version']);
    ejecutar('cmake', ['--version']);
  }
  if (opciones.musica === 'mp3') ejecutar(process.env.FFMPEG || 'ffmpeg', ['-version']);
  const generados = join(raiz, 'apps/juego/trabajo/ios/recursos', randomUUID());
  mkdirSync(generados, { recursive: true });
  console.log('Preparando la copia local para iPad: ' + generados);
  const informe = await prepararRecursosIOS(raiz, generados, opciones);
  console.log(
    `Recursos: ${informe.imagenes.length} imágenes, cinco canciones y ${informe.archivos_datos} archivos del juego. Maestros intactos.`,
  );
  if (!opciones.soloRecursos) {
    ejecutar('cmake', argumentosCmakeIOS(raiz, generados, opciones));
    const proyecto = join(
      raiz,
      'apps/juego/trabajo/ios',
      opciones.simulador ? 'simulador' : 'dispositivo',
      'CurseRebornIOS.xcodeproj',
    );
    console.log(
      'Proyecto: ' + proyecto + '\nEn Xcode: selecciona tu equipo de firma, el iPad y Run.',
    );
    if (opciones.abrir) ejecutar('open', [proyecto]);
  }
} catch (error) {
  console.error('Preparación iOS: ' + error.message);
  process.exitCode = 1;
}
