#include <Arduino.h>
#include <driver/i2s.h>

// Your working pinout:
static const int PIN_I2S_BCLK = 11;  // MAX98357 BCLK
static const int PIN_I2S_LRCK = 10;  // MAX98357 LRC/WS
static const int PIN_I2S_DOUT = 9;  // MAX98357 DIN

static const i2s_port_t I2S_PORT = I2S_NUM_0;

// Must match ffmpeg conversion settings:
static const int SAMPLE_RATE = 16000;

#include "audio.h"
// xxd will likely generate: unsigned char voice_raw[]; unsigned int voice_raw_len;
// If your filename differs, adjust the names below accordingly.
extern unsigned char voice_raw[];
extern unsigned int voice_raw_len;

static void i2sStart() {
  i2s_config_t cfg = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // we will duplicate mono to L+R
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pins = {
    .bck_io_num = PIN_I2S_BCLK,
    .ws_io_num = PIN_I2S_LRCK,
    .data_out_num = PIN_I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  esp_err_t e;
  e = i2s_driver_install(I2S_PORT, &cfg, 0, nullptr);
  if (e != ESP_OK) {
    Serial.printf("i2s_driver_install failed: %d\n", (int)e);
    while (true) delay(1000);
  }

  e = i2s_set_pin(I2S_PORT, &pins);
  if (e != ESP_OK) {
    Serial.printf("i2s_set_pin failed: %d\n", (int)e);
    while (true) delay(1000);
  }

  i2s_zero_dma_buffer(I2S_PORT);
}

void setup() {
  Serial.begin(115200);
  delay(1500);


  i2sStart();
}

void loop() {
  const int16_t* mono = (const int16_t*)voice_raw;
  const size_t mono_samples = voice_raw_len / 2;

  // Buffer for 256 stereo frames (L+R)
  int16_t stereo[256 * 2];

  size_t i = 0;
  while (i < mono_samples) {
    size_t frames = 256;
    if (i + frames > mono_samples) frames = mono_samples - i;

    for (size_t f = 0; f < frames; f++) {
      int16_t s = mono[i + f];
      stereo[2 * f + 0] = s; // Left
      stereo[2 * f + 1] = s; // Right
    }

    size_t written = 0;
    i2s_write(I2S_PORT, stereo, frames * 2 * sizeof(int16_t), &written, portMAX_DELAY);
    i += frames;
  }

  delay(1000); // pause then repeat
}