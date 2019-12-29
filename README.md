# RetroFW scaling demo

This demo shows the difference in quality between native 1:2 Pixel Aspect Ratio 320x480 and upscaled 320x240 resolutions.

## Building

### RetroFW

To build for RetroFW, run:

```bash
BUILDROOT=<path to buildroot> ./build-retrofw.sh
```

The package will be built at `build-retrofw/scaling_demo.opk`.

### Host

To build for your host system, install SDL 1.2 and freetype2, then run:

```bash
mkdir -p build
cd build
cmake ..
cmake --build .
```

The binary will be built at `build/scaling_demo`.

## Image DPI

`lenna_128x256.png` has its DPI metadata set correctly:

```bash
exiv2 -pv lenna_128x256.png
0x011a Image        XResolution                 Rational    1  72/1
0x011b Image        YResolution                 Rational    1  144/1
```

It was set with the following command:

```bash
exiv2 -m <(printf 'set Exif.Image.XResolution 72/1\nset Exif.Image.YResolution 144/1') lenna_128x256.png
```

This information is not yet used by the demo app.
