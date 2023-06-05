# MiniISA

Basic instruction set with a small virtual machine and assembler.

## Build

```bash
meson setup build/ --wipe # 1
env -C build/ meson compile # 2
```

By default, the `debug` buildtype is chosen by meson. To change it you can add the `--buildtype` option to command `# 1` to set the new buildtype. Valid buildtypes are `debug`, `debugoptimized` and `release`.

```bash
meson setup builddir --wipe --buildtype=release
```
