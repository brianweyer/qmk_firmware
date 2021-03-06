#include "knob_v2.h"

bool knob_prev_a = false;
knob_report_t knob_report = {.dir = 0, .phase = 0};

void knob_init(void) {
    // I use pins D1 (ISR1) & D4 for a knob.

    // Set pin mode for D4 as input.
    DDRD &= ~(0UL << PD6);

    // Enable internal pull-up for D4.
    // This is done by "writing" 1 to a pin that has its mode set to input.
    PORTD |= (1 << PD6);

    // Enable interrupt for D1
    // For more info on the below flags see this awesome section 11.1 (pages 89-90) here:
    // https://cdn-shop.adafruit.com/datasheets/atmel-7766-8-bit-avr-atmega16u4-32u4_datasheet.pdf
    // Set pin mode & pull-up.
    DDRD &= ~(0UL << PD2);
    PORTD |= (1UL << PD2);

    // INT:    33221100
    EICRA |= 0b00010000;  // 0b01 - any edge
    // INT:     6  3210
    EIMSK |= 0b00000100;
}

ISR(INT1_vect) {
    // Port PD1 (Pin 2)
    bool a = PIND & (1 << PD2);

    if (knob_prev_a != a) {
        // "A" channel has REALLY changed.
        knob_report.phase = a;
        knob_prev_a = a;
        bool b = PIND & (1 << PD6);
        if (a == b) {
            // Halfway through CCW rotation (A == B)
            //
            //         +---YOU ARE HERE (A=1, B=1)
            //         |     +---OR HERE (A=0, B=0)
            //         |     |
            //         v     v
            // A: _____/^^^^^\__
            // B: __/^^^^^\_____
            knob_report.dir++;
        } else {
            // Halfway through CW rotation (A != B)
            //
            //         +---YOU ARE HERE (A=1, B=0)
            //         |     +---OR HERE (A=0, B=1)
            //         |     |
            //         v     v
            // A: _____/^^^^^\_____
            // B: ________/^^^^^\__
            knob_report.dir--;
        }
    }
}

knob_report_t knob_report_read(void) {
    // Return knob report.
    return knob_report;
}

void knob_report_reset(void) {
    // Call this ASAP once you've processed the previous knob report.
    // TODO: This should probably be called within `knob_report_read`.
    knob_report = (knob_report_t){.dir = 0, .phase = 0};
}

