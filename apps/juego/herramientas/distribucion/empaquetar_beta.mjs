import {
  copyFileSync,
  existsSync,
  mkdirSync,
  readFileSync,
  statSync,
  writeFileSync,
} from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';
import { randomUUID } from 'node:crypto';
import { gestionarSdl } from '../gestionar_sdl.mjs';
import { huella, listarArchivos, rutaDentro, verificarPaquete } from './archivos.mjs';
import { optimizarImagen } from './optimizar_assets.mjs';
import { analizarOpciones } from './opciones.mjs';
import { seleccionarHabitaciones } from '../gestionar_habitaciones.mjs';
import { prepararMusica } from '../audio/preparar_musica.mjs';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../../..');

// Ejecuta Git o PowerShell con argumentos separados y sin abrir ventanas de consola.
function ejecutar(programa, argumentos, env = process.env) {
  const r = spawnSync(programa, argumentos, {
    cwd: raiz,
    env,
    encoding: 'utf8',
    windowsHide: true,
    maxBuffer: 16 * 1024 * 1024,
    timeout: 120000,
  });
  if (r.error || r.status !== 0)
    throw new Error(r.error?.message || r.stderr || r.stdout || 'No se completa ' + programa);
  return r.stdout.trim();
}

// Acompaña Munt con el CRT x64 redistribuible de Visual Studio, sin depender del PC del desarrollador.
function copiarRuntimeWindows(escribir) {
  const vswhere = join(
    process.env['ProgramFiles(x86)'] || 'C:/Program Files (x86)',
    'Microsoft Visual Studio/Installer/vswhere.exe',
  );
  const vs =
    process.env.VSINSTALLDIR ||
    ejecutar(vswhere, [
      '-latest',
      '-products',
      '*',
      '-requires',
      'Microsoft.VisualStudio.Component.VC.Tools.x86.x64',
      '-property',
      'installationPath',
      '-utf8',
    ]);
  const redist = join(vs, 'VC/Redist/MSVC');
  const candidatos = listarArchivos(redist)
    .filter((ruta) => /^\d[^/]*\/x64\/Microsoft\.VC\d+\.CRT\/msvcp140\.dll$/i.test(ruta))
    .sort((a, b) => b.localeCompare(a, 'en', { numeric: true }));
  if (!candidatos.length)
    throw new Error('Falta el CRT x64 redistribuible de Visual Studio para acompañar Munt.');
  const carpeta = dirname(join(redist, candidatos[0]));
  for (const ruta of listarArchivos(carpeta).filter((ruta) => ruta.endsWith('.dll')))
    escribir(ruta, readFileSync(join(carpeta, ruta)));
}

// Solo incorpora los archivos realmente referenciados por los catálogos de esta compilación.
function recursosNecesarios(compilacion) {
  const rutas = new Set([
    'assets/lanzador/habitaciones/habitaciones.json',
    'assets/lanzador/presentacion/presentacion.json',
    'assets/lanzador/presentacion/curs-of-enchantia-reborn-logo.png',
    'assets/marcos/catalogo.json',
    'assets/marcos/cielo_encantado/catalogo.json',
    'assets/original/decorados/catalogo.json',
    'assets/lanzador/sonidos_pc/arranque.wav',
    'assets/lanzador/sonidos_pc/boton.wav',
    'assets/lanzador/sonidos_pc/ORIGEN.txt',
    'assets/lanzador/icono/curse-reborn.png',
  ]);
  for (const nombre of ['habitaciones', 'presentacion', 'marcos', 'decorados']) {
    const cabecera = readFileSync(join(compilacion, `catalogo_${nombre}.hpp`), 'utf8');
    for (const coincidencia of cabecera.matchAll(/"(assets\/[^"\n]+\.png)"/g)) {
      rutas.add(coincidencia[1]);
      if (nombre === 'marcos') rutas.add(coincidencia[1] + '.json');
    }
  }
  return [...rutas].sort();
}

