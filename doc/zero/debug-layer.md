# :bulb: Debug Layer

It is a layer that consolidates functions unnecessary for actual operation,
intended for development and debugging.

## usage

edit the `build/conf/bblayers.conf` file
```
...
BBLAYERS ?= " \
  <path-to-workspace>/scsat1-rpi/zero/meta-debug \ # add
  <path-to-workspace>/scsat1-rpi/zero/meta \
  ...
  "
```

## Features

- root login (no passwd)
- `can-utils` package (cangen, candump etc.)
- Wi-Fi
- ssh server

> [!NOTE]
> When using WiFi, it is necessary to add the `SSID` and `PASS` to `build/conf/local.conf` as follows.
> ```
> ...
> DEBUG_WIFI_SSID = "SSID"
> DEBUG_WIFI_PASS = "PASS"
> ```
