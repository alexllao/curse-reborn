import { resolve } from 'node:path';

export function opcionesIOS(argumentos, raiz) {
  const opciones = {
    simulador: false,
    soloRecursos: false,
    abrir: true,
    imagenes: 'alta',
    musica: 'mp3',
    datos: resolve(raiz, 'apps/juego/datos_originales/CURSE'),
    equipo: '',
  };
  const vistos = new Set();
  for (let i = 0; i < argumentos.length; ++i) {
    const arg = argumentos[i];
    if (vistos.has(arg)) throw new Error('Parámetro repetido: ' + arg);
    vistos.add(arg);
    if (arg === '--simulador') opciones.simulador = true;
    else if (arg === '--solo-recursos') opciones.soloRecursos = true;
    else if (arg === '--sin-abrir') opciones.abrir = false;
    else if (['--datos', '--imagenes', '--musica', '--equipo'].includes(arg)) {
      const valor = argumentos[++i];
      if (!valor || valor.startsWith('--')) throw new Error('Falta el valor de ' + arg);
      if (arg === '--datos') opciones.datos = resolve(valor);
      else opciones[arg.slice(2)] = valor;
    } else throw new Error('Parámetro iOS desconocido: ' + arg);
  }
  if (!['alta', 'equilibrada', 'compacta', 'sin-perdida'].includes(opciones.imagenes))
    throw new Error('Usa --imagenes alta|equilibrada|compacta|sin-perdida.');
  if (!['mp3', 'wav'].includes(opciones.musica)) throw new Error('Usa --musica mp3|wav.');
  if (opciones.equipo && !/^[A-Z0-9]{10}$/.test(opciones.equipo))
    throw new Error('Team ID de Apple inválido.');
  return opciones;
}

// No firma ni compila: genera un proyecto que el usuario revisa y ejecuta desde Xcode.
export function argumentosCmakeIOS(raiz, generados, opciones, arquitectura = process.arch) {
  const sdk = opciones.simulador ? 'iphonesimulator' : 'iphoneos';
  const cpu = opciones.simulador && arquitectura === 'x64' ? 'x86_64' : 'arm64';
  return [
    '-S',
    resolve(raiz, 'apps/juego/plataformas/ios'),
    '-B',
    resolve(raiz, 'apps/juego/trabajo/ios', opciones.simulador ? 'simulador' : 'dispositivo'),
    '-G',
    'Xcode',
    '-DCMAKE_SYSTEM_NAME=iOS',
    '-DCMAKE_OSX_SYSROOT=' + sdk,
    '-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY',
    '-DCMAKE_OSX_ARCHITECTURES=' + cpu,
    '-DCMAKE_OSX_DEPLOYMENT_TARGET=15.0',
    '-DCURSE_IOS_GENERADOS=' + generados,
    ...(opciones.equipo ? ['-DCURSE_IOS_EQUIPO=' + opciones.equipo] : []),
  ];
}
