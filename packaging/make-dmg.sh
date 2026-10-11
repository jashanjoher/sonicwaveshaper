#!/bin/bash
# Builds SonicWaveshaper-macOS.dmg from the CMake build output.
set -euo pipefail

OUT="build/SonicWaveshaper_artefacts/Release"
STAGE="dmg-stage"

# Stop with a clear message if any plugin bundle is missing.
for p in "$OUT/VST3/SonicWaveshaper.vst3" "$OUT/AU/SonicWaveshaper.component" "$OUT/Standalone/SonicWaveshaper.app"; do
  if [ ! -e "$p" ]; then
    echo "Missing: $p"
    echo "What the build produced:"
    find build -maxdepth 4 -print
    exit 1
  fi
done

rm -rf "$STAGE"
mkdir "$STAGE"

cp -R "$OUT/VST3/SonicWaveshaper.vst3" "$STAGE/"
cp -R "$OUT/AU/SonicWaveshaper.component" "$STAGE/"
cp -R "$OUT/Standalone/SonicWaveshaper.app" "$STAGE/"

ln -s /Library/Audio/Plug-Ins/VST3 "$STAGE/1-VST3-folder"
ln -s /Library/Audio/Plug-Ins/Components "$STAGE/2-AU-folder"
ln -s /Applications "$STAGE/3-Applications"

cp packaging/README-install.txt "$STAGE/READ ME FIRST.txt"

hdiutil create -volname "SonicWaveshaper" -srcfolder "$STAGE" -ov -format UDZO SonicWaveshaper-macOS.dmg
echo "Done: SonicWaveshaper-macOS.dmg"
