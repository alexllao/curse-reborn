import { spawnSync } from 'node:child_process';
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { gestionarSdl } from './gestionar_sdl.mjs';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');

// Compara las mismas entradas con y sin recargas; todo el perfil y las pruebas quedan en trabajo/.
async function auditar() {
  if (process.platform !== 'win32') throw new Error('Este recorrido está comprobado en Windows.');
  const argumentos = process.argv.slice(2);
  if (
    argumentos.some((a) => !['--sin-compilar', '--solo-laberinto'].includes(a)) ||
    new Set(argumentos).size !== argumentos.length
  )
    throw new Error('Uso: pnpm comprobar:continuidad [--sin-compilar] [--solo-laberinto].');
  const reutilizar = argumentos.includes('--sin-compilar');
  if (!reutilizar) await gestionarSdl('sdl:compilar', []);
  const carpeta = join(raiz, 'apps/juego/trabajo', 'continuidad_' + Date.now());
  mkdirSync(carpeta, { recursive: true });
  const informe = {
    resultado: 'en curso',
    compilacion: reutilizar ? 'reutilizada' : 'actualizada',
    casos: [],
    limite:
      'Recargas en reposo con menú habilitado y continuidad de estas entradas; no agota todas las alternativas ni estados de guardado.',
  };
  const rutaInforme = join(carpeta, 'resultado.json');
  // Conserva también el último caso completado si una variante detecta una diferencia.
  const registrar = () => writeFileSync(rutaInforme, JSON.stringify(informe, null, 2) + '\n');
  registrar();
  const laberinto = [
    'borde_valle_laberinto_alternativa_115_regreso',
    'borde_valle_laberinto_alternativa_204_regreso',
  ];
  const casos = argumentos.includes('--solo-laberinto')
    ? laberinto
    : [
        'borde_valle_bruja_desenlace_final',
        'regreso_33',
        'regreso_tienda_38',
        'regreso_41',
        ...laberinto,
      ];
  for (const caso of casos) {
    const destino = join(carpeta, caso);
    mkdirSync(destino, { recursive: true });
    for (const modo of ['control', 'recargas']) {
      const perfil = join(destino, modo);
      mkdirSync(perfil, { recursive: true });
      const env = {
        ...process.env,
        SDL_RENDER_DRIVER: 'software',
        CURSE_REBORN_PARTIDAS: join(perfil, 'perfil.sav'),
        CURSE_REBORN_TIEMPO: join(perfil, 'tiempo.json'),
      };
      delete env.CURSE_REBORN_COMPROBAR_RECARGAS;
      if (modo === 'recargas') env.CURSE_REBORN_COMPROBAR_RECARGAS = perfil;
      const r = spawnSync(
        join(raiz, 'apps/juego/trabajo/aplicacion/curse-reborn.exe'),
        [
          join(raiz, 'apps/juego/datos_originales/CURSE/DATA'),
          '--comprobar-final-poblacion-' + caso,
          join(destino, modo + '.bmp'),
        ],
        {
          cwd: raiz,
          env,
          encoding: 'utf8',
          windowsHide: true,
          timeout: 120000,
          maxBuffer: 32 * 1024 * 1024,
        },
      );
      writeFileSync(join(destino, modo + '.log'), (r.stdout || '') + (r.stderr || ''));
      if (r.error || r.status !== 0) {
        informe.resultado = 'fallo';
        informe.fallo = { caso, modo, salida: r.status, error: r.error?.message };
        registrar();
        throw new Error(`Falla ${caso}/${modo}; consultar ${destino}.`);
      }
    }
    if (
      !readFileSync(join(destino, 'control.bmp')).equals(
        readFileSync(join(destino, 'recargas.bmp')),
      )
    ) {
      informe.resultado = 'diferencia visual';
      informe.fallo = { caso };
      registrar();
      throw new Error('La continuidad cambia el resultado de ' + caso);
    }
    const puntos = readFileSync(join(destino, 'recargas/continuidad.tsv'), 'utf8')
      .trim()
      .split('\n');
    const escenas = [
      ...new Set(puntos.map((linea) => Number(linea.match(/_escena_(\d+)/)[1]))),
    ].sort((a, b) => a - b);
    informe.casos.push({
      caso,
      puntos: puntos.length,
      escenas,
      resultado: 'misma imagen y recorrido completado',
    });
    registrar();
    console.log(
      `${caso}: ${puntos.length} recargas en ${escenas.length} escenas; mismo resultado que sin cargar.`,
    );
  }
  informe.resultado = 'coinciden';
  registrar();
  console.log('Informe: ' + rutaInforme);
}

auditar().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
