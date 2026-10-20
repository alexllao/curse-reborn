import { rutaJuego, rutaAssets } from './rutas_proyecto.mjs';
import { obtenerFuentesMotor } from './fuentes_motor.mjs';
import { prepararMt32 } from './gestionar_mt32.mjs';
import { prepararHabitaciones } from './gestionar_habitaciones.mjs';
import { prepararSonidosPC } from './gestionar_sonidos_pc.mjs';
import { prepararPresentacion } from './gestionar_presentacion.mjs';
import { prepararMarcos } from './gestionar_marcos.mjs';
/** Compilación y comprobación de Curse Reborn con SDL, sin ejecutar código DOS. */
import { createHash } from 'node:crypto';
import { copyFileSync, existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { inflateSync } from 'node:zlib';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
const configuracion = JSON.parse(
  readFileSync(join(raiz, 'apps/juego/herramientas', 'dependencias_sdl.json'), 'utf8'),
);
const dependencias = join(raiz, 'apps', 'juego', 'trabajo', 'dependencias');
const aplicacion = join(raiz, 'apps', 'juego', 'trabajo', 'aplicacion');
const datos = join(raiz, 'apps/juego/datos_originales', 'CURSE', 'DATA');

function ejecutar(programa, argumentos, variables = process.env, capturar = false, ocultar = true) {
  const resultado = spawnSync(programa, argumentos, {
    cwd: raiz,
    env: variables,
    stdio: capturar ? 'pipe' : 'inherit',
    encoding: 'utf8',
    windowsHide: ocultar,
    shell: false,
    // Las habitaciones exportan sus variantes y fases en ventanas ocultas.
    timeout:
      argumentos.includes('--comprobar-reinicio') || argumentos.includes('--comprobar-monitor')
        ? 30000
        : argumentos.some((argumento) => argumento.startsWith('--comprobar'))
          ? 15000
          : undefined,
    windowsVerbatimArguments:
      process.platform === 'win32' && programa.toLowerCase().endsWith('cmd.exe'),
  });
  if (resultado.error || resultado.status !== 0) {
    throw new Error(
      `No se ha podido ejecutar ${programa}. ${resultado.error?.message || resultado.stderr || 'Revisa el diagnóstico anterior.'}`,
    );
  }
  return resultado.stdout?.trim() || '';
}

async function preparar() {
  if (process.platform !== 'win32') {
    console.log(
      'Usa una instalación SDL3 compatible con CMake; consulta apps/juego/documentacion/ventana_sdl.md.',
    );
    return null;
  }
  if (process.arch !== 'x64')
    throw new Error('El paquete SDL preparado requiere Windows y Node de 64 bits.');
  const carpeta = join(dependencias, `SDL3-${configuracion.version}`);
  const archivos = ['include/SDL3/SDL.h', 'lib/x64/SDL3.dll', 'lib/x64/SDL3.lib', 'LICENSE.txt'];
  if (archivos.every((archivo) => existsSync(join(carpeta, archivo)))) return carpeta;
  mkdirSync(dependencias, { recursive: true });
  const archivo = join(dependencias, configuracion.windows.archivo);
  if (!existsSync(archivo)) {
    console.log(`Descargando SDL ${configuracion.version} desde su publicación oficial…`);
    const respuesta = await fetch(configuracion.windows.url, {
      signal: AbortSignal.timeout(60000),
    });
    if (!respuesta.ok) throw new Error(`La descarga de SDL ha fallado: ${respuesta.status}.`);
    writeFileSync(archivo, Buffer.from(await respuesta.arrayBuffer()));
  }
  const huella = createHash('sha256').update(readFileSync(archivo)).digest('hex');
  if (huella !== configuracion.windows.sha256)
    throw new Error('La huella del paquete SDL no coincide; no se extraerá.');
  ejecutar(
    'powershell.exe',
    [
      '-NoProfile',
      '-NonInteractive',
      '-Command',
      'Expand-Archive -LiteralPath $env:CURSE_ARCHIVO_SDL -DestinationPath $env:CURSE_CARPETA_SDL -Force',
    ],
    { ...process.env, CURSE_ARCHIVO_SDL: archivo, CURSE_CARPETA_SDL: dependencias },
  );
  if (!existsSync(join(carpeta, 'lib', 'x64', 'SDL3.lib')))
    throw new Error('El paquete SDL no contiene la biblioteca esperada.');
  return carpeta;
}

function entornoCompilador() {
  const localizador = join(
    process.env['ProgramFiles(x86)'] || 'C:/Program Files (x86)',
    'Microsoft Visual Studio',
    'Installer',
    'vswhere.exe',
  );
  const instalacion =
    process.env.VSINSTALLDIR ||
    (existsSync(localizador)
      ? ejecutar(
          localizador,
          [
            '-latest',
            '-products',
            '*',
            '-requires',
            'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
            '-property',
            'installationPath',
            '-utf8',
          ],
          process.env,
          true,
        )
      : '');
  const guion = join(instalacion, 'VC', 'Auxiliary', 'Build', 'vcvars64.bat');
  if (!instalacion || !existsSync(guion)) {
    throw new Error(
      'Faltan las herramientas C++ de Visual Studio con Windows SDK. Instálalas o usa una consola de desarrollo configurada.',
    );
  }
  const salida = ejecutar(
    'cmd.exe',
    ['/d', '/s', '/c', 'chcp 65001 >nul && call "%CURSE_VCVARS%" >nul && set'],
    { ...process.env, CURSE_VCVARS: guion },
    true,
  );
  const variables = { ...process.env };
  for (const linea of salida.split(/\r?\n/)) {
    const separador = linea.indexOf('=');
    if (separador <= 0) continue;
    const nombre = linea.slice(0, separador);
    for (const previo of Object.keys(variables))
      if (previo.toLowerCase() === nombre.toLowerCase()) delete variables[previo];
    variables[nombre] = linea.slice(separador + 1);
  }
  return variables;
}

async function compilar(distribucion = false, habitacion = 'inicial') {
  const aplicacion = rutaJuego(raiz, 'trabajo', distribucion ? 'aplicacion_beta' : 'aplicacion');
  mkdirSync(aplicacion, { recursive: true });
  const versionJuego = JSON.parse(readFileSync(rutaJuego(raiz, 'package.json'), 'utf8')).version;
  if (!/^\d+\.\d+\.\d+(?:-[a-zA-Z0-9.-]+)?$/.test(versionJuego))
    throw new Error('Versión del juego inválida.');
  const opl2 = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/herramientas', 'dependencias_opl2.json'), 'utf8'),
  );
  for (const [archivo, huella] of Object.entries(opl2.sha256))
    if (
      createHash('sha256')
        .update(readFileSync(rutaJuego(raiz, 'terceros', 'ymfm', archivo)))
        .digest('hex') !== huella
    )
      throw new Error(`La fuente OPL2 no coincide con la revisión documentada: ${archivo}.`);
  if (process.platform !== 'win32') {
    ejecutar('cmake', ['-S', rutaJuego(raiz), '-B', aplicacion, '-DCMAKE_BUILD_TYPE=Release']);
    ejecutar('cmake', ['--build', aplicacion, '--config', 'Release']);
    const directo = join(aplicacion, 'curse-reborn');
    return existsSync(directo) ? directo : join(aplicacion, 'Release', 'curse-reborn');
  }
  const sdl = await preparar();
  prepararMarcos(raiz, aplicacion);
  prepararHabitaciones(raiz, aplicacion, distribucion ? habitacion : 'todas');
  prepararSonidosPC(raiz, aplicacion);
  prepararPresentacion(raiz, aplicacion);
  const variables = entornoCompilador();
  const mt32 = await prepararMt32(raiz, ejecutar, variables);
  const icono = rutaAssets(raiz, 'lanzador', 'icono', 'curse-reborn.ico');
  const carpetaIcono = join(aplicacion, 'assets', 'lanzador', 'icono');
  mkdirSync(carpetaIcono, { recursive: true });
  copyFileSync(
    rutaAssets(raiz, 'lanzador', 'icono', 'curse-reborn.png'),
    join(carpetaIcono, 'curse-reborn.png'),
  );
  const recursoIcono = join(aplicacion, 'icono.rc'),
    objetoIcono = join(aplicacion, 'icono.res');
  writeFileSync(recursoIcono, `1 ICON "${icono.replaceAll('\\', '/')}"\n`);
  ejecutar(process.env.RC || 'llvm-rc', ['/FO', objetoIcono, recursoIcono], variables);
  console.log('Compilando Curse Reborn con SDL…');
  ejecutar(
    process.env.CXX || 'clang++',
    [
      '--target=x86_64-pc-windows-msvc',
      `-DCURSE_REBORN_VERSION="${versionJuego}"`,
      '-std=c++17',
      '-O2',
      ...(distribucion ? ['-DNDEBUG', '-ffunction-sections', '-fdata-sections'] : []),
      '-Wall',
      '-Wextra',
      '-Werror',
      '-fno-exceptions',
      '-fno-rtti',
      '-fuse-ld=lld',
      '-Xlinker',
      distribucion ? '/subsystem:windows' : '/subsystem:console',
      ...(distribucion ? ['-Xlinker', '/opt:ref', '-Xlinker', '/opt:icf'] : []),
      '-I',
      join(sdl, 'include'),
      '-I',
      join(mt32, 'include'),
      '-I',
      aplicacion,
      rutaJuego(raiz, 'plataformas', 'sdl', 'principal.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'video', 'filtros.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'video', 'marcos.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'lanzador', 'habitacion.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'interfaz', 'partidas.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'interfaz', 'contador_fps.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'tiempo_juego.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'mazmorra.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'introduccion.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'audio.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'musica_adlib.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'musica_mt32.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'opl.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'opl2.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'ymfm_opl.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'ymfm_adpcm.cpp'),
      rutaJuego(raiz, 'plataformas', 'sdl', 'audio', 'ymfm_pcm.cpp'),
      ...obtenerFuentesMotor(raiz),
      join(sdl, 'lib', 'x64', 'SDL3.lib'),
      objetoIcono,
      '-o',
      join(aplicacion, 'curse-reborn.exe'),
    ],
    variables,
  );
  copyFileSync(join(mt32, 'bin', 'mt32emu-2.dll'), join(aplicacion, 'mt32emu.dll'));
  copyFileSync(
    join(mt32, 'share', 'doc', 'munt', 'libmt32emu', 'COPYING.txt'),
    join(aplicacion, 'LICENCIA_GPL_MT32EMU.txt'),
  );
  copyFileSync(
    join(mt32, 'share', 'doc', 'munt', 'libmt32emu', 'COPYING.LESSER.txt'),
    join(aplicacion, 'LICENCIA_MT32EMU.txt'),
  );
  copyFileSync(
    rutaJuego(raiz, 'terceros', 'ymfm', 'LICENSE'),
    join(aplicacion, 'LICENCIA_YMFM.txt'),
  );
  copyFileSync(join(sdl, 'lib', 'x64', 'SDL3.dll'), join(aplicacion, 'SDL3.dll'));
  copyFileSync(join(sdl, 'LICENSE.txt'), join(aplicacion, 'LICENCIA_SDL3.txt'));
  copyFileSync(
    rutaJuego(raiz, 'terceros', 'nuked_opl3', 'LICENSE'),
    join(aplicacion, 'LICENCIA_NUKED_OPL3.txt'),
  );
  return join(aplicacion, 'curse-reborn.exe');
}

// Convierte la referencia indexada a RGB para compararla con una captura SDL.
function leerReferencia(escena = false, animada = false, ruta = null) {
  const archivo = readFileSync(
    ruta ??
      (escena
        ? join(
            raiz,
            'apps',
            'juego',
            'trabajo',
            'comparacion_objetos',
            animada ? 'mazmorra_animada_cpp.png' : 'mazmorra_objetos_cpp.png',
          )
        : join(raiz, 'apps', 'juego', 'trabajo', 'vista_titulo', 'titulo_cpp.png')),
  );
  let posicion = 8,
    paleta,
    cabecera;
  const comprimidos = [];
  while (posicion < archivo.length) {
    const longitud = archivo.readUInt32BE(posicion);
    const tipo = archivo.toString('ascii', posicion + 4, posicion + 8);
    const contenido = archivo.subarray(posicion + 8, posicion + 8 + longitud);
    if (tipo === 'IHDR') cabecera = contenido;
    if (tipo === 'PLTE') paleta = contenido;
    if (tipo === 'IDAT') comprimidos.push(contenido);
    posicion += longitud + 12;
  }
  if (
    !cabecera ||
    cabecera.readUInt32BE(0) !== 320 ||
    cabecera.readUInt32BE(4) !== 200 ||
    cabecera[8] !== 8 ||
    cabecera[9] !== 3 ||
    !paleta
  ) {
    throw new Error('La referencia PNG no tiene el formato esperado.');
  }
  const indices = inflateSync(Buffer.concat(comprimidos));
  const rgb = Buffer.alloc(64000 * 3);
  for (let y = 0; y < 200; ++y) {
    if (indices[y * 321] !== 0) throw new Error('Filtro PNG inesperado en la referencia.');
    for (let x = 0; x < 320; ++x)
      paleta.copy(
        rgb,
        (y * 320 + x) * 3,
        indices[y * 321 + x + 1] * 3,
        indices[y * 321 + x + 1] * 3 + 3,
      );
  }
  return rgb;
}

function leerCaptura(archivo, ancho = 320, alto = 200) {
  const bmp = readFileSync(archivo);
  if (
    bmp.toString('ascii', 0, 2) !== 'BM' ||
    bmp.readInt32LE(18) !== ancho ||
    Math.abs(bmp.readInt32LE(22)) !== alto ||
    bmp.readUInt16LE(28) !== 24 ||
    bmp.readUInt32LE(30) !== 0
  )
    throw new Error('La captura BMP no tiene el formato RGB esperado.');
  const inicio = bmp.readUInt32LE(10),
    invertida = bmp.readInt32LE(22) > 0;
  const rgb = Buffer.alloc(ancho * alto * 3);
  const paso_fila = Math.ceil((ancho * 3) / 4) * 4;
  if (inicio + paso_fila * alto > bmp.length)
    throw new Error('La captura BMP tiene filas incompletas.');
  for (let y = 0; y < alto; ++y)
    for (let x = 0; x < ancho; ++x) {
      const origen = inicio + (invertida ? alto - 1 - y : y) * paso_fila + x * 3;
      const destino = (y * ancho + x) * 3;
      rgb[destino] = bmp[origen + 2];
      rgb[destino + 1] = bmp[origen + 1];
      rgb[destino + 2] = bmp[origen];
    }
  return rgb;
}

// Prepara la dependencia, compila y abre o comprueba la presentación nativa solicitada.
// Reúne hitos y preparadores del valle para añadir salas sin duplicar el recorrido común.
function configurarComprobacionValle(argumentos) {
  const recorridos = [
    {
      argumento: '--remoto',
      nombre: 'remoto',
      referencia: 'ramas_valle',
      prefijo: 'borde_',
      comparadores: ['comparar:ramas-valle'],
      hitos: ['remote_entrada', 'remote_reaccion', 'remote_rechazo'],
      alcance: 'entrada60 enREMOTE, reacción y rechazo108; tres imágenes y restauración',
    },
    {
      argumento: '--celeste',
      nombre: 'celeste',
      referencia: 'ramas_valle',
      prefijo: 'borde_',
      comparadores: ['comparar:ramas-valle'],
      hitos: ['celeste_entrada', 'post_regreso_rama', 'post_objeto_111'],
      alcance: 'entrada56 enCELESTE, regreso61 y recogida111; tres imágenes y restauración',
    },
    {
      argumento: '--dinosaurio',
      nombre: 'dinosaurio',
      referencia: 'dinosaurio',
      prefijo: 'borde_',
      comparadores: ['comparar:dinosaurio'],
      hitos: [
        'pelo_recogido',
        'dinosaurio_desde_post',
        'dinosaurio_estornudo',
        'dinosaurio_sello_boligrafo',
        'dinosaurio_regreso_post',
        'dinosaurio_desde_celeste',
      ],
      alcance:
        'pelo110, entrada57, estornudo, sello109/bolígrafo101, regreso58 y acceso54; seis imágenes y restauración',
    },
    {
      argumento: '--nube-valle',
      nombre: 'nube',
      referencia: 'nube',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:grabacion-completa', 'comparar:entrega-grabacion', 'comparar:nube'],
      hitos: ['nube_entrada', 'nube_impulso_alternado', 'nube_bolsa_recogida', 'nube_regreso_post'],
      alcance:
        'grabación, correo, pase, nube, bolsa121 y regreso67; cuatro imágenes y restauración',
    },
    {
      argumento: '--cueva-valle',
      nombre: 'cueva_valle',
      referencia: 'cueva_valle',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:cueva-valle'],
      hitos: ['cueva_entrada', 'cueva_derrumbe', 'cueva_detergente', 'cueva_regreso_dino'],
      alcance:
        'entrada64, petición de ayuda, detergente120 y regreso65; cuatro imágenes y restauración',
    },
    {
      argumento: '--barco-valle',
      nombre: 'barco_valle',
      referencia: 'puente_barco',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:guardia-barco', 'comparar:puente-barco'],
      hitos: [
        'barco_primer_tablon',
        'barco_tablon_horizontal',
        'barco_puente_completo',
        'barco_trapo_recogido',
        'barco_regreso_celeste',
      ],
      alcance:
        'calcetín, moneda, guardia, entrada62, ambos tablones, puente, trapo119 y regreso63; cinco imágenes y restauración',
    },
    {
      argumento: '--plataforma-valle',
      nombre: 'plataforma_valle',
      referencia: 'plataforma_valle',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:plataforma-valle'],
      hitos: [
        'plataforma_bandeja_recogida',
        'plataforma_puerta_dino',
        'plataforma_entrada',
        'plataforma_detergente',
        'plataforma_boton_descubierto',
        'plataforma_barrera_abierta',
        'plataforma_caida_izquierda',
        'plataforma_canicas',
        'plataforma_cruzada',
        'plataforma_caida_derecha',
        'plataforma_ventilador',
        'plataforma_regreso_campo',
      ],
      alcance:
        'bandeja, limpieza, botón, canicas, cruce, caídas recuperadas, ventilador y regreso69; doce imágenes y restauración',
    },
    {
      argumento: '--megafono-palacio',
      nombre: 'megafono_palacio',
      referencia: 'laterales_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'megafono_tirada',
        'megafono_puerta',
        'megafono_entrada',
        'megafono_ante_objeto',
        'megafono_recogido',
        'megafono_regreso',
        'megafono_dados',
      ],
      alcance: 'recogida152, puertas reales y regreso98; 7 imágenes y restauración',
    },
    {
      argumento: '--hielo-palacio',
      nombre: 'hielo_palacio',
      referencia: 'laterales_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'hielo_tirada',
        'hielo_puerta',
        'hielo_entrada',
        'hielo_ante_objeto',
        'hielo_recogido',
        'hielo_regreso',
        'hielo_dados',
      ],
      alcance: 'recogida153, puertas reales y regreso100; 7 imágenes y restauración',
    },
    {
      argumento: '--desenlace',
      nombre: 'desenlace',
      referencia: 'desenlace',
      prefijo: 'borde_valle_bruja_',
      comparadores: [],
      hitos: [
        'desenlace_entrada',
        'desenlace_16',
        'desenlace_32',
        'desenlace_48',
        'desenlace_reposo',
        'desenlace_final',
      ],
      alcance:
        'entrada125, inventario y temas vaciados, animación completa del campo y reposo; restauración',
    },
    {
      argumento: '--bruja',
      nombre: 'bruja',
      referencia: 'bruja',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'bruja_entrada',
        'bruja_fantasma',
        'bruja_aspiradora',
        'bruja_extintor',
        'bruja_ventilador',
        'bruja_anillo_32',
        'bruja_anillo_64',
      ],
      alcance: 'duelo72, aspiradora, extintor, ventilador y anillo; restauración y ruta real',
    },
    {
      argumento: '--biblioteca-castillo',
      nombre: 'biblioteca_castillo',
      referencia: 'biblioteca_castillo',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'biblioteca_entrada',
        'biblioteca_control',
        'biblioteca_regreso',
        'biblioteca_segunda_visita',
        'biblioteca_libro',
        'biblioteca_examinada',
        'biblioteca_abriendo',
        'biblioteca_abierta',
      ],
      alcance:
        'entrada122, regreso123, anillo conservado y pasadizo176; restauración y rutas reales',
    },
    {
      argumento: '--estudio-castillo',
      nombre: 'estudio_castillo',
      referencia: 'estudio_castillo',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'estudio_entrada',
        'estudio_control',
        'estudio_anillo',
        'estudio_recogido',
        'estudio_regreso',
        'estudio_segunda_visita',
      ],
      alcance: 'entrada120, anillo175, regreso121, segunda visita y restauración',
    },
    {
      argumento: '--vestibulo-castillo',
      nombre: 'vestibulo_castillo',
      referencia: 'vestibulo_castillo',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: ['vestibulo_entrada', 'vestibulo_control', 'vestibulo_izquierda', 'vestibulo_derecha'],
      alcance: 'entrada119, ambas escaleras, restauración y navegación entre vistas',
    },
    {
      argumento: '--puente-castillo',
      nombre: 'puente_castillo',
      referencia: 'puente_castillo',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'puente_castillo_entrada',
        'puente_castillo_32',
        'puente_castillo_64',
        'puente_castillo_80',
      ],
      alcance: 'entrada118, derrumbe del puente, canales originales y restauración',
    },
    {
      argumento: '--verja',
      nombre: 'verja',
      referencia: 'verja',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: ['verja_entrada', 'verja_control', 'verja_cerrada', 'verja_abriendo', 'verja_abierta'],
      alcance: 'entrada117, apertura174, canales originales y restauración',
    },
    {
      argumento: '--bosque',
      nombre: 'bosque',
      referencia: 'bosque',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: ['bosque_entrada', 'bosque_control', 'bosque_sendero'],
      alcance: 'entrada116, sendero de COPSE66, restauración y navegación entre vistas',
    },
    {
      argumento: '--cementerio',
      nombre: 'cementerio',
      referencia: 'cementerio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'cementerio_entrada',
        'cementerio_control',
        'cementerio_pala',
        'cementerio_vampiro_tumba',
        'cementerio_util_169',
        'cementerio_pala_vampiro',
        'cementerio_util_170',
        'cementerio_util_173',
        'cementerio_piedra',
        'cementerio_segundo',
        'cementerio_ajo',
        'cementerio_cruz',
        'cementerio_platillos',
        'cementerio_util_171',
        'cementerio_regreso',
        'cementerio_util_167',
        'cementerio_puerta',
      ],
      alcance:
        'cementerios65/77, útiles167–173, vampiros, piedra, ajo, cruz, regreso115 y verja; restauración y rutas reales',
    },
    {
      argumento: '--ataud',
      nombre: 'ataud',
      referencia: 'ataud',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'ataud_mago',
        'ataud_entrada',
        'ataud_control',
        'ataud_hueso',
        'ataud_excavar_32',
        'ataud_excavar_64',
      ],
      alcance:
        'tercer pago real al mago, entrada112 con modelo25, hueso165 y excavación; seis imágenes y restauración',
    },
    {
      argumento: '--guardia-palacio',
      nombre: 'guardia_palacio',
      referencia: 'guardia_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'guardia_entrada',
        'guardia_control',
        'guardia_columna',
        'guardia_dormido',
        'guardia_cerilla',
        'guardia_fuego_48',
        'guardia_fuego_96',
        'guardia_fuego_192',
        'guardia_regreso_benn',
        'guardia_benn_control',
      ],
      alcance:
        'columna, sueño, cerilla162, caja161, extintor164 y regreso111 a Benn; diez imágenes y restauración',
    },
    {
      argumento: '--laberinto-palacio',
      nombre: 'laberinto_palacio',
      referencia: 'laberinto_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'laberinto_59_entrada',
        'laberinto_59_control',
        'laberinto_61_entrada',
        'laberinto_61_control',
        'laberinto_58_entrada',
        'laberinto_58_control',
        'laberinto_60_entrada',
        'laberinto_60_control',
        'laberinto_62_entrada',
        'laberinto_62_control',
        'laberinto_62_ante_cerillas',
        'laberinto_62_cerillas',
        'laberinto_59_con_cerillas',
      ],
      alcance:
        'cuatro salas, rutas con azar original, retorno a la mano y recogida161/162; trece imágenes, restauración y salida107 pendiente',
    },
    {
      argumento: '--mano-palacio',
      nombre: 'mano_palacio',
      referencia: 'mano_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'mano_entrada',
        'mano_control',
        'mano_atrapado',
        'mano_libera_8',
        'mano_libera_24',
        'mano_libera_40',
        'mano_liberado',
      ],
      alcance:
        'entrada104, captura por la mano, aceite144 y liberación; siete imágenes y restauración; destino105 pendiente',
    },
    {
      argumento: '--reina-hielo',
      nombre: 'reina_hielo',
      referencia: 'reina_hielo',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'reina_entrada',
        'reina_control',
        'reina_salto_temprano',
        'reina_golpe',
        'reina_recuperada',
        'reina_preparada',
        'reina_salto_primero',
        'reina_entre_saltos',
        'reina_salto_segundo',
        'reina_trono',
        'reina_expulsion',
      ],
      alcance:
        'entrada103, salto temprano, golpe y recuperación, dos proyectiles esquivados y trono; once imágenes y restauración; destino104 pendiente',
    },
    {
      argumento: '--laser-palacio',
      nombre: 'laser_palacio',
      referencia: 'laser_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'laser_entrada',
        'laser_control',
        'laser_pieza_erronea',
        'laser_pieza_recuperada',
        'laser_lupa',
        'laser_cubito',
        'laser_carambano',
        'laser_cristal',
        'laser_reja_32',
      ],
      alcance:
        'entrada90, error y recuperación de pieza, cuatro reflectores, explosión y gato bajo la reja; nueve imágenes y restauración; destino103 pendiente',
    },
    {
      argumento: '--resolucion-palacio',
      nombre: 'resolucion_palacio',
      referencia: 'resolucion_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'poleas_con_arma',
        'poleas_bajadas',
        'poleas_ante_utiles',
        'poleas_aceite',
        'poleas_cristal',
        'ventana_con_utiles',
        'ventana_ante_silbato',
        'ventana_silbato',
        'ventana_megafono',
        'ventana_abierta',
      ],
      alcance:
        'útiles reales, poleas, silbato, combinación y apertura de ventana; diez imágenes y restauración; continuación90 pendiente',
    },
    {
      argumento: '--carambano-palacio',
      nombre: 'carambano_palacio',
      referencia: 'laterales_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'carambano_tirada',
        'carambano_puerta',
        'carambano_entrada',
        'carambano_bajado',
        'carambano_ante_objeto',
        'carambano_recogido',
        'carambano_regreso',
        'carambano_dados',
        'laterales_utiles',
      ],
      alcance: 'recogida154, puertas reales y regreso102; 9 imágenes y restauración',
    },
    {
      argumento: '--arma-palacio',
      nombre: 'arma_palacio',
      referencia: 'laterales_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'arma_tirada',
        'arma_puerta',
        'arma_entrada',
        'arma_bajado',
        'arma_ante_objeto',
        'arma_recogido',
        'arma_regreso',
        'arma_dados',
      ],
      alcance:
        'entrada95, escoba141 que baja151, recogida y regreso96; ocho imágenes y restauración',
    },
    {
      argumento: '--lupa-palacio',
      nombre: 'lupa_palacio',
      referencia: 'utiles_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'lupa_tirada',
        'lupa_puerta',
        'lupa_entrada',
        'lupa_ante_objeto',
        'lupa_recogido',
        'lupa_regreso',
        'utiles_dados',
      ],
      alcance:
        'tiradas persistentes, entrada93, lupa150, regreso94 y vuelta a los dados; siete imágenes y restauración',
    },
    {
      argumento: '--gato-palacio',
      nombre: 'gato_palacio',
      referencia: 'utiles_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'gato_tirada',
        'gato_puerta',
        'gato_entrada',
        'gato_ante_objeto',
        'gato_recogido',
        'gato_regreso',
      ],
      alcance: 'tiradas reales, entrada91, gato149 y regreso92; seis imágenes y restauración',
    },
    {
      argumento: '--ventana-palacio',
      nombre: 'ventana_palacio',
      referencia: 'corredores_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'hielo_pasillo3',
        'hielo_ante_ventana',
        'hielo_ventana',
        'hielo_ventana_cerrada',
        'hielo_vuelta2',
        'hielo_fin_corredores',
      ],
      alcance:
        'entrada88, inspección146, salto rechazado sin abrir la ventana y regreso89; seis imágenes y restauración',
    },
    {
      argumento: '--pasillo-hielo',
      nombre: 'pasillo_hielo',
      referencia: 'corredores_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: ['hielo_pasillo2', 'hielo_vuelta1'],
      alcance: 'entrada86 y regreso87 con puertas persistentes; dos imágenes y restauración',
    },
    {
      argumento: '--poleas-palacio',
      nombre: 'poleas_palacio',
      referencia: 'corredores_palacio',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'poleas_entrada',
        'poleas_ante_inspeccion',
        'poleas_inspeccion',
        'poleas_regreso_dados',
        'poleas_regreso',
      ],
      alcance: 'inspección143 y regreso persistente a los dados; cinco imágenes y restauración',
    },
    {
      argumento: '--dados-palacio',
      nombre: 'dados_palacio',
      referencia: 'dados_palacio',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:dados-palacio'],
      hitos: [
        'dados_entrada',
        'dados_control',
        'dados_ante_escoba',
        'dados_escoba',
        'dados_ante_mesa',
        'dados_recogidos',
        'dados_tirada',
        'dados_retirada',
        'dados_segunda_tirada',
      ],
      alcance:
        'escoba, dados, puerta y segunda tirada con azar persistente; nueve imágenes y restauración',
    },
    {
      argumento: '--entrada-palacio',
      nombre: 'entrada_palacio',
      referencia: 'entrada_palacio',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:entrada-palacio'],
      hitos: [
        'palacio_entrada',
        'palacio_control',
        'palacio_ante_botones',
        'palacio_error_139',
        'palacio_boton_136',
        'palacio_boton_137',
        'palacio_boton_138',
        'palacio_boton_139',
      ],
      alcance:
        'llegada82, error y secuencia de los cuatro carámbanos; ocho imágenes y restauración',
    },
    {
      argumento: '--lago-helado',
      nombre: 'lago_helado',
      referencia: 'lago_helado',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:lago-helado'],
      hitos: [
        'lago_entrada',
        'lago_control',
        'lago_ante_cenizas',
        'lago_salto135',
        'lago_cenizas',
        'lago_criatura_64',
      ],
      alcance:
        'rescate y llegada81, salto recuperado y cenizas que llaman a la criatura; seis imágenes y restauración',
    },
    {
      argumento: '--rescate-nieve',
      nombre: 'rescate_nieve',
      referencia: 'morsa',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: [
        'rescate_entrada',
        'rescate_ante_tablon',
        'rescate_tablon',
        'esquimal_con_tablon',
        'esquimal_tablon_colocado',
        'esquimal_pez',
        'esquimal_entrega',
        'esquimal_cana',
        'morsa_con_cana',
        'morsa_amigo_liberado',
        'morsa_cenizas',
        'rescate_amigos_listos',
        'rescate_cornisa',
        'rescate_ayuda',
      ],
      alcance:
        'tablón, pez, entrega al esquimal, caña, hielo, cenizas y cornisa; catorce imágenes y restauración',
    },
    {
      argumento: '--esquimal',
      nombre: 'esquimal',
      referencia: 'morsa',
      prefijo: 'borde_valle_',
      comparadores: [],
      hitos: ['esquimal_entrada'],
      alcance:
        'entrada78 desde el perfume real y conservación de la partida; una imagen y restauración; el puzle continúa con el tablón de40',
    },
    {
      argumento: '--morsa',
      nombre: 'morsa',
      referencia: 'morsa',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:morsa'],
      hitos: [
        'morsa_entrada',
        'morsa_ante_desodorante',
        'morsa_desodorante',
        'morsa_perfumado',
        'morsa_ante_saludo',
        'morsa_saludada',
      ],
      alcance: 'entrada80, desodorante, uso y saludo a la morsa; seis imágenes y restauración',
    },
    {
      argumento: '--ser-azul',
      nombre: 'ser_azul',
      referencia: 'ser_azul',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:ser-azul'],
      hitos: ['nieve_control', 'nieve_ante_bola', 'nieve_bola_recogida', 'nieve_ser_liberado'],
      alcance:
        'entrada74 desde vestido real, recogida134, lanzamiento y transformación; cuatro imágenes y restauración',
    },
    {
      argumento: '--regreso-benn',
      nombre: 'regreso_benn',
      referencia: 'regreso_benn',
      prefijo: 'borde_valle_',
      comparadores: ['comparar:regreso-benn'],
      hitos: [
        'benn_regreso',
        'benn_frente_vendedor',
        'benn_pago_real',
        'benn_vestido_real',
        'probador_regreso_valle',
        'probador_vestido_real',
      ],
      alcance:
        'regreso después del ventilador, pago real aBenn, vestido79 y probador; seis imágenes y restauración. No carga la sala43.',
    },
  ];
  return recorridos.find((recorrido) => argumentos.includes(recorrido.argumento));
}

