# eqMac preset: Samsung Dolby (Dynamic)

Derived from Samsung's own Dolby Atmos (DAX3) tuning for this laptop, read from the Windows driver
package (`dax3_ext_rtk.inf`, file `DEV_0298_SUBSYS_144DC1CC_PCI_SUBSYS_C1CC144D.xml`, profile `dynamic`):
speaker high-pass 80 Hz (4th order), audio-optimizer speaker correction and IEQ "balanced" (amount 5).
Compressor, volume leveler, virtual bass and virtualizer are not reproduced.

Advanced (10-band) equalizer in [eqMac](https://github.com/bitgapp/eqMac):

| Band | 32 | 64 | 125 | 250 | 500 | 1k | 2k | 4k | 8k | 16k | Global |
|---|---|---|---|---|---|---|---|---|---|---|---|
| dB | -12 | -5 | +1 | +3 | +4 | 0 | +2.5 | 0 | -4.5 | -5 | -4 |

Speaker correction (audio-optimizer, dB) from the tuning file at the Dolby band centres (48 kHz):
328 Hz +3.5, 469 Hz +2.5, 844/1031 Hz -2.1, 1688 Hz -1.0, 2250 Hz +1.0, 3750 Hz -1.0, 4688 Hz -2.0,
5813 Hz -2.4, 7125/9000 Hz -3.0, 13875 Hz +3.0, 19688 Hz +6.6.
