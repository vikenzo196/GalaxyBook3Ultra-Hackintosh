# Samsung Galaxy Book3 Ultra (NP964XFH) – macOS Tahoe Hackintosh

OpenCore EFI and notes for the **Samsung Galaxy Book3 Ultra (NP964XFH)** running **macOS Tahoe 26.7.1 (25G241)**, with the Intel Iris Xe driving the internal 2880x1800 OLED at 120 Hz.

> Work in progress: it is stable for daily use, but several things are still missing (see below).
> No serial numbers are included. You must generate your own SMBIOS values.

## Hardware

| | |
|---|---|
| CPU | Intel Core i9-13900H (hybrid P/E cores) |
| iGPU | Iris Xe, device ID 0xA7A0 |
| dGPU | NVIDIA RTX 4070, disabled (`SSDT-dGPU-Off`) |
| Display | Samsung ATNA60CL03 OLED, 2880x1800 @ 120 Hz, eDP with DSC |
| Wi-Fi / BT | Intel AX211 |
| Audio | Realtek ALC298 |


## Status

**Working**
- Graphics acceleration on the internal OLED at full resolution and 120 Hz
- Brightness slider and Fn+F2/F3 keys
- Trackpad as a precision touchpad (VoodooI2CHID)
- Wi-Fi (itlwm)
- Audio (ALC298)
- CPU boost (single core ~3.97 -> ~4.88 GHz with XE-PM)

**Work in progress**
- Scrolling fluidity: each frame is flipped twice by the hardware (about 80 WindowServer flips/s vs 160 hardware flips/s at 120 Hz); being traced
- Display sleep: with the backlight SSDT it can hang the GPU, so `pmset -a displaysleep 0 halfdim 0` is used for now
- AirPlay screen mirroring: the encoder works, the TV stays on the first frame
- S3 sleep and Bluetooth: parked (the Bluetooth kexts are not included)
- iPhone calls / Continuity: itlwm is seen as Ethernet, not AirPort

**Not supported (no driver available)**
- Webcam (Intel IPU6 MIPI)
- Touch ID (Elan fingerprint reader)

## Fixes

| Problem | Fix |
|---|---|
| OLED brightness did nothing / flashed | The panel only answers the VESA DPCD interface. The level register is 11 bits wide (max 0x7FF, read from DPCD 0x724..0x726); the firmware's Intel TCON brightness bit (0x344 bit 4) is cleared first. Boot-arg `-wex-edp-auxbl` in the WhateverXe fork, plus `SSDT-PNLF` |
| Fn+F2/F3 did nothing | EC `_Q63/_Q64` only notify the display when the Windows driver is ready. `SSDT-BRTKEYS` plus the DSDT rename send the key codes to VoodooPS2/BrightnessKeys |
| Low single-core clocks | XE-PM (`-wex-pm`): rescales HWP on hybrid P-cores, EPP 0, C-state tuning |
| `mediaanalysisd` / AirPlay crash (`EXC_BAD_ACCESS` in `AVD_DestroyAVDAccelerator`) | 8-byte patch in the TGL VA driver's VPP path (not part of this EFI, applied on the system volume) |

The modified WhateverXe source is at <https://github.com/vikenzo196/WhateverXe>; `WhateverXe-dsc.kext` in this EFI is built from it.

## Installation

1. Take `BOOTx64.efi` and the base layout from the official [OpenCore release](https://github.com/acidanthera/OpenCorePkg/releases); this repo only ships `EFI/OC`.
2. Copy `EFI/OC` next to it.
3. Generate your own SMBIOS (for example with GenSMBIOS) for `MacBookPro16,2` and fill in `PlatformInfo > Generic`: `SystemSerialNumber`, `MLB`, `SystemUUID`, `ROM` (6 bytes, for example your Mac's `en0` address). They are set to `CHANGE_ME` / `CHANGE` here.
4. Review the boot-args: `-v debug=0x100 keepsyms=1 msgbuf=1048576` are debug options and can be removed once everything works.

`ACPI/DSDT.aml` is a patched dump of this laptop's firmware table. It depends on the BIOS version and you should replace it with your own dump if yours differs.

## Credits

Acidanthera (OpenCore, Lilu, VirtualSMC, AppleALC, WhateverGreen and friends), the WhateverXe and NootedGreen authors, the VoodooI2C and VoodooPS2 teams, OpenIntelWireless (itlwm), USBToolBox.
