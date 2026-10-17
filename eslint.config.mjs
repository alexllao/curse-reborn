import js from '@eslint/js';
import globals from 'globals';
import prettier from 'eslint-config-prettier/flat';

export default [
  {
    ignores: [
      'datos_originales/**',
      'recursos_extraidos/**',
      'trabajo/**',
      'node_modules/**',
      'compilacion/**',
      'build/**',
      'cmake-build-*/**',
      'licencias/**',
    ],
  },
  {
    ...js.configs.recommended,
    files: ['**/*.{js,mjs,cjs}'],
    languageOptions: { ecmaVersion: 2022, globals: globals.node },
  },
  prettier,
];
