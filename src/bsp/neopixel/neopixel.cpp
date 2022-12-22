#include "include/neopixel.h"

bool bsp_neopixel_init(bsp_neopixel_handler_t *handler_p_) {
  if (handler_p_->is_init == true) {
    return false;
  }
  handler_p_->is_init = true;

  handler_p_->pixel_h = new Adafruit_NeoPixel(handler_p_->pixel_cnt,
                                              handler_p_->pin_num,
                                              NEO_GRBW + NEO_KHZ800);
  handler_p_->pixel_h->begin();
  handler_p_->pixel_h->show();
  handler_p_->pixel_h->setBrightness(255);

  return true;
}

bool bsp_neopixel_set(bsp_neopixel_handler_t *handler_p_, uint8_t R_, uint8_t G_, uint8_t B_, uint8_t W_) {
  if (handler_p_->is_init == false) {
    return false;
  }

  uint32_t color = handler_p_->pixel_h->Color(R_, G_, B_, W_);
  for (size_t i = 0; i < handler_p_->pixel_cnt; i++) {
    handler_p_->pixel_h->setPixelColor(i, color);
  }
  handler_p_->pixel_h->show();

  return true;
}
