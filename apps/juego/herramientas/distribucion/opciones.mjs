// Rechaza parámetros incompletos o repetidos antes de compilar y fija el catálogo inicial por defecto.
export function analizarOpciones(argumentos) {
  const opciones = { conDatos: false, perfil: 'alta', habitacion: 'inicial' };
  const vistos = new Set();
  for (let i = 0; i < argumentos.length; ++i) {
    const opcion = argumentos[i];
    if (vistos.has(opcion)) throw new Error('Parámetro repetido: ' + opcion);
    vistos.add(opcion);
    if (opcion === '--con-datos') opciones.conDatos = true;
    else if (opcion === '--sin-perdida' || opcion === '--imagenes') {
      if (opcion === '--sin-perdida' ? vistos.has('--imagenes') : vistos.has('--sin-perdida'))
        throw new Error('No combines --imagenes con --sin-perdida.');
      const valor = opcion === '--sin-perdida' ? 'sin-perdida' : argumentos[++i];
      if (!['alta', 'equilibrada', 'compacta', 'sin-perdida'].includes(valor))
        throw new Error('Indica --imagenes <alta|equilibrada|compacta|sin-perdida>.');
      opciones.perfil = valor;
    } else if (opcion === '--habitacion') {
      const valor = argumentos[++i];
      if (!valor || !/^[a-z0-9][a-z0-9_-]*$/.test(valor))
        throw new Error('Indica --habitacion <id|inicial|todas>.');
      opciones.habitacion = valor;
    } else throw new Error('Parámetro desconocido: ' + opcion);
  }
  return opciones;
}
