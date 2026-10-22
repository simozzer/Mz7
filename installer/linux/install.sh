#!/bin/bash
#
# Mz7 for Linux: put everything where hosts look for it, for this user, without an installer.
#
#     bash install.sh
#
# What it does, all of it inside your own home folder - nothing needs a password:
#
#   Mz7.vst3   ->  ~/.vst3
#   Mz7.lv2    ->  ~/.lv2
#   Mz7        ->  ~/.local/bin   (the standalone app)
#
# - whichever of them this download has. Those are the folders every Linux host - Ardour,
# Bitwig, Reaper, Carla, Qtractor, Zrythm - searches for plug-ins.
#
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"

# One bundle into place, if this download has it: the old one out first, so nothing stale
# survives inside it.
place () {
    local what="$1" where="$2"
    [ -e "$here/$what" ] || return 0
    mkdir -p "$where"
    rm -rf "${where:?}/$what"
    cp -r "$here/$what" "$where/$what"
    echo "  $what  ->  $where"
}

echo "Installing Mz7"
place "Mz7.vst3" "$HOME/.vst3"
place "Mz7.lv2"  "$HOME/.lv2"
place "Mz7"      "$HOME/.local/bin"

echo
echo "Done. In your DAW, rescan plug-ins; Mz7 is under simozzer."
