/** Entrada común desde pnpm; las herramientas de análisis usan un Python local. */
import { existsSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const entorno = join(raiz, 'trabajo', 'entorno');
const pythonLocal = process.platform === 'win32'
  ? join(entorno, 'Scripts', 'python.exe')
  : join(entorno, 'bin', 'python');
const variables = { ...process.env, PYTHONUTF8: '1', PYTHONIOENCODING: 'utf-8' };

function ejecutar(programa, argumentos) {
  const resultado = spawnSync(programa, argumentos, {
    cwd: raiz,
    env: variables,
    stdio: 'inherit',
    shell: false,
  });
  if (resultado.error) {
    throw new Error(`No se ha podido ejecutar ${programa}: ${resultado.error.message}`);
  }
  if (resultado.status !== 0) {
    process.exit(resultado.status ?? 1);
  }
}

function buscarPython() {
  const candidatos = process.env.PYTHON
    ? [[process.env.PYTHON, []]]
    : process.platform === 'win32'
      ? [['py', ['-3']], ['python', []], ['python3', []]]
      : [['python3', []], ['python', []]];
  for (const [programa, argumentos] of candidatos) {
    const resultado = spawnSync(programa, [...argumentos, '-c',
      'import sys; sys.exit(0 if sys.version_info >= (3, 9) else 1)'],
    { cwd: raiz, env: variables, encoding: 'utf8', windowsHide: true });
    if (!resultado.error && resultado.status === 0) {
      return [programa, argumentos];
    }
  }
  throw new Error('No se encuentra Python 3.9 o posterior. Instálalo o indica la ruta del ejecutable en la variable PYTHON.');
}

function comprobarEntorno() {
  if (!existsSync(pythonLocal)) {
    throw new Error('Falta el entorno de análisis. Ejecuta «pnpm preparar».');
  }
  const resultado = spawnSync(pythonLocal, ['-c',
    'import importlib.metadata, sys; sys.exit(0 if importlib.metadata.version("capstone") == "5.0.7" else 1)'],
  { cwd: raiz, env: variables, stdio: 'ignore', windowsHide: true });
  if (resultado.error || resultado.status !== 0) {
    throw new Error('Las dependencias de análisis no están preparadas. Ejecuta «pnpm preparar».');
  }
}

function preparar() {
  if (!existsSync(pythonLocal)) {
    const [programa, argumentos] = buscarPython();
    console.log('Creando el entorno local de Python en trabajo/entorno…');
    ejecutar(programa, [...argumentos, '-m', 'venv', entorno]);
  }
  console.log('Instalando las dependencias del desensamblador…');
  ejecutar(pythonLocal, ['-m', 'pip', '--isolated', 'install',
    '--disable-pip-version-check', '-r', join(raiz, 'herramientas', 'requisitos.txt')]);
  comprobarEntorno();
  console.log('Preparación completada. Puedes ejecutar «pnpm analizar» y «pnpm comprobar».');
}

const [accion, ...argumentos] = process.argv.slice(2);
try {
  switch (accion) {
    case 'preparar':
      preparar();
      break;
    case 'analizar':
      comprobarEntorno();
      ejecutar(pythonLocal, [join(raiz, 'herramientas', 'analizar_ejecutable.py'),
        '--raiz', '0000:0238', '--raiz', '0000:6A8C', '--raiz', '0000:8C51', ...argumentos]);
      break;
    case 'comprobar':
      comprobarEntorno();
      ejecutar(pythonLocal, [join(raiz, 'pruebas', 'comprobar_analisis.py'), ...argumentos]);
      break;
    case 'estado':
      console.log(`Node: ${process.version}`);
      console.log(`Carpeta del proyecto: ${raiz}`);
      console.log(`Ejecutable original: ${existsSync(join(raiz, 'datos_originales', 'CURSE.EXE')) ? 'disponible' : 'pendiente de copiar a datos_originales/'}`);
      console.log(`Entorno local de Python: ${existsSync(pythonLocal) ? 'disponible' : 'pendiente de preparar'}`);
      comprobarEntorno();
      console.log('Dependencias de análisis: preparadas.');
      break;
    default:
      throw new Error('Acción desconocida. Usa «pnpm preparar», «pnpm analizar», «pnpm comprobar» o «pnpm estado».');
  }
} catch (error) {
  console.error(`Error: ${error.message}`);
  process.exitCode = 1;
}
