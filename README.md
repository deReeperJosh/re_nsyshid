# re_nsyshid

re_nsyshid is a Wii U Plugin which allows end users to emulate USB peripheral devices

Requires the [Aroma](https://github.com/wiiu-env/Aroma) environment.  

> :warning: This plugin is still a work in progress, and some issues are expected!

The re_nsyshid plugin allows you to toggle the "Emulation Status" on and off - meaning that you can choose to either use your physical USB devices when the Emulation Status is disabled, or when enabled it will send commands to the device you have selected in the plugin config menu.

> :warning: It is highly recommended to set your chosen Emulation Status and chosen Emulated Device prior to opening your game, as unexpected errors may occur when trying to toggle this in game.

## Usage

- Download the [latest release](https://github.com/deReeperJosh/re_nsyshidV2/releases) or the artifacts of latest nightly build from [here](https://nightly.link/deReeperJosh/re_nsyshidV2/workflows/build/main/re_nsyshid.zip).
- Copy the contents of the downloaded *`.zip`* file to your target environment.
- Copy your NFC Figure dumps to `wiiu/re_nsyshid`. Subfolders are also supported and can be browsed from the configuration menu.
- Open the plugin configuration menu with L + Down + SELECT.
- Enable Emulation, and choose an Emulated Device.
- Choose a Figure for your chosen device, for example enter Skylander Manager if you chose the Skylander Portal, and choose a Skylander to load in a specific slot.
- Figures can be quickly removed from slots by hovering over the currently loaded slot and pressing the X button

## Planned features

In future releases, more USB devices are planned to be supported - such as the Disney Infinity Base, Lego Dimensions Toypad and Kamen Rider Ride Gate

## Building

Building re_nsyshid using the Dockerfile is recommended:

```bash
# Build docker image (only needed once)
docker build . -t re_nsyshid_builder

# make 
docker run -it --rm -v ${PWD}:/project re_nsyshid_builder make

# make clean
docker run -it --rm -v ${PWD}:/project re_nsyshid_builder make clean
```
