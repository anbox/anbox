# Generate Android EmuGL source

Parts of the EmuGL layer are generated with a tool called emugen (see
external/android-emugl/host/tools/emugen).

To generate the source again after a modification simply call

```
$ scripts/update-emugl-sources.sh --emugen=<path to emugen>/emugen
```

The definition of the various attributes/types/functions can be found in

 * external/android-emugl/host/libs/renderControl_dec
 * external/android-emugl/host/libs/GLESv1_dec
 * external/android-emugl/host/libs/GLESv2_dec
