Mz7 @VERSION@ - a DX7 emulation for Windows (64-bit)
https://github.com/simozzer/Mz7

The easiest way to install is Mz7-@VERSION@-win64-Setup.exe from the releases page,
which installs either or both of these and adds an uninstaller. This zip is for
installing by hand.

VST3
  Copy the Mz7.vst3 folder into either
    %LOCALAPPDATA%\Programs\Common\VST3        (just you - no admin needed)
    C:\Program Files\Common Files\VST3         (everyone - needs admin)
  then rescan plugins in your DAW. If it does not appear, add the first folder
  to the DAW's VST3 plugin paths.

Standalone
  Run Mz7.exe. Options (top left) chooses the audio and MIDI devices.

Windows may warn that the app is from an unknown publisher: the binaries are
not code-signed. Choose "More info" then "Run anyway", or build from source.

Created by Simon Moscrop.
Free software under the GNU AGPL v3 - see LICENSE. Source code:
https://github.com/simozzer/Mz7/tree/v@VERSION@
