# ESP-IDF Manager & Launcher

Command-line tool written in C for installing, managing, and running multiple ESP-IDF versions from a single user account.

## Requisitos

- Linux
- GCC and Make
- Git
- Bash
- Internet connection to clone ESP-IDF

ESP-IDF installation also downloads and installs its tools and dependencies using Espressif's `install.sh` script.

## Compilar

From the project directory:

```sh
make
```

This builds the `esp-idf` executable. To remove the generated executable:

```sh
make clean
```

## Instalar el comando

```sh
make install
```

The executable is copied to `~/.local/bin/esp-idf`. Make sure `~/.local/bin` is included in your `PATH`.

To uninstall the executable:

```sh
make uninstall
```

This target only removes the installed command; it does not remove downloaded ESP-IDF versions.

## Uso

### Install a version

```sh
esp-idf -i
```

The installer offers these predefined options:

- ESP-IDF v4.4
- ESP-IDF v5.0
- ESP-IDF v5.1
- ESP-IDF v5.2
- ESP-IDF v5.3
- ESP-IDF v5.4
- `master` branch
- A custom branch or tag

Versions are cloned under `~/esp/`. For example, the `release/v5.4` branch is placed in `~/esp/esp-idf-release-v5.4`.

### List installed versions

```sh
esp-idf -l
# Also: esp-idf --list
```

### Open a shell with a version

Use the name shown in the version list, or the branch-derived name (slashes are replaced with hyphens):

```sh
esp-idf -v release-v5.4
```

Type `exit` to leave the shell and close the ESP-IDF environment.

### Set the default version

```sh
esp-idf -d release-v5.4
```

Afterward, running `esp-idf` without arguments opens a shell with that version. The setting is saved in `~/.config/esp-idf/default`.

### Run a command in the environment

```sh
esp-idf -x release-v5.4 "idf.py --version"
```

### Uninstall a version

```sh
esp-idf -u release-v5.4
```

**Warning:** this permanently deletes that version's directory under `~/esp/`.

### Help

```sh
esp-idf
```

With no arguments, the program opens the default version if one is configured; otherwise, it displays the help text.

## Storage locations

- Installations: `~/esp/esp-idf-<version-name>`
- Default version: `~/.config/esp-idf/default`
- Installed executable: `~/.local/bin/esp-idf`
