# ESP-IDF Manager & Launcher

Herramienta de línea de comandos en C para instalar, administrar y ejecutar varias versiones de ESP-IDF desde una misma cuenta de usuario.

## Requisitos

- Linux
- GCC y Make
- Git
- Bash
- Conexión a Internet para clonar ESP-IDF

La instalación de ESP-IDF también descarga e instala sus propias herramientas y dependencias mediante el script `install.sh` de Espressif.

## Compilar

Desde el directorio del proyecto:

```sh
make
```

Esto genera el ejecutable `esp-idf`. Para borrar el ejecutable generado:

```sh
make clean
```

## Instalar el comando

```sh
make install
```

El ejecutable se copia a `~/.local/bin/esp-idf`. Asegúrate de que `~/.local/bin` esté incluido en tu variable `PATH`.

Para desinstalar el ejecutable:

```sh
make uninstall
```

Este objetivo solo elimina el comando instalado; no elimina las versiones de ESP-IDF descargadas.

## Uso

### Instalar una versión

```sh
esp-idf -i
```

El instalador ofrece estas opciones predefinidas:

- ESP-IDF v4.4
- ESP-IDF v5.0
- ESP-IDF v5.1
- ESP-IDF v5.2
- ESP-IDF v5.3
- ESP-IDF v5.4
- Rama `master`
- Una rama o etiqueta personalizada

Las versiones se clonan bajo `~/esp/`. Por ejemplo, la rama `release/v5.4` queda en `~/esp/esp-idf-release-v5.4`.

### Listar versiones instaladas

```sh
esp-idf -l
# También: esp-idf --list
```

### Abrir una shell con una versión

Usa el nombre mostrado por el listado o derivado de la rama (las barras se reemplazan por guiones):

```sh
esp-idf -v release-v5.4
```

Al salir de la shell con `exit`, se cierra el entorno de ESP-IDF.

### Configurar la versión predeterminada

```sh
esp-idf -d release-v5.4
```

Después, ejecutar `esp-idf` sin argumentos abre la shell con esa versión. La configuración se guarda en `~/.config/esp-idf/default`.

### Ejecutar un comando en el entorno

```sh
esp-idf -x release-v5.4 "idf.py --version"
```

### Desinstalar una versión

```sh
esp-idf -u release-v5.4
```

**Atención:** esta opción elimina permanentemente el directorio de esa versión dentro de `~/esp/`.

### Ayuda

```sh
esp-idf
```

Sin argumentos, el programa abre la versión predeterminada si está configurada; si no, muestra la ayuda.

## Estructura de almacenamiento

- Instalaciones: `~/esp/esp-idf-<nombre-de-versión>`
- Versión predeterminada: `~/.config/esp-idf/default`
- Ejecutable instalado: `~/.local/bin/esp-idf`
