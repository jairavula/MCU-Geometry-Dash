/*
 * Application.h
 *
 *  Created on: Dec 29, 2019
 *      Author: Matthew Zhong
 *  Supervisor: Leyla Nazhand-Ali
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <HAL/HAL.h>

typedef enum { splashScreen, mainMenuScreen, gameScreen, highScoresScreen, instructionsScreen} _screenState;

struct _Application {
  // Put your application members and FSM state variables here!
  // =========================================================================
  UART_Baudrate baudChoice;
  bool firstCall;
  SWTimer timer;
};

struct _Gamesettings {
    _screenState screenState;

    bool loadTitleScreen;
    bool loadInstructionsScreen;
    bool loadHighScoresScreen;
    bool loadGameScreen;


};

struct _Rectangle{
    int r11;
    int r12;
    int r21;
    int r22;
};

typedef struct _Gamesettings Gamesettings;
typedef struct _Application Application;
typedef struct _Rectangle Rectangle;

// Called only a single time - inside of main(), where the application is
// constructed
Application Application_construct();

// Called once per super-loop of the main application.
void Application_loop(Application* app, HAL* hal);

// Called whenever the UART module needs to be updated
void Application_updateCommunications(Application* app, HAL* hal);

// Interprets an incoming character and echoes back to terminal what kind of
// character was received (number, letter, or other)
char Application_interpretIncomingChar(char);

#endif /* APPLICATION_H_ */
