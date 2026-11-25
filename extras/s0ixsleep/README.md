# S0ixSleep settings for this laptop

`s0ixsleep.conf` goes to `/usr/local/etc/` after installing the S0ixSleep daemon
(<https://github.com/vikenzo196/S0ixSleep>):

- `EC_IGNORE`: the embedded controller events that must not wake the laptop (battery, AC, thermal, fans, UCSI, SCAI);
  power button `_Q50` and lid `_Q5E`/`_Q5F` still wake.
- `LID_EC=83.0`: S0ixSleep also reads the lid state (`LSTE`) from the EC during the sleep, because after some sleeps
  the EC does not send `_Q5F` when the lid opens (S0ixSleep 0.1.2 or newer).
- `GT_RESET=0`: the WhateverXe fork resets the GT itself from S0ixSleep's wait hook (boot-arg `-wex-s0ixsleep`).
- Hourly dark wake, battery guard (clean shutdown at 10 % after a wake, 4 % while awake) and bag guard: defaults.
