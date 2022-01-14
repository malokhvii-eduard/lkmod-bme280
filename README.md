<!-- markdownlint-disable MD033 -->
<!-- markdownlint-disable MD041 -->

<div align="center">
  <h2 align="center">💧 Kernel space driver for Bosch Sensortec BME280</h2>
  <p align="center">
    The driver allows using the sensor via
    <a href="https://www.i2c-bus.org" aria-label="I2C Bus">I2C bus</a> on a
    single-board computer like
    <a href="https://www.raspberrypi.com" aria-label="Raspberry Pi">Raspberry Pi</a>,
    <a href="http://www.orangepi.org" aria-label="Orange Pi">Orange Pi</a>,
    <a href="https://tinker-board.asus.com/product/tinker-board.html"
      aria-label="Asus Tinker Board">Asus Tinker Board</a>, etc.
  </p>

  <p id="shields" align="center" markdown="1">

[![License](https://img.shields.io/badge/license-MIT-3178C6?style=flat)](LICENSE)
[![pre-commit](https://img.shields.io/badge/pre--commit-enabled-brightgreen?logo=pre-commit&logoColor=white)][github-pre-commit]
![Style Guide](https://img.shields.io/badge/code%20style-linux-FFC557?style=flat)
![clang-format](https://img.shields.io/badge/formatter-clang--format-262D3A?style=flat)
[![markdownlint](https://img.shields.io/badge/linter-markdownlint-000?style=flat)][github-markdownlint]
[![commitlint](https://img.shields.io/badge/linter-commitlint-F7B93E?style=flat)][github-commitlint]
![platform](https://img.shields.io/badge/platform-linux-FFC557?style=flat)
[![Tested on Raspberry Pi](https://img.shields.io/badge/tested%20on-raspberry%20pi-A22846)][raspberrypi]
[![Tested on Tinker Board](https://img.shields.io/badge/tested%20on-tinker%20board-005571)][tinker-board]

  </p>
</div>

## 🎉 Features

- 🗃️ Provides an interface in [*sysfs*][man-sysfs] and
[*procfs*][man-proc] for sensor settings, calibration data, measurements
- ⚙️ Supports multiple sensors on the same I2C adapter
- 🔌 Supports multiple I2C adapters
- ⚗️ Tested on: [*Rasbperry Pi 3B+*][raspberrypi],
[*Asus Tinker Board*][tinker-board]

## 🌻 Motivation

Final project for my education in
[Linux Kernel GL BaseCamp][linux-kernel-basecamp]. I participated in
the course from November 2019 until March 2020.

## ✨ Getting Started

### 📚 Prerequisites

Firstly you will need to install build dependencies such as compiler
(`build-essential`, `g++-arm-linux-gnueabihf`, `gdb-multiarch`) and
linux-headers. Next, you will need to get source code for exact kernel version
you are running. You can find the kernel version via `uname -r`.

### 📦 Installation

1. Clone the *Repository*
2. Build this *Kernel Module* (`make clean modules_release`)
3. Install this *Kernel Module* (`make modules_install`)

### 👀 Usage

1. Connect the sensor to your host
2. Scan I2C bus to find the sensor address (`i2cdetect`)
3. Initialize the sensor from user space (`echo "bme280 'your address, usually
0x76 or 0x77'" > /sys/bus/i2c/devices/i2c-'your adapter number'/new_device`)

## ❓ FAQs

<!-- FAQ 1 -->
<!-- markdownlint-disable MD013 -->
### 🙋‍♂️ How to access sensor settings, calibration data, measurements?
<!-- markdownlint-enable MD013 -->

<details>
  <summary>
    👉
    <a href="https://man7.org/linux/man-pages/man5/sysfs.5.html"
      aria-label="filesystem for exporting kernel objects">sysfs</a>
    &ndash; filesystem for exporting kernel objects
  </summary>
  <br>

| Mapping                        | Operations | Description                    |
| ------------------------------ | ---------- | ------------------------------ |
| /sys/class/bme280/i2c          | read/write | I2C adapter and device address |
| /sys/class/bme280/chip_id      | read       | Chip identifier                |
| /sys/class/bme280/reset        | write      | Reset                          |
| /sys/class/bme280/mode         | read/write | Power mode                     |
| /sys/class/bme280/osrs_p       | read/write | Pressure oversampling          |
| /sys/class/bme280/osrs_t       | read/write | Temperature oversampling       |
| /sys/class/bme280/osrs_h       | read/write | Humidity oversampling          |
| /sys/class/bme280/filter       | read/write | Filter coefficient             |
| /sys/class/bme280/standby_time | read/write | Standby time                   |
| /sys/class/bme280/pressure     | read       | Pressure (Pa)                  |
| /sys/class/bme280/temperature  | read       | Temperature (°C * 100)         |
| /sys/class/bme280/humidity     | read       | Humidity (% * 1024)            |

</details>

<details>
  <summary>
    👉
    <a href="https://man7.org/linux/man-pages/man5/proc.5.html"
      aria-label="process information pseudo-filesystem">procfs</a>
    &ndash; process information pseudo-filesystem</i>
  </summary>
  <br>

| Mapping           | Operations | Description                   |
| ----------------- | ---------- | ----------------------------- |
| /proc/bme280info  | read       | Device information as a table |
| /proc/bme280calib | read       | Calibration data as a table   |

</details>

<!-- FAQ 2 -->
### 🙋‍♂️ How to switch to another sensor?

👉 If you want to switch to another sensor, use `/sys/bme280/i2c`
mapping, write to it a number of I2C adapter in decimal and device address in
hex (`echo "0 0x77" > /sys/bme280/i2c`).

## 🛠️ Tech Stack

<!-- markdownlint-disable MD013 -->
[![EditorConfig](https://img.shields.io/badge/EditorConfig-FEFEFE?logo=editorconfig&logoColor=000&style=flat)][editorconfig]
![Markdown](https://img.shields.io/badge/Markdown-000?logo=markdown&logoColor=fff&style=flat)
![C](https://img.shields.io/badge/C-A8B9CC?logo=c&logoColor=fff&style=flat)
![Makefile](https://img.shields.io/badge/Make-A42E2B?logo=gnu&logoColor=fff&style=flat)
[![pre-commit](https://img.shields.io/badge/pre--commit-FAB040?logo=precommit&logoColor=fff&style=flat)][github-pre-commit]
![clang-format](https://img.shields.io/badge/clang--format-262D3A?logo=llvm&logoColor=fff&style=flat)
[![commitlint](https://img.shields.io/badge/commitlint-F7B93E?logo=c&logoColor=000&style=flat)][github-commitlint]
[![markdownlint](https://img.shields.io/badge/markdownlint-000?logo=markdown&logoColor=fff&style=flat)][github-markdownlint]
![Linux](https://img.shields.io/badge/Linux-FFC557?logo=linux&logoColor=000&style=flat)
[![Linux Kernel](https://img.shields.io/badge/Linux%20Kernel-FFC557?logo=linux&logoColor=000&style=flat)](github-linux)
[![BME280](https://img.shields.io/badge/BME280-EA0016?logo=bosch&logoColor=fff&style=flat)][bosch-sensortec-bme280]
[![Raspberry Pi](https://img.shields.io/badge/Raspberry%20Pi-A22846?logo=raspberrypi&logoColor=fff&style=flat)][raspberrypi]
[![Tinker Board](https://img.shields.io/badge/Tinker%20Board-005571?logo=asus&logoColor=fff&style=flat)][tinker-board]
[![Shields.io](https://img.shields.io/badge/Shields.io-000?logo=shieldsdotio&logoColor=fff&style=flat)][shields]
[![Git](https://img.shields.io/badge/Git-F05032?logo=git&logoColor=fff&style=flat)][git-scm]
[![GitHub](https://img.shields.io/badge/GitHub-181717?logo=github&logoColor=fff&style=flat)][github]
<!-- markdownlint-enable MD013 -->

## ✍️ Contributing

👍🎉 *First off, thanks for taking the time to contribute!* 🎉👍

Contributions are what make the open source community such an amazing place to
be learn, inspire, and create. Any contributions you make are **greatly
appreciated**.

1. Fork the *Project*
2. Create your *Feature Branch* (`git checkout -b feature/awesome-feature`)
3. Commit your *Changes* (`git commit -m 'Add awesome feature'`)
4. Push to the *Branch* (`git push origin feature/awesome-feature`)
5. Open a *Pull Request*

## ⚠️ License

`lkmod-bme280` is licenced under the MIT License. See the [LICENSE](LICENSE) for
more information.

<!-- markdownlint-disable MD013 -->
<!-- Linux manual links -->
[man-proc]: https://man7.org/linux/man-pages/man5/proc.5.html
[man-sysfs]: https://man7.org/linux/man-pages/man5/sysfs.5.html

<!-- Github links -->
[github-commitlint]: https://github.com/conventional-changelog/commitlint
[github-linux]: https://github.com/torvalds/linux
[github-markdownlint]: https://github.com/DavidAnson/markdownlint
[github-pre-commit]: https://github.com/pre-commit/pre-commit
[github]: https://github.com

<!-- Other links -->
[bosch-sensortec-bme280]: https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280
[editorconfig]: https://editorconfig.org
[git-scm]: https://git-scm.com
[linux-kernel-basecamp]: https://www.globallogic.com/ua/gl-basecamp/linux-kernel/
[raspberrypi]: https://www.raspberrypi.com
[shields]: https://shields.io
[tinker-board]: https://tinker-board.asus.com/product/tinker-board.html
