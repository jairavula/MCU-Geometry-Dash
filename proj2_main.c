/**
 * Starter code for Project 2. Good luck!
 *
 * We recommending copy/pasting your HAL folder from Project 1
 * into this project.
 */

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* HAL and Application includes */
#include <Application.h>
#include <HAL/HAL.h>
#include <HAL/Timer.h>

void Screen_manager(HAL *hal_p, Application *app_p);
void Splash_screen(HAL *hal_p, Gamesettings *game);
void Title_screen(HAL *hal_p, Gamesettings *game);

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void InitNonBlockingLED() {
  GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
  GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
}

// Non-blocking check. Whenever Launchpad S1 is pressed, LED1 turns on.
static void PollNonBlockingLED() {
  GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
  if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN1) == 0) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
  }
}

int main() {
  WDT_A_holdTimer();

  // Initialize the system clock and background hardware timer, used to enable
     // software timers to time their measurements properly.
     InitSystemTiming();

     // Initialize the main Application object and HAL object
     HAL hal = HAL_construct();
     Application app = Application_construct();

  // Do not remove this line. This is your non-blocking check.
  InitNonBlockingLED();
  while (1) {
    // Do not remove this line. This is your non-blocking check.
   HAL_refresh(&hal);
   Application_loop(&app, &hal);
   PollNonBlockingLED();
  }
}

Application Application_construct()
{
    Application app;
    // Initialize local application state variables here!
    app.baudChoice = BAUD_9600;
    app.firstCall = true;
    app.timer = SWTimer_construct(TITLE_SCREEN_WAIT);
    SWTimer_start(&app.timer);

    return app;
}

void Application_loop(Application *app_p, HAL *hal_p)
{
// Entire game is handled by the screen manager
    Screen_manager(hal_p, app_p);
}

void Screen_manager(HAL *hal_p, Application *app_p)
{ // Initializes the game settings for a new game
static Gamesettings game = {splashScreen};
    switch (game.screenState) // FSM logic for each screen setting
    {
    // entry point of game, leads to game settings or instructions screen through button taps
    case splashScreen:
        Splash_screen(hal_p, &game);
        if (SWTimer_expired(&app_p->timer)){
            game.screenState = mainMenuScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case mainMenuScreen:
        Title_screen(hal_p, &game);
    }
}


void Splash_screen(HAL *hal_p, Gamesettings *game){

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Spring 2024 Project", -1, 5, 5, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "ECE Surfers", -1, 5, 13, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Jai Ravula", -1, 5, 21, true);
}

void Title_screen(HAL *hal_p, Gamesettings *game){

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Play ECE Surfers", -1, 5, 40, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Instructions", -1, 5, 48, true);
     Graphics_drawString(&hal_p->g_sContext, (int8_t*) "View High Scores", -1, 5, 56, true);

}
