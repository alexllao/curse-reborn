/** Formato de C++ y comprobaciones Python sobre el código propio. */
import { spawnSync } from 'node:child_process';
import { existsSync } from 'node:fs';
import { migrarTrabajoLocal } from './rutas_proyecto.mjs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
migrarTrabajoLocal(raiz);
const python = join(
  raiz,
  'apps',
  'juego',
  'trabajo',
  'entorno',
  process.platform === 'win32' ? 'Scripts/python.exe' : 'bin/python',
);
const variables = { ...process.env, PYTHONUTF8: '1', PYTHONIOENCODING: 'utf-8' };

// Ejecuta una herramienta desde la raíz y propaga su fallo para detener la comprobación.
function ejecutar(programa, argumentos, capturar = false) {
  const resultado = spawnSync(programa, argumentos, {
    cwd: raiz,
    env: variables,
    encoding: 'utf8',
    windowsHide: true,
    stdio: capturar ? 'pipe' : 'inherit',
  });
  if (resultado.error) {
    throw new Error(`No se ha podido ejecutar ${programa}: ${resultado.error.message}`);
  }
  if (resultado.status !== 0) process.exit(resultado.status ?? 1);
  return resultado.stdout;
}

try {
  const [accion] = process.argv.slice(2);
  if (accion === 'revisar') {
    console.log('Revisando las herramientas y comparadores Python con Ruff…');
    ejecutar(python, ['-m', 'ruff', 'check', 'apps/juego/herramientas', 'apps/juego/pruebas']);
  } else if (accion === 'formatear' || accion === 'comprobar') {
    const escribir = accion === 'formatear';
    const formateador = process.env.CLANG_FORMAT || 'clang-format';
    const version = ejecutar(formateador, ['--version'], true);
    if (!/version 21\./.test(version)) {
      throw new Error(
        'El formato de C++ requiere clang-format 21. Puedes indicar su ruta con CLANG_FORMAT.',
      );
    }
    const archivos = ejecutar(
      'git',
      [
        'ls-files',
        '-z',
        '--cached',
        '--others',
        '--exclude-standard',
        '--',
        'apps/juego/motor',
        'apps/juego/plataformas',
        'apps/juego/pruebas',
      ],
      true,
    )
      .split('\0')
      .filter((archivo) => /\.(cpp|hpp)$/.test(archivo) && existsSync(join(raiz, archivo)));
    console.log(
      `${escribir ? 'Aplicando' : 'Comprobando'} el formato de ${archivos.length} archivos C++…`,
    );
    ejecutar(formateador, [...(escribir ? ['-i'] : ['--dry-run', '--Werror']), ...archivos]);
    console.log(`${escribir ? 'Aplicando' : 'Comprobando'} el formato de Python…`);
    ejecutar(python, [
      '-m',
      'ruff',
      'format',
      ...(escribir ? [] : ['--check']),
      'apps/juego/herramientas',
      'apps/juego/pruebas',
    ]);
  } else {
    throw new Error(
      'Usa formatear, comprobar o revisar. Para preparar Python y Ruff, ejecuta «pnpm preparar».',
    );
  }
} catch (error) {
  console.error(`Error: ${error.message}`);
  process.exitCode = 1;
}
