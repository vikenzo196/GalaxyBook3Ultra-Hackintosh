# Keyboard backlight (GalaxyBookKbd)

macOS has no driver for the Samsung Galaxy Book keyboard backlight. `GalaxyBookKbd.kext` (in the EFI) drives it through
the Samsung `SCAI` ACPI device (`SAM0429`), using the same `CSFI` protocol as Linux's `samsung-galaxybook` driver.

- The backlight turns off with the display and comes back on with it.
- `kbdlight 0`…`kbdlight 3` sets the level; `kbdlight` alone prints the status.
- Safety rule: the kext makes **no firmware calls at boot**. They are sent after login by `gbkbd-init` (a LaunchDaemon),
  so a firmware problem can never cause a boot loop. Emergency boot-arg: `-gbkbdoff`.
- `SSDT-GBKB` (source here, compiled `.aml` in the EFI) adds a `GBKB0000` bridge device that only exists under macOS
  (`_OSI("Darwin")`), because AppleACPIPlatform only evaluates methods relative to the device node.
- Needs `SCAI` to be present: `_STA` requires OSYS ≥ 2009, which is why the DSDT in this EFI has `_OSI` renamed to `XOSI`.
- Fn+F9 is not supported: on this model the key reaches neither PS/2 nor the SCAI notifications.

Install the user-space part: `bash install.sh`.

Source of the kext: `kext-src/` (`make SDK=/path/to/MacKernelSDK`).
