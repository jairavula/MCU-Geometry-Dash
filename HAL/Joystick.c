/*
 * Joystick.c
 *
 *  Created on: Mar 27, 2024
 *      Author: jaikr
 */

// Constructs joystick object
// Initializes output FSMS

#include <HAL/Joystick.h>

#define LEFT_THRESHHOLD 3000
#define RIGHT_THRESHHOLD 13000
#define TOP_THRESHHOLD 13000
#define BOTTOM_THRESHHOLD 3000

enum _JoystickDebounceStateLeft { NOT_LEFT, LEFT };
typedef enum _JoystickDebounceStateLeft JoystickDebounceStateLeft;

enum _JoystickDebounceStateRight { NOT_RIGHT, RIGHT };
typedef enum _JoystickDebounceStateRight JoystickDebounceStateRight;

enum _JoystickDebounceStateTop { NOT_TOP, TOP };
typedef enum _JoystickDebounceStateTop JoystickDebounceStateTop;

enum _JoystickDebounceStateBottom { NOT_BOTTOM, BOTTOM };
typedef enum _JoystickDebounceStateBottom JoystickDebounceStateBottom;


void startADC() {
    // Starts the ADC with the first conversion
    // in repeat-mode, subsequent conversions run automatically
    ADC14_enableConversion();
    ADC14_toggleConversionTrigger();
}

void initADC() {
    ADC14_enableModule();

    ADC14_initModule(ADC_CLOCKSOURCE_SYSOSC,
                     ADC_PREDIVIDER_1,
                     ADC_DIVIDER_1,
                     0
    );

    // This configures the ADC to store output results
    // in ADC_MEM0 for joystick X.
    ADC14_configureMultiSequenceMode(ADC_MEM0, ADC_MEM1, true);

    // This configures the ADC in manual conversion mode
    // Software will start each conversion.
    ADC14_enableSampleTimer(ADC_AUTOMATIC_ITERATION);
}

void initJoyStick() {

    // This configures ADC_MEM0 to store the result from
    // input channel A15 (Joystick X), in non-differential input mode
    // (non-differential means: only a single input pin)
    // The reference for Vref- and Vref+ are VSS and VCC respectively
    ADC14_configureConversionMemory(ADC_MEM0,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A15,                 // joystick X
                                    ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input
    // A15 is multiplexed on GPIO port P6 pin PIN0
    // TODO: which one of GPIO_PRIMARY_MODULE_FUNCTION, or
    //                    GPIO_SECONDARY_MODULE_FUNCTION, or
    //                    GPIO_TERTIARY_MODULE_FUNCTION
    // should be used in place of 0 as the last argument?
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
                                               GPIO_PIN0,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

    // TODO: add joystick Y
    ADC14_configureConversionMemory(ADC_MEM1,
                                    ADC_VREFPOS_AVCC_VREFNEG_VSS,
                                    ADC_INPUT_A9,                 // joystick X
                                    ADC_NONDIFFERENTIAL_INPUTS);

    // This selects the GPIO as analog input
    // A15 is multiplexed on GPIO port P6 pin PIN0
    // TODO: which one of GPIO_PRIMARY_MODULE_FUNCTION, or
    //                    GPIO_SECONDARY_MODULE_FUNCTION, or
    //                    GPIO_TERTIARY_MODULE_FUNCTION
    // should be used in place of 0 as the last argument?
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                               GPIO_PIN4,
                                               GPIO_TERTIARY_MODULE_FUNCTION);

}

void getSampleJoyStick(unsigned *X, unsigned *Y) {
    // ADC runs in continuous mode, we just read the conversion buffers
    *X = ADC14_getResult(ADC_MEM0);
    *Y = ADC14_getResult(ADC_MEM1);

}


Joystick Joystick_construct() {
    // The Joystick object which will be returned at the end of construction
    Joystick Joystick;

    initADC();
    initJoyStick();
    startADC();

    // Initialize all buffered outputs of the Joystick
    // Joystick.pushState = RELEASED;
    // Joystick.isTapped = false;

    // Return the constructed Joystick object to the user
    return Joystick;
}
// Refreshes the input of the joystick by polling x and y channel
void Joystick_refresh(Joystick* joystick_p){
    joystick_p->x = ADC14_getResult(ADC_MEM0);
    joystick_p->y = ADC14_getResult(ADC_MEM1);

    joystick_p->isTappedToLeft = Joystick_isTappedToLeft(joystick_p);
    joystick_p->isTappedToRight = Joystick_isTappedToRight(joystick_p);
    joystick_p->isTappedToTop = Joystick_isTappedToTop(joystick_p);
    joystick_p->isTappedToBottom = Joystick_isTappedToBottom(joystick_p);

    joystick_p->isPressedToLeft = Joystick_isPressedtoLeft(joystick_p);
    joystick_p->isPressedToRight = Joystick_isPressedtoRight(joystick_p);




}

bool Joystick_isPressedtoLeft(Joystick* joystick_p){
    return (joystick_p->x < LEFT_THRESHHOLD);
}
bool Joystick_isPressedtoRight(Joystick* joystick_p){
    return (joystick_p->x > RIGHT_THRESHHOLD);
}
bool Joystick_isPressedtoTop(Joystick* joystick_p){
    return (joystick_p->y > TOP_THRESHHOLD);
}
bool Joystick_isPressedtoBottom(Joystick* joystick_p){
    return (joystick_p->y < BOTTOM_THRESHHOLD);
}
bool Joystick_isTappedToLeft(Joystick* joystick_p){
    static JoystickDebounceStateLeft state = NOT_LEFT;
    bool output = false;

    switch(state) {
    case NOT_LEFT:
        if (joystick_p->x < LEFT_THRESHHOLD){
            state = LEFT;
            output = true;
        }
        break;
    case LEFT:
        if (joystick_p->x > LEFT_THRESHHOLD){
            state = NOT_LEFT;
            output = false;
        }
        break;
    }

    return output;
}

bool Joystick_isTappedToRight(Joystick* joystick_p){
    static JoystickDebounceStateRight state = NOT_RIGHT;
    bool output = false;

    switch(state) {
    case NOT_RIGHT:
        if (joystick_p->x > RIGHT_THRESHHOLD){
            state = RIGHT;
            output = true;
        }
        break;
    case RIGHT:
        if (joystick_p->x < RIGHT_THRESHHOLD){
            state = NOT_RIGHT;
            output = false;
        }
        break;
    }

    return output;
}

bool Joystick_isTappedToTop(Joystick* joystick_p){
    static JoystickDebounceStateTop state = NOT_TOP;
    bool output = false;

    switch(state) {
    case NOT_TOP:
        if (joystick_p->y > TOP_THRESHHOLD){
            state = TOP;
            output = true;
        }
        break;
    case TOP:
        if (joystick_p->y < TOP_THRESHHOLD){
            state = NOT_TOP;
            output = false;
        }
        break;
    }

    return output;
}

bool Joystick_isTappedToBottom(Joystick* joystick_p){
    static JoystickDebounceStateBottom state = NOT_BOTTOM;
    bool output = false;

    switch(state) {
    case NOT_BOTTOM:
        if (joystick_p->y < BOTTOM_THRESHHOLD){
            state = BOTTOM;
            output = true;
        }
        break;
    case BOTTOM:
        if (joystick_p->y > BOTTOM_THRESHHOLD){
            state = NOT_BOTTOM;
            output = false;
        }
        break;
    }

    return output;
}