// Construye una distribución Windows nueva; nunca sobrescribe maestros, partidas ni paquetes anteriores.
async function empaquetar(argumentos) {
  if (process.platform !== 'win32' || process.arch !== 'x64')
    throw new Error('Este builder está validado solo para Windows x64.');
  if (argumentos[0] === '--verificar' && argumentos.length === 2) {
    const informe = verificarPaquete(resolve(argumentos[1]));
    console.log(`Paquete ${informe.version}: ${informe.archivos.length} archivos verificados.`);
    return;
  }
  const { conDatos, perfil, habitacion } = analizarOpciones(argumentos);
  console.log('Compresión de fondos: ' + perfil + ' (resolución original).');
  const maestroHabitaciones = JSON.parse(
    readFileSync(join(raiz, 'packages/assets/lanzador/habitaciones/habitaciones.json'), 'utf8'),
  );
  const inicial = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/configuracion/inicial.json'), 'utf8'),
  );
  if (inicial.habitacion) maestroHabitaciones.inicial = inicial.habitacion;
  const catalogoHabitaciones = seleccionarHabitaciones(maestroHabitaciones, habitacion);
  const version = JSON.parse(readFileSync(join(raiz, 'apps/juego/package.json'), 'utf8')).version;
  if (!/^\d+\.\d+\.\d+(?:-[a-zA-Z0-9.-]+)?$/.test(version))
    throw new Error('Versión del paquete inválida.');
  const revision = ejecutar('git', ['rev-parse', 'HEAD']);
  const cambios = ejecutar('git', ['status', '--porcelain']).length > 0;
  const datosOriginales = join(raiz, 'apps/juego/datos_originales/CURSE');
  if (conDatos) {
    const mapa = JSON.parse(
      readFileSync(join(raiz, 'apps/juego/documentacion/mapa_ejecutable.json'), 'utf8'),
    );
    if (huella(readFileSync(join(datosOriginales, 'CURSE.EXE'))) !== mapa.sha256)
      throw new Error('CURSE.EXE no coincide con la versión reconstruida.');
  }
  await gestionarSdl('sdl:compilar', ['--distribucion', '--habitacion', habitacion]);
  const compilacion = join(raiz, 'apps/juego/trabajo/aplicacion_beta');
  const fecha = new Date().toISOString();
  const identificador = randomUUID().slice(0, 8);
  const nombre = `curse-of-enchantia-reborn-${version}-win-x64-${fecha.replace(/[-:.]/g, '')}-${identificador}`;
  const dist = join(raiz, 'apps/juego/dist'),
    carpeta = rutaDentro(
      dist,
      `${fecha.slice(0, 10).replaceAll('-', '')}-${identificador}/CurseReborn-${version}`,
    );
  mkdirSync(carpeta, { recursive: true });
  const imagenes = [];
  // Escribe una copia dentro de la carpeta nueva y crea exclusivamente sus subdirectorios.
  const escribir = (ruta, contenido) => {
    const destino = rutaDentro(carpeta, ruta);
    mkdirSync(dirname(destino), { recursive: true });
    writeFileSync(destino, contenido);
  };
  for (const ruta of [
    'curse-reborn.exe',
    'SDL3.dll',
    'mt32emu.dll',
    'LICENCIA_SDL3.txt',
    'LICENCIA_MT32EMU.txt',
    'LICENCIA_GPL_MT32EMU.txt',
    'LICENCIA_YMFM.txt',
    'LICENCIA_NUKED_OPL3.txt',
  ])
    escribir(ruta, readFileSync(join(compilacion, ruta)));
  copiarRuntimeWindows(escribir);
  const canciones = prepararMusica(raiz, carpeta, true);
  const huellasMaestros = JSON.parse(
    readFileSync(join(raiz, 'packages/assets/huellas.json'), 'utf8'),
  );
  const configEntrega = JSON.parse(readFileSync(join(carpeta, 'configuracion.json'), 'utf8'));
  configEntrega.habitacion = catalogoHabitaciones.inicial;
  escribir('configuracion.json', JSON.stringify(configEntrega, null, 2) + '\n');
  for (const ruta of recursosNecesarios(compilacion)) {
    const original = readFileSync(rutaDentro(compilacion, ruta));
    if (!ruta.endsWith('.png')) {
      escribir(ruta, original);
      continue;
    }
    const fotografia =
      ruta.startsWith('assets/lanzador/habitaciones/') ||
      (ruta.startsWith('assets/lanzador/presentacion/') && !ruta.endsWith('-logo.png')) ||
      (ruta.startsWith('assets/marcos/cielo_encantado/') && !ruta.includes('/overlay/'));
    if (huellasMaestros.imagenes[ruta.slice('assets/'.length)] !== huella(original))
      throw new Error('El maestro gráfico cambió: revisa su SHA-256: ' + ruta);
    const resultado = await optimizarImagen(original, perfil, fotografia);
    const destinoImagen = ruta.slice(0, -4) + resultado.extension;
    escribir(destinoImagen, resultado.datos);
    imagenes.push({
      ruta: destinoImagen,
      ruta_maestro: ruta,
      antes: original.length,
      sha256_maestro: huella(original),
      sha256: huella(resultado.datos),
      despues: resultado.datos.length,
      metodo: resultado.metodo,
      ancho: resultado.ancho,
      alto: resultado.alto,
    });
    console.log(
      `Imagen ${imagenes.length}: ${destinoImagen} · ${resultado.metodo} (${original.length} → ${resultado.datos.length} bytes)`,
    );
  }
  // Los metadatos de marco describen la copia entregada; los maestros conservan su huella original.
  for (const i of imagenes.filter((i) => /assets\/marcos\//.test(i.ruta))) {
    const ruta = i.ruta_maestro + '.json';
    if (!existsSync(join(carpeta, ruta))) continue;
    const c = JSON.parse(readFileSync(join(carpeta, ruta), 'utf8'));
    if (c.sourceSha256) c.sourceSha256 = huella(readFileSync(join(carpeta, i.ruta)));
    escribir(ruta, JSON.stringify(c, null, 2) + '\n');
  }
  const destinoDatos = 'datos_originales/CURSE/';
  mkdirSync(join(carpeta, destinoDatos, 'DATA'), { recursive: true });
  mkdirSync(join(carpeta, 'datos_originales/ROMS'), { recursive: true });
  if (conDatos) {
    escribir(destinoDatos + 'CURSE.EXE', readFileSync(join(datosOriginales, 'CURSE.EXE')));
    for (const ruta of listarArchivos(join(datosOriginales, 'DATA'))) {
      if (!/\.(DAT|PAL|MAP|SPR|BRD|SFX|RS[1-5])$/i.test(ruta)) continue;
      escribir(
        destinoDatos + 'DATA/' + ruta,
        readFileSync(rutaDentro(join(datosOriginales, 'DATA'), ruta)),
      );
    }
  }
  escribir(
    'datos_originales/LEEME.txt',
    'Coloca CURSE/CURSE.EXE y toda la carpeta CURSE/DATA de tu copia del juego.\r\nROM MT-32 opcionales: ROMS/MT32_CONTROL.ROM y ROMS/MT32_PCM.ROM.\r\nEl builder nunca incorpora ROM ni partidas.\r\n',
  );
  for (const idioma of ['en', 'es'])
    escribir(
      `Jugar-${idioma}.cmd`,
      `@echo off\r\n"%~dp0curse-reborn.exe" "%~dp0datos_originales\\CURSE\\DATA" ${idioma}\r\n`,
    );
  copyFileSync(join(raiz, 'apps/juego/documentacion/controles.md'), join(carpeta, 'CONTROLES.md'));
  escribir(
    'LEEME.txt',
    `Curse of Enchantia: Reborn ${version}\r\nPort por Àlex Llaó. Compilado ${fecha}.\r\n\r\nExtrae el ZIP completo antes de jugar. Abre curse-reborn.exe (inglés), Jugar-es.cmd o Jugar-en.cmd.\r\n${conDatos ? 'Incluye la copia local del juego utilizada para las pruebas.' : 'Antes de jugar coloca tu copia del juego siguiendo datos_originales/LEEME.txt.'}\r\nLas partidas y el tiempo se guardan en el perfil del usuario, fuera de esta carpeta.\r\nRevisión de alternativas y continuidad de partidas todavía en curso. No es una entrega 1.0 final.\r\n\r\nLicencias de bibliotecas adjuntas. No se incluyen ROM ni partidas.\r\n`,
  );
  escribir(
    'BIBLIOTECAS.txt',
    'Código del port y referencias de las bibliotecas: https://github.com/alexllao/curse-reborn\r\nSDL3: https://github.com/libsdl-org/SDL (licencia adjunta).\r\nMunt/libmt32emu: revisión3b05ec276f9e605af86b0eaef7f5eda43477a31f, https://github.com/munt/munt (LGPL y GPL adjuntas). La DLL se mantiene sustituible.\r\nymfm: https://github.com/aaronsgiles/ymfm (licencia adjunta).\r\nNuked OPL3: https://github.com/nukeykt/Nuked-OPL3 (licencia adjunta).\r\nMicrosoft Visual C++ Runtime x64: archivos redistribuibles de Visual Studio, copyright Microsoft Corporation.\r\n',
  );
  const comprobaciones = [];
  if (conDatos) {
    const pruebas = join(raiz, 'apps/juego/trabajo', 'paquete_' + randomUUID());
    mkdirSync(pruebas, { recursive: true });
    const env = {
      ...process.env,
      SDL_RENDER_DRIVER: 'software',
      SDL_AUDIODRIVER: 'dummy',
      CURSE_REBORN_PARTIDAS: join(pruebas, 'CURSE.SAV'),
      CURSE_REBORN_TIEMPO: join(pruebas, 'tiempo.json'),
    };
    for (const [opcion, salida] of [
      ['--comprobar-habitaciones', 'habitaciones'],
      ['--comprobar-remaster', 'musica.json'],
      ['--comprobar-final-poblacion-borde_valle_bruja_desenlace_final', 'final.bmp'],
    ]) {
      console.log('Verificando la copia empaquetada: ' + opcion);
      const log = ejecutar(
        join(carpeta, 'curse-reborn.exe'),
        [join(carpeta, 'datos_originales/CURSE/DATA'), opcion, join(pruebas, salida)],
        env,
      );
      writeFileSync(join(pruebas, salida + '.log'), log);
      if (salida === 'final.bmp' && !existsSync(join(pruebas, salida)))
        throw new Error('La prueba SDL no ha generado su imagen final.');
      if (
        salida === 'habitaciones' &&
        !listarArchivos(pruebas).some(
          (ruta) => ruta.startsWith('habitaciones.') && ruta.endsWith('.png'),
        )
      )
        throw new Error('La prueba SDL no ha generado capturas de las habitaciones.');
      comprobaciones.push(opcion);
    }
  }
  const antes = imagenes.reduce((n, i) => n + i.antes, 0),
    despues = imagenes.reduce((n, i) => n + i.despues, 0);
  const archivos = listarArchivos(carpeta).map((ruta) => {
    const datos = readFileSync(join(carpeta, ruta));
    return { ruta, bytes: datos.length, sha256: huella(datos) };
  });
  escribir(
    'paquete.json',
    JSON.stringify(
      {
        version,
        revision,
        cambios_locales: cambios,
        fecha,
        plataforma: 'win-x64',
        perfil,
        datos_originales: conDatos,
        roms: false,
        habitaciones: {
          inicial: catalogoHabitaciones.inicial,
          incluidas: catalogoHabitaciones.habitaciones.map((h) => h.id),
        },
        comprobaciones,
        optimizacion: ['O2', 'NDEBUG', 'secciones', 'OPT:REF', 'OPT:ICF'],
        imagenes: { antes, despues, archivos: imagenes },
        canciones,
        archivos,
      },
      null,
      2,
    ) + '\n',
  );
  verificarPaquete(carpeta);
  const zip = rutaDentro(dist, nombre + '.zip');
  ejecutar(
    'powershell.exe',
    [
      '-NoProfile',
      '-NonInteractive',
      '-Command',
      'Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::CreateFromDirectory($env:CURSE_PAQUETE, $env:CURSE_ZIP, [System.IO.Compression.CompressionLevel]::Optimal, $true)',
    ],
    { ...process.env, CURSE_PAQUETE: carpeta, CURSE_ZIP: zip },
  );
  writeFileSync(zip + '.sha256', huella(readFileSync(zip)) + '  ' + nombre + '.zip\n');
  const bytesCarpeta =
    archivos.reduce((total, archivo) => total + archivo.bytes, 0) +
    statSync(join(carpeta, 'paquete.json')).size;
  const bytesZip = statSync(zip).size;
  const mostrarTamano = (bytes) =>
    `${(bytes / 1048576).toFixed(2)} MiB (${bytes.toLocaleString('es-ES')} bytes)`;
  console.log(
    `Paquete verificado: ${carpeta}\nZIP: ${zip}\nImágenes: ${(antes / 1048576).toFixed(1)} → ${(despues / 1048576).toFixed(1)} MiB (${(100 * (1 - despues / antes)).toFixed(1)} % menos).\nTamaño del paquete (carpeta): ${mostrarTamano(bytesCarpeta)}\nTamaño del ZIP: ${mostrarTamano(bytesZip)}`,
  );
}

empaquetar(process.argv.slice(2)).catch((error) => {
  console.error('Error de empaquetado: ' + error.message);
  process.exitCode = 1;
});
