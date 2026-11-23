# Samsung Galaxy Book3 Ultra (NP964XFH) – macOS Tahoe Hackintosh

OpenCore EFI and notes for the **Samsung Galaxy Book3 Ultra (NP964XFH)** running **macOS Tahoe 26.7.3 (25G342)**, with the Intel Iris Xe driving the internal 2880x1800 OLED at 120 Hz.

> Stable for daily use. No serial numbers are included: generate your own SMBIOS values (see Installation).

## Hardware

| | |
|---|---|
| CPU | Intel Core i9-13900H (6 P-cores + 8 E-cores, 20 threads) |
| iGPU | Iris Xe, device ID 0xA7A0 |
| dGPU | NVIDIA RTX 4070, powered off (`SSDT-dGPU-Off`). macOS has no driver for it |
| Display | Samsung ATNA60CL03 OLED, 2880x1800 @ 120 Hz, eDP 1.4 with DSC |
| Wi-Fi / BT | Intel AX211 |
| Audio | Realtek ALC298 (subsystem 144D:C1CC) + 4 Samsung smart amps |
| Trackpad | I2C precision touchpad (IMG4100) |
| Fingerprint | EGIS ETU905A80-E (USB 1c7a:05a1) |

## Status

**Working**
- Graphics acceleration on the internal OLED at full resolution and **120 Hz** (Metal 2, hardware video decode), using a
  [WhateverXe fork](https://github.com/vikenzo196/WhateverXe) that keeps the firmware-trained eDP link and its DSC setup
- Clean boot and shutdown, with no white bands during the handoff (see Fixes)
- Display sleep / wake, brightness slider and Fn+F2/F3 keys
- **External monitor over USB-C** (DP alt mode), tested with a Dell WD19 dock: one monitor, extended or mirrored,
  plugged at boot or hot-plugged, with the dock's USB and Ethernet. MST docks run in SST mode, so one external monitor
- Correct colors from the login window (display profile, see Fixes)
- Trackpad as a precision touchpad with gestures; **moving a finger wakes the display**, even after a long sleep
- Keyboard backlight: turns off and on with the display, level 0–3 from the command line, with [GalaxyBookKbd](https://github.com/vikenzo196/GalaxyBookKbd) (user-space part also in `extras/keyboard-backlight/`)
- P/E-core aware scheduling ([IntelHybridCores](https://github.com/rodgod-rr/IntelHybridCores)): background work runs on the E-cores
- CPU boost (single core ~4.8 GHz with XE-PM); unused PCI functions put in D3
- Wi-Fi (itlwm, patched for a network-switch panic, see `patches/`)
- Bluetooth (AX211 on internal port HS10, lshbluesky's Tahoe fork of IntelBluetoothFirmware + IntelBTPatcher)
- Audio (ALC298 + amplifier init); optional Dolby-derived EQ, see `audio/`
- Internal microphone via [TGLDmic](https://github.com/lamjiidii1/TGLDmic) (Raptor Lake-P support from
  [PR #1](https://github.com/lamjiidii1/TGLDmic/pull/1)), including Siri; Core Audio plug-in in `audio/TGLDmic/`
- Apple Music, through a small sandbox wrapper (`extras/apple-music/`)
- Continuity over Bluetooth: Handoff and Universal Clipboard

**Limits / known issues**
- S3 sleep: not supported by this firmware (it cold-boots on resume), so sleep is disabled
- Battery life: about 15–16 W at idle, so roughly 3.5 h. The package stays in PC2/PC3, as it does on Windows with the
  vendor drivers: this is a platform limit. Dark mode saves about 1 W on the OLED.
- Scrolling in heavy apps (App Store, WhatsApp) has occasional hitches at 120 Hz. Instruments attributes them to the
  apps' own main-thread work, not to the GPU or the driver
- The animated lock-screen wallpaper has occasional micro-stutter: WindowServer is close to the 8.3 ms budget at 120 Hz
- AirPlay screen mirroring: the encoder works, but the TV stays on the first frame
- AirDrop, Apple Watch unlock, Instant Hotspot: no AWDL with itlwm

**Not supported (no driver available)**
- Webcam (Intel IPU6 MIPI)
- Fingerprint reader: firmware 9050.1.2.33 keeps prints in a protected store; see
  [hack-touchid#15](https://github.com/hackintosh-user/hack-touchid/issues/15)
- RTX 4070: no NVIDIA driver exists for macOS after Kepler. It stays powered off to save battery

## Fixes

| Problem | Fix |
|---|---|
| No acceleration / black panel on the DSC OLED | WhateverXe fork with `-wex-edp-keep -wex-edp-dsc`: keep the GOP's trained eDP link and DSC/joiner configuration instead of retraining. The official build stops at the Type-C hook gate on this board ([WhateverXe#43](https://github.com/b00t0x/WhateverXe/issues/43)) |
| External monitor detected but black (USB-C dock) | The fork hides the dock's MST capability (SST mode) and keeps CDCLK running when an output is added next to the kept eDP pipe. Before, both refusals faulted every later output operation ([WhateverXe fork 0.8.3-gb3u.2](https://github.com/vikenzo196/WhateverXe/releases/tag/0.8.3-gb3u.2)) |
| White bands for ~1 s at boot and 2–3 s at shutdown | The native framebuffer switches the primary plane to Y-tiling while it still scans out the linear firmware image. The fork keeps the plane dark in those windows ([WhateverXe#189](https://github.com/b00t0x/WhateverXe/issues/189)) |
| ~10 s pause at the progress bar during boot | `gfxrst=4`: skip the slow copy of the firmware image through an uncached mapping |
| OLED brightness did nothing / flashed | The panel only answers the VESA DPCD interface (11-bit level, DPCD 0x724..0x726); `-wex-edp-auxbl` in the fork, plus `SSDT-PNLF` |
| Fn+F2/F3 did nothing | `SSDT-BRTKEYS` plus a DSDT rename send the EC brightness events to BrightnessKeys |
| Colors change a few seconds after login | The display profile was set for one user only. Set it for all users (ColorSync, "any user" scope) and keep Accessibility → Display contrast at 0. A non-zero contrast also shows a "Display contrast increased" popup at every login |
| Trackpad only woke the display with a click | VoodooI2CHID declares user activity on touch ([VoodooI2CHID#83](https://github.com/VoodooI2C/VoodooI2CHID/pull/83)). VoodooI2C, its plug-ins and VoodooI2CHID in this EFI are built **together** from one tree: never swap only the HID kext |
| Background work running on the P-cores | IntelHybridCores (mode 2) with CpuTopologyRebuild `ctrsmt=off` (`ctrsmt=full` disables its scheduler). Background package power drops by 18–33% |
| Low single-core clocks | XE-PM (`-wex-pm`) in the fork: HWP rescale on hybrid P-cores, EPP 0 |
| Kernel panic when switching Wi-Fi networks | Patched itlwm, see `patches/itlwm` ([itlwm#1078](https://github.com/OpenIntelWireless/itlwm/pull/1078)) |
| Full-screen video crashed WindowServer | WindowServer YUV patch in the fork (YUV layers composited by Metal instead of GVA). Do **not** disable it with `-wex-ws-gva`: the lock-screen wallpaper video then hangs the GPU |
| Speakers sound thin compared with Windows | EQ derived from Samsung's Dolby tuning for this model, see `audio/` |

The modified WhateverXe source is at <https://github.com/vikenzo196/WhateverXe> (branch `fb-norefresh`: upstream 0.8.3
plus this laptop's eDP DSC/AUX backlight, XE-PM, GPU-reset, WindowServer YUV, boot/shutdown handoff and USB-C output work).
`WhateverXe-dsc.kext` in this EFI is built from it.

## Installation

1. Take `BOOTx64.efi` and the base layout from the official [OpenCore release](https://github.com/acidanthera/OpenCorePkg/releases); this repo only ships `EFI/OC`.
2. Copy `EFI/OC` next to it.
3. Generate your own SMBIOS (for example with GenSMBIOS) for `MacBookPro16,2` and fill in `PlatformInfo > Generic`: `SystemSerialNumber`, `MLB`, `SystemUUID`, `ROM` (6 bytes, for example your Mac's `en0` address). They are set to `CHANGE_ME` / `CHANGE` here.
4. Install [WhateverXe Graphics 0.4.4](https://github.com/b00t0x/WhateverXe-Graphics/releases) in macOS (TGL driver kexts in `/Library/Extensions`, `AppleIntelTGLCompat` 0.8.1).
5. Optional user-space parts: keyboard backlight (`extras/keyboard-backlight/install.sh`), microphone plug-in (`audio/TGLDmic/install-hal.sh`), Apple Music wrapper (`extras/apple-music/`).
6. Keep a second OpenCore on a USB stick as a fallback. If it lacks `SSDT-PNLF`, brightness stays at minimum.

Notes:
- `ACPI/DSDT.aml` is a patched dump of this laptop's firmware table (`_OSI` → `XOSI`, EC brightness rename). It depends on the BIOS version: replace it with your own patched dump if yours differs.
- OpenCore boot-args are applied from `config.plist` at every boot (NVRAM Delete + Add). Change them there, not only with `nvram`.
- The picker timeout is 1 s: press any key to stop it. File logging is off (`Misc/Debug/Target = 3`; 67 turns it back on).

## Credits

Acidanthera (OpenCore, Lilu, VirtualSMC, AppleALC, WhateverGreen and friends), b00t0x (WhateverXe, WhateverXe Graphics),
the NootedGreen authors, the VoodooI2C and VoodooPS2 teams, OpenIntelWireless (itlwm, IntelBluetoothFirmware) and
lshbluesky (Tahoe Bluetooth fork), rodgod-rr (IntelHybridCores), lamjiidii1 (TGLDmic), USBToolBox, and the authors of
Linux's `samsung-galaxybook` driver (protocol used by GalaxyBookKbd).

## License

The files written for this repository (the OpenCore `config.plist`, the ACPI sources, GalaxyBookKbd, the notes and the
scripts under `audio/` and `extras/`) are released under the [MIT License](LICENSE).

That license does not cover the bundled third-party binaries (kexts, OpenCore, drivers, firmware) or the patches
under `patches/`. Each of those keeps the license of its upstream project: see [THIRD-PARTY.md](THIRD-PARTY.md).
