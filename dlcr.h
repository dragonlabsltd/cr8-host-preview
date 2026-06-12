#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define DLCR_CHANNEL_COUNT  8
#define DLCR_SERIAL_LEN     16
#define DLCR_SAMPLE_RATE    12.5e6

// Detect C++
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Dragon Labs Coherent Receiver Device. Calls on these objects are NOT thread-safe unless specifically stated otherwise.
*/
struct dlcr;
typedef struct dlcr dlcr_t;

/**
 * Device Information.
*/
struct dlcr_info {
    /**
     * Serial number of the devices as a null-terminated string.
    */
    char serial[DLCR_SERIAL_LEN+1];
};
typedef struct dlcr_info dlcr_info_t;

struct dlcr_dev_info {
    /**
     * Hardware version.
    */
    uint8_t hw_ver_major;
    uint8_t hw_ver_minor;

    /**
     * Firmware version.
    */
    uint8_t fw_ver_major;
    uint8_t fw_ver_minor;
    uint8_t fw_ver_build;
};
typedef struct dlcr_dev_info dlcr_dev_info_t;

/**
 * Channel enum. By design, a channel is always represented by `1 << channel_id`, with `channel_id` being zero-based.
*/
enum dlcr_channel {
    DLCR_CHAN_NONE = 0x00,
    DLCR_CHAN_1    = (1 << 0),
    DLCR_CHAN_2    = (1 << 1),
    DLCR_CHAN_3    = (1 << 2),
    DLCR_CHAN_4    = (1 << 3),
    DLCR_CHAN_5    = (1 << 4),
    DLCR_CHAN_6    = (1 << 5),
    DLCR_CHAN_7    = (1 << 6),
    DLCR_CHAN_8    = (1 << 7),
    DLCR_CHAN_ALL  = 0xFF
};
typedef enum dlcr_channel dlcr_channel_t;

/**
 * Clock reference.
*/
enum dlcr_clock {
    /**
     * On-board TXCO.
    */
    DLCR_CLOCK_INTERNAL = 0x00,

    /**
     * External 10MHz reference.
    */
    DLCR_CLOCK_EXTERNAL = 0x01
};
typedef enum dlcr_clock dlcr_clock_t;

#pragma pack(push, 1)
/**
 * Floating point complex sample.
*/
struct dlcr_complex {
    /**
     * Real component.
    */
    float re;

    /**
     * Imaginary component.
    */
    float im;
};
typedef struct dlcr_complex dlcr_complex_t;
#pragma pack(pop)

/**
 * Sample buffer callback.
*/
typedef void (*dlcr_callback_t)(dlcr_complex_t* samples[DLCR_CHANNEL_COUNT], size_t count, size_t drops, void* ctx);

/**
 * Get a list of Dragon Labs CR-8 devices on the system.
 * @param devices Pointer to an array of device info.
 * @return Number of devices found or error code.
*/
int dlcr_list_devices(dlcr_info_t** devices);

/**
 * Free a device list returned by `dlcr_list_devices()`.
 * @param devices Device list to free.
*/
void dlcr_free_device_list(dlcr_info_t* devices);

/**
 * Open a Dragon Labs device.
 * @param dev Newly open device.
 * @param serial Serial number of the device to open as returned in the device list. If an empty string is passed, the first device found will be open.
 * @return 0 on success, error code otherwise.
*/
int dlcr_open(dlcr_t** dev, const char* serial);

/**
 * Close a Dragon Labs device.
 * @param dev Device to be closed.
*/
void dlcr_close(dlcr_t* dev);

/**
 * Get the device information like hardware and firmware versions.
*/
void dlcr_get_dev_info(dlcr_t* dev, dlcr_dev_info_t* info);

/**
 * Start the receiver.
 * @param dev Device to start.
 * @param buffer_size Requested number of samples per returned buffer. The number of returned samples will always be lower or equal to this value.
 * @param callback Callback to handle sample buffers.
 * @param ctx User context passed as-is to the callback.
 * @return 0 on success, error code otherwise.
*/
int dlcr_start(dlcr_t* dev, size_t buffer_size, dlcr_callback_t callback, void* ctx);

