//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32-S3,                                                              *
//**********************************************************************************************************
//
// first release on 11/2018
// Version 3  , Jul.02/2020
//
//
// THE SOFTWARE IS PROVIDED "AS IS" FOR PRIVATE USE ONLY, IT IS NOT FOR COMMERCIAL USE IN WHOLE OR PART OR CONCEPT.
// FOR PERSONAL USE IT IS SUPPLIED WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR
// OR COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
//

#include "Arduino.h"
//#include "WiFiMulti.h"
#include <WiFi.h>
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"

#include "esp_heap_caps.h"
#include "driver/i2s_std.h"

// Digital I/O used
//#define SD_CS          5
//#define SPI_MOSI      23
//#define SPI_MISO      19
//#define SPI_SCK       18
#define I2S_DOUT      8 //25
#define I2S_BCLK      7 //27
#define I2S_LRC       9 //26

Audio audio;
//WiFiMulti wifiMulti;
String ssid =     "KiranF19";
String password = "kiran@8695";

void setup() 
{
    //pinMode(SD_CS, OUTPUT);      digitalWrite(SD_CS, HIGH);
    //SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    //SPI.setFrequency(1000000);

    Serial.begin(115200);
    //Serial.printf("Internal heap: %u\n", ESP.getFreeHeap());
    //Serial.printf("PSRAM size: %u\n", ESP.getPsramSize());
    //Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());
    //Serial.printf("Flash chip size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));

//    SD.begin(SD_CS);
/*    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid.c_str(), password.c_str());
    wifiMulti.run();
    if(WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    Serial.print(" status="); Serial.println(WiFi.status());
  }

  Serial.println("\nCONNECTED!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

    Serial.printf("Largest free block after WiFi: %u\n",
               heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Serial.printf("Internal heap: %u\n", ESP.getFreeHeap());
    Serial.printf("PSRAM size: %u\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %u\n", ESP.getFreePsram());

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12); // 0...21
    
     //audio.connecttoFS(SD, "test1.mp3");
//    audio.connecttoFS(SD, "test.wav");
//    audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
//    audio.connecttohost("http://macslons-irish-pub-radio.com/media.asx");
//    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.aac"); //  128k aac
//    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3
//    audio.connecttohost("https://github.com/kiranshingote007/ESP32/blob/main/test.mp3");
//    audio.connecttohost("https://github.com/kiranshingote007/ESP32/raw/refs/heads/main/test.mp3");
      audio.connecttohost("https://raw.githubusercontent.com/kiranshingote007/ESP32/main/test.mp3");

      Serial.printf("Connected to http");
}

void loop()
{
    audio.loop();
    if(Serial.available())
    { // put streamURL in serial monitor
        //audio.stopSong();
        //String r=Serial.readString(); r.trim();
        //if(r.length()>5) audio.connecttohost(r.c_str());
        //log_i("free heap=%i", ESP.getFreeHeap());
        
        if (Serial.available()) 
        {
            char c = Serial.read();
            if (c == 's') 
            {
                bool running = audio.isRunning();
                if (running) 
                {
                    Serial.println("Stopping...");
                    audio.stopSong();
                } 
                else
                {
                    Serial.println("Reconnecting...");
                    audio.connecttohost("https://raw.githubusercontent.com/kiranshingote007/ESP32/main/test.mp3");
                    Serial.printf("Internal heap: %u\n", ESP.getFreeHeap());
                }    
            }
        }
    }
}
