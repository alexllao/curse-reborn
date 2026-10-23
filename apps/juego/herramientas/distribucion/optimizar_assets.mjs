import sharp from 'sharp';

export const calidadesJpeg = Object.freeze({ alta: 92, equilibrada: 85, compacta: 75 });

// Convierte únicamente fondos opacos de la copia; logos, alfa y pixel art conservan PNG.
export async function optimizarImagen(original, perfil = 'alta', fotografia = false) {
  if (perfil !== 'sin-perdida' && !Object.hasOwn(calidadesJpeg, perfil))
    throw new Error('Perfil de imágenes inválido.');
  const antes = await sharp(original).metadata();
  if (antes.format !== 'png' || antes.pages > 1) throw new Error('Se esperaba un PNG estático.');
  if (fotografia && perfil !== 'sin-perdida') {
    const opaca = !antes.hasAlpha || (await sharp(original).stats()).isOpaque;
    if (opaca) {
      const calidad = calidadesJpeg[perfil];
      const datos = await sharp(original)
        .removeAlpha()
        .jpeg({ quality: calidad, chromaSubsampling: '4:4:4', mozjpeg: true })
        .toBuffer();
      const despues = await sharp(datos).metadata();
      if (despues.width !== antes.width || despues.height !== antes.height)
        throw new Error('La conversión JPEG ha cambiado las dimensiones.');
      if (datos.length < original.length)
        return {
          datos,
          metodo: `jpeg-${calidad}-444`,
          extension: '.jpg',
          ancho: antes.width,
          alto: antes.height,
        };
    }
  }
  return { ...(await optimizarPng(original, 'sin-perdida')), extension: '.png' };
}

// Reduce bytes de una copia PNG; conserva tamaño y alfa para no desplazar las zonas del monitor.
export async function optimizarPng(original, perfil = 'equilibrado', fotografia = false) {
  if (!['equilibrado', 'sin-perdida'].includes(perfil))
    throw new Error('Perfil de imágenes inválido.');
  const antes = await sharp(original).metadata();
  if (antes.format !== 'png' || antes.pages > 1) throw new Error('Se esperaba un PNG estático.');
  const paleta = fotografia && !antes.hasAlpha && perfil === 'equilibrado';
  const candidato = await sharp(original)
    .png({
      compressionLevel: 9,
      adaptiveFiltering: true,
      palette: paleta,
      ...(paleta ? { quality: 95, colours: 256, dither: 0.5, effort: 10 } : {}),
    })
    .toBuffer();
  const despues = await sharp(candidato).metadata();
  if (
    antes.width !== despues.width ||
    antes.height !== despues.height ||
    antes.hasAlpha !== despues.hasAlpha
  )
    throw new Error('La optimización ha cambiado dimensiones o transparencia.');
  if (!paleta) {
    const a = await sharp(original).ensureAlpha().raw().toBuffer();
    const b = await sharp(candidato).ensureAlpha().raw().toBuffer();
    if (!a.equals(b)) throw new Error('La compresión sin pérdida ha cambiado píxeles.');
  }
  return {
    datos: candidato.length < original.length ? candidato : original,
    metodo:
      candidato.length >= original.length
        ? 'original-mas-pequeno'
        : paleta
          ? 'paleta-256'
          : 'png-sin-perdida',
    ancho: antes.width,
    alto: antes.height,
  };
}
