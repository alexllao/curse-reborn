import { obtenerFuentesMotor } from './fuentes_motor.mjs';
import { prepararMt32 } from './gestionar_mt32.mjs';
/** Compilación y comprobación de la presentación SDL y la mazmorra, sin ejecutar código DOS. */
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
  const variables = entornoCompilador();
  const mt32 = await prepararMt32(raiz, ejecutar, variables);
  console.log('Compilando la presentación SDL y la mazmorra…');
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
      join(raiz, 'plataformas', 'sdl', 'principal.cpp'),
      join(raiz, 'plataformas', 'sdl', 'filtros.cpp'),
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
  for (let y = 0; y < alto; ++y)
    for (let x = 0; x < ancho; ++x) {
      const origen = inicio + ((invertida ? alto - 1 - y : y) * ancho + x) * 3;
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
      accion === 'comprobar:puzle' ||
      accion === 'comprobar:jugabilidad'
    )
      necesarios.push(
        'DUNGEON.MAP',
        'DUNGEON.SPR',
        ...(accion === 'ventana' ||
        accion === 'comprobar:caida' ||
        accion === 'comprobar:corredor' ||
        accion === 'comprobar:lago'
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
      accion === 'comprobar:lago'
    )
      necesarios.push(
        'DROP.MAP',
        'DROP.SPR',
        'LAKEBED.MAP',
        'LAKEBED.SPR',
        'PLUNGE.BRD',
        'PLOK.BRD',
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
      'Core, título, introducción, mazmorra y caída: F5/F6 recorren las vistas de desarrollo; en la mazmorra Escape o botón derecho abre/cierra el menú; F7–F10 controlan la depuración; F11 cambia el filtro de pantalla. Tras la liberación se camina con clic o flechas; la puerta abierta permite ir al corredor y regresar. F6 desde la mazmorra abre una sesión nueva del corredor; faltan los demás puzles.',
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
  if (accion === 'comprobar:filtros') {
    ejecutar(process.execPath, [join(raiz, 'herramientas', 'gestionar_proyecto.mjs'), 'titulo']);
    const carpeta = join(raiz, 'trabajo', 'comprobacion_filtros');
    mkdirSync(carpeta, { recursive: true });
    const prefijo = join(carpeta, 'filtro');
    ejecutar(programa, [datos, '--comprobar-filtros', prefijo], {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
    });
    const capturas = Array.from({ length: 5 }, (_, i) =>
      leerCaptura(`${prefijo}.${i}.bmp`, 960, 600),
    );
    if (!capturas[0].equals(capturas[4]))
      throw new Error('El ciclo F11 no recupera la imagen original.');
    if (
      new Set(capturas.slice(0, 4).map((rgb) => createHash('sha256').update(rgb).digest('hex')))
        .size !== 4
    )
      throw new Error('Los cuatro modos de pantalla no producen imágenes diferentes.');
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
          modos: ['original', 'suavizado', 'líneas de escaneo', 'CRT suave'],
          capturas: 5,
          resolucion: [960, 600],
          original_conservado: true,
          ciclo_reversible: true,
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Filtros SDL: cuatro modos diferentes, ciclo F11 reversible y 64.000 píxeles originales conservados.',
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
      ['--comprobar-lago-concha', 'lago_concha'],
      ['--comprobar-lago-viaje', 'lago_tortuga_viaje'],
      ['--comprobar-lago-tortuga', 'lago_tortuga'],
      ['--comprobar-lago-picana', 'lago_picana'],
      ['--comprobar-lago-tiburon', 'lago_tiburon'],
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
          capturas: 7,
          pixeles_por_captura: 64000,
          limite: 'Recorrido hasta aturdir al tiburón; almeja y salida pendientes.',
        },
        null,
        2,
      ) + '\n',
    );
    console.log(
      'Lago SDL: coinciden siete capturas hasta la concha, el transporte sobre las anguilas, la picana y el tiburón aturdido.',
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
