
// SPDX-License-Identiefier: MIT
// SPDX-CopyRightText: 2025 Badge.Team

#include "asp/audio.h"
#include <stdlib.h>
#include "asp/err.h"
#include "bsp/audio.h"
#include "driver/i2s_common.h"
#include "driver/i2s_types.h"
#include "err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Forward declarations of the launcher's audio mixer (main/audio_mixer.h).
// The launcher owns the I2S channel; plugin writes go through the mixer so
// concurrent plugins don't trample each other's DMA buffers.
extern size_t audio_mixer_write(TaskHandle_t task, const void* samples, size_t size_bytes, int64_t timeout_ms);
extern bool   audio_mixer_start(TaskHandle_t task);
extern bool   audio_mixer_stop(TaskHandle_t task);

// Set audio sampling rate.
asp_err_t asp_audio_set_rate(uint32_t rate_hz) {
    return asp_esp_err_conv(bsp_audio_set_rate(rate_hz));
}

// Get current volume.
asp_err_t asp_audio_get_volume(float* out_percentage) {
    return asp_esp_err_conv(bsp_audio_get_volume(out_percentage));
}

// Set audio volume.
asp_err_t asp_audio_set_volume(float percentage) {
    return asp_esp_err_conv(bsp_audio_set_volume(percentage));
}

// Enable/disable the audio amplifier.
asp_err_t asp_audio_set_amplifier(bool enabled) {
    return asp_esp_err_conv(bsp_audio_set_amplifier(enabled));
}

// Stop this plugin's audio stream. Pauses mixing for the stream and discards
// any samples still queued. Other plugins are unaffected.
asp_err_t asp_audio_stop() {
    return audio_mixer_stop(xTaskGetCurrentTaskHandle()) ? ASP_OK : ASP_ERR_FAIL;
}

// Start (or resume) this plugin's audio stream.
asp_err_t asp_audio_start() {
    return audio_mixer_start(xTaskGetCurrentTaskHandle()) ? ASP_OK : ASP_ERR_FAIL;
}

// Write audio samples.
asp_err_t asp_audio_write(void* samples, size_t samples_size, int64_t timeout_ms) {
    if (samples == NULL || samples_size == 0) return ASP_ERR_PARAM;
    size_t sent = audio_mixer_write(xTaskGetCurrentTaskHandle(), samples, samples_size, timeout_ms);
    return (sent == samples_size) ? ASP_OK : ASP_ERR_FAIL;
}
