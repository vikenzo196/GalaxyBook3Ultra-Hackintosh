# Bundled third-party binaries

These files are included unmodified, or built from source as noted, for convenience. Each keeps its own license; check the upstream project before redistributing.

| Component | Version in this EFI |
|---|---|
| Lilu (built from source 0249c3a with a 16 KB wrapper pool, see `patches/Lilu`) | 1.7.3 |
| S0ixSleep (BSD-3-Clause, github.com/vikenzo196/S0ixSleep) | 0.1.2 (F5BF6FB7) |
| HibernationFixup | 1.5.4 |
| AMFIPass | 1.4.1 |
| WhateverXe-dsc (fork build, branch `fb-norefresh`, github.com/vikenzo196/WhateverXe) | 0.8.3-gb3u.3 (98CDDCD5) |
| AppleALC | 1.9.8 (rebuilt: ALC298 layout 17 without inputs, see `patches/`) |
| SimpleHDA | 1.0.2 |
| BrightnessKeys | 1.0.4 |
| CPUFriend / CPUFriendDataProvider | 1.3.0 / 1.0.1 |
| CpuTopologyRebuild | 2.0.2 |
| ECEnabler | 1.0.6 |
| RTCMemoryFixup | 1.0.7 |
| RestrictEvents | 1.1.7 |
| VirtualSMC, SMCBatteryManager, SMCLightSensor, SMCProcessor, SMCSuperIO | 1.3.9 |
| USBToolBox / UTBMap | 1.2.0 / 1.1 |
| XHCI-unsupported | 0.9.2 |
| VoodooI2C (+ VoodooGPIO 1.1, VoodooI2CServices 1, VoodooInput 1.1.7), built from VoodooI2C master 927797d | 2.9.1 |
| VoodooI2CHID, built in the same tree: master 476c90a + touch-wake commit ([VoodooI2CHID#83](https://github.com/VoodooI2C/VoodooI2CHID/pull/83)) | 1.0 |
| VoodooPS2Controller | 2.3.8 |
| itlwm | 2.3.0 (patched, see `patches/`) |
| IntelBluetoothFirmware / IntelBTPatcher (lshbluesky fork, GPL-3.0) | 2.5.3 |
| TGLDmic (BSD-3-Clause; embeds Intel-signed SOF firmware, BSD-3-Clause + Intel patent licence) | 0.6.0 + Raptor Lake-P IDs + Siri data source (kext in EFI, Core Audio plug-in in `audio/TGLDmic/`) |
| IntelHybridCores (MIT, github.com/rodgod-rr/IntelHybridCores), built from source 360863e | 0.2.0 |
| GalaxyBookKbd (MIT, github.com/vikenzo196/GalaxyBookKbd; protocol from Linux `samsung-galaxybook`) | 1.0.0 (v7) |
| BlueToolFixup | 2.7.3 |
| OpenCore drivers | HfsPlus, OpenRuntime, ResetNvramEntry, apfs_aligned |
