# Samsung Galaxy Book3 Ultra (NP964XFH) – macOS Tahoe Hackintosh

OpenCore EFI and notes for the **Samsung Galaxy Book3 Ultra (NP964XFH)** running **macOS Tahoe 26.7.3 (25G342)**, with the Intel Iris Xe driving the internal 2880x1800 OLED at 120 Hz.

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
| Audio | Realtek ALC298 (subsystem 144D:C1CC) + 4 Samsung smart amps |
| Fingerprint | EGIS ETU905A80-E (USB 1c7a:05a1) |


## Status

**Working**
- Graphics acceleration on the internal OLED at full resolution and 120 Hz (Metal, hardware video decode)
- Display sleep / wake (`-wex-edp-blank`)
- Brightness slider and Fn+F2/F3 keys
- Trackpad as a precision touchpad (VoodooI2CHID)
- Wi-Fi (itlwm, patched for a network-switch panic, see `patches/`)
- Audio (ALC298 + amplifier init); optional Dolby-derived EQ, see `audio/`
- Bluetooth (AX211, `8087:0033` on internal port HS10): IntelBluetoothFirmware + IntelBTPatcher 2.5.3 from the
  [lshbluesky fork](https://github.com/lshbluesky/IntelBluetoothFirmware) (Tahoe support), BlueToolFixup, `-ibtcompatbeta`
- CPU boost (single core ~3.97 -> ~4.88 GHz with XE-PM); unused PCI functions put in D3 by XE-PM

**Work in progress**
- GPU hang recovery: the engine reset now recovers the render engine (RCS register replay, RCS reset tail after a
  render-domain reset, the donor's post-resume notification), but a workload that hangs again on every replay can
  still starve WindowServer until its watchdog fires. Diagnostics are built into the fork (`WEXRecovery*` in ioreg)
- Package C-states stay at PC2/PC3 (also on Windows with the vendor drivers): platform limit
- AirPlay screen mirroring: the encoder works, the TV stays on the first frame
- S3 sleep: parked (the firmware's S3 path does not resume)
- iPhone calls / Continuity: itlwm is seen as Ethernet, not AirPort

**Not supported (no driver available)**
- Webcam (Intel IPU6 MIPI)
- Fingerprint reader (EGIS ETU905A80-E): firmware 9050.1.2.33 keeps prints in a protected store; see
  [hack-touchid#15](https://github.com/hackintosh-user/hack-touchid/issues/15)

## Fixes

| Problem | Fix |
|---|---|
| OLED brightness did nothing / flashed | The panel only answers the VESA DPCD interface. The level register is 11 bits wide (max 0x7FF, read from DPCD 0x724..0x726); the firmware's Intel TCON brightness bit (0x344 bit 4) is cleared first. Boot-arg `-wex-edp-auxbl` in the WhateverXe fork, plus `SSDT-PNLF` |
| Fn+F2/F3 did nothing | EC `_Q63/_Q64` only notify the display when the Windows driver is ready. `SSDT-BRTKEYS` plus the DSDT rename send the key codes to VoodooPS2/BrightnessKeys |
| Low single-core clocks | XE-PM (`-wex-pm`): rescales HWP on hybrid P-cores, EPP 0, C-state tuning |
| `mediaanalysisd` / AirPlay / VideoToolbox crashes | Fixed upstream in [WhateverXe-Graphics](https://github.com/b00t0x/WhateverXe-Graphics/releases) 0.4.3+ (use 0.4.4 with WhateverXe 0.8.2) |
| Kernel panic when switching Wi-Fi networks (`zfree_drop: double free` in `iwx_send_cmd`) | Patched itlwm, see `patches/itlwm` |
| Speakers sound thin compared with Windows (Dolby Atmos) | EQ derived from Samsung's Dolby tuning for this model, see `audio/` |
| Slow shutdown | `BlueToolFixup` |

The modified WhateverXe source is at <https://github.com/vikenzo196/WhateverXe> (branch `gpu-reset-082`: upstream 0.8.2 plus this laptop's eDP DSC/AUX backlight, XE-PM and GPU-reset work); `WhateverXe-dsc.kext` in this EFI is built from it.

## Installation

1. Take `BOOTx64.efi` and the base layout from the official [OpenCore release](https://github.com/acidanthera/OpenCorePkg/releases); this repo only ships `EFI/OC`.
2. Copy `EFI/OC` next to it.
3. Generate your own SMBIOS (for example with GenSMBIOS) for `MacBookPro16,2` and fill in `PlatformInfo > Generic`: `SystemSerialNumber`, `MLB`, `SystemUUID`, `ROM` (6 bytes, for example your Mac's `en0` address). They are set to `CHANGE_ME` / `CHANGE` here.
4. Install [WhateverXe Graphics 0.4.4](https://github.com/b00t0x/WhateverXe-Graphics/releases) in macOS (TGL driver
   kexts in `/Library/Extensions`, `AppleIntelTGLCompat` 0.8.1). Older Compat builds only accept exact macOS
   builds and lose acceleration after a point update.
5. Review the boot-args: verbose mode (`-v`) is off. `keepsyms=1 msgbuf=1048576` only help reading panics and kernel logs and can be removed if not needed.

`ACPI/DSDT.aml` is a patched dump of this laptop's firmware table. It depends on the BIOS version and you should replace it with your own dump if yours differs.

## Credits

Acidanthera (OpenCore, Lilu, VirtualSMC, AppleALC, WhateverGreen and friends), the WhateverXe and NootedGreen authors, the VoodooI2C and VoodooPS2 teams, OpenIntelWireless (itlwm, IntelBluetoothFirmware) and lshbluesky (Tahoe Bluetooth fork), USBToolBox.
