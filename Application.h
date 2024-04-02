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
#define MAX_OBSTACLES 10

typedef enum { splashScreen, mainMenuScreen, gameScreen, highScoresScreen, instructionsScreen, gameOverScreen} _screenState;


typedef struct _Gamesettings Gamesettings;
typedef struct _Application Application;
typedef struct _Rectangle Rectangle;
typedef struct _Obstacle Obstacle;

struct _Application {
  // Put your application members and FSM state variables here!
  // =========================================================================
  UART_Baudrate baudChoice;
  bool firstCall;
  SWTimer timer;
};

struct _Obstacle{
    Graphics_Rectangle rect;
    bool isActive;
    bool hasCollided;
};

struct _Gamesettings {
    _screenState screenState;

    bool loadTitleScreen;
    bool loadInstructionsScreen;
    bool loadHighScoresScreen;
    bool loadGameScreen;
    bool loadGameOverScreen;

    Graphics_Rectangle playerPos;
    Graphics_Rectangle lastPlayerPos;
    int lives;
    int currentScore;


    SWTimer timer;
    int numObstacles;
    Obstacle obstacles[MAX_OBSTACLES];

};


struct _Rectangle{
    int r11;
    int r12;
    int r21;
    int r22;
};



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
