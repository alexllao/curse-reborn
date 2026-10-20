# Archivos del juego original

Copia aquí tu juego original de **Curse of Enchantia para DOS** y, si quieres música Roland MT-32, sus ROM. Estos archivos no se descargan ni se suben a Git. El paquete beta predeterminado los deja fuera; `pnpm empaquetar:beta --con-datos` copia el ejecutable y los recursos locales del juego, excluyendo ROM y partidas. Véase [Distribución](../documentacion/distribucion.md).

La estructura debe quedar así, sin añadir otra carpeta intermedia:

```text
apps/juego/datos_originales/
├── CURSE/
│   ├── CURSE.EXE
│   └── DATA/
│       ├── CORE.DAT
│       ├── CORE.PAL
│       ├── TITLE.DAT
│       ├── TITLE.PAL
│       └── ... resto de archivos originales de DATA
└── ROMS/
    ├── MT32_CONTROL.ROM
    └── MT32_PCM.ROM
```

En `CURSE/` puedes copiar la carpeta completa del juego: conserva `CURSE.EXE` y todo su directorio `DATA/`. No basta con copiar el ejecutable o unas pocas imágenes.

En `ROMS/` coloca las dos ROM del MT-32 con los nombres indicados. La referencia utilizada para comprobar el port es MT-32 antiguo, Control 1.07 y su PCM correspondiente. Las ROM solo son necesarias para la opción de música MT-32; sin ellas el port utiliza AdLib.

Después, desde la raíz del repositorio:

```text
pnpm install --frozen-lockfile
pnpm preparar
pnpm ventana
```

El comando de preparación obtiene las dependencias de desarrollo; no descarga el juego ni las ROM. Los ejecutables y resultados generados se guardan en `apps/juego/trabajo/`.
