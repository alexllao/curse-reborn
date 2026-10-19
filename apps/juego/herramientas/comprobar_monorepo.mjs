/** Comprueba el contrato de rutas del workspace sin abrir el juego ni necesitar recursos comerciales. */
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { obtenerFuentesMotor } from './fuentes_motor.mjs';
import { rutaJuego, rutaAssets } from './rutas_proyecto.mjs';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');

// Detiene la auditoría indicando qué contrato del monorepositorio se ha roto.
function exigir(condicion, mensaje) {
  if (!condicion) throw new Error(mensaje);
}

// Recorre solo las fuentes propias, sin entrar en terceros ni en resultados de compilación.
function fuentesEn(carpeta) {
  return readdirSync(carpeta, { withFileTypes: true }).flatMap((entrada) => {
    const ruta = join(carpeta, entrada.name);
    return entrada.isDirectory()
      ? fuentesEn(ruta)
      : /\.(cpp|hpp)$/.test(entrada.name)
        ? [ruta]
        : [];
  });
}

try {
  exigir(existsSync(join(raiz, 'pnpm-workspace.yaml')), 'Falta el workspace de pnpm.');
  for (const [ruta, nombre] of [
    ['apps/juego', '@curse-reborn/juego'],
    ['apps/web', '@curse-reborn/web'],
    ['packages/assets', '@curse-reborn/assets'],
  ]) {
    const paquete = JSON.parse(readFileSync(join(raiz, ruta, 'package.json'), 'utf8'));
    exigir(paquete.name === nombre && paquete.private === true, `Paquete inválido: ${ruta}.`);
  }
  const fuentes = obtenerFuentesMotor(raiz);
  exigir(
    fuentes.length > 0 && new Set(fuentes).size === fuentes.length,
    'Manifiesto de fuentes vacío o duplicado.',
  );
  for (const fuente of fuentes)
    exigir(
      existsSync(fuente) && fuente.startsWith(rutaJuego(raiz, 'motor')),
      `Fuente ausente o ajena al motor: ${fuente}.`,
    );
  const propias = [
    ...fuentesEn(rutaJuego(raiz, 'motor')),
    ...fuentesEn(rutaJuego(raiz, 'plataformas')),
    ...fuentesEn(join(raiz, 'apps/juego/pruebas')),
  ];
  let cabeceras = 0;
  for (const fuente of propias) {
    for (const coincidencia of readFileSync(fuente, 'utf8').matchAll(/#include\s+"(\.[^"\n]+)"/g)) {
      exigir(
        existsSync(resolve(dirname(fuente), coincidencia[1])),
        `Include roto en ${fuente}: ${coincidencia[1]}.`,
      );
      ++cabeceras;
    }
  }
  const mapa = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/documentacion', 'mapa_ejecutable.json'), 'utf8'),
  );
  let traslados = 0;
  // Las referencias del ensamblador deben apuntar al único motor que se compila.
  const revisarMapa = (valor) => {
    if (!valor || typeof valor !== 'object') return;
    if (typeof valor.traslado === 'string') {
      exigir(existsSync(join(raiz, valor.traslado)), `Traslado inexistente: ${valor.traslado}.`);
      ++traslados;
    }
    for (const hijo of Object.values(valor)) revisarMapa(hijo);
  };
  revisarMapa(mapa);
  for (const archivo of [
    'original/marcos/catalogo.json',
    'original/habitaciones/habitaciones.json',
    'original/decorados/catalogo.json',
  ])
    exigir(existsSync(rutaAssets(raiz, archivo)), `Falta el recurso compartido ${archivo}.`);
  console.log(
    `Monorepo: 3 paquetes privados, ${fuentes.length} fuentes del motor, ${cabeceras} includes relativos y ${traslados} referencias del ensamblador comprobados.`,
  );
  console.log(
    'La web y WebAssembly siguen pendientes; el motor nativo conserva su única implementación.',
  );
} catch (error) {
  console.error(`Error: ${error.message}`);
  process.exitCode = 1;
}
