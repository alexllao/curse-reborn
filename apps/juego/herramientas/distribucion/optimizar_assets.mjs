import sharp from 'sharp';

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
