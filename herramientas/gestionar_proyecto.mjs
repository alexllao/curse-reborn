/** Entrada común desde pnpm; las herramientas de análisis usan un Python local. */
import { existsSync, mkdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { gestionarSdl } from './gestionar_sdl.mjs';
import { obtenerFuentesMotor } from './fuentes_motor.mjs';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const entorno = join(raiz, 'trabajo', 'entorno');
const pythonLocal =
  process.platform === 'win32'
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
      ? [
          ['py', ['-3']],
          ['python', []],
          ['python3', []],
        ]
      : [
          ['python3', []],
          ['python', []],
        ];
  for (const [programa, argumentos] of candidatos) {
    const resultado = spawnSync(
      programa,
      [...argumentos, '-c', 'import sys; sys.exit(0 if sys.version_info >= (3, 9) else 1)'],
      { cwd: raiz, env: variables, encoding: 'utf8', windowsHide: true },
    );
    if (!resultado.error && resultado.status === 0) {
      return [programa, argumentos];
    }
  }
  throw new Error(
    'No se encuentra Python 3.9 o posterior. Instálalo o indica la ruta del ejecutable en la variable PYTHON.',
  );
}

function comprobarEntorno(requiereEmulador = false) {
  if (!existsSync(pythonLocal)) {
    throw new Error('Falta el entorno de análisis. Ejecuta «pnpm preparar».');
  }
  const paquetes = requiereEmulador
    ? { capstone: '5.0.7', unicorn: '2.1.4' }
    : { capstone: '5.0.7' };
  const resultado = spawnSync(
    pythonLocal,
    [
      '-c',
      'import importlib.metadata, json, sys; paquetes = json.loads(sys.argv[1]); sys.exit(0 if all(importlib.metadata.version(n) == v for n, v in paquetes.items()) else 1)',
      JSON.stringify(paquetes),
    ],
    { cwd: raiz, env: variables, stdio: 'ignore', windowsHide: true },
  );
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
  ejecutar(pythonLocal, [
    '-m',
    'pip',
    '--isolated',
    'install',
    '--disable-pip-version-check',
    '-r',
    join(raiz, 'herramientas', 'requisitos.txt'),
  ]);
  comprobarEntorno();
  console.log('Preparación completada. Puedes ejecutar «pnpm analizar» y «pnpm comprobar».');
}

function compararAcciones(tipo = 'acciones', opcionesComparacion = []) {
  comprobarEntorno(tipo !== 'titulo');
  if (process.platform === 'win32' && process.arch !== 'x64') {
    throw new Error(
      'La compilación de esta comparación en Windows requiere Node y Python de 64 bits.',
    );
  }
  const carpeta = join(raiz, 'trabajo', 'comparacion_acciones');
  mkdirSync(carpeta, { recursive: true });
  const biblioteca = join(
    carpeta,
    process.platform === 'win32' ? 'tabla_acciones.dll' : 'tabla_acciones.so',
  );
  const compilador = process.env.CXX || 'clang++';
  const opciones = [
    '-std=c++17',
    '-O2',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-builtin',
    '-fno-stack-protector',
    '-nostdlib',
  ];
  if (process.platform === 'win32') {
    opciones.push('--target=x86_64-pc-windows-msvc', '-shared', '-fuse-ld=lld', '-Wl,/noentry');
  } else if (process.platform === 'darwin') {
    opciones.push('-dynamiclib', '-fPIC');
  } else {
    opciones.push('-shared', '-fPIC');
  }
  console.log('Compilando los bloques C++ reconstruidos…');
  ejecutar(compilador, [
    ...opciones,
    ...obtenerFuentesMotor(raiz),
    join(raiz, 'pruebas', 'puente_tabla.cpp'),
    '-o',
    biblioteca,
  ]);
  const comparadores = {
    acciones: 'comparar_acciones.py',
    aleatorio: 'comparar_aleatorio.py',
    resultados: 'comparar_resultados.py',
    avance: 'comparar_avance.py',
    entidades: 'comparar_entidades.py',
    objetos: ['comparar_objetos.py', 'comparar_composicion.py'],
    introduccion: 'comparar_introduccion.py',
    menu: 'comparar_menu.py',
    jugabilidad: 'comparar_jugabilidad.py',
    puzle: 'comparar_puzle.py',
    corredor: 'comparar_corredor.py',
    caida: 'comparar_caida.py',
    lago: 'comparar_lago.py',
    poza: 'comparar_poza.py',
    cuevas: 'comparar_cuevas.py',
    catapulta: 'comparar_catapulta.py',
    final_cuevas: 'comparar_final_cuevas.py',
    partidas: 'comparar_partidas.py',
    panel_partidas: 'comparar_panel_partidas.py',
    informacion: 'comparar_informacion.py',
    campo: 'comparar_campo.py',
    asaltante: 'comparar_asaltante.py',
    poblacion: 'comparar_poblacion.py',
    calles: 'comparar_calles.py',
    tiendas: 'comparar_tiendas.py',
    probador: 'comparar_probador.py',
    borde: 'comparar_borde.py',
    recorrido_borde: 'comparar_recorrido_borde.py',
    acciones_borde: 'comparar_acciones_borde.py',
    loro: 'comparar_loro.py',
    regreso_loro: 'comparar_regreso_loro.py',
    ramas_valle: 'comparar_ramas_valle.py',
    dinosaurio: 'comparar_dinosaurio.py',
    correo: 'comparar_correo.py',
    grabacion_valle: 'comparar_grabacion_valle.py',
    grabacion_completa: 'comparar_grabacion_completa.py',
    opciones: 'auditar_opciones.py',
    secuencias: 'comparar_secuencias.py',
    operaciones: 'comparar_operaciones.py',
    altavoz: 'comparar_altavoz.py',
    audio: ['comparar_altavoz.py', 'comparar_audio.py'],
    musica: 'comparar_musica.py',
    musica_rs: 'comparar_musica_rs.py',
    midi_dosbox: 'comparar_midi_dosbox.py',
    digital: 'comparar_digital.py',
    video: 'comparar_video.py',
    rnc: 'comparar_rnc.py',
    rnc2: 'comparar_rnc2.py',
    escena: ['comparar_rnc2.py', 'comparar_escena.py'],
    presentacion: ['comparar_rnc2.py', 'comparar_escena.py', 'comparar_fundidos.py'],
    titulo: 'generar_titulo.py',
    graficos: ['comparar_video.py', 'comparar_rnc.py'],
  }[tipo];
  for (const comparador of Array.isArray(comparadores) ? comparadores : [comparadores]) {
    ejecutar(pythonLocal, [
      join(raiz, tipo === 'titulo' ? 'herramientas' : 'pruebas', comparador),
      '--biblioteca',
      biblioteca,
      ...opcionesComparacion,
    ]);
  }
}

