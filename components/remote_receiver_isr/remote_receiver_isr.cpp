#include "remote_receiver_isr.h"

#include <cinttypes>

namespace esphome {
namespace remote_receiver_isr {

static const char *const TAG = "remote_receiver_isr";

void IRAM_ATTR HOT RemoteStore::gpio_intr(RemoteStore *arg) {
  uint32_t now = micros();
  int32_t level = arg->pin.digital_read() ?  1 : 0;
  int32_t diff = now - arg->prev_micros;

  // sanity check
  if (level != arg->next) {
    arg->count = 0;
    return;
  }
  arg->next ^= 1;

  // check for idle
  if (diff > arg->idle) {
    if (arg->count >= arg->sequence) {
      arg->size[arg->write] = arg->count;
      arg->write = (arg->write + 1) & 3;
    }
    arg->count = 0;
  }

  // wait until the end of off
  if (level == 1) {
    if (arg->count >= 2) {
      if (arg->count >= arg->sequence) {
        // copy the rest of the data
        arg->buffer[arg->write][arg->count++] = arg->prev_diff;
        arg->buffer[arg->write][arg->count++] = diff;
        if (arg->count >= arg->capture) {
          arg->size[arg->write] = arg->count;
          arg->write = (arg->write + 1) & 3;
          arg->count = 0;
        }
      } else if (level == 1) {
        // check if current bit matches
        if ((std::abs(arg->len1 - arg->prev_diff) < arg->delta && std::abs(arg->len2 - diff) < arg->delta) ||
            (std::abs(arg->len2 - arg->prev_diff) < arg->delta && std::abs(arg->len1 - diff) < arg->delta)) {
          arg->buffer[arg->write][arg->count++] = arg->prev_diff;
          arg->buffer[arg->write][arg->count++] = diff;
        } else {
          arg->count = 0;
        }
      }
    }

    // start a new sequence
    if (arg->count == 0) {
      arg->len1 = arg->prev_diff;
      arg->len2 = diff;
      if (arg->len1 >= arg->shortest && arg->len2 >= arg->shortest) { 
        arg->delta = (std::min(arg->len1, arg->len2) * 4) / 16;
        arg->buffer[arg->write][arg->count++] = arg->prev_diff;
        arg->buffer[arg->write][arg->count++] = diff;
      }
    }
  }
  
  // update prev
  arg->prev_micros = now;
  arg->prev_diff = diff;
}

void RemoteReceiverIsr::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Remote Receiver ISR...");  
  this->store_.buffer = new int32_t[4][1024];
  memset((uint8_t*)this->store_.buffer, 0, sizeof(int32_t[4][1024]));
  this->pin_->setup();
  this->store_.pin = pin_->to_isr();
  this->pin_->attach_interrupt(RemoteStore::gpio_intr, &this->store_, gpio::INTERRUPT_ANY_EDGE);
}

void RemoteReceiverIsr::loop() {
  if (this->store_.read != this->store_.write) {
    uint32_t size = this->store_.size[this->store_.read];
    std::vector<int32_t> x;
    for (int32_t i = 0; i < size; i++) {
      int32_t val = this->store_.buffer[this->store_.read][i];
      if ((i & 1) == 0) {
        x.push_back(+val);
      } else {
        x.push_back(-val);
      }
    }
    this->store_.read = (this->store_.read + 1) & 3;
    this->raw_trigger_->trigger(x);
  }
}

void RemoteReceiverIsr::dump_config() {
  ESP_LOGCONFIG(TAG, "Remote Remote Receiver ISR:");

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Configuring Remote Receiver ISR failed");
  }
}

}  // namespace remote_receiver_isr
}  // namespace esphome
