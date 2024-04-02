# LVGL Test App


## Installing ESP-IDF 

### Windows

Follow instructions until `Launching ESP-IDF Environment` topic at https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html to install ESP-IDF v5.2.1.

After that, open a Powershell terminal, `cd` into where you've installed ESP-IDF (e.g. `cd C:\esp\esp-idf`) and run:

```powershell
./export.ps1
```

This will set the environment variables for ESP-IDF that will allow you to build a project.




### Linux / MacOS

```bash

mkdir -p ~/esp
cd ~/esp
git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git


cd ~/esp/esp-idf
./install.sh all


source ~/esp/esp-idf/export.sh
```

## Building

After cloning this repository, you need to init the submodules (LVGL and LVGL ESP32 drivers):

```bash
git submodule update --init --recursive
```


With the environment variables set, you can now build the project. 

```bash
cd <PROJECT_DIR>

idf.py menuconfig
```

This will open a configuration menu. Everything is already configured on `sdkconfig`, so you don't need to change anything, just press `S` to save and `ESC` to exit.

If you want to check out some options, go to `component config` -> `LVGL` and `LVGL ESP Drivers` to see some of the options available.


Run `idf.py build` to build the project.


To flash, run `idf.py -p <PORT> flash` where `<PORT>` is the serial port of your ESP32 device.