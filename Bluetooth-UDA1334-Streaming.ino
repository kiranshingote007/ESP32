#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
 
const uint8_t I2S_SCK = 27;       /* Audio data bit clock */
const uint8_t I2S_WS = 26;       /* Audio data left and right clock */
const uint8_t I2S_SDOUT = 25;    /* ESP32 audio data output (to speakers) */
I2SClass i2s;
 
BluetoothA2DPSink a2dp_sink(i2s);

void clearBT() 
{
  esp_bluedroid_disable();
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  esp_bluedroid_enable();
}
void setup() 
{
  delay(500);
  Serial.begin(115200);

  clearBT();
  delay(500);

  i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);

  if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) 
  {
      Serial.println("Failed to initialize I2S!");
      while (1); // do nothing
  }
 
  delay(500); // small delay for BT stack stability

  // Start Bluetooth sink
  a2dp_sink.start("MyMusic");

  Serial.println("Bluetooth A2DP Sink started. Now try connecting!");
}
void loop() {}