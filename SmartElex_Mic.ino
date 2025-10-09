#include <driver/i2s.h>

// I2S configuration
#define I2S_NUM         I2S_NUM_0  // I2S port number
#define I2S_WS_PIN      25         // Word Select pin (LRCLK)
#define I2S_SCK_PIN     26         // Serial Clock pin (BCLK)
#define I2S_SD_PIN      22         // Serial Data pin (DOUT from mic)

// Note Connect SEL pin to GND

void setup() 
{
  Serial.begin(115200); // Serial output for debugging/audio samples
  delay(500);
  Serial.println("I2S Microphone Test");

  // Configure I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Master mode, RX only
    .sample_rate = 16000,                               // 16 kHz sample rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,       // 16-bit audio
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,        // Read left channel only
    .communication_format = I2S_COMM_FORMAT_I2S,        // Standard I2S format
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,           // Interrupt level
    .dma_buf_count = 8,                                 // Number of DMA buffers
    .dma_buf_len = 64,                                  // Buffer length
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  // Configure I2S pins
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,      // Bit Clock
    .ws_io_num = I2S_WS_PIN,        // Word Select
    .data_out_num = I2S_PIN_NO_CHANGE, // We don't transmit
    .data_in_num = I2S_SD_PIN       // Serial Data input
  };

  // Install and start I2S driver
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM, &pin_config);

  // Optional: Clear any old data
  i2s_zero_dma_buffer(I2S_NUM);
}

void loop() {
  int16_t audio_buffer[256];  // buffer to hold audio samples (16-bit each)
  size_t bytes_read = 0;

  // Read from I2S
  i2s_read(I2S_NUM, (void*)audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);

  int samples_read = bytes_read / sizeof(int16_t);

  // Output samples to Serial
  for (int i = 0; i < samples_read; i++) {
    Serial.println(audio_buffer[i]);  // Print each sample as a signed 16-bit integer
  }
}
