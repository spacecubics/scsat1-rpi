# :package: install package

Install Zephyr SDK

```shell
$ wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/zephyr-sdk-0.16.1_linux-x86_64.tar.xz
$ wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1/sha256.sum | shasum --check --ignore-missing

$ tar xvf zephyr-sdk-0.16.1_linux-x86_64.tar.xz

$ cd zephyr-sdk-0.16.1
$ ./setup.sh
```

# :hammer: build

```shell
$ mkdir workspace
$ cd workspace
$ pipenv shell
$ pipenv install west pip
$ west init -m https://github.com/spacecubics/scsat1-rpi
$ west update
$ pip install -r zephyr/scripts/requirements.txt
$ west build -b rpi_pico scsat1-rpi/pico
```

# :rocket: writing to Flash Memory

> **Note**  
> Connect arduino nano and PC while holding down the boolsel switch on nano

`***` : Arduino Nano device-name

```shell
$ west flash
# or
$ cp build/zephyr/zephyr.uf2 ***
```