/**
 * Stop the receiver. If started using the synchronous API, any call to `dlcr_sync_rx()` MUST have returned before calling this function.
 * @param dev Device to start.
 * @return 0 on success, error code otherwise.
*/
int dlcr_stop(dlcr_t* dev);

/**
 * Start the receiver using the synchronous API. The `dlcr_sync_stop()` function must then be used to stop the device.
 * Note: The synchronous API is lower performance than the asynchronous API.
 * @param dev Device to start.
 * @param buffer_size Size of the circular buffer in samples per channel.
 * @return 0 on success, error code otherwise.
*/
int dlcr_sync_start(dlcr_t* dev, size_t buffer_size);

/**
 * Receive samples using the synchronous API. Device must have been started using `dlcr_sync_start()`.
 * Note: The synchronous API is lower performance than the asynchronous API.
 * This function is thread-safe.
 * @param dev Device to receive samples from.
 * @param samples Buffer to write the received samples to.
 * @param count Number of samples to receive per channel.
 * @return Number of samples received, zero if the transfer was aborted or negative on error.
*/
int dlcr_sync_rx(dlcr_t* dev, dlcr_complex_t* samples[DLCR_CHANNEL_COUNT], size_t count);

/**
 * Abort the synchronous transfer, which causes any call to `dlcr_sync_rx()` to return 0 immediately.
 * Note: The synchronous API is lower performance than the asynchronous API.
 * @param dev Device for which to abort the synchronous transfer.
 * @return 0 on success, error code otherwise.
*/
int dlcr_sync_abort(dlcr_t* dev);

/**
 * Enable a set of channels.
 * @param dev Open device.
 * @param channels Channels to enable.
*/
int dlcr_enable_channel(dlcr_t* dev, dlcr_channel_t channels);

/**
 * Disable a set of channels.
 * @param dev Open device.
 * @param channels Channels to disable.
*/
int dlcr_disable_channel(dlcr_t* dev, dlcr_channel_t channels);

/**
 * Set the frequency of one or multiple enabled channels and perform calibration.
 * @param dev Open device.
 * @param channels Channels for which to set the frequency.
 * @param freq Frequency to tune the channels to.
 * @param coherent Whether or not to run the channels coherently. If false, the channels may not be phase-aligned.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_freq(dlcr_t* dev, dlcr_channel_t channels, double freq, bool coherent);

/**
 * Set the gain of the LNA stage of one or multiple enabled channels.
 * @param dev Open device.
 * @param channels Channels for which to set the LNA gain.
 * @param gain LNA gain from 0 to 14.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_lna_gain(dlcr_t* dev, dlcr_channel_t channels, int gain);

/**
 * Set the gain of the mixer stage of one or multiple enabled channels.
 * @param dev Open device.
 * @param channels Channels for which to set the mixer gain.
 * @param gain Mixer gain from 0 to 15.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_mixer_gain(dlcr_t* dev, dlcr_channel_t channels, int gain);

/**
 * Set the gain of the VGA stage of one or multiple enabled channels.
 * @param dev Open device.
 * @param channels Channels for which to set the VGA gain.
 * @param gain VGA gain from 0 to 15.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_vga_gain(dlcr_t* dev, dlcr_channel_t channels, int gain);

/**
 * Set the overall gain of one or multiple enabled channels.
 * @param dev Open device.
 * @param channels Channels for which to set the mixer gain.
 * @param gain Overall gain from 0 to 44.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_gain(dlcr_t* dev, dlcr_channel_t channels, int gain);

/**
 * Select the clock source for the device.
 * @param dev Open device.
 * @param clock Clock source to use as the reference clock.
 * @return 0 on success, negative otherwise.
*/
int dlcr_set_clock_source(dlcr_t* dev, dlcr_clock_t clock);

// Detect C++
#ifdef __cplusplus
}
#endif
