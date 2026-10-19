/** Referencias MIDI estándar desde los paquetes RS leídos por 7E3B–7EA1; no modifica el juego. */
import { readFileSync, writeFileSync, mkdirSync, copyFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createHash } from 'node:crypto';

const raiz = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');

// Codifica un tiempo o tamaño de MIDI estándar con sus grupos de siete bits, más significativo primero.
function variable(valor) {
  if (!Number.isInteger(valor) || valor < 0 || valor > 0x0fffffff)
    throw new Error('Tiempo MIDI fuera de rango.');
  const bytes = [valor & 127];
  for (valor >>>= 7; valor; valor >>>= 7) bytes.unshift((valor & 127) | 128);
  return Buffer.from(bytes);
}

// Construye un metadato de pista sin interpretarlo como una orden musical del recurso RS.
function meta(tipo, bytes) {
  return Buffer.concat([Buffer.from([255, tipo]), variable(bytes.length), bytes]);
}

// Conserva demoras F8, marcas F9 y estado MIDI implícito; termina antes de ejecutar el primer bucle FF.
function leerRecurso(banco) {
  let posicion = 0,
    estado = 0,
    pulsos = 0,
    paquetes = 0;
  const eventos = [],
    momentos = new Map();
  // Cada paquete se acota al recurso: un archivo truncado se rechaza antes de escribir el MIDI.
  const tomar = () => {
    if (posicion >= banco.length) throw new Error('Recurso RS truncado.');
    return banco[posicion++];
  };
  while (posicion < banco.length && paquetes < 100000) {
    if (banco[posicion] === 255) {
      if (posicion + 2 >= banco.length) throw new Error('Salto RS truncado.');
      const destino = banco.readUInt16LE(posicion + 1);
      if (!momentos.has(destino)) throw new Error('El salto RS no apunta a un paquete anterior.');
      return {
        eventos,
        pulsos,
        paquetes,
        bucle: momentos.get(destino),
        desplazamiento_bucle: destino,
      };
    }
    momentos.set(posicion, pulsos);
    const demora = tomar();
    pulsos += Math.min(demora, 240);
    paquetes++;
    if (demora >= 240) continue;
    let primero = tomar();
    if (primero >= 240) continue;
    if (primero >= 128) {
      estado = primero;
      primero = tomar();
    }
    if (estado < 128 || estado >= 240 || primero >= 128)
      throw new Error('Mensaje de canal RS inválido.');
    const bytes = [estado, primero];
    if (estado < 192 || estado >= 224) {
      const segundo = tomar();
      if (segundo >= 128) throw new Error('Dato MIDI fuera de rango.');
      bytes.push(segundo);
    }
    eventos.push({ tiempo: pulsos, bytes: Buffer.from(bytes) });
  }
  throw new Error('No se encuentra el bucle original del recurso RS.');
}

// Escribe SMF cero a 120 pulsos por negra, con programas MT-32 y marcadores de repetición.
function crearMidi(recurso, tempo, nombre) {
  const microsegundos = Math.round(60000000 / tempo),
    marcaTempo = Buffer.alloc(3);
  marcaTempo.writeUIntBE(microsegundos, 0, 3);
  const eventos = [
    { tiempo: 0, bytes: meta(3, Buffer.from(`${nombre} - Roland MT-32`)) },
    { tiempo: 0, bytes: meta(81, marcaTempo) },
    { tiempo: recurso.bucle, bytes: meta(6, Buffer.from('LOOPSTART')) },
    ...recurso.eventos,
    { tiempo: recurso.pulsos, bytes: meta(6, Buffer.from('LOOPEND')) },
    { tiempo: recurso.pulsos, bytes: meta(47, Buffer.alloc(0)) },
  ].sort((a, b) => a.tiempo - b.tiempo);
  const partes = [];
  let anterior = 0;
  for (const evento of eventos) {
    partes.push(variable(evento.tiempo - anterior), evento.bytes);
    anterior = evento.tiempo;
  }
  const pista = Buffer.concat(partes),
    cabecera = Buffer.alloc(22);
  cabecera.write('MThd');
  cabecera.writeUInt32BE(6, 4);
  cabecera.writeUInt16BE(0, 8);
  cabecera.writeUInt16BE(1, 10);
  cabecera.writeUInt16BE(120, 12);
  cabecera.write('MTrk', 14);
  cabecera.writeUInt32BE(pista.length, 18);
  return Buffer.concat([cabecera, pista]);
}

