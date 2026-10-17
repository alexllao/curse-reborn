import { obtenerFuentesMotor } from './fuentes_motor.mjs';
import { prepararMt32 } from './gestionar_mt32.mjs';
import { prepararMarcos } from './gestionar_marcos.mjs';
/** Compilación y comprobación de Curse Reborn con SDL, sin ejecutar código DOS. */
import { createHash } from 'node:crypto';
import { copyFileSync, existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import { inflateSync } from 'node:zlib';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const configuracion = JSON.parse(
  readFileSync(join(raiz, 'herramientas', 'dependencias_sdl.json'), 'utf8'),
);
const dependencias = join(raiz, 'trabajo', 'dependencias');
const aplicacion = join(raiz, 'trabajo', 'aplicacion');
const datos = join(raiz, 'datos_originales', 'CURSE', 'DATA');

function ejecutar(programa, argumentos, variables = process.env, capturar = false, ocultar = true) {
  const resultado = spawnSync(programa, argumentos, {
    cwd: raiz,
    env: variables,
    stdio: capturar ? 'pipe' : 'inherit',
    encoding: 'utf8',
    windowsHide: ocultar,
    shell: false,
    timeout: argumentos.some((argumento) => argumento.startsWith('--comprobar'))
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
      'Usa una instalación SDL3 compatible con CMake; consulta documentacion/ventana_sdl.md.',
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

async function compilar() {
  mkdirSync(aplicacion, { recursive: true });
  const opl2 = JSON.parse(
    readFileSync(join(raiz, 'herramientas', 'dependencias_opl2.json'), 'utf8'),
  );
  for (const [archivo, huella] of Object.entries(opl2.sha256))
    if (
      createHash('sha256')
        .update(readFileSync(join(raiz, 'terceros', 'ymfm', archivo)))
        .digest('hex') !== huella
    )
      throw new Error(`La fuente OPL2 no coincide con la revisión documentada: ${archivo}.`);
  if (process.platform !== 'win32') {
    ejecutar('cmake', ['-S', raiz, '-B', aplicacion, '-DCMAKE_BUILD_TYPE=Release']);
    ejecutar('cmake', ['--build', aplicacion, '--config', 'Release']);
    const directo = join(aplicacion, 'curse-reborn');
    return existsSync(directo) ? directo : join(aplicacion, 'Release', 'curse-reborn');
  }
  const sdl = await preparar();
  prepararMarcos(raiz, aplicacion);
  const variables = entornoCompilador();
  const mt32 = await prepararMt32(raiz, ejecutar, variables);
  console.log('Compilando Curse Reborn con SDL…');
  ejecutar(
    process.env.CXX || 'clang++',
    [
      '--target=x86_64-pc-windows-msvc',
      '-std=c++17',
      '-O2',
      '-Wall',
      '-Wextra',
      '-Werror',
      '-fno-exceptions',
      '-fno-rtti',
      '-fuse-ld=lld',
      '-Xlinker',
      '/subsystem:console',
      '-I',
      join(sdl, 'include'),
      '-I',
      join(mt32, 'include'),
      '-I',
      aplicacion,
      join(raiz, 'plataformas', 'sdl', 'principal.cpp'),
      join(raiz, 'plataformas', 'sdl', 'filtros.cpp'),
      join(raiz, 'plataformas', 'sdl', 'marcos.cpp'),
      join(raiz, 'plataformas', 'sdl', 'partidas.cpp'),
      join(raiz, 'plataformas', 'sdl', 'contador_fps.cpp'),
      join(raiz, 'plataformas', 'sdl', 'mazmorra.cpp'),
      join(raiz, 'plataformas', 'sdl', 'introduccion.cpp'),
      join(raiz, 'plataformas', 'sdl', 'audio.cpp'),
      join(raiz, 'plataformas', 'sdl', 'musica_adlib.cpp'),
      join(raiz, 'plataformas', 'sdl', 'musica_mt32.cpp'),
      join(raiz, 'plataformas', 'sdl', 'opl.cpp'),
      join(raiz, 'plataformas', 'sdl', 'opl2.cpp'),
      join(raiz, 'plataformas', 'sdl', 'ymfm_opl.cpp'),
      join(raiz, 'plataformas', 'sdl', 'ymfm_adpcm.cpp'),
      join(raiz, 'plataformas', 'sdl', 'ymfm_pcm.cpp'),
      ...obtenerFuentesMotor(raiz),
      join(sdl, 'lib', 'x64', 'SDL3.lib'),
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
  copyFileSync(join(raiz, 'terceros', 'ymfm', 'LICENSE'), join(aplicacion, 'LICENCIA_YMFM.txt'));
  copyFileSync(join(sdl, 'lib', 'x64', 'SDL3.dll'), join(aplicacion, 'SDL3.dll'));
  copyFileSync(join(sdl, 'LICENSE.txt'), join(aplicacion, 'LICENCIA_SDL3.txt'));
  copyFileSync(
    join(raiz, 'terceros', 'nuked_opl3', 'LICENSE'),
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
            'trabajo',
            'comparacion_objetos',
            animada ? 'mazmorra_animada_cpp.png' : 'mazmorra_objetos_cpp.png',
          )
        : join(raiz, 'trabajo', 'vista_titulo', 'titulo_cpp.png')),
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
export async function gestionarSdl(accion, argumentos) {
  const sinSonido =
    accion === 'ventana' && argumentos.length === 1 && argumentos[0] === '--sin-sonido';
  if (argumentos.length && !sinSonido && accion !== 'exportar:musica')
    throw new Error('Solo ventana admite el argumento --sin-sonido.');
  if (accion === 'sdl:preparar') {
    if (await preparar()) console.log('Dependencia SDL preparada.');
    return;
  }
  if (accion === 'comprobar:vistas') {
    const programa = await compilar();
    const carpeta = join(raiz, 'trabajo', 'comprobacion_vistas');
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
    const carpeta = join(raiz, 'trabajo', 'exportacion_musica');
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
    const carpeta = join(raiz, 'trabajo', 'comparacion_musica_rs');
    mkdirSync(carpeta, { recursive: true });
    ejecutar(programa, [datos, '--comprobar-mt32', join(carpeta, 'mazmorra_mt32.wav')], {
      ...process.env,
      SDL_AUDIO_DRIVER: 'dummy',
    });
    return;
  }
  if (accion === 'comprobar:musica') {
    const programa = await compilar();
    const carpeta = join(raiz, 'trabajo', 'comparacion_musica');
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
        join(raiz, 'trabajo', 'comparacion_digital', 'efectos_digitales.wav'),
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
        join(raiz, 'trabajo', 'comparacion_audio', 'efecto_original.wav'),
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
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
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
        'BOXDET.DAT',
      );
    if (
      accion === 'ventana' ||
      accion === 'comprobar:escena' ||
      accion === 'comprobar:introduccion' ||
      accion === 'comprobar:menu' ||
      accion === 'comprobar:caida' ||
      accion === 'comprobar:corredor' ||
      accion === 'comprobar:lago' ||
      accion === 'comprobar:poza' ||
      [
        'comprobar:cuevas',
        'comprobar:catapulta',
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
        'comprobar:final-cuevas',
        'comprobar:partidas',
      ].includes(accion)
    )
      necesarios.push('WELL.MAP', 'WELL.SPR', 'CAVERBMI.MAP', 'CAVERBMI.SPR');
    if (
      accion === 'ventana' ||
      ['comprobar:catapulta', 'comprobar:final-cuevas', 'comprobar:partidas'].includes(accion)
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
      );
    const faltantes = necesarios.filter((nombre) => !existsSync(join(datos, nombre)));
    if (faltantes.length)
      throw new Error(`Faltan ${faltantes.join(', ')} en datos_originales/CURSE/DATA.`);
    if (
      accion === 'ventana' ||
      accion === 'comprobar:escena' ||
      accion === 'comprobar:introduccion' ||
      accion === 'comprobar:menu' ||
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
        readFileSync(join(raiz, 'documentacion', 'mapa_ejecutable.json'), 'utf8'),
      );
      if (createHash('sha256').update(readFileSync(ejecutable)).digest('hex') !== mapa.sha256)
        throw new Error('La huella de CURSE.EXE no coincide con la versión reconstruida.');
    }
  }
  const programa = await compilar();
  if (accion === 'sdl:compilar') {
    console.log(`Aplicación compilada: ${programa}`);
    return;
  }
  if (accion === 'ventana') {
    console.log(
      'Core, título, introducción, mazmorra y caída: F5/F6 recorren las vistas de desarrollo; en la mazmorra Escape o botón derecho abre/cierra el menú; F7–F10 controlan la depuración; F11 cambia el filtro de pantalla y F12 muestra/oculta los FPS. Tras la liberación se camina con clic o flechas; la puerta abierta permite ir al corredor y regresar. F6 desde la mazmorra abre una sesión nueva del corredor; faltan los demás puzles.',
    );
    ejecutar(programa, [datos, ...(sinSonido ? ['--sin-sonido'] : [])], process.env, false, false);
    return;
  }
  if (accion === 'comprobar:caida') {
    ejecutar(process.execPath, [
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:caida',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_caida_sdl');
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
            join(raiz, 'trabajo', 'comparacion_caida', `${nombre}_cpp.png`),
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
  if (accion === 'comprobar:fps') {
    const carpeta = join(raiz, 'trabajo', 'comprobacion_fps');
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
        join(raiz, 'pruebas', 'comparar_escaladores.cpp'),
        '-o',
        prueba,
      ],
      process.platform === 'win32' ? entornoCompilador() : process.env,
    );
    ejecutar(prueba, []);
    ejecutar(process.execPath, [join(raiz, 'herramientas', 'gestionar_proyecto.mjs'), 'titulo']);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_filtros');
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
    const carpeta = join(raiz, 'trabajo', 'comprobacion_marcos');
    mkdirSync(carpeta, { recursive: true });
    for (const [opcion, ancho, alto] of [
      ['marcos', 1280, 720],
      ['marcos-vertical', 720, 1280],
      ['marcos-ajustado', 960, 600],
      ['marcos-impar', 1279, 721],
      ['marcos-filtro', 1280, 720],
      ['marcos-corredor', 1280, 720],
      ['marcos-lago', 1280, 720],
      ['marcos-mazmorra', 1280, 720],
    ]) {
      const prefijo = join(carpeta, opcion);
      ejecutar(programa, [datos, `--comprobar-${opcion}`, prefijo], {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
      });
      const capturas = [0, 1, 2].map((i) => leerCaptura(`${prefijo}.${i}.bmp`, ancho, alto));
      if (!capturas[0].equals(capturas[2]))
        throw new Error(`F1 no recupera las bandas originales: ${opcion}.`);
      const escala = Math.min(ancho / 320, alto / 200);
      const x0 = (ancho - 320 * escala) / 2,
        y0 = (alto - 200 * escala) / 2;
      const solapeX = Math.min(2 * escala, x0 * 0.15),
        solapeY = Math.min(2 * escala, y0 * 0.15);
      const catalogo = JSON.parse(
        readFileSync(join(raiz, 'assets', 'original', 'marcos', 'catalogo.json'), 'utf8'),
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
        readFileSync(join(raiz, 'assets', 'original', 'marcos', `${nombre}.png.json`), 'utf8'),
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
            y >= Math.ceil(y0 + solapeY) &&
            y < Math.floor(alto - y0 - solapeY)
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
          capturas: 29,
          animacion_ms: 250,
          zoom_inicial: 1.1,
          formatos: [
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
  if (accion === 'comprobar:partidas') {
    ejecutar(process.execPath, [
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:partidas',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_partidas_sdl');
    mkdirSync(carpeta, { recursive: true });
    const perfil = join(carpeta, `perfil_${Date.now()}.sav`);
    const original = join(raiz, 'trabajo', 'comparacion_partidas', 'original_dos.sav');
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
          ranuras: 7,
          importacion_dos: true,
          borrado_confirmado: true,
          perfil_aislado: perfil,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Partidas SDL: guardado, restauración, importación DOS sin sobrescribir otras ranuras y borrado confirmado.',
    );
    return;
  }
  if (accion === 'comprobar:final-cuevas') {
    ejecutar(process.execPath, [
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:final-cuevas',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_final_cuevas_sdl');
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
            join(raiz, 'trabajo', 'comparacion_final_cuevas', `${nombre}_cpp.png`),
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
            'Ruta de las quince escenas hasta salir de las cuevas; comprobaciones descritas en documentacion/final_cuevas.md.',
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:catapulta',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_catapulta_sdl');
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
            join(raiz, 'trabajo', 'comparacion_catapulta', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:cuevas',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_cuevas_sdl');
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
            join(raiz, 'trabajo', 'comparacion_cuevas', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:poza',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_poza_sdl');
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
            join(raiz, 'trabajo', 'comparacion_poza', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:lago',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_lago_sdl');
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
            join(raiz, 'trabajo', 'comparacion_lago', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:corredor',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_corredor_sdl');
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
            join(raiz, 'trabajo', 'comparacion_corredor', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:puzle',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_puzle_sdl');
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
          join(raiz, 'trabajo', 'comparacion_puzle', 'interaccion_32_cpp.png'),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:jugabilidad',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_jugabilidad_sdl');
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
            join(raiz, 'trabajo', 'comparacion_jugabilidad', `${nombre}_cpp.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:menu',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_menu_sdl');
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
              join(raiz, 'trabajo', 'comparacion_menu', `${nombre}.png`),
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
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:introduccion',
      '--muestra',
    ]);
    ejecutar(process.execPath, [
      join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
      'comparar:objetos',
      '--muestra',
    ]);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_introduccion_sdl');
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
            join(raiz, 'trabajo', 'comparacion_introduccion', `${nombre}.png`),
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
    join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
    ...(escena ? ['comparar:objetos', '--muestra'] : ['titulo']),
  ]);
  const carpeta = join(
    raiz,
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