const [accion, ...argumentos] = process.argv.slice(2);
try {
  switch (accion) {
    case 'comprobar:regreso-loro':
      await gestionarSdl('comprobar:poblacion', ['--regreso-loro']);
      break;
    case 'comprobar:borde':
      await gestionarSdl('comprobar:poblacion', ['--borde']);
      break;
    case 'comprobar:entrada-borde':
      await gestionarSdl('comprobar:poblacion', ['--entrada-borde']);
      break;
    case 'comprobar:probador':
      await gestionarSdl('comprobar:poblacion', ['--probador']);
      break;
    case 'comprobar:tiendas':
      await gestionarSdl('comprobar:poblacion', ['--tiendas']);
      break;
    case 'comprobar:calles':
      await gestionarSdl('comprobar:poblacion', ['--calles']);
      break;
    case 'sdl:preparar':
    case 'sdl:compilar':
    case 'ventana':
    case 'comprobar:vistas':
    case 'comprobar:caida':
    case 'comprobar:lago':
    case 'comprobar:poza':
    case 'comprobar:poblacion':
    case 'comprobar:asaltante':
    case 'comprobar:campo':
    case 'comprobar:informacion':
    case 'comprobar:partidas':
    case 'comprobar:final-cuevas':
    case 'comprobar:catapulta':
    case 'comprobar:cuevas':
    case 'comprobar:corredor':
    case 'comprobar:puzle':
    case 'comprobar:jugabilidad':
    case 'comprobar:menu':
    case 'comprobar:introduccion':
    case 'comprobar:fps':
    case 'comprobar:tiempo':
    case 'comprobar:filtros':
    case 'comprobar:reinicio':
    case 'comprobar:monitor':
    case 'comprobar:marcos':
    case 'comprobar:decorados':
    case 'comprobar:cursor':
    case 'comprobar:ventana':
    case 'comprobar:escena':
      await gestionarSdl(accion, argumentos);
      break;
    case 'preparar':
      preparar();
      break;
    case 'analizar':
      comprobarEntorno();
      ejecutar(pythonLocal, [
        join(raiz, 'herramientas', 'analizar_ejecutable.py'),
        '--mapa',
        join(raiz, 'documentacion', 'mapa_ejecutable.json'),
        ...argumentos,
      ]);
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
    case 'comparar:objetos':
      compararAcciones('objetos', argumentos);
      break;
    case 'comparar:secuencias':
      compararAcciones('secuencias', argumentos);
      break;
    case 'comparar:operaciones':
      compararAcciones('operaciones', argumentos);
      break;
    case 'comprobar:mt32':
      compararAcciones('musica_rs', argumentos);
      await gestionarSdl('comprobar:mt32', []);
      ejecutar(pythonLocal, [join(raiz, 'pruebas', 'comparar_sintesis_mt32.py')]);
      break;
    case 'exportar:musica':
      await gestionarSdl('exportar:musica', argumentos);
      break;
    case 'comparar:digital':
      compararAcciones('digital', argumentos);
      break;
    case 'comprobar:digital':
      compararAcciones('digital', argumentos);
      await gestionarSdl('comprobar:digital', []);
      break;
    case 'comparar:dosbox-midi':
      compararAcciones('midi_dosbox', argumentos);
      break;
    case 'comparar:musica-rs':
      compararAcciones('musica_rs', argumentos);
      break;
    case 'comprobar:musica':
      compararAcciones('musica', argumentos);
      await gestionarSdl('comprobar:musica', []);
      break;
    case 'comparar:musica':
      compararAcciones('musica', argumentos);
      break;
    case 'comprobar:audio':
      compararAcciones('audio', argumentos);
      await gestionarSdl('comprobar:audio', []);
      break;
    case 'comparar:audio':
      compararAcciones('audio', argumentos);
      break;
    case 'comparar:altavoz':
      compararAcciones('altavoz', argumentos);
      break;
    case 'comparar:video':
      compararAcciones('video', argumentos);
      break;
    case 'comparar:rnc':
      compararAcciones('rnc', argumentos);
      break;
    case 'comparar:rnc2':
      compararAcciones('rnc2', argumentos);
      break;
    case 'comparar:escena':
      compararAcciones('escena', argumentos);
      break;
    case 'comparar:lago':
      compararAcciones('lago', argumentos);
      break;
    case 'comparar:poza':
      compararAcciones('poza', argumentos);
      break;
    case 'comparar:cuevas':
      compararAcciones('cuevas', argumentos);
      break;
    case 'comparar:opciones':
      compararAcciones('opciones', argumentos);
      break;
    case 'comparar:poblacion':
      compararAcciones('poblacion', argumentos);
      break;
    case 'comparar:grabacion-completa':
      compararAcciones('grabacion_completa', argumentos);
      break;
    case 'comparar:grabacion-valle':
      compararAcciones('grabacion_valle', argumentos);
      break;
    case 'comparar:correo':
      compararAcciones('correo', argumentos);
      break;
    case 'comparar:dinosaurio':
      compararAcciones('dinosaurio', argumentos);
      break;
    case 'comparar:ramas-valle':
      compararAcciones('ramas_valle', argumentos);
      break;
    case 'comparar:regreso-loro':
      compararAcciones('regreso_loro', argumentos);
      break;
    case 'comparar:loro':
      compararAcciones('loro', argumentos);
      break;
    case 'comparar:acciones-borde':
      compararAcciones('acciones_borde', argumentos);
      break;
    case 'comparar:recorrido-borde':
      compararAcciones('recorrido_borde', argumentos);
      break;
    case 'comparar:borde':
      compararAcciones('borde', argumentos);
      break;
    case 'comparar:probador':
      compararAcciones('probador', argumentos);
      break;
    case 'comparar:tiendas':
      compararAcciones('tiendas', argumentos);
      break;
    case 'comparar:calles':
      compararAcciones('calles', argumentos);
      break;
    case 'comparar:asaltante':
      compararAcciones('asaltante', argumentos);
      break;
    case 'comparar:campo':
      compararAcciones('campo', argumentos);
      break;
    case 'comparar:informacion':
      compararAcciones('informacion', argumentos);
      break;
    case 'comparar:panel-partidas':
      compararAcciones('panel_partidas', argumentos);
      break;
    case 'comparar:partidas':
      compararAcciones('partidas', argumentos);
      break;
    case 'comparar:final-cuevas':
      compararAcciones('final_cuevas', argumentos);
      break;
    case 'comparar:catapulta':
      compararAcciones('catapulta', argumentos);
      break;
    case 'comparar:caida':
      compararAcciones('caida', argumentos);
      break;
    case 'comparar:corredor':
      compararAcciones('corredor', argumentos);
      break;
    case 'comparar:puzle':
      compararAcciones('puzle', argumentos);
      break;
    case 'comparar:jugabilidad':
      compararAcciones('jugabilidad', argumentos);
      break;
    case 'comparar:menu':
      compararAcciones('menu', argumentos);
      break;
    case 'comparar:introduccion':
      compararAcciones('introduccion', argumentos);
      break;
    case 'comparar:presentacion':
      compararAcciones('presentacion', argumentos);
      break;
    case 'titulo':
      compararAcciones('titulo', argumentos);
      break;
    case 'comparar:graficos':
      compararAcciones('graficos', argumentos);
      break;
    case 'estado':
      console.log(`Node: ${process.version}`);
      console.log(`Carpeta del proyecto: ${raiz}`);
      console.log(
        `Ejecutable original: ${existsSync(join(raiz, 'datos_originales', 'CURSE', 'CURSE.EXE')) ? 'disponible' : 'pendiente de copiar a datos_originales/CURSE/'}`,
      );
      console.log(
        `Carpeta de ROM del MT-32: ${existsSync(join(raiz, 'datos_originales', 'ROMS')) ? 'disponible (contenido sin verificar)' : 'no disponible; el análisis estático no la necesita'}`,
      );
      console.log(
        `Entorno local de Python: ${existsSync(pythonLocal) ? 'disponible' : 'pendiente de preparar'}`,
      );
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
