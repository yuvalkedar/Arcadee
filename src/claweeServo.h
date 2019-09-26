#pragma once

#include <Servo.h>

class ClaweeServo {
   public:
    ClaweeServo(uint8_t interval);

    void Attach(uint8_t pin);

    void Detach();

    void Restart();

    void Magazine_restart();

    void Load();

    void Update();

   private:
    Servo servo;
    uint8_t position;
    uint8_t increment;
    uint8_t servo_update;
    uint32_t last_update;
};
