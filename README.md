# MiniISA

Basic instruction set with a small virtual machine and assembler.

## Build

```bash
meson setup --prefix /usr --wipe --buildtype=debug build # 1
meson compile -C build # 2
DESTDIR=pkg meson install -C build # 3
```

By default, the `debug` buildtype is chosen by meson. To change it you can add the `--buildtype` option to command `# 1` to set the new buildtype. Valid buildtypes are `debug`, `debugoptimized` and `release`.

```bash
meson setup build --prefix /usr --wipe --buildtype=release
```

Tests are disabled by default. You can enable them by passing `-Ddo_tests=true` in the "setup" command:

```bash
meson setup --prefix /usr --wipe --buildtype=debug -Ddo_tests=true build
```
