# :package: install package

Install Zephyr SDK

```shell
$ wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-x86_64.tar.xz
$ wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/sha256.sum | shasum --check --ignore-missing

$ tar xvf zephyr-sdk-0.16.1_linux-x86_64.tar.xz

$ cd zephyr-sdk-0.16.1
$ ./setup.sh
```
# :hammer: prepare

```shell
$ mkdir workspace
$ pipenv shell
$ pipenv install west pip
$ west init -m https://github.com/spacecubics/scsat1-rpi
$ west update
$ pip install -r zephyr/scripts/requirements.txt
```

# :hammer: build bootloader

Pico is equipped with MCUBoot to enable OTA updates from the Zero.
First, build a bootloader based on MCUBoot and flash `build/zephyr/zephyr.uf2` file to the target device by following the instructions in `Writing to Flash Memory`.
If you do not require OTA updates, this step is unnecessary.

```shell
$ cd scsat1-rpi/pico/bootloader/
$ ./build-boot.sh
```

# :hammer: build application for OTA

To generate an image file for OTA updates, follow the steps below to build it,
and then use the MCU client to flash the `build/zephyr/zephyr.signed.bin` file.

```shell
$ cd scsat1-rpi/pico
$ rm -rf build && west build -b rpi_pico -- -DMCUBOOT=enable
```

# :hammer: build application without MCUBoot

```
$ cd scsat1-rpi/pico
$ rm -rf build && west build -b rpi_pico
```

## Build Options

using `UART1` for console
```
$ cd scsat1-rpi/pico
$ rm -rf build && west build -b rpi_pico -- -DCONSOLE=uart1
```

# :rocket: writing to Flash Memory

> **Note**  
> Connect arduino nano and PC while holding down the boolsel switch on nano

`***` : RaspberryPi Pico device-name

```shell
$ west flash
# or
$ cp build/zephyr/zephyr.uf2 ***
```
