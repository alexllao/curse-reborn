/** Compilación y comprobación de la primera presentación SDL, sin ejecutar el EXE original. */
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
    timeout: argumentos.includes('--comprobar') ? 15000 : undefined,
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
  if (process.platform !== 'win32') {
    ejecutar('cmake', ['-S', raiz, '-B', aplicacion, '-DCMAKE_BUILD_TYPE=Release']);
    ejecutar('cmake', ['--build', aplicacion, '--config', 'Release']);
    const directo = join(aplicacion, 'curse-reborn');
    return existsSync(directo) ? directo : join(aplicacion, 'Release', 'curse-reborn');
  }
  const sdl = await preparar();
  console.log('Compilando la primera pantalla SDL…');
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
      join(raiz, 'plataformas', 'sdl', 'principal.cpp'),
      join(raiz, 'motor', 'recursos', 'titulo.cpp'),
      join(raiz, 'motor', 'recursos', 'rnc.cpp'),
      join(raiz, 'motor', 'recursos', 'rnc2.cpp'),
      join(raiz, 'motor', 'recursos', 'escena.cpp'),
      join(raiz, 'motor', 'video', 'planos.cpp'),
      join(raiz, 'motor', 'video', 'sprites.cpp'),
      join(raiz, 'motor', 'video', 'fundido.cpp'),
      join(sdl, 'lib', 'x64', 'SDL3.lib'),
      '-o',
      join(aplicacion, 'curse-reborn.exe'),
    ],
    entornoCompilador(),
  );
  copyFileSync(join(sdl, 'lib', 'x64', 'SDL3.dll'), join(aplicacion, 'SDL3.dll'));
  copyFileSync(join(sdl, 'LICENSE.txt'), join(aplicacion, 'LICENCIA_SDL3.txt'));
  return join(aplicacion, 'curse-reborn.exe');
}

// Convierte la referencia indexada a RGB para compararla con una captura SDL.
function leerReferencia(escena = false) {
  const archivo = readFileSync(
    escena
      ? join(raiz, 'trabajo', 'comparacion_escena', 'mazmorra_cpp.png')
      : join(raiz, 'trabajo', 'vista_titulo', 'titulo_cpp.png'),
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

function leerCaptura(archivo) {
  const bmp = readFileSync(archivo);
  if (
    bmp.toString('ascii', 0, 2) !== 'BM' ||
    bmp.readInt32LE(18) !== 320 ||
    Math.abs(bmp.readInt32LE(22)) !== 200 ||
    bmp.readUInt16LE(28) !== 24 ||
    bmp.readUInt32LE(30) !== 0
  )
    throw new Error('La captura BMP no tiene el formato RGB esperado.');
  const inicio = bmp.readUInt32LE(10),
    invertida = bmp.readInt32LE(22) > 0;
  const rgb = Buffer.alloc(64000 * 3);
  for (let y = 0; y < 200; ++y)
    for (let x = 0; x < 320; ++x) {
      const origen = inicio + ((invertida ? 199 - y : y) * 320 + x) * 3;
      const destino = (y * 320 + x) * 3;
      rgb[destino] = bmp[origen + 2];
      rgb[destino + 1] = bmp[origen + 1];
      rgb[destino + 2] = bmp[origen];
    }
  return rgb;
}

// Prepara la dependencia, compila y abre o comprueba la presentación nativa solicitada.
export async function gestionarSdl(accion, argumentos) {
  if (argumentos.length)
    throw new Error('Estos comandos todavía no admiten argumentos adicionales.');
  if (accion === 'sdl:preparar') {
    if (await preparar()) console.log('Dependencia SDL preparada.');
    return;
  }
  if (accion !== 'sdl:compilar') {
    const necesarios = ['TITLE.DAT', 'TITLE.PAL', 'CORE.DAT', 'CORE.PAL'];
    if (accion === 'ventana' || accion === 'comprobar:escena')
      necesarios.push('DUNGEON.MAP', 'MANA.BRD', 'SPRITES.PAL');
    const faltantes = necesarios.filter((nombre) => !existsSync(join(datos, nombre)));
    if (faltantes.length)
      throw new Error(`Faltan ${faltantes.join(', ')} en datos_originales/CURSE/DATA.`);
  }
  const programa = await compilar();
  if (accion === 'sdl:compilar') {
    console.log(`Aplicación compilada: ${programa}`);
    return;
  }
  if (accion === 'ventana') {
    console.log(
      'Core, título y vista inicial: una tecla inicia el fundido; F6 avanza y F5 retrocede directamente; Escape cierra. Todavía sin control del personaje.',
    );
    ejecutar(programa, [datos], process.env, false, false);
    return;
  }
  const escena = accion === 'comprobar:escena';
  ejecutar(process.execPath, [
    join(raiz, 'herramientas', 'gestionar_proyecto.mjs'),
    ...(escena ? ['comparar:escena', '--muestra'] : ['titulo']),
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
    sha256_rgb: createHash('sha256').update(observado).digest('hex'),
    alcance:
      'Lectura de recursos, núcleo C++, textura SDL y captura del fotograma. Sin control del personaje, audio ni verificación del dibujante acelerado.',
  };
  writeFileSync(join(carpeta, 'resultado.json'), `${JSON.stringify(informe, null, 2)}\n`);
  console.log('Coinciden los 64.000 píxeles dibujados por SDL con la referencia.');
  console.log(`Captura e informe local: ${carpeta}`);
}
