#!/bin/bash
#
# Mz7 for macOS: put everything where it belongs, for this user, without an installer.
#
# Run it through bash rather than double-clicking it:
#
#     bash ~/Downloads/Mz7-*-macOS-beta/install.sh
#
# (type "bash ", drag this file into the Terminal window, press Return). Going through bash
# is what lets it run at all: macOS refuses to open a downloaded script or plugin that has
# not been through Apple's notarization, and this project does not go through Apple.
#
# What it does, all of it inside your own home folder - nothing needs an administrator:
#
#   Mz7.vst3        ->  ~/Library/Audio/Plug-Ins/VST3
#   Mz7.component   ->  ~/Library/Audio/Plug-Ins/Components   (the Audio Unit, for Logic)
#   Mz7.app         ->  /Applications, or ~/Applications if that is not writable
#
# - whichever of them this download has - and it takes the "downloaded from the internet"
# quarantine flag off each, which is the one thing standing between an unnotarized plugin and
# a host that will load it.
#
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"

vst3_dir="$HOME/Library/Audio/Plug-Ins/VST3"
au_dir="$HOME/Library/Audio/Plug-Ins/Components"
if [ -w /Applications ]; then app_dir="/Applications"; else app_dir="$HOME/Applications"; fi

# One bundle into place, if this download has it: the old one out first, so nothing stale
# survives inside it.
place () {
    local what="$1" where="$2"
    [ -e "$here/$what" ] || return 0
    mkdir -p "$where"
    rm -rf "$where/$what"
    ditto "$here/$what" "$where/$what"
    xattr -dr com.apple.quarantine "$where/$what" 2>/dev/null || true
    echo "  $what  ->  $where"
}

echo "Installing Mz7"
place "Mz7.vst3"      "$vst3_dir"
place "Mz7.component" "$au_dir"
place "Mz7.app"       "$app_dir"

# macOS caches the list of Audio Units; this makes it look again, so Logic finds Mz7
# without a restart. Harmless if nothing is running.
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo
echo "Done. In your DAW, rescan plug-ins; Mz7 is under simozzer."
