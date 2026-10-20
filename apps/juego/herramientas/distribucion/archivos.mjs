import { lstatSync, readdirSync, readFileSync } from 'node:fs';
import { isAbsolute, join, relative, resolve } from 'node:path';
import { createHash } from 'node:crypto';

// Mantiene todas las lecturas y escrituras de una lista relativa dentro de su raíz.
export function rutaDentro(raiz, nombre) {
  const destino = resolve(raiz, nombre),
    ruta = relative(resolve(raiz), destino);
  if (
    !ruta ||
    isAbsolute(nombre) ||
    ruta === '..' ||
    ruta.startsWith('..\\') ||
    ruta.startsWith('../')
  )
    throw new Error('Ruta fuera de la carpeta del paquete: ' + nombre);
  return destino;
}

// Enumera archivos regulares de forma estable; un enlace no debe incorporar contenido ajeno.
export function listarArchivos(carpeta, prefijo = '') {
  return readdirSync(carpeta)
    .sort()
    .flatMap((nombre) => {
      const ruta = join(carpeta, nombre),
        info = lstatSync(ruta),
        relativa = prefijo + nombre;
      if (info.isSymbolicLink()) throw new Error('No se empaquetan enlaces: ' + relativa);
      if (info.isDirectory()) return listarArchivos(ruta, relativa + '/');
      if (!info.isFile()) throw new Error('Archivo especial no admitido: ' + relativa);
      return [relativa];
    });
}

// Calcula la huella que permitirá verificar el contenido extraído por el probador.
export function huella(datos) {
  return createHash('sha256').update(datos).digest('hex');
}

// Comprueba tamaños, huellas y exclusiones antes de producir el ZIP y después de extraerlo.
export function verificarPaquete(carpeta) {
  const presentes = listarArchivos(carpeta);
  const manifiesto = JSON.parse(readFileSync(join(carpeta, 'paquete.json'), 'utf8'));
  const esperados = new Set(['paquete.json']);
  for (const archivo of manifiesto.archivos) {
    if (/\.(sav|rom|pdb|obj|o|cfg)$/i.test(archivo.ruta))
      throw new Error('Archivo privado o de trabajo en el paquete.');
    const datos = readFileSync(rutaDentro(carpeta, archivo.ruta));
    if (
      esperados.has(archivo.ruta) ||
      datos.length !== archivo.bytes ||
      huella(datos) !== archivo.sha256
    )
      throw new Error('Contenido alterado o duplicado: ' + archivo.ruta);
    esperados.add(archivo.ruta);
  }
  if (presentes.length !== esperados.size || presentes.some((ruta) => !esperados.has(ruta)))
    throw new Error('La carpeta contiene archivos ajenos al manifiesto.');
  return manifiesto;
}
