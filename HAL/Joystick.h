/*
 * Joystick.h
 *
 *  Created on: Mar 27, 2024
 *      Author: jaikr
 */

#ifndef HAL_JOYSTICK_H_
#define HAL_JOYSTICK_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>



struct _Joystick {

    uint_fast16_t x;
    uint_fast16_t y;
    bool isTappedToLeft;
    bool isTappedToRight;
    bool isTappedToTop;
    bool isTappedToBottom;

    bool isPressedToLeft;
    bool isPressedToRight;
    bool isPressedToTop;
    bool isPressedToBottom;


};
typedef struct _Joystick Joystick;

/** Constructs a new Joystick object, given a valid port and pin. */
Joystick Joystick_construct();

/** Given a Joystick, determines if the switch is currently pushed to the left */
bool Joystick_isPressedtoLeft(Joystick* Joystick);

/** Given a Joystick, determines if the switch is currently pushed to the right */
bool Joystick_isPressedtoRight(Joystick* Joystick);

bool Joystick_isPressedtoTop(Joystick* Joystick);

bool Joystick_isPressedtoBottom(Joystick* Joystick);
/** Given a Joystick, determines if it was "tapped" to left - pressed left and released */
bool Joystick_isTappedToTop(Joystick* Joystick);

/** Given a Joystick, determines if it was "tapped" to right - pressed right and released */
bool Joystick_isTappedToBottom(Joystick* Joystick);

bool Joystick_isTappedToRight(Joystick* Joystick);

bool Joystick_isTappedToLeft(Joystick* Joystick);

/** Refreshes this Joystick so the Joystick FSM now has new outputs to interpret */
void Joystick_refresh(Joystick* Joystick);





#endif /* HAL_JOYSTICK_H_ */
