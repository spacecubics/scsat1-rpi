# :bulb: requirement

- At least **90 Gbyte** of disk space
- At least **8 Gbytes** of RAM

# :package: install package

Install packages for your environment

> [!CAUTION]
> Requires `gcc-13` or lower
> 
> ref. #55

## Ubuntu

```shell
$ sudo apt install gawk wget git diffstat unzip texinfo gcc build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev python3-subunit mesa-common-dev zstd liblz4-tool file locales bmap-tools
$ sudo locale-gen en_US.UTF-8
```

## Other Distribution

https://docs.yoctoproject.org/ref-manual/system-requirements.html#required-packages-for-the-build-host

# :hammer: build

run in bash or zsh
```shell
$ git clone https://github.com/spacecubics/scsat1-rpi && cd scsat1-rpi
$ git submodule update --init
$ cd zero
$ TEMPLATECONF=$(readlink -f ./meta-scsat1-rpi/conf) source ./poky/oe-init-build-env
$ bitbake core-image-minimal
```

# :rocket: writing to SD card

`***` : SD card device-name

```shell
$ bmaptool copy tmp-glibc/deploy/images/raspberrypi0-2w-64/core-image-minimal-raspberrypi0-2w-64.wic.bz2 ***
```