export async function gestionarSdl(accion, argumentos) {
  const idiomas = argumentos.filter((valor) => valor === 'es' || valor === 'en');
  if (
    idiomas.length > 1 ||
    (idiomas.length && !['ventana', 'comprobar:informacion'].includes(accion))
  )
    throw new Error('Indica un solo idioma: en o es.');
  const idioma = idiomas[0] ?? 'en';
  argumentos = argumentos.filter((valor) => valor !== 'es' && valor !== 'en');
  const sinSonido =
    accion === 'ventana' && argumentos.length === 1 && argumentos[0] === '--sin-sonido';
  const comprobarCalles =
    accion === 'comprobar:poblacion' &&
    argumentos.length === 1 &&
    [
      '--calles',
      '--tiendas',
      '--probador',
      '--entrada-borde',
      '--borde',
      '--regreso-loro',
      '--poste',
      '--remoto',
      '--celeste',
      '--dinosaurio',
      '--nube-valle',
      '--cueva-valle',
      '--barco-valle',
      '--plataforma-valle',
      '--regreso-benn',
      '--ser-azul',
      '--morsa',
      '--esquimal',
      '--rescate-nieve',
      '--lago-helado',
      '--entrada-palacio',
      '--dados-palacio',
      '--poleas-palacio',
      '--pasillo-hielo',
      '--ventana-palacio',
      '--gato-palacio',
      '--lupa-palacio',
      '--arma-palacio',
      '--carambano-palacio',
      '--resolucion-palacio',
      '--laser-palacio',
      '--reina-hielo',
      '--mano-palacio',
      '--laberinto-palacio',
      '--guardia-palacio',
      '--ataud',
      '--cementerio',
      '--bosque',
      '--verja',
      '--puente-castillo',
      '--vestibulo-castillo',
      '--estudio-castillo',
      '--biblioteca-castillo',
      '--bruja',
      '--desenlace',
      '--hielo-palacio',
      '--megafono-palacio',
    ].includes(argumentos[0]);
  const distribucion =
    accion === 'sdl:compilar' &&
    argumentos[0] === '--distribucion' &&
    (argumentos.length === 1 ||
      (argumentos.length === 3 &&
        argumentos[1] === '--habitacion' &&
        /^[a-z0-9][a-z0-9_-]*$/.test(argumentos[2])));
  const habitacionDistribucion =
    distribucion && argumentos.length === 3 ? argumentos[2] : 'inicial';
  if (
    argumentos.length &&
    !sinSonido &&
    !comprobarCalles &&
    !distribucion &&
    accion !== 'exportar:musica'
  )
    throw new Error('Solo ventana admite el argumento --sin-sonido.');
  if (accion === 'sdl:preparar') {
    if (await preparar()) console.log('Dependencia SDL preparada.');
    return;
  }
  if (accion === 'comprobar:vistas') {
    const programa = await compilar();
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_vistas');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'ultimo_fotograma.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    return;
  }
  if (accion === 'exportar:musica') {
    const segundos =
      argumentos.length === 0
        ? 0
        : argumentos.length === 2 && argumentos[0] === '--segundos'
          ? Number(argumentos[1])
          : NaN;
    if (
      !Number.isInteger(segundos) ||
      segundos < 0 ||
      segundos > 600 ||
      (argumentos.length && segundos === 0)
    )
      throw new Error('Uso: pnpm exportar:musica [--segundos 1…600].');
    const programa = await compilar();
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'exportacion_musica');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--exportar-musica', carpeta, String(segundos)]);
    const pistas = [];
    for (let tema = 0; tema < 2; tema++)
      for (const chip of ['adlib_opl2', 'sb16_opl3'])
        pistas.push({ archivo: `${chip}_tema_${tema}.wav`, canales: 1, tema_interno: tema });
    for (let banco = 1; banco <= 5; banco++)
      pistas.push({ archivo: `mt32_curse_rs${banco}.wav`, canales: 2, banco_rs: banco });
    for (const pista of pistas) {
      const wav = readFileSync(join(carpeta, pista.archivo));
      if (
        wav.toString('ascii', 0, 4) !== 'RIFF' ||
        wav.toString('ascii', 8, 12) !== 'WAVE' ||
        wav.readUInt16LE(22) !== pista.canales ||
        wav.readUInt32LE(24) !== 48000 ||
        wav.readUInt16LE(34) !== 16 ||
        wav.readUInt32LE(40) !== wav.length - 44 ||
        wav.readUInt32LE(40) === 0 ||
        (segundos && wav.readUInt32LE(40) !== segundos * 48000 * pista.canales * 2)
      )
        throw new Error(`WAV exportado incorrecto: ${pista.archivo}.`);
      pista.sha256 = createHash('sha256').update(wav).digest('hex');
      pista.muestras_por_canal = wav.readUInt32LE(40) / (pista.canales * 2);
      pista.duracion_segundos = pista.muestras_por_canal / 48000;
    }
    writeFileSync(
      join(carpeta, 'pistas.json'),
      JSON.stringify(
        {
          duracion_fija_segundos: segundos || null,
          modo: segundos ? 'duración elegida' : 'entrada y primer recorrido de bucles originales',
          frecuencia_hz: 48000,
          bits: 16,
          pistas,
          origen: 'CURSE.EXE y recursos originales locales; las pistas conservan sus bucles.',
          nota: 'Por defecto RS termina en su primer salto de repetición; FM termina cuando todos sus canales han completado un recorrido. --segundos fuerza un corte de prueba. El tema FM cero es auxiliar; el original arranca con el uno. Sin efectos ni volumen de la partida.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log(`Nueve referencias WAV exportadas: ${carpeta}`);
    return;
  }
  if (accion === 'comprobar:mt32') {
    const programa = await compilar();
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_musica_rs');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-mt32', join(carpeta, 'mazmorra_mt32.wav')], {
      ...process.env,
      SDL_AUDIO_DRIVER: 'dummy',
    });
    return;
  }
  if (accion === 'comprobar:musica') {
    const programa = await compilar();
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_musica');
    mkdirSync(carpeta, { recursive: true });
    const wav = join(carpeta, 'musica_adlib.wav');
    ejecutar(programa, [datos, '--comprobar-musica', wav], {
      ...process.env,
      SDL_AUDIO_DRIVER: 'dummy',
    });
    console.log(`Muestra de música: ${wav}`);
    return;
  }
  if (accion === 'comprobar:digital') {
    const programa = await compilar();
    ejecutar(
      programa,
      [
        datos,
        '--comprobar-digital',
        join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_digital', 'efectos_digitales.wav'),
      ],
      {
        ...process.env,
        SDL_AUDIO_DRIVER: 'dummy',
      },
    );
    return;
  }
  if (accion === 'comprobar:audio') {
    const programa = await compilar();
    ejecutar(
      programa,
      [
        datos,
        '--comprobar-audio',
        join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_audio', 'efecto_original.wav'),
      ],
      { ...process.env, SDL_AUDIO_DRIVER: 'dummy' },
    );
    return;
  }
  if (accion !== 'sdl:compilar') {
    const necesarios = ['TITLE.DAT', 'TITLE.PAL', 'CORE.DAT', 'CORE.PAL'];
    if (
      accion === 'ventana' ||
      accion === 'comprobar:escena' ||
      accion === 'comprobar:introduccion' ||
      accion === 'comprobar:menu' ||
      accion === 'comprobar:informacion' ||
      accion === 'comprobar:campo' ||
      accion === 'comprobar:asaltante' ||
      accion === 'comprobar:poblacion' ||
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion) ||
      accion === 'comprobar:puzle' ||
      accion === 'comprobar:jugabilidad'
    )
      necesarios.push(
        'DUNGEON.MAP',
        'DUNGEON.SPR',
        ...(accion === 'ventana' ||
        accion === 'comprobar:caida' ||
        accion === 'comprobar:corredor' ||
        accion === 'comprobar:lago' ||
        accion === 'comprobar:poza' ||
        [
          'comprobar:cuevas',
          'comprobar:catapulta',
          'comprobar:campo',
          'comprobar:asaltante',
          'comprobar:poblacion',
          'comprobar:final-cuevas',
          'comprobar:partidas',
        ].includes(accion)
          ? ['CORRIDOR.MAP', 'CORRIDOR.SPR']
          : []),
        'MANA.BRD',
        'STD.BRD',
        'SPRITES.PAL',
        'MENU.DAT',
        'ICONS.SPR',
        'MOUSE.SPR',
        'FONT.SPR',
        'BOXDET.DAT',
      );
    if (
      accion === 'ventana' ||
      accion === 'comprobar:escena' ||
      accion === 'comprobar:introduccion' ||
      accion === 'comprobar:menu' ||
      accion === 'comprobar:informacion' ||
      accion === 'comprobar:campo' ||
      accion === 'comprobar:asaltante' ||
      accion === 'comprobar:poblacion' ||
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion) ||
      accion === 'comprobar:puzle' ||
      accion === 'comprobar:jugabilidad'
    ) {
      necesarios.push('CAULDRN.PAL', 'BALCONY.PAL', 'BASEBALL.PAL', 'BASEBAT.DAT');
      for (let i = 1; i <= 4; ++i) necesarios.push(`CAULDRN${i}.DAT`);
      for (let i = 1; i <= 5; ++i) necesarios.push(`BALCONY${i}.DAT`);
      for (let i = 1; i <= 6; ++i) necesarios.push(`BASEBAL${i}.DAT`);
    }
    if (
      accion === 'ventana' ||
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push(
        'DROP.MAP',
        'DROP.SPR',
        'LAKEBED.MAP',
        'LAKEBED.SPR',
        'PLUNGE.BRD',
        'PLOK.BRD',
      );
    if (
      accion === 'ventana' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push('POOL.MAP', 'POOL.SPR', 'EMERGE.BRD');
    if (
      accion === 'ventana' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push('CAVECOR.MAP', 'CAVECOR.SPR');
    if (
      accion === 'ventana' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push('WELL.MAP', 'WELL.SPR', 'CAVERBMI.MAP', 'CAVERBMI.SPR');
    if (
      accion === 'ventana' ||
      [
        'comprobar:campo',
        'comprobar:asaltante',
        'comprobar:poblacion',
        'comprobar:catapulta',
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push(
        'SEESAW.MAP',
        'SEESAW.SPR',
        'CAVE.MAP',
        'CAVE.SPR',
        'SHAFT.MAP',
        'SHAFT.SPR',
        'COUNTRY1.MAP',
        'COUNTRY1.SPR',
        'COUNTRY2.MAP',
        'COUNTRY2.SPR',
        'DRAGON.MAP',
        'DRAGON.SPR',
      );
    if (accion === 'ventana' || accion === 'comprobar:poblacion')
      necesarios.push(
        ...[
          'FRANKS',
          'XROADS',
          'SORCERY',
          'MAGE',
          'EDGE',
          'ROGUE',
          'GYPSY',
          'DERELICT',
          'INNSIDE',
        ].flatMap((banco) => [banco + '.MAP', banco + '.SPR']),
        'SMALL.BRD',
      );
    const faltantes = necesarios.filter((nombre) => !existsSync(join(datos, nombre)));
    if (faltantes.length)
      throw new Error(`Faltan ${faltantes.join(', ')} en apps/juego/datos_originales/CURSE/DATA.`);
    if (
      accion === 'ventana' ||
      accion === 'comprobar:escena' ||
      accion === 'comprobar:introduccion' ||
      accion === 'comprobar:menu' ||
      accion === 'comprobar:informacion' ||
      accion === 'comprobar:campo' ||
      accion === 'comprobar:asaltante' ||
      accion === 'comprobar:poblacion' ||
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      accion === 'comprobar:cuevas' ||
      accion === 'comprobar:puzle' ||
      accion === 'comprobar:jugabilidad'
    ) {
      const ejecutable = join(datos, '..', 'CURSE.EXE');
      if (!existsSync(ejecutable))
        throw new Error(
          'Falta CURSE.EXE junto a la carpeta DATA: se necesitan sus tablas de objetos y guiones.',
        );
      const mapa = JSON.parse(
        readFileSync(join(raiz, 'apps/juego/documentacion', 'mapa_ejecutable.json'), 'utf8'),
      );
      if (createHash('sha256').update(readFileSync(ejecutable)).digest('hex') !== mapa.sha256)
        throw new Error('La huella de CURSE.EXE no coincide con la versión reconstruida.');
    }
  }
  const programa = await compilar(distribucion, habitacionDistribucion);
  if (accion === 'sdl:compilar') {
    console.log(`Aplicación compilada: ${programa}`);
    return;
  }
  if (accion === 'ventana') {
    console.log(
      'Habitación inicial: pulsa el botón del PC para encender. Una vez cargado el juego, pulsa el botón del monitor para acercarlo; dentro de la pantalla puedes jugar en pequeño. Ctrl+R vuelve a la habitación y Ctrl+H cambia de diseño. F5/F6 recorren las vistas de desarrollo; M silencia/reactiva el sonido. Consulta apps/juego/documentacion/controles.md para las demás opciones.',
    );
    ejecutar(
      programa,
      [datos, idioma, ...(sinSonido ? ['--sin-sonido'] : [])],
      process.env,
      false,
      false,
    );
    return;
  }
  if (accion === 'comprobar:caida') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:caida',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_caida_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['caida-16', 'caida_16'],
      ['caida-32', 'caida_32'],
      ['agua-24', 'agua_24'],
    ]) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, `--comprobar-${opcion}`, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_caida', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de las instrucciones originales.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify({ resultado: 'coinciden', capturas: 3, pixeles_por_captura: 64000 }, null, 2) +
        '\n',
    );
    console.log('Caída SDL: coinciden dos fases de DROP y la llegada sumergida a LAKEBED.');
    return;
  }
  if (accion === 'comprobar:avisos') {
    const programa = await compilar();
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_avisos');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-avisos', join(carpeta, 'aviso')], {
      ...process.env,
      SDL_VIDEODRIVER: 'dummy',
    });
    console.log('Avisos SDL: letras blancas, fundido, desaparición y encuadre conservado.');
    return;
  }
  if (accion === 'comprobar:tiempo') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_tiempo');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-tiempo', carpeta]);
    return;
  }
  if (accion === 'comprobar:reinicio') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_reinicio');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-reinicio', join(carpeta, 'monitor')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
      SDL_AUDIODRIVER: 'dummy',
    });
    return;
  }
  if (accion === 'comprobar:monitor' || accion === 'comprobar:habitaciones') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_monitor');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-monitor', join(carpeta, 'monitor')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
      SDL_AUDIODRIVER: 'dummy',
    });
    ejecutar(programa, [datos, '--comprobar-arranque', join(carpeta, 'arranque_real.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
      SDL_AUDIODRIVER: 'dummy',
      CURSE_REBORN_TIEMPO: join(carpeta, 'tiempo_prueba.json'),
      CURSE_REBORN_PARTIDAS: join(carpeta, 'perfil_prueba.sav'),
    });
    return;
  }
  if (accion === 'comprobar:cursor') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_cursor');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-cursor', join(carpeta, 'mano_original.png')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
      CURSE_REBORN_PARTIDAS: join(carpeta, 'perfil_cursor.sav'),
    });
    return;
  }
  if (accion === 'comprobar:decorados') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_decorados');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-decorados', join(carpeta, 'frank')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    return;
  }
  if (accion === 'comprobar:fps') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_fps');
    mkdirSync(carpeta, { recursive: true });
    const prefijo = join(carpeta, 'fps');
    ejecutar(programa, [datos, '--comprobar-fps', prefijo], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    const capturas = Array.from({ length: 3 }, (_, i) =>
      leerCaptura(`${prefijo}.${i}.bmp`, 1000, 700),
    );
    for (const indice of [0, 2]) {
      let cambios = 0;
      for (let y = 0; y < 700; y++)
        for (let x = 0; x < 1000; x++)
          for (let c = 0; c < 3; c++) {
            const p = (y * 1000 + x) * 3 + c;
            if (capturas[indice][p] !== capturas[1][p]) {
              cambios++;
              if (x < 912 || x >= 992 || y < 8 || y >= 26)
                throw new Error('El contador FPS modifica el juego fuera de su placa.');
            }
          }
      if (!cambios) throw new Error('F12 no cambia la visibilidad del contador.');
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          tecla: 'F12',
          capturas: 3,
          resolucion: [1000, 700],
          juego_y_bandas_conservados: true,
          contador: 'fotogramas de presentación por segundo; no modifica el reloj del juego',
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'FPS SDL: F12 muestra y oculta el indicador; juego y bandas negras se conservan fuera de la placa.',
    );
    return;
  }
  if (accion === 'comprobar:filtros') {
    const prueba = join(
      aplicacion,
      process.platform === 'win32' ? 'escaladores.exe' : 'escaladores',
    );
    ejecutar(
      process.env.CXX || 'clang++',
      [
        '-std=c++17',
        '-O2',
        '-Wall',
        '-Wextra',
        '-Werror',
        join(raiz, 'apps/juego/pruebas/video', 'comparar_escaladores.cpp'),
        '-o',
        prueba,
      ],
      process.platform === 'win32' ? entornoCompilador() : process.env,
    );
    ejecutar(prueba, []);
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'titulo',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_filtros');
    mkdirSync(carpeta, { recursive: true });
    const prefijo = join(carpeta, 'filtro');
    ejecutar(programa, [datos, '--comprobar-filtros', prefijo], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    const capturas = Array.from({ length: 7 }, (_, i) =>
      leerCaptura(`${prefijo}.${i}.bmp`, 960, 600),
    );
    if (!capturas[0].equals(capturas[6]))
      throw new Error('El ciclo F11 no recupera la imagen original.');
    if (
      new Set(capturas.slice(0, 6).map((rgb) => createHash('sha256').update(rgb).digest('hex')))
        .size !== 6
    )
      throw new Error('Los seis modos de pantalla no producen imágenes diferentes.');
    const original = Buffer.alloc(64000 * 3);
    for (let y = 0; y < 200; y++)
      for (let x = 0; x < 320; x++) {
        const origen = ((y * 3 + 1) * 960 + x * 3 + 1) * 3;
        capturas[0].copy(original, (y * 320 + x) * 3, origen, origen + 3);
      }
    if (!original.equals(leerReferencia()))
      throw new Error('El modo sin filtro cambia los píxeles originales.');
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          tecla: 'F11',
          modos: ['original', 'suavizado', 'líneas de escaneo', 'CRT suave', 'xBRZ 2x', '2xSaI'],
          capturas: 7,
          resolucion: [960, 600],
          original_conservado: true,
          ciclo_reversible: true,
          pantalla_completa_sin_bordes: true,
          regreso_a_ventana: true,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Filtros SDL: seis modos diferentes, ciclo F11 reversible y 64.000 píxeles originales conservados.',
    );
    return;
  }
  if (accion === 'comprobar:marcos') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_marcos');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, ancho, alto] of [
      ['marcos-cielo-32', 1920, 540],
      ['marcos-cielo-24', 1440, 540],
      ['marcos-cielo-4', 960, 720],
      ['marcos', 1280, 720],
      ['marcos-vertical', 720, 1280],
      ['marcos-ajustado', 960, 600],
      ['marcos-impar', 1279, 721],
      ['marcos-filtro', 1280, 720],
      ['marcos-modo-1', 1280, 720],
      ['marcos-modo-2', 1280, 720],
      ['marcos-modo-3', 1280, 720],
      ['marcos-modo-5', 1280, 720],
      ['marcos-corredor', 1280, 720],
      ['marcos-lago', 1280, 720],
      ['marcos-mazmorra', 1280, 720],
    ]) {
      const prefijo = join(carpeta, opcion);
      ejecutar(programa, [datos, `--comprobar-${opcion}`, prefijo], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      const capturas = [0, 1, 2, 3].map((i) => leerCaptura(`${prefijo}.${i}.bmp`, ancho, alto));
      if (!capturas[0].equals(capturas[3]))
        throw new Error(`F1 no recupera las bandas originales: ${opcion}.`);
      const escala = Math.min(ancho / 320, alto / 200);
      const x0 = (ancho - 320 * escala) / 2,
        y0 = (alto - 200 * escala) / 2;
      const solapeX = Math.min(2 * escala, x0 * 0.15),
        solapeY = Math.min(2 * escala, y0 * 0.15);
      const catalogo = JSON.parse(
        readFileSync(rutaAssets(raiz, 'original', 'marcos', 'catalogo.json'), 'utf8'),
      );
      const contexto =
        opcion === 'marcos-mazmorra'
          ? 'escena_0'
          : opcion === 'marcos-corredor'
            ? 'escena_1'
            : opcion === 'marcos-lago'
              ? 'escena_3'
              : 'titulo';
      const nombre = catalogo.pantallas[contexto] ?? catalogo.predeterminado;
      const marco = JSON.parse(
        readFileSync(rutaAssets(raiz, 'original', 'marcos', `${nombre}.png.json`), 'utf8'),
      );
      const margenIzquierdo =
        x0 > 0
          ? (marco.innerPadding?.left ?? 0) *
            Math.max(
              (200 * escala) / marco.left.height,
              Math.floor(x0 + solapeX) / (marco.left.width - (marco.innerPadding?.left ?? 0)),
            )
          : 0;
      const margenDerecho =
        x0 > 0
          ? (marco.innerPadding?.right ?? 0) *
            Math.max(
              (200 * escala) / marco.right.height,
              Math.floor(x0 + solapeX) / (marco.right.width - (marco.innerPadding?.right ?? 0)),
            )
          : 0;
      const margenSuperior =
        y0 > 0
          ? (marco.innerPadding?.left ?? 0) *
            Math.max(
              (320 * escala) / marco.left.height,
              Math.floor(y0 + solapeY) / (marco.left.width - (marco.innerPadding?.left ?? 0)),
            )
          : 0;
      const margenInferior =
        y0 > 0
          ? (marco.innerPadding?.right ?? 0) *
            Math.max(
              (320 * escala) / marco.right.height,
              Math.floor(y0 + solapeY) / (marco.right.width - (marco.innerPadding?.right ?? 0)),
            )
          : 0;
      // El panorama solo rellena bandas: ningún píxel interior puede cambiar.
      let panorama = 0;
      for (let y = 0; y < alto; y++)
        for (let x = 0; x < ancho; x++) {
          const p = (y * ancho + x) * 3;
          if (capturas[0].subarray(p, p + 3).equals(capturas[2].subarray(p, p + 3))) continue;
          if (
            x >= Math.ceil(x0) &&
            x < Math.floor(ancho - x0) &&
            y >= Math.ceil(y0) &&
            y < Math.floor(alto - y0)
          )
            throw new Error(`El panorama invade el juego: ${opcion}, ${x}, ${y}.`);
          panorama++;
        }
      if ((opcion === 'marcos-ajustado') !== (panorama === 0))
        throw new Error(`Presencia de panorama incorrecta: ${opcion}.`);
      let modificados = 0;
      let relieve = 0;
      for (let y = 0; y < alto; y++)
        for (let x = 0; x < ancho; x++) {
          const p = (y * ancho + x) * 3;
          const distinto =
            capturas[0][p] !== capturas[1][p] ||
            capturas[0][p + 1] !== capturas[1][p + 1] ||
            capturas[0][p + 2] !== capturas[1][p + 2];
          if (!distinto) continue;
          if (
            x >= Math.ceil(x0 + solapeX + margenIzquierdo) &&
            x < Math.floor(ancho - x0 - solapeX - margenDerecho) &&
            y >= Math.ceil(y0 + solapeY + margenSuperior) &&
            y < Math.floor(alto - y0 - solapeY - margenInferior)
          )
            throw new Error(`El marco modifica el área del juego: ${opcion}, ${x}, ${y}.`);
          if (
            y >= Math.ceil(y0 + solapeY) &&
            y < Math.floor(alto - y0 - solapeY) &&
            x >= Math.ceil(x0 + solapeX) &&
            x < Math.floor(ancho - x0 - solapeX)
          )
            relieve++;
          modificados++;
        }
      if ((opcion === 'marcos-ajustado') !== (modificados === 0))
        throw new Error(`Presencia de marco incorrecta: ${opcion}.`);
      if (x0 > 0 && margenIzquierdo > 0 && relieve === 0)
        throw new Error(`El remate interior sigue cortado: ${opcion}.`);
      if (y0 > 0 && margenSuperior > 0 && relieve === 0)
        throw new Error(`El remate girado sigue cortado: ${opcion}.`);
      console.log(
        `${opcion}: F1 reversible; ${modificados} píxeles decorados y ${relieve} de relieve interior conservados.`,
      );
    }
    const animacion = join(carpeta, 'animacion');
    ejecutar(programa, [datos, '--comprobar-marcos-animacion', animacion], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    const fases = [0, 1, 2, 3, 4].map((i) => leerCaptura(`${animacion}.${i}.bmp`, 1280, 720));
    if (
      !fases[0].equals(fases[4]) ||
      !fases[1].equals(fases[3]) ||
      fases[1].equals(fases[0]) ||
      fases[1].equals(fases[2])
    )
      throw new Error(
        'El zoom y fundido de F1 no conservan las fases intermedias y su recorrido inverso.',
      );
    for (const fase of fases)
      for (let y = 0; y < 720; y++)
        if (
          !fase
            .subarray((y * 1280 + 128) * 3, (y * 1280 + 1152) * 3)
            .equals(fases[0].subarray((y * 1280 + 128) * 3, (y * 1280 + 1152) * 3))
        )
          throw new Error('La animación del marco cambia el interior del juego.');
    console.log(
      'Animación F1: zoom y fundido de 250 ms, fases intermedias simétricas y juego conservado.',
    );
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 65,
          panorama: 'Cielo encantado: 32:9, 24:9, 16:9 y 4:3; 16:10 sin bandas',
          filtros_marco: 'seis modos compartidos con el juego',
          animacion_ms: 250,
          bandas_horizontales: 'predeterminado girado 90 grados; panorama horizontal sin giro',
          zoom_inicial: 1.1,
          formatos: [
            '32:9',
            '24:9',
            '4:3',
            'apaisado',
            'vertical',
            'ajustado',
            'impar',
            'xBRZ',
            'corredor',
            'lago',
            'mazmorra',
          ],
          interior_conservado: true,
          remate_artistico_completo: true,
          solape_logico_adicional: 2,
          ciclo_reversible: true,
        },
        null,
        2,
      ) + '\n',
    );
    return;
  }
  if (accion === 'comprobar:poblacion') {
    const pruebaValle = configurarComprobacionValle(argumentos);
    const poste = Boolean(pruebaValle) || argumentos.includes('--poste');
    const regresoLoro = poste || argumentos.includes('--regreso-loro');
    const cantidadVistas = [
      ...readFileSync(rutaJuego(raiz, 'plataformas/sdl/vistas.hpp'), 'utf8').matchAll(
        /\{\d+, \d+, (?:\d+|0xffff), "/g,
      ),
    ].length;
    const borde = regresoLoro || argumentos.includes('--borde');
    const entradaBorde = borde || argumentos.includes('--entrada-borde');
    const probador = argumentos.includes('--probador');
    const tiendas = argumentos.includes('--tiendas') || probador;
    const calles = argumentos.includes('--calles') || tiendas;
    const grupo = pruebaValle
      ? pruebaValle.nombre
      : poste
        ? 'poste'
        : regresoLoro
          ? 'regreso_loro'
          : borde
            ? 'borde'
            : entradaBorde
              ? 'entrada_borde'
              : probador
                ? 'probador'
                : tiendas
                  ? 'tiendas'
                  : calles
                    ? 'calles'
                    : 'poblacion';
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', `comprobacion_${grupo}_sdl`);
    mkdirSync(carpeta, { recursive: true });
    if (pruebaValle) {
      if (
        !existsSync(
          join(raiz, 'apps/juego/trabajo/comparacion_regreso_loro/valle_post_entrada.ds.bin'),
        )
      )
        throw new Error('Prepara primero las referencias comunes con pnpm comprobar:poste.');
      for (const comparador of pruebaValle.comparadores)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas/gestionar_proyecto.mjs'),
          comparador,
          '--muestra',
        ]);
    } else {
      const entrada = join(carpeta, 'orden_entrada.bmp');
      ejecutar(
        programa,
        [datos, '--comprobar-final-poblacion-entrada', entrada],
        { ...process.env, SDL_RENDER_DRIVER: 'software' },
        true,
      );
      ejecutar(
        process.execPath,
        [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:poblacion',
          '--muestra',
        ],
        { ...process.env, CURSE_REBORN_ORDEN_ENTRADA: entrada + '.ds.bin' },
      );
      if (calles)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:calles',
        ]);
      if (tiendas)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:tiendas',
        ]);
      if (probador)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:probador',
        ]);
      if (entradaBorde)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:recorrido-borde',
        ]);
      if (borde) {
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:acciones-borde',
        ]);
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:loro',
        ]);
      }
      if (regresoLoro)
        ejecutar(process.execPath, [
          join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
          'comparar:regreso-loro',
        ]);
    }
    const capturas = pruebaValle
      ? pruebaValle.hitos.map((nombre) => [pruebaValle.prefijo + nombre, nombre])
      : poste
        ? [['borde_valle_post_entrada', 'valle_post_entrada']]
        : regresoLoro
          ? ['regreso_mago', 'regreso_mago_pagado', 'regreso_guardia'].map((nombre) => [
              'borde_' + nombre,
              nombre,
            ])
          : borde
            ? [
                ...[
                  'borde_roca_empujada',
                  'borde_roca_saltada',
                  'borde_guantes',
                  'borde_puente',
                  'borde_cuerda',
                  'borde_electrico',
                  'borde_caramelo',
                  'borde_caramelo_puesto',
                  'borde_roca_grande',
                  'borde_cuerda_lanzada',
                  'borde_abismo_saltado',
                  'borde_inscripcion',
                  'borde_puerta_abierta',
                ].map((nombre) => [nombre, nombre]),
                ...['loro_cargado', 'loro_entrada', 'loro_regreso'].map((nombre) => [
                  'borde_' + nombre,
                  nombre,
                ]),
              ]
            : entradaBorde
              ? [['borde', 'borde_cargado']]
              : probador
                ? ['probador_entrada', 'probador_regreso'].map((nombre) => [nombre, nombre])
                : tiendas
                  ? [
                      'tienda_35',
                      'regreso_tienda_35',
                      'tienda_38',
                      'adivina_saludo',
                      'adivina_respuesta',
                      'regreso_tienda_38',
                      'tienda_39',
                      'regreso_tienda_39',
                    ].map((nombre) => [nombre, nombre])
                  : calles
                    ? [
                        ['calle_29', 'calle_29'],
                        ['regreso_29', 'regreso_29'],
                        ['calle_30', 'calle_30'],
                        ['regreso_30', 'regreso_30'],
                        ['calle_33', 'calle_33'],
                        ['regreso_33', 'regreso_33'],
                        ['calle_41', 'calle_41'],
                        ['posadero', 'posadero'],
                        ['regreso_41', 'regreso_41'],
                      ]
                    : [
                        ['entrada', 'poblacion_entrada'],
                        ['guardias', 'poblacion_guardias'],
                        ['frank', 'poblacion_frank'],
                        ['saludo', 'poblacion_saludo'],
                        ['comida', 'poblacion_comida'],
                        ['comer', 'poblacion_comer'],
                        ['consumida', 'poblacion_consumida'],
                        ['regreso', 'poblacion_regreso'],
                        ['fuente', 'poblacion_fuente'],
                        ['maravillas', 'poblacion_maravillas'],
                        ['mago', 'poblacion_mago'],
                        ['transporte', 'poblacion_transporte'],
                      ];
    for (const [opcion, nombre] of capturas) {
      const captura = join(carpeta, nombre + '.bmp');
      const registro = ejecutar(
        programa,
        [datos, '--comprobar-final-poblacion-' + opcion, captura],
        { ...process.env, SDL_RENDER_DRIVER: 'software' },
        true,
      );
      writeFileSync(join(carpeta, nombre + '.log'), registro + '\n');
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(
              raiz,
              'apps',
              'juego',
              'trabajo',
              pruebaValle
                ? 'comparacion_' + pruebaValle.referencia
                : regresoLoro
                  ? 'comparacion_regreso_loro'
                  : borde
                    ? nombre.startsWith('loro_')
                      ? 'comparacion_loro'
                      : 'comparacion_acciones_borde'
                    : entradaBorde
                      ? 'comparacion_recorrido_borde'
                      : probador
                        ? 'comparacion_probador'
                        : tiendas
                          ? 'comparacion_tiendas'
                          : calles
                            ? 'comparacion_calles'
                            : 'comparacion_poblacion',
              nombre + '_cpp.png',
            ),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la referencia original.`);
    }
    const llegada = join(carpeta, 'llegada_borde.bmp');
    ejecutar(
      programa,
      [datos, '--comprobar-final-poblacion-borde', llegada],
      {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      },
      true,
    );
    const estadoLlegada = readFileSync(llegada + '.ds.bin');
    if (
      estadoLlegada.length !== 65536 ||
      estadoLlegada.readUInt16LE(0x8082) !== 30 ||
      estadoLlegada.readUInt16LE(0x3598) !== 47
    )
      throw new Error('El transporte SDL no llega a la entrada original de EDGE.');
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: capturas.length,
          pixeles_por_captura: 64000,
          llegada_borde: pruebaValle
            ? pruebaValle.alcance
            : poste
              ? 'llegada real aPOST35 desde el guardia y restauración de la partida'
              : regresoLoro
                ? 'regreso al mago y guardia;3 capturas contrastadas; POST todavía fuera de SDL'
                : entradaBorde
                  ? borde
                    ? 'recorrido completo de EDGE, visita al loro y regreso;16 capturas contrastadas'
                    : 'entrada y composición comprobadas; el recorrido se verifica con comprobar:borde'
                  : 'carga y salida comprobadas; no se contrasta la composición en este comando',
          vistas_desarrollo: cantidadVistas,
          cambios_de_vista: cantidadVistas * 2,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      `Población: ${capturas.length} capturas SDL coincidentes y ${cantidadVistas} vistas F5/F6. Resultados: ${carpeta}`,
    );
    return;
  }
  if (accion === 'comprobar:asaltante') {
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_asaltante_sdl');
    mkdirSync(carpeta, { recursive: true });
    const entrada = join(carpeta, 'orden_entrada.bmp');
    ejecutar(
      programa,
      [datos, '--comprobar-final-asaltante-entrada', entrada],
      { ...process.env, SDL_RENDER_DRIVER: 'software' },
      true,
    );
    ejecutar(
      process.execPath,
      [
        join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
        'comparar:asaltante',
        '--muestra',
      ],
      { ...process.env, CURSE_REBORN_ORDEN_ENTRADA: entrada + '.ds.bin' },
    );
    const capturas = [
      ['entrada', 'asaltante_entrada'],
      ['tropiezo', 'asaltante_tropiezo'],
      ['espada-suelo', 'asaltante_espada_suelo'],
      ['espada', 'asaltante_espada'],
      ['ataque-inicio', 'asaltante_ataque_inicio'],
      ['ataque-0', 'asaltante_ataque_0'],
      ['ataque-2', 'asaltante_ataque_2'],
      ['ataque-7', 'asaltante_ataque_7'],
      ['dinero', 'asaltante_dinero'],
      ['camino', 'asaltante_camino'],
      ['sendero', 'asaltante_sendero'],
      ['salida', 'asaltante_salida'],
    ];
    for (const [opcion, nombre] of capturas) {
      const captura = join(carpeta, nombre + '.bmp');
      const registro = ejecutar(
        programa,
        [datos, '--comprobar-final-asaltante-' + opcion, captura],
        { ...process.env, SDL_RENDER_DRIVER: 'software' },
        true,
      );
      writeFileSync(join(carpeta, nombre + '.log'), registro + '\n');
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_asaltante', nombre + '_cpp.png'),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la referencia original.`);
    }
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: capturas.length,
          pixeles_por_captura: 64000,
          vistas_jugables: 17,
          cambios_de_vista: 34,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      `Asaltante: doce capturas SDL coincidentes y diecisiete vistas F5/F6. Resultados: ${carpeta}`,
    );
    return;
  }
  if (accion === 'comprobar:campo') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:campo',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_campo_sdl');
    mkdirSync(carpeta, { recursive: true });
    const capturas = [
      ['0', 'campo_0'],
      ['32', 'campo_32'],
      ['64', 'campo_64'],
      ['96', 'campo_96'],
      ['160', 'campo_160'],
      ['camino-120', 'campo_camino_120'],
      ['camino-150', 'campo_camino_150'],
      ['sendero', 'campo_sendero'],
      ['salida', 'campo_salida'],
    ];
    for (const [opcion, nombre] of capturas) {
      const captura = join(carpeta, nombre + '.bmp');
      const registro = ejecutar(
        programa,
        [datos, '--comprobar-final-campo-' + opcion, captura],
        { ...process.env, SDL_RENDER_DRIVER: 'software' },
        true,
      );
      writeFileSync(join(carpeta, nombre + '.log'), registro + '\n');
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_campo', nombre + '_cpp.png'),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la referencia original.`);
    }
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: capturas.length,
          pixeles_por_captura: 64000,
          vistas_jugables: 17,
          cambios_de_vista: 34,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      `Campo: nueve capturas SDL coincidentes y diecisiete vistas F5/F6. Resultados: ${carpeta}`,
    );
    return;
  }
  if (accion === 'comprobar:informacion') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:informacion',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_informacion_sdl', idioma);
    mkdirSync(carpeta, { recursive: true });
    for (let pagina = 0; pagina < 5; ++pagina) {
      const registro = ejecutar(
        programa,
        [datos, `--comprobar-info-${pagina}`, join(carpeta, `pagina_${pagina + 1}.bmp`), idioma],
        {
          ...process.env,
          SDL_RENDER_DRIVER: 'software',
          CURSE_REBORN_PARTIDAS: join(carpeta, 'no_debe_crearse.sav'),
        },
        true,
      );
      writeFileSync(join(carpeta, `pagina_${pagina + 1}.log`), registro + '\n');
    }
    if (existsSync(join(carpeta, 'no_debe_crearse.sav')))
      throw new Error('Información ha creado un perfil de partidas.');
    console.log(
      `Información: cuatro páginas originales y créditos del port, cierres y estado conservado. Capturas: ${carpeta}`,
    );
    return;
  }
  if (accion === 'comprobar:partidas') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:partidas',
    ]);
    ejecutar(
      process.platform === 'win32'
        ? join(raiz, 'apps', 'juego', 'trabajo', 'entorno', 'Scripts', 'python.exe')
        : join(raiz, 'apps', 'juego', 'trabajo', 'entorno', 'bin', 'python'),
      [
        join(raiz, 'apps/juego/pruebas', 'ejecutar.py'),
        'comparar_panel_partidas.py',
        '--biblioteca',
        join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_acciones', 'tabla_acciones.dll'),
      ],
    );
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_partidas_sdl');
    mkdirSync(carpeta, { recursive: true });
    const perfil = join(carpeta, `perfil_${Date.now()}.sav`);
    const original = join(
      raiz,
      'apps',
      'juego',
      'trabajo',
      'comparacion_partidas',
      'original_dos.sav',
    );
    if (!existsSync(original))
      throw new Error(
        'Falta la partida de referencia DOS: ejecuta pnpm comparar:final-cuevas --muestra y repite.',
      );
    const registro = ejecutar(
      programa,
      [datos, '--comprobar-final-partidas', join(carpeta, 'panel.bmp')],
      {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
        CURSE_REBORN_PARTIDAS: perfil,
        CURSE_REBORN_IMPORTAR_PRUEBA: original,
      },
      true,
    );
    writeFileSync(join(carpeta, 'acciones.log'), registro + '\n');
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          ranuras: 8,
          importacion_dos: true,
          panel_original: true,
          editor_original: true,
          composiciones_originales: 23,
          compatibilidad_nombres_anteriores: 4,
          borrado_inmediato_original: true,
          perfil_aislado: perfil,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Partidas SDL: guardado, restauración, importación DOS sin sobrescribir otras ranuras y paneles originales sin botones añadidos.',
    );
    return;
  }
  if (accion === 'comprobar:final-cuevas') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:final-cuevas',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_final_cuevas_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['--comprobar-final-moneda', 'final_moneda'],
      ['--comprobar-final-agujero-65', 'final_agujero_65'],
      ['--comprobar-final-agujero-66', 'final_agujero_66'],
      ['--comprobar-final-agujero-67', 'final_agujero_67'],
      ['--comprobar-final-agujero-68', 'final_agujero_68'],
      ['--comprobar-final-ramitas', 'final_ramitas'],
      ['--comprobar-final-trampa', 'final_trampa'],
      ['--comprobar-final-barro', 'final_barro'],
      ['--comprobar-final-ofertas-pozo', 'final_ofertas_pozo'],
      ['--comprobar-final-casco-suelo', 'final_casco_suelo'],
      ['--comprobar-final-casco', 'final_casco'],
      ['--comprobar-final-desprendimiento', 'final_desprendimiento'],
      ['--comprobar-final-cubo', 'final_cubo'],
      ['--comprobar-final-dentro-cubo', 'final_dentro_cubo'],
      ['--comprobar-final-mascara', 'final_mascara'],
      ['--comprobar-final-salida-cuevas', 'final_salida_cuevas'],
    ]) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, opcion, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_final_cuevas', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la partida original.`);
    }
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 16,
          pixeles_por_captura: 64000,
          cambios_de_vista: 30,
          limite:
            'Ruta de las quince escenas hasta salir de las cuevas; comprobaciones descritas en apps/juego/documentacion/final_cuevas.md.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log('Final de las cuevas SDL: coinciden dieciséis capturas y treinta cambios F5/F6.');
    return;
  }
  if (accion === 'comprobar:catapulta') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:catapulta',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_catapulta_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['--comprobar-catapulta-corredor', 'catapulta_corredor'],
      ['--comprobar-catapulta-monitor', 'catapulta_monitor'],
      ['--comprobar-catapulta-tablon', 'catapulta_tablon'],
      ['--comprobar-catapulta-cornisa', 'catapulta_cornisa'],
      ['--comprobar-catapulta-iman', 'catapulta_iman'],
      ['--comprobar-catapulta-alambre', 'catapulta_alambre'],
    ]) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, opcion, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_catapulta', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la partida original.`);
    }
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 6,
          pixeles_por_captura: 64000,
          cambios_de_vista: 30,
          limite:
            'Catapulta, regreso de la cornisa e imán con cordel hasta el alambre; el resto se comprueba con comprobar:final-cuevas.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log('Catapulta SDL: coinciden seis capturas hasta el alambre y treinta cambios F5/F6.');
    return;
  }
  if (accion === 'comprobar:cuevas') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:cuevas',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_cuevas_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['--comprobar-cuevas-piedras-uno', 'cuevas_piedras_uno'],
      ['--comprobar-cuevas-primera-entrega', 'cuevas_primera_entrega'],
      ['--comprobar-cuevas-piedras-tres', 'cuevas_piedras_tres'],
      ['--comprobar-cuevas-cuerda', 'cuevas_cuerda'],
    ]) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, opcion, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_cuevas', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la partida original.`);
    }
    ejecutar(programa, [datos, '--comprobar-vistas', join(carpeta, 'vistas.bmp')], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 4,
          pixeles_por_captura: 64000,
          cambios_de_vista: 30,
          limite:
            'Recogida de doce piedras y cuerda; otras acciones y puzles de las cuevas pendientes.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log('Cuevas SDL: coinciden cuatro capturas hasta la cuerda y treinta cambios F5/F6.');
    return;
  }
  if (accion === 'comprobar:poza') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:poza',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_poza_sdl');
    mkdirSync(carpeta, { recursive: true });
    const capturas = [
      ['--comprobar-poza-algas', 'poza_algas'],
      ['--comprobar-poza-puerta', 'poza_puerta'],
      ['--comprobar-cuevas-corredor-entrada', 'cuevas_corredor_entrada'],
      ['--comprobar-cuevas-corredor', 'cuevas_corredor'],
    ];
    for (const [opcion, nombre] of capturas) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, opcion, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_poza', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la partida original.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: capturas.length,
          pixeles_por_captura: 64000,
          limite:
            'Algas, botón, puerta secreta y entrada al corredor; puzles posteriores pendientes.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Poza SDL: coinciden cuatro capturas hasta atravesar la puerta secreta y entrar al corredor de las cuevas.',
    );
    return;
  }
  if (accion === 'comprobar:lago') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:lago',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_lago_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['--comprobar-lago-pecera', 'lago_pecera_128'],
      ['--comprobar-lago', 'lago_oxigeno'],
      ['--comprobar-lago-hablar', 'lago_hablar'],
      ['--comprobar-lago-concha', 'lago_concha'],
      ['--comprobar-lago-viaje', 'lago_tortuga_viaje'],
      ['--comprobar-lago-tortuga', 'lago_tortuga'],
      ['--comprobar-lago-picana', 'lago_picana'],
      ['--comprobar-lago-tiburon', 'lago_tiburon'],
      ['--comprobar-lago-almeja', 'lago_almeja'],
      ['--comprobar-lago-tapon', 'lago_tapon'],
      ['--comprobar-cueva-entrada', 'cueva_entrada'],
      ['--comprobar-cueva-emergida', 'cueva_emergida'],
    ]) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, opcion, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_lago', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${nombre} difiere de la partida original.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 12,
          pixeles_por_captura: 64000,
          limite:
            'Recorrido hasta salir del lago y emerger en la cueva; acciones de las cuevas pendientes.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Lago SDL: coinciden doce capturas, incluido hablar con Mr. Fish, el salto de la almeja, el tapón y la llegada a la cueva.',
    );
    return;
  }
  if (accion === 'comprobar:corredor') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:corredor',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_corredor_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['entrada', 'corredor_entrada'],
      ['mecanismo', 'corredor_mecanismo'],
      ['regreso', 'mazmorra_regreso'],
      ['pecera', 'corredor_pecera'],
      ['golpe', 'corredor_golpe'],
      ['caida', 'corredor_caida'],
      ['agua', 'corredor_agua'],
      ['audio', 'corredor_agua'],
    ]) {
      const captura = join(carpeta, `${opcion}.bmp`);
      ejecutar(programa, [datos, `--comprobar-corredor-${opcion}`, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
        SDL_AUDIO_DRIVER: 'dummy',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_corredor', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura SDL de ${opcion} difiere de la transición original.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify({ resultado: 'coinciden', capturas: 8, pixeles_por_captura: 64000 }, null, 2) +
        '\n',
    );
    console.log(
      'Corredor SDL: coinciden ocho capturas; pecera, golpes, regreso y llegada al agua con inventario persistente, también con audio activo.',
    );
    return;
  }
  if (accion === 'comprobar:puzle') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:puzle',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_puzle_sdl');
    mkdirSync(carpeta, { recursive: true });
    const captura = join(carpeta, 'puerta_abierta.bmp');
    ejecutar(programa, [datos, '--comprobar-puzle', captura], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    if (
      !leerCaptura(captura).equals(
        leerReferencia(
          false,
          false,
          join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_puzle', 'interaccion_32_cpp.png'),
        ),
      )
    )
      throw new Error('La apertura de la puerta en SDL difiere del recorrido original.');
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify({ resultado: 'coinciden', capturas: 1, pixeles_por_captura: 64000 }, null, 2) +
        '\n',
    );
    console.log(
      'Puzle SDL: coincide la imagen tras liberar a Brad, empujar la pared, recoger el clip y abrir la puerta.',
    );
    return;
  }
  if (accion === 'comprobar:jugabilidad') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:jugabilidad',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_jugabilidad_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const nombre of ['liberacion', 'movimiento']) {
      const captura = join(carpeta, `${nombre}.bmp`);
      ejecutar(programa, [datos, `--comprobar-${nombre}`, captura], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      if (
        !leerCaptura(captura).equals(
          leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_jugabilidad', `${nombre}_cpp.png`),
          ),
        )
      )
        throw new Error(`La captura de ${nombre} difiere del recorrido original.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        { resultado: 'coinciden', capturas: 2, puerta_inmovil: true, flechas_y_raton: true },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Jugabilidad SDL: coinciden la liberación con la puerta en su sitio y el movimiento por flechas y ratón.',
    );
    return;
  }
  if (accion === 'comprobar:menu') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:menu',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_menu_sdl');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, nombre] of [
      ['menu', 'menu_escena_cpp'],
      ['orden', 'interaccion_32_cpp'],
      ['depuracion', null],
    ]) {
      const ruta = join(carpeta, `${opcion}.bmp`);
      ejecutar(programa, [datos, `--comprobar-${opcion}`, ruta], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      const observado = leerCaptura(ruta);
      if (nombre) {
        if (
          !observado.equals(
            leerReferencia(
              false,
              false,
              join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_menu', `${nombre}.png`),
            ),
          )
        )
          throw new Error(`La captura de ${opcion} no coincide con el original.`);
      } else {
        for (const color of [
          [40, 255, 80],
          [0, 220, 255],
          [255, 60, 220],
          [255, 150, 0],
          [255, 255, 0],
        ]) {
          let presente = false;
          for (let i = 0; i < observado.length; i += 3)
            if (
              observado[i] === color[0] &&
              observado[i + 1] === color[1] &&
              observado[i + 2] === color[2]
            ) {
              presente = true;
              break;
            }
          if (!presente) throw new Error('Falta un color de las cajas de depuración.');
        }
      }
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: 3,
          menu_y_primera_orden: true,
          cinco_colores_depuracion: true,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Menú SDL: coinciden el panel y la primera interacción; se ven las cinco categorías de depuración.',
    );
    return;
  }
  if (accion === 'comprobar:introduccion') {
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:introduccion',
      '--muestra',
    ]);
    ejecutar(process.execPath, [
      join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
      'comparar:objetos',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'comprobacion_introduccion_sdl');
    mkdirSync(carpeta, { recursive: true });
    const capturas = [
      ['caldero', 'cauldrn2'],
      ['balcon', 'balcony2'],
      ['beisbol', 'basebal1'],
      ['fin-intro', null],
      ['salto-intro', null],
    ];
    for (const [opcion, nombre] of capturas) {
      const ruta = join(carpeta, `${opcion}.bmp`);
      ejecutar(programa, [datos, `--comprobar-${opcion}`, ruta], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      const referencia = nombre
        ? leerReferencia(
            false,
            false,
            join(raiz, 'apps', 'juego', 'trabajo', 'comparacion_introduccion', `${nombre}.png`),
          )
        : leerReferencia(true);
      if (!leerCaptura(ruta).equals(referencia))
        throw new Error(`La captura de ${opcion} difiere de la referencia original.`);
    }
    writeFileSync(
      join(carpeta, 'resultado.json'),
      JSON.stringify(
        {
          resultado: 'coinciden',
          capturas: capturas.length,
          pixeles_por_captura: 64000,
          final_y_omision_entran_en_mazmorra: true,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Introducción SDL: coinciden caldero, balcón, béisbol y entrada en mazmorra al terminar u omitir.',
    );
    return;
  }
  const escena = accion === 'comprobar:escena';
  ejecutar(process.execPath, [
    join(raiz, 'apps/juego/herramientas', 'gestionar_proyecto.mjs'),
    ...(escena ? ['comparar:objetos', '--muestra'] : ['titulo']),
  ]);
  const carpeta = join(
    raiz,
    'apps',
    'juego',
    'trabajo',
    escena ? 'comprobacion_escena_sdl' : 'comprobacion_ventana',
  );
  mkdirSync(carpeta, { recursive: true });
  const captura = join(carpeta, 'fotograma.bmp');
  ejecutar(programa, [datos, escena ? '--comprobar-escena' : '--comprobar', captura], {
    ...process.env,
    SDL_RENDER_DRIVER: 'software',
  });
  const esperado = leerReferencia(escena),
    observado = leerCaptura(captura);
  if (!observado.equals(esperado))
    throw new Error('El fotograma SDL difiere de la referencia reconstruida.');
  if (escena) {
    const animacion = join(carpeta, 'tras_32_pasos.bmp');
    ejecutar(programa, [datos, '--comprobar-animacion', animacion], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    if (!leerCaptura(animacion).equals(leerReferencia(true, true)))
      throw new Error('Los objetos animados en SDL difieren de la referencia original.');
  }
  if (!escena) {
    const transicion = join(carpeta, 'tras_fundido.bmp');
    ejecutar(programa, [datos, '--comprobar-transicion', transicion], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    if (!leerCaptura(transicion).equals(esperado))
      throw new Error('El título tras el fundido de Core difiere de la referencia.');
  }
  const informe = {
    version_sdl: configuracion.version,
    plataforma: process.platform,
    pixeles_comparados: 64000,
    resultado: 'coinciden',
    dibujante: 'software',
    ventana_oculta: true,
    cierre_por_escape: true,
    pantalla: escena ? 'mazmorra' : 'titulo',
    avance_por_f6: escena,
    transicion_core_titulo: !escena,
    pasos_animados: escena ? 32 : 0,
    lectura_datos_exe: escena,
    sha256_rgb: createHash('sha256').update(observado).digest('hex'),
    alcance:
      'Lectura de recursos, núcleo C++, textura SDL y captura del fotograma. Sin control del personaje, audio ni verificación del dibujante acelerado.',
  };
  writeFileSync(join(carpeta, 'resultado.json'), `${JSON.stringify(informe, null, 2)}\n`);
  console.log('Coinciden los 64.000 píxeles dibujados por SDL con la referencia.');
  console.log(`Captura e informe local: ${carpeta}`);
}
