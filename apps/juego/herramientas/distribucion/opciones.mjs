// Rechaza parámetros incompletos o repetidos antes de compilar y fija el catálogo inicial por defecto.
export function analizarOpciones(argumentos) {
  const opciones = { conDatos: false, perfil: 'equilibrado', habitacion: 'inicial' };
  const vistos = new Set();
  for (let i = 0; i < argumentos.length; ++i) {
    const opcion = argumentos[i];
    if (vistos.has(opcion)) throw new Error('Parámetro repetido: ' + opcion);
    vistos.add(opcion);
    if (opcion === '--con-datos') opciones.conDatos = true;
    else if (opcion === '--sin-perdida') opciones.perfil = 'sin-perdida';
    else if (opcion === '--habitacion') {
      const valor = argumentos[++i];
      if (!valor || !/^[a-z0-9][a-z0-9_-]*$/.test(valor))
        throw new Error('Indica --habitacion <id|inicial|todas>.');
      opciones.habitacion = valor;
    } else throw new Error('Parámetro desconocido: ' + opcion);
  }
  return opciones;
}
