# Dragon Labs CR-8 Host Driver and Utilities

This repository contains the source code for the host driver and utilities for the Dragon Labs CR-8 coherent SDR receiver.

## Table of Contents

1. [Installing the driver and host utilities](#example)
    1. [Linux](#example)
    1. [MacOS](#example)
    1. [Windows](#example)
2. [Building from source](#example)
    1. [Linux / MacOS](#example)
    2. [Windows](#example)
3. [Included command-line utilities](#example)
    1. [Enumerating devices](#example)
    2. [Upgrading the firmware](#example)
4. [Using the host library](#example)
    1. [Linking to the library](#example)
    2. [Listing available devices](#example)
    3. [Opening and closing a device](#example)
    4. [Getting information on a device](#example)
    5. [Starting and stopping a device](#example)
    6. [Alternative synchronous API](#example)
    7. [Enabling and Disabling Channels](#example)
    8. [Tuning](#example)
    9. [Setting the Gains](#example)
    10. [Selecting a Clock Source](#example)

## Installing the driver and host utilities

The proceedure to install the host utilities depends on the platform they are to be installed on.

### Linux

There is no prebuilt package yet, you must build from source, see below for details.

### MacOS

There is no prebuilt package yet, you must build from source, see below for details.

### Windows

There is no prebuilt package yet, you must build from source, see below for details.

## Building from source

The proceedure to build the host utilities depends on the platform they are to be built for.

### Linux / MacOS

First, install `libvolk` and `libusb` using your distro's package managers. The exact names of the package depends on the distro you are running. For example, on Ubuntu 24.04, this would be:

```sh
sudo apt install libvolk-dev libusb-1.0-0-dev
```

Next, download the source code for the host tools here: https://github.com/dragonlabsltd/cr8_host. You can then extract the zip and `cd` into the folder where you extracted the code, create a build directory, and `cd` into that:

```sh
cd /path/to/dlcr_host
mkdir build
cd build
```

Then, generate build files using `cmake`.

```sh
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
```

Finally, build and install the driver and utilities using `make`:

```sh
make
sudo make install
```

### Windows

First, install PothosSDR in order to have `libvolk`, and install `libusb` using `vcpkg`:

```sh
vcpkg install libusb:x64-windows
```

Next, cd into the folder where you extracted or cloned the code from this repository, create a build directory, and `cd` into that:

```sh
cd /path/to/dlcr_host
mkdir build
cd build
```

Then, generate build files using `cmake` making sure to point it to `vcpkg` and `pkgconf`:

```sh
cmake .. "-DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DPKG_CONFIG_EXECUTABLE=C:/path/to/vcpkg/installed/x64-windows/tools/pkgconf/pkgconf.exe"
```

Finally, build and install the driver and utilities using `cmake`:

```sh
cmake --build . --config Release
cmake --install .
```

## Included command-line utilities

A number of command line utilities are included to enumerate, manage and use the Dragon Labs CR-8 devices.
All the utilities include some built-in documentation in the form of the `-h` or `--help` command line arguments.

### Enumerating devices

Use the `dlcr_info` utility to list available devices and show their information.

```sh
user@host$ dlcr_info

Found 1 device:
 [000000000012] Hardware Version = 1.2, Firmware Version = 2.4.0

user@host$
```

### Upgrading the firmware

The firmware of your device can be upgraded using the `dlcr_flash` utility. Point it to the firmware file using the `-w` parameter, and optionally specify the serial number of the device to flash if you have multiple plugged in.

> [!WARNING]  
> Make sure that both your device and PC are powered from a stable source.
> If the device were to lose power during flashing, it will likely no longer function until a clean firmware is flashed over JTAG.

```sh
user@host$ dlcr_flash -w /path/to/cr8_firmware_v2.4.0_hw1.2.bin -d optionally_your_serial_number

===== Device Information =====
Serial Number:    000000000012
Firmware Version: 2.3.0
Hardware Version: 1.2

WARNING:
========
You are about to flash a new firmware to the device shown above.
Make sure your computer and the device are powered from a stable source.
Once this process starts, it should not be interrupted.
If any issue occurs during flashing, try flashing again without power cycling the device.
If the issue is still not resolved and the firmware is corrupted, do not panic.
The device contains a special rescue firmware which should allow additional attempts.

Would you like to continue? <y/N>y

Erasing:   [##################################################] 100% 
Writing:   [##################################################] 100% 
Verifying: [##################################################] 100% 

Flash successful!
Please power-cycle your device.

user@host$
```

As stated by the flashing software, make sure to fully power cycle the device after a successful firmware upgrade.

## Using the host library

This section details the use of `libdlcr`, the library that allows to interact programmatically with Dragon Labs CR-8 devices.

### Linking to the library

The library can be linked to either manually using `-ldlcr`, or through pkg-config integration in your desired build tools. For example, using CMake, you coud link to the library using the following code.

```cmake
# Find libdlcr
pkg_check_modules(LIBDLCR REQUIRED libdlcr)

# Add its link directories, include directories and libraries to the app
target_include_directories(my_app PRIVATE ${LIBDLCR_INCLUDE_DIRS})
target_link_directories(my_app PRIVATE ${LIBDLCR_LIBRARY_DIRS})
target_link_libraries(my_app PRIVATE ${LIBDLCR_LIBRARIES})
```

### Listing available devices

Available devices can be listed using the `dlcr_list_devices()` function. This function returns the number of devices found or a negative value on error.

```c
// Get a list of available devices
dlcr_info_t* dev_list;
int count = dlcr_list_devices(&dev_list);

// Check if there was an error while listing the devices
if (count < 0) { /* ... */ }

// Show all available devices
for (int i = 0; i < count; i++) {
    // Display the serial number
    printf("%s\n", dev_list[i].serial);
}
```

### Opening and closing a device

To open a device, use the `dlcr_open()` function and optionally provide a serial number. Using an empty string as the serial number will open the first available device.

```c
// Attempt to open the device using its serial number
dlcr_dev_t* dev;
int err = dlcr_open(&dev, "my-serial");

// Check if it was opened successfully
if (err) { /* ... */ }
```

When you are done using the device, you can simply close it using the `dlcr_close()` function. Before closing, this function will automatically stop the device and put it into standby.

```c
// Close the device
dlcr_close(dev);
```

### Getting information on a device

To get information about a device, use the `dlcr_get_dev_info()` function. Since this information is cached when opening the device, this call always succeeds on an open device, no need to worry about error checking.

```c
// Get the device information
dlcr_dev_info_t info;
dlcr_get_dev_info(dev, &info);

// Display the device information
printf("Hardware Version: %d.%d\n", info.hw_ver_major, info.hw_ver_minor);
printf("Firmware Version: %d.%d.%d\n", info.fw_ver_major, info.fw_ver_minor, info.fw_ver_build);
```

### Starting and stopping a device

Once a device is open, you can command it to start sending samples using the `dlcr_start()` function. This requires defining your own callback function which will get called when new samples are received. The arguments of the callback function are as follows: `samples` is set of buffers containing the samples for each channel, `count` indicates the number of samples per channel, `drops` indicates the number of dropped samples per channel and `ctx`

> [!NOTE]  
> The callback function must block **as little as possible**. Any blocking will cause the entire software-side data path to lock up which may cause sample drops, or at worst prevent commands from being sent to the device since the acknowledgements would be stuck waiting for the data path to unlock.

In addition to the callback, the `dlcr_start()` function takes a few more arguments. The buffer size is the maximum buffer size that your callback can accept. The device may call the callback with less samples, but never zero, and never more than the value you specified. Finally, an arbitrary user context may be provided to be passed to your callback.

```c
// Define the callback
void my_callback(dlcr_complex_t* samples[DLCR_CHANNEL_COUNT], size_t count, size_t drops, void* ctx) {
    // Process the samples
}

...

// Start the device
int err = dlcr_start(dev, BUFFER_SIZE, my_callback, &my_context_or_null);

// Check if the device started successfully
if (err) { /* ... */ }
```

When you no longer need to be receiving samples, you may call the `dlcr_stop()` function. This also automatically puts all channels into standby.

```c
// Stop receiving samples
int err = dlcr_stop(dev);

// Check if the device stopped successfully
if (err) { /* ... */ }
```

### Alternative synchronous API

The asynchronous API is powerful but may be overkill in situations where you do not care about dropping some samples such as radio direction finding. To remedy this, a synchronous API is included which automatically buffers up samples for you in a circular buffer and lets you receive the latest samples at your convenience. To use this API, start the device using the `dlcr_sync_start()` function instead of the regular start function.

> [!NOTE]  
> The synchronous API is inherently lower performance than the asynchronous API because it has to buffer the samples instead of sending them directly to you. Always prefer the asynchronous API if you care about throughput and minimizing sample drops.

The `dlcr_sync_start()` function takes a buffer size parameter which allows to set the size of the internal circular buffer in samples per channel. The size of this buffer needs to be large enough to absorb delays between reads, otherwise sample drops may occur. A value of 1 million is a good starting point.

```c
// Start receiving samples using the synchronous API
int err = dlcr_sync_start(dev, BUFFER_SIZE);

// Check if the device started successfully
if (err) { /* ... */ }
```

After this, samples can be received using the `dlcr_sync_rx()` function. This function takes a set of buffers in which the received samples will be stored as well as the number of samples per channel to read.

```c
// Buffers to be used to read the samples (remember to allocate them...)
dlcr_complex_t samples[DLCR_CHANNEL_COUNT];

...

// Receive samples using the synchronous API
int err = dlcr_sync_rx(dev, samples, READ_SIZE);

// Check that the samples were read successfully
if (err) { /* ... */ }
```

Before stopping the device, any call to `dlcr_sync_rx()` MUST have returned. Since that function is blocking, a `dlcr_sync_abort()` function is provided to force the `dlcr_sync_rx()` to return. You can then, for example, wait for the worker thread to exit, or employ any other synchronization mechanism to ensure that the `dlcr_sync_rx()` will no longer be called, after which you can call `dlcr_stop()`.

```c
// Force dlcr_sync_rx() to return immediately
dlcr_sync_abort(dev);

// Wait for the worker thread to exit
join_thread(worker);

// Stop the device
int err = dlcr_stop(dev);

// Check if the device stopped successfully
if (err) { /* ... */ }
```

### Enabling and Disabling Channels

Before any signals can be received on channels, they must be enabled. This is done using the `dlcr_enable_channel()` function as shown below.

> [!NOTE]  
> All channels are always disabled by default when opening a device and must be enabled manually.

```c
// Enable channels 1 and 2
int err = dlcr_enable_channel(dev, DLCR_CHAN_1 | DLCR_CHAN_2);

// Check if the channels were enabled successfully
if (err) { /* ... */ }
```

If all the channels should be enabled, use `DLCR_CHAN_ALL` which is a short-hand for every channel OR'd together. Once channels are no longer needed, they can be disabled using `dlcr_disable_channel()` as shown below. Note that `dlcr_stop()` automatically disables all channels, so there is no need to manually disable the channels if you are completely stopping the receiver.

```c
// Disable channels 1 and 2
int err = dlcr_disable_channel(dev, DLCR_CHAN_1 | DLCR_CHAN_2);

// Check if the channels were disabled successfully
if (err) { /* ... */ }
```

# Tuning

The channels can be tuned using the `dlcr_set_freq()` function as shown below. This function takes as argument the channels to tune, the, the frequency to tune them to, and whether the channels should operate coherently. If phase-coherence is not immediately necessary, tuning with it disabled is faster than with it enabled. For coherent operation, this option must be set to true.

> [!NOTE]  
> A channel must be enabled before it can be tuned.

```c
// Tune channels 1 and 2 to 100MHz coherently
int err = dlcr_set_freq(dev, DLCR_CHAN_1 | DLCR_CHAN_2, 100e6, true);

// Check if tuning was successful
if (err) { /* ... */ }
```

# Setting the Gains

TODO

# Selecting a Clock Source

The device can operate either from its internal TCXO or an external 10MHz reference. It can be switched between the two using the `dlcr_set_clock_source()` function as shown below.

> [!NOTE]  
> The internal reference is always selected by default when opening a device.

```c
// Select the external clock reference
int err = dlcr_set_clock_source(dev, DLCR_CLOCK_EXTERNAL);

// Check if the clock source was selected successfully
if (err) { /* ... */ }
```

When using the external reference, the LED next to the reference input should light up green. If the LED lights up red, it means that the receiver is not getting a proper 10MHz 3.3v CMOS reference on its input. When using the internal reference, the LED is always off.