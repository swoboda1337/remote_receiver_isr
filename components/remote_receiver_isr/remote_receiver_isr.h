#pragma once

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include <vector>

namespace esphome {
namespace remote_receiver_isr {

struct RemoteStore {
  static void gpio_intr(RemoteStore *arg);
  volatile int32_t (*buffer)[1024]{nullptr};
  volatile uint32_t size[4] = {0};
  volatile uint32_t next{0};
  volatile uint32_t count{0};
  volatile uint32_t capture{156};
  volatile uint32_t sequence{6};
  volatile uint32_t idle{8000};
  volatile uint32_t write{0};
  volatile uint32_t read{0};
  volatile uint32_t delta{0};
  volatile uint32_t shortest{100};
  volatile uint32_t prev_micros{0};
  volatile int32_t prev_diff{0};
  volatile int32_t len1{0};
  volatile int32_t len2{0};
  ISRInternalGPIOPin pin;
};

class RemoteReceiverIsr : public Component {
 public:
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void set_pin(InternalGPIOPin *pin) { this->pin_ = pin; }
  void set_shortest(uint32_t shortest) { this->store_.shortest = shortest; }
  void set_capture(uint32_t capture) { this->store_.capture = capture; }
  void set_idle(uint32_t idle) { this->store_.idle = idle; }
  void setup() override;
  void loop() override;
  void dump_config() override;
  Trigger<std::vector<int32_t>> *get_packet_trigger() const { return this->raw_trigger_; };

 protected:
  Trigger<std::vector<int32_t>> *raw_trigger_{new Trigger<std::vector<int32_t>>()};
  InternalGPIOPin *pin_{nullptr};
  RemoteStore store_;
};

}  // namespace remote_receiver_isr
}  // namespace esphome
