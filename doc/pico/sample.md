# :bulb: Sample Code

## Hello Console

Sample program to output the string "Hello" from the console.

```
$ west build -b rpi_pico --build-dir build-console-hello scsat1-rpi/pico/sample/console-hello
```

## Echo Server

Sample program to return the string received over UART.

```
$ west build -b rpi_pico --build-dir build-console-echo scsat1-rpi/pico/sample/console-echo
```

## Common Build Options

using `UART1` for console
```
$ west build -b rpi_pico ... -- -DCONSOLE=uart1
```

using `USB` for console
```
$ west build -b rpi_pico ... -- -DCONSOLE=usb
```

output logging
```
$ west build -b rpi_pico ... -- -DALLOW_DEBUG=y
```

> [!TIP]
> Using console changes and logging together
> ```
> $ west build -b rpi_pico ... -- -DCONSOLE=uart1 -DALLOW_DEBUG=y
> ```
