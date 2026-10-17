import { createHash } from 'node:crypto';
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';

// Prepara una biblioteca compartida oficial y fija su fuente; las ROM nunca se descargan.
export async function prepararMt32(raiz, ejecutar, variables) {
  const ajustes = JSON.parse(
    readFileSync(join(raiz, 'herramientas', 'dependencias_mt32.json'), 'utf8'),
  );
  const dependencias = join(raiz, 'trabajo', 'dependencias');
  const destino = join(dependencias, 'mt32emu');
  const biblioteca = join(destino, 'bin', 'mt32emu-2.dll');
  if (existsSync(biblioteca) && existsSync(join(destino, 'include', 'mt32emu', 'mt32emu.h')))
    return destino;
  mkdirSync(dependencias, { recursive: true });
  const archivo = join(dependencias, `munt-${ajustes.revision}.zip`);
  if (!existsSync(archivo)) {
    console.log('Descargando Munt desde su repositorio oficial…');
    const respuesta = await fetch(ajustes.url, { signal: AbortSignal.timeout(60000) });
    if (!respuesta.ok) throw new Error(`No se ha podido descargar Munt: ${respuesta.status}.`);
    writeFileSync(archivo, Buffer.from(await respuesta.arrayBuffer()));
  }
  if (createHash('sha256').update(readFileSync(archivo)).digest('hex') !== ajustes.sha256)
    throw new Error('La huella de Munt no coincide con la revisión fijada.');
  const fuente = join(dependencias, `munt-${ajustes.revision}`);
  if (!existsSync(join(fuente, 'mt32emu', 'CMakeLists.txt')))
    ejecutar(
      'powershell.exe',
      [
        '-NoProfile',
        '-NonInteractive',
        '-Command',
        'Expand-Archive -LiteralPath $env:CURSE_ARCHIVO_MT32 -DestinationPath $env:CURSE_CARPETA_MT32 -Force',
      ],
      { ...variables, CURSE_ARCHIVO_MT32: archivo, CURSE_CARPETA_MT32: dependencias },
    );
  const herramientas = join(raiz, 'trabajo', 'entorno', 'Scripts');
  const cmake = join(herramientas, 'cmake.exe');
  const ninja = join(herramientas, 'ninja.exe');
  if (!existsSync(cmake) || !existsSync(ninja))
    ejecutar(
      join(herramientas, 'python.exe'),
      ['-m', 'pip', 'install', 'cmake==3.31.6', 'ninja==1.11.1.4'],
      variables,
    );
  const compilacion = join(dependencias, 'compilacion_mt32');
  console.log('Preparando la biblioteca MT-32 compartida…');
  ejecutar(
    cmake,
    [
      '-S',
      join(fuente, 'mt32emu'),
      '-B',
      compilacion,
      '-G',
      'Ninja',
      '-DCMAKE_BUILD_TYPE=Release',
      `-DCMAKE_INSTALL_PREFIX=${destino}`,
      `-DCMAKE_MAKE_PROGRAM=${ninja}`,
      '-DCMAKE_C_COMPILER=clang',
      '-DCMAKE_CXX_COMPILER=clang++',
      '-DBUILD_TESTING=OFF',
      '-Dlibmt32emu_REQUIRE_ANSI=OFF',
      '-DCMAKE_CXX_STANDARD=17',
      '-Dlibmt32emu_CPP_INTERFACE=OFF',
      '-Dlibmt32emu_SHARED=ON',
    ],
    variables,
  );
  ejecutar(cmake, ['--build', compilacion, '--parallel', '4'], variables);
  ejecutar(cmake, ['--install', compilacion], variables);
  if (!existsSync(biblioteca))
    throw new Error('La compilación MT-32 no ha generado la biblioteca esperada.');
  return destino;
}
