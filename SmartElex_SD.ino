#include <Arduino.h>
#include <driver/i2s.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// -------- SD Card SPI Pins --------
#define SD_CS       5
#define SPI_MOSI   23
#define SPI_MISO   19
#define SPI_SCK    18

// -------- I2S Microphone Pins --------
#define I2S_NUM    I2S_NUM_0
#define I2S_WS_PIN 25   // LRCLK (Word Select)
#define I2S_SCK_PIN 26  // BCLK  (Serial Clock)
#define I2S_SD_PIN 22   // DOUT  (Data from mic)

// Please Note Connect SEL pin to GND

// -------- Audio Parameters --------
#define SAMPLE_RATE     16000
#define SAMPLE_BITS     16
#define CHANNELS        1
#define RECORD_TIME     8   // seconds
#define WAV_FILE_NAME   "/record.wav"
#define WAV_HEADER_SIZE 44

// -------- Function Prototypes --------
void generate_wav_header(uint8_t *header, uint32_t data_size, uint32_t sample_rate);
void record_wav();

void setup()
{
   // ---- Initialize SD card SPI ----
  delay(1000);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);   // deselect SD card before SPI begin
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);   // safe frequency when using both SPI & I2S

  Serial.begin(115200);
  
  Serial.println("\n=== SmartElex I2S Mic → SD Card WAV Recorder ===");

  if (!SD.begin(SD_CS)) 
  {
      Serial.println("SD init failed");
      while(1);
  }
  Serial.println("SD OK");

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
  Serial.println("I2S microphone initialized successfully.");

  // ---- Start Recording ----
  record_wav();
}

void loop() 
{
  int16_t audio_buffer[256];  // buffer to hold audio samples (16-bit each)
  size_t bytes_read = 0;

  // Read from I2S
  i2s_read(I2S_NUM, (void*)audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);

  int samples_read = bytes_read / sizeof(int16_t);

  // Output samples to Serial
  for (int i = 0; i < samples_read; i++) 
  {
    Serial.println(audio_buffer[i]);  // Print each sample as a signed 16-bit integer
  }
}

// -------- Record WAV Function --------
void record_wav() {
  uint32_t record_size = SAMPLE_RATE * (SAMPLE_BITS / 8) * RECORD_TIME * CHANNELS;
  Serial.printf("🎙️ Recording %d bytes (%d seconds)...\n", record_size, RECORD_TIME);

  File file = SD.open(WAV_FILE_NAME, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file on SD!");
    return;
  }

  uint8_t wav_header[WAV_HEADER_SIZE];
  generate_wav_header(wav_header, record_size, SAMPLE_RATE);
  file.write(wav_header, WAV_HEADER_SIZE);

  int16_t *buffer = (int16_t *)malloc(1024);
  if (!buffer) {
    Serial.println("Memory allocation failed!");
    file.close();
    return;
  }

  uint32_t bytes_written = 0;
  size_t bytes_read = 0;

  while (bytes_written < record_size) {
    i2s_read(I2S_NUM, (void*)buffer, 1024, &bytes_read, portMAX_DELAY);
    if (bytes_read > 0) {
      file.write((uint8_t *)buffer, bytes_read);
      bytes_written += bytes_read;
    }
  }

  free(buffer);
  file.close();
  Serial.println("Recording complete! File saved as '/record.wav'.");
}

// -------- WAV Header Generator --------
void generate_wav_header(uint8_t *header, uint32_t data_size, uint32_t sample_rate) {
  uint32_t byte_rate = sample_rate * (SAMPLE_BITS / 8) * CHANNELS;
  uint32_t block_align = CHANNELS * (SAMPLE_BITS / 8);
  uint32_t file_size = data_size + 36;

  const uint8_t header_template[44] = {
    'R','I','F','F',
    0,0,0,0,   // file size
    'W','A','V','E',
    'f','m','t',' ',
    16,0,0,0,  // Subchunk1Size (16)
    1,0,        // AudioFormat (1 = PCM)
    CHANNELS,0,
    0,0,0,0,   // SampleRate
    0,0,0,0,   // ByteRate
    block_align,0,
    SAMPLE_BITS,0,
    'd','a','t','a',
    0,0,0,0    // Data size
  };

  memcpy(header, header_template, 44);
  header[4] = (uint8_t)(file_size & 0xFF);
  header[5] = (uint8_t)((file_size >> 8) & 0xFF);
  header[6] = (uint8_t)((file_size >> 16) & 0xFF);
  header[7] = (uint8_t)((file_size >> 24) & 0xFF);

  header[24] = (uint8_t)(sample_rate & 0xFF);
  header[25] = (uint8_t)((sample_rate >> 8) & 0xFF);
  header[26] = (uint8_t)((sample_rate >> 16) & 0xFF);
  header[27] = (uint8_t)((sample_rate >> 24) & 0xFF);

  header[28] = (uint8_t)(byte_rate & 0xFF);
  header[29] = (uint8_t)((byte_rate >> 8) & 0xFF);
  header[30] = (uint8_t)((byte_rate >> 16) & 0xFF);
  header[31] = (uint8_t)((byte_rate >> 24) & 0xFF);

  header[40] = (uint8_t)(data_size & 0xFF);
  header[41] = (uint8_t)((data_size >> 8) & 0xFF);
  header[42] = (uint8_t)((data_size >> 16) & 0xFF);
  header[43] = (uint8_t)((data_size >> 24) & 0xFF);
}
