#!/usr/bin/env bash

set -euo pipefail

check_buildroot() {
  if ! [[ -d $BUILDROOT ]]; then
    echo "Please set the BUILDROOT environment variable"
    exit 1
  fi
}

make_buildroot() {
  cd "$BUILDROOT"
  # Check dependencies manually as it's much faster than `make`.
  local -a deps=()
  if ! [[ -f output/staging/usr/include/SDL/SDL.h ]]; then
    deps+=(SDL)
  fi
  if ! [[ -f output/staging/usr/include/SDL/SDL_image.h ]]; then
    deps+=(sdl_image)
  fi
  if ! [[ -d output/staging/usr/include/freetype2/ ]]; then
    deps+=(freetype)
  fi
  if ! [[ -f output/host/usr/share/buildroot/toolchainfile.cmake ]]; then
    deps+=(toolchain)
  fi
  if (( ${#deps[@]} )); then
    make "${deps[@]}" BR2_JLEVEL=0
  fi
  cd -
}

build() {
  mkdir -p build-retrofw
  cd build-retrofw
  cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DRETROFW=1 \
    '-DQUIT_KEY_LABEL="SELECT"' \
    '-DCHANGE_MODE_KEY_LABEL="START"' \
    -DCMAKE_TOOLCHAIN_FILE="$BUILDROOT/output/host/usr/share/buildroot/toolchainfile.cmake"
  cmake --build .
  cd -
}

package_opk() {
  cd build-retrofw
  cat > default.retrofw.desktop <<EOF
[Desktop Entry]
Name=Scaling Demo
Comment=Native vs IPU-upscaled comparison
Exec=scaling_demo.dge
StartupNotify=true
Terminal=false
Type=Application
StartupNotify=true
Categories=applications;
X-OD-Selector=/home/retrofw/apps/gmenu2x/skins/Default/
X-OD-Filter=.ttf
EOF
  mksquashfs \
    default.retrofw.desktop \
    scaling_demo.dge \
    ../lenna_128x128.png \
    ../lenna_128x256.png \
    retrofw_scaling_demo.opk \
  -all-root -no-xattrs -noappend -no-exports
  cd -
}

main() {
  check_buildroot
  set -x
  make_buildroot
  build
  package_opk
}

main