# Apple Music (FairPlay) with WhateverXe Graphics

With the Intel VA driver from WhateverXe Graphics installed, Music.app plays nothing from the Apple Music catalogue:
the time advances (or playback stops) and there is no sound. The log shows

    AVD Error: Gen75IntelGVAGetDriverParameter: ME not loaded
    GVA error: flow_KE1 getParam 7
    Key request data generation FAILED because we were unable to generate a key (err = -15842)

FairPlay tries the hardware key exchange through the VA driver, and that fails on this machine (also with a VAME
bundle installed: the ME channel opens, the key is still refused). When the VA driver cannot be loaded, FairPlay
falls back to its software path and the key request succeeds.

So Music is started in a sandbox that hides only that bundle from it. Everything else keeps hardware decoding.

    sudo mkdir -p /usr/local/share
    sudo cp music-novagva.sb /usr/local/share/
    sandbox-exec -f /usr/local/share/music-novagva.sb /System/Applications/Music.app/Contents/MacOS/Music &

To get a clickable launcher:

    osacompile -o "/Applications/Music FairPlay.app" -e 'do shell script "/usr/bin/sandbox-exec -f /usr/local/share/music-novagva.sb /System/Applications/Music.app/Contents/MacOS/Music >/dev/null 2>&1 &"'

Music opened from its normal icon runs without the sandbox and stays silent.