try {
  const exe = readFileSync(join(raiz, 'apps/juego/datos_originales', 'CURSE', 'CURSE.EXE'));
  const mapa = JSON.parse(
    readFileSync(join(raiz, 'apps/juego/documentacion', 'mapa_ejecutable.json'), 'utf8'),
  );
  if (createHash('sha256').update(exe).digest('hex') !== mapa.sha256)
    throw new Error('El ejecutable no corresponde a la versión reconstruida.');
  const carpeta = join(raiz, 'apps', 'juego', 'trabajo', 'referencias_musicales');
  mkdirSync(carpeta, { recursive: true });
  const pistas = [];
  for (let indice = 0; indice < 5; indice++) {
    const nombre = `CURSE.RS${indice + 1}`;
    const original = join(raiz, 'apps/juego/datos_originales', 'CURSE', 'DATA', nombre);
    const bytes = readFileSync(original);
    const tempo = exe[0xf510 + 0x80b9 + indice * 4];
    const recurso = leerRecurso(bytes),
      midi = crearMidi(recurso, tempo, nombre);
    const archivo = `curse_rs${indice + 1}_original.mid`;
    copyFileSync(original, join(carpeta, nombre));
    writeFileSync(join(carpeta, archivo), midi);
    pistas.push({
      original: nombre,
      midi: archivo,
      tempo_bpm: tempo,
      mensajes: recurso.eventos.length,
      paquetes: recurso.paquetes,
      pulsos: recurso.pulsos,
      inicio_bucle_pulsos: recurso.bucle,
      desplazamiento_bucle_rs: recurso.desplazamiento_bucle,
      duracion_segundos: (recurso.pulsos * 60) / (tempo * 120),
      inicio_bucle_segundos: (recurso.bucle * 60) / (tempo * 120),
      sha256_original: createHash('sha256').update(bytes).digest('hex'),
      sha256_midi: createHash('sha256').update(midi).digest('hex'),
    });
    console.log(
      `${nombre}: ${recurso.eventos.length} mensajes, tempo ${tempo}, bucle ${recurso.bucle}–${recurso.pulsos} pulsos.`,
    );
  }
  writeFileSync(
    join(carpeta, 'pistas.json'),
    JSON.stringify(
      {
        formato_midi: 0,
        pulsos_por_negra: 120,
        sintetizador_original: 'Roland MT-32',
        pistas,
        nota: 'Los programas MT-32 no corresponden a General MIDI. LOOPSTART/LOOPEND conservan entrada y primera repetición; SMF redondea el tempo a microsegundos enteros por negra.',
      },
      null,
      2,
    ) + '\n',
  );
  writeFileSync(
    join(carpeta, 'LEEME.txt'),
    `REFERENCIAS MUSICALES ORIGINALES — CURSE REBORN

Cada CURSE.RS es una copia sin modificar del recurso aportado por Àlex Llaó.
Cada MIDI conserva los mensajes de canal, notas, programas, demoras y tempo del lector 7E3B–7EA1 reconstruido. Los marcadores LOOPSTART y LOOPEND señalan el bucle original, incluyendo su entrada previa. pistas.json contiene tiempos, desplazamientos y huellas.

Son programas de Roland MT-32: un sintetizador General MIDI puede cambiar los instrumentos. El MIDI sirve para estudiar y reorquestar las notas; no incluye las ROM, el timbre ni la reverberación del aparato. No incorpora un arreglo sinfónico.

El juego original es de Core Design. Àlex Llaó es el autor del port Curse Reborn.
Esta carpeta es material local para el futuro remaster; no se publica en Git.
Regeneración: pnpm exportar:midi
`,
  );
  console.log(`Referencias locales: ${carpeta}`);
} catch (error) {
  console.error(`Error: ${error.message}`);
  process.exitCode = 1;
}
