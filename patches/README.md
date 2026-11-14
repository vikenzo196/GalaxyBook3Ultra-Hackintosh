# Patches

- `itlwm/itlwm-doublefree.patch` — against [itlwm](https://github.com/OpenIntelWireless/itlwm) v2.3.0.
  Fixes a `zfree_drop: double free` kernel panic in `ItlIwx::iwx_send_cmd` when switching networks:
  `splnet()`/`splx()` are no-ops in this port, so concurrent callers could pick the same command slot and
  free the same `sc_cmd_resp_pkt` buffer twice. Adds one mutex around slot reservation and every access to
  `sc_cmd_resp_pkt` (never held across `tsleep`); after the wait each caller takes/frees only its own buffer.
  The `itlwm.kext` in this EFI is built with it (Xcode 26.5, MacKernelSDK).
- `HeliPort/heliport-tahoe-menu.patch` — against [HeliPort](https://github.com/OpenIntelWireless/HeliPort) 1.5.0
  (cc0068b). Tahoe-style menu: Wi-Fi switch header, circular signal badge (blue when connected), no duplicate
  of the connected network, "Known/Other Networks" sections. Keychain reads moved off the menu thread.
  A rebuilt HeliPort is a different app for the Keychain: re-add your saved networks after installing it.
