#include <Arducam_Mega.h>
#include <Wire.h>
#include <SPI.h>

#define CAM_CS 7
Arducam_Mega myCamera(CAM_CS);

static const uint32_t BAUD = 921600;
static const uint8_t CHUNK = 200; // must be < 255 per library note

void setup() {
  Serial.begin(BAUD);
  delay(1500);

  Serial.println("BOOT");
  Wire.begin();
  SPI.begin();

  CamStatus rc = myCamera.begin();
  Serial.print("CAM_BEGIN=");
  Serial.println((int)rc);
}

void loop() {
  CamStatus st = myCamera.takePicture(CAM_IMAGE_MODE_QVGA, CAM_IMAGE_PIX_FMT_JPG);
  if (st != CAM_ERR_SUCCESS) {
    Serial.print("CAP_ERR=");
    Serial.println((int)st);
    delay(200);
    return;
  }

  uint32_t total = myCamera.getTotalLength();
  if (total == 0) {
    Serial.println("CAP_EMPTY");
    delay(200);
    return;
  }

  Serial.print("FRAME ");
  Serial.println(total);

  uint8_t buf[CHUNK];
  uint32_t sent = 0;

  while (sent < total) {
    uint8_t toRead = (total - sent > CHUNK) ? CHUNK : (uint8_t)(total - sent);
    uint8_t n = myCamera.readBuff(buf, toRead);

    if (n == 0) {
      Serial.println("\nREAD_ERR");
      break;
    }

    Serial.write(buf, n);
    sent += n;
  }

  Serial.println();
  Serial.println("FRAME_END");
  delay(200);
}
