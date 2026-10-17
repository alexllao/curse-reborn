/** Entrada común desde pnpm; las herramientas de análisis usan un Python local. */
import { existsSync, mkdirSync } from 'node:fs';
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

function comprobarEntorno(requiereEmulador = false) {
  if (!existsSync(pythonLocal)) {
    throw new Error('Falta el entorno de análisis. Ejecuta «pnpm preparar».');
  }
  const paquetes = requiereEmulador ? { capstone: '5.0.7', unicorn: '2.1.4' } : { capstone: '5.0.7' };
  const resultado = spawnSync(pythonLocal, ['-c',
    'import importlib.metadata, json, sys; paquetes = json.loads(sys.argv[1]); sys.exit(0 if all(importlib.metadata.version(n) == v for n, v in paquetes.items()) else 1)',
    JSON.stringify(paquetes)],
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

function compararAcciones(tipo = 'acciones', opcionesComparacion = []) {
  comprobarEntorno(true);
  if (process.platform === 'win32' && process.arch !== 'x64') {
    throw new Error('La compilación de esta comparación en Windows requiere Node y Python de 64 bits.');
  }
  const carpeta = join(raiz, 'trabajo', 'comparacion_acciones');
  mkdirSync(carpeta, { recursive: true });
  const biblioteca = join(carpeta, process.platform === 'win32' ? 'tabla_acciones.dll' : 'tabla_acciones.so');
  const compilador = process.env.CXX || 'clang++';
  const opciones = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
    '-fno-exceptions', '-fno-rtti', '-fno-builtin', '-fno-stack-protector', '-nostdlib'];
  if (process.platform === 'win32') {
    opciones.push('--target=x86_64-pc-windows-msvc', '-shared', '-fuse-ld=lld', '-Wl,/noentry');
  } else if (process.platform === 'darwin') {
    opciones.push('-dynamiclib', '-fPIC');
  } else {
    opciones.push('-shared', '-fPIC');
  }
  console.log('Compilando los bloques C++ reconstruidos…');
  ejecutar(compilador, [...opciones, join(raiz, 'motor', 'acciones', 'construir_tabla.cpp'),
    join(raiz, 'motor', 'acciones', 'seleccionar_coincidencias.cpp'),
    join(raiz, 'motor', 'acciones', 'retorno_accion.cpp'),
    join(raiz, 'motor', 'acciones', 'preparar_resultados.cpp'),
    join(raiz, 'motor', 'acciones', 'iniciar_accion.cpp'),
    join(raiz, 'motor', 'acciones', 'lista_identificadores.cpp'),
    join(raiz, 'motor', 'movimiento', 'direccion.cpp'),
    join(raiz, 'motor', 'movimiento', 'planificar.cpp'),
    join(raiz, 'motor', 'movimiento', 'avanzar.cpp'),
    join(raiz, 'motor', 'entidades', 'crear.cpp'),
    join(raiz, 'motor', 'entidades', 'actualizar.cpp'),
    join(raiz, 'motor', 'entidades', 'secuencias.cpp'),
    join(raiz, 'motor', 'sonido', 'seleccionar.cpp'),
    join(raiz, 'motor', 'sonido', 'altavoz.cpp'),
    join(raiz, 'motor', 'video', 'planos.cpp'),
    join(raiz, 'motor', 'aleatorio', 'generador.cpp'),
    join(raiz, 'pruebas', 'puente_tabla.cpp'), '-o', biblioteca]);
  const comparador = { acciones: 'comparar_acciones.py', aleatorio: 'comparar_aleatorio.py',
    resultados: 'comparar_resultados.py', avance: 'comparar_avance.py', entidades: 'comparar_entidades.py',
    secuencias: 'comparar_secuencias.py', operaciones: 'comparar_operaciones.py', altavoz: 'comparar_altavoz.py',
    video: 'comparar_video.py' }[tipo];
  ejecutar(pythonLocal, [join(raiz, 'pruebas', comparador), '--biblioteca', biblioteca, ...opcionesComparacion]);
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
        '--mapa', join(raiz, 'documentacion', 'mapa_ejecutable.json'), ...argumentos]);
      break;
    case 'comprobar':
      comprobarEntorno();
      ejecutar(pythonLocal, [join(raiz, 'pruebas', 'comprobar_analisis.py'), ...argumentos]);
      break;
    case 'comparar:acciones':
      compararAcciones();
      break;
    case 'comparar:aleatorio':
      compararAcciones('aleatorio');
      break;
    case 'comparar:resultados':
      compararAcciones('resultados');
      break;
    case 'comparar:avance':
      compararAcciones('avance', argumentos);
      break;
    case 'comparar:entidades':
      compararAcciones('entidades', argumentos);
      break;
    case 'comparar:secuencias':
      compararAcciones('secuencias', argumentos);
      break;
    case 'comparar:operaciones':
      compararAcciones('operaciones', argumentos);
      break;
    case 'comparar:altavoz':
      compararAcciones('altavoz', argumentos);
      break;
    case 'comparar:video':
      compararAcciones('video', argumentos);
      break;
    case 'estado':
      console.log(`Node: ${process.version}`);
      console.log(`Carpeta del proyecto: ${raiz}`);
      console.log(`Ejecutable original: ${existsSync(join(raiz, 'datos_originales', 'CURSE', 'CURSE.EXE')) ? 'disponible' : 'pendiente de copiar a datos_originales/CURSE/'}`);
      console.log(`Carpeta de ROM del MT-32: ${existsSync(join(raiz, 'datos_originales', 'ROMS')) ? 'disponible (contenido sin verificar)' : 'no disponible; el análisis estático no la necesita'}`);
      console.log(`Entorno local de Python: ${existsSync(pythonLocal) ? 'disponible' : 'pendiente de preparar'}`);
      comprobarEntorno();
      console.log('Dependencias de análisis: preparadas.');
      break;
    default:
      throw new Error('Acción desconocida. Consulta los comandos disponibles con «pnpm run».');
  }
} catch (error) {
  console.error(`Error: ${error.message}`);
  process.exitCode = 1;
}
