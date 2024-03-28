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

#define TITLE_SCREEN_PLAY_CURSOR_POS 40
#define TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS 48
#define TITLE_SCREEN_SCORES_CURSOR_POS 56
#define TITLE_SCREEN_CURSOR_OFFSET 8
#define JOYSTICK_COOLDOWN 3
#define MAX_LIVES 3

#define LEFT_BOUND 0
#define RIGHT_BOUND 128
#define TOP_BOUND 50
#define BOTTOM_BOUND 95

void Screen_manager(HAL *hal_p, Application *app_p);
void Splash_screen(HAL *hal_p, Gamesettings *game);
void Title_screen(HAL *hal_p, Gamesettings *game);
void Instructions_screen(HAL* hal_p, Gamesettings* game);
void Highscores_screen(HAL* hal_p, Gamesettings* game);
void Game_screen(HAL* hal_p, Gamesettings* game);
void Draw_player(HAL *hal_p,Graphics_Rectangle *playerPos);
void Erase_player(HAL *hal_p,Graphics_Rectangle *playerPos);
void Player_movementLogic(HAL* hal_p);
void Game_screenGraphics(HAL* hal_p, Gamesettings *game);
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
    static Gamesettings game = {splashScreen, false, false, false, false};
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
        if (game.loadInstructionsScreen == true){
            game.screenState = instructionsScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        if (game.loadHighScoresScreen == true){
            game.screenState = highScoresScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        if (game.loadGameScreen == true){
            game.screenState = gameScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case gameScreen:
        Game_screen(hal_p, &game);
        break;
    case instructionsScreen:
        Instructions_screen(hal_p, &game);
        if (game.loadTitleScreen == true){
            game.screenState = mainMenuScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case highScoresScreen:
        Highscores_screen(hal_p, &game);
        if (game.loadTitleScreen == true){
            game.screenState = mainMenuScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
            break;

        }
    }
}


void Splash_screen(HAL *hal_p, Gamesettings *game){

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Spring 2024 Project", -1, 5, 5, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "ECE Surfers", -1, 5, 13, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Jai Ravula", -1, 5, 21, true);
}

void Title_screen(HAL *hal_p, Gamesettings *game){

    static int cursorPos = TITLE_SCREEN_PLAY_CURSOR_POS;
    static int joystickTimer;

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Play ECE Surfers", -1, 5, 40, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Instructions", -1, 5, 48, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "View High Scores", -1, 5, 56, true);

    if (hal_p->joystick.isTappedToBottom && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 40, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 48, true);
        cursorPos += TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToBottom && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 48, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 56, true);
        cursorPos += TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToTop && cursorPos == TITLE_SCREEN_SCORES_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 56, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 48, true);
        cursorPos -= TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToTop && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 48, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 40, true);
        cursorPos -= TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }

    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadGameScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadInstructionsScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_SCORES_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadHighScoresScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    joystickTimer++;
}

void Instructions_screen(HAL* hal_p, Gamesettings* game){
    static int buttonTimer = 0;
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "INSTRUCTIONS SCREEN", -1, 5, 5, true);

    if(Button_isPressed(&hal_p->boosterpackJS) && buttonTimer > JOYSTICK_COOLDOWN){
        game->loadInstructionsScreen = false;
        game->loadTitleScreen = true;
        buttonTimer = 0;
    }
    buttonTimer++;
}

void Highscores_screen(HAL* hal_p, Gamesettings* game){
    static int buttonTimer = 0;
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "HIGH SCORES", -1, 5, 5, true);

    if(Button_isPressed(&hal_p->boosterpackJS) && buttonTimer > JOYSTICK_COOLDOWN){
        game->loadHighScoresScreen = false;
        game->loadTitleScreen = true;
        buttonTimer = 0;
    }
    buttonTimer++;
}

void Game_screen(HAL* hal_p, Gamesettings* game){


    Player_movementLogic(hal_p);
    Game_screenGraphics(hal_p, game);

}

void Game_screenGraphics(HAL* hal_p, Gamesettings *game){
   static int i;
    Graphics_Rectangle bar1 = {0, 25, 128, 25};
    Graphics_Rectangle bar2 = {0, 50, 128, 50};
    Graphics_Rectangle bar3 = {0, 75, 128, 75};
    Graphics_Rectangle bar4 = {0, 100, 128, 100};

    static Graphics_Rectangle lives = {64, 110, 68, 114};


    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Score: 0 ", -1, 30, 5, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Lives: ", -1, 20, 110, true);


    Graphics_drawRectangle(&hal_p->g_sContext, &bar1);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar2);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar3);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar4);

    for (i=0; i < MAX_LIVES; i++){
        Graphics_drawRectangle(&hal_p->g_sContext, &lives);
        lives.xMin += 10;
        lives.xMax += 10;
    }
    lives.xMin = 64;
    lives.xMax = 68;







}

void Player_movementLogic(HAL* hal_p){

    static Graphics_Rectangle playerPos = {50, 45, 55, 50};
    static Graphics_Rectangle lastPlayerPos = {50, 45, 55, 50};


    if (hal_p->joystick.isPressedToRight && playerPos.xMax < RIGHT_BOUND){
        playerPos.xMin++;
        playerPos.xMax++;
        Erase_player(hal_p, &lastPlayerPos);
        Draw_player(hal_p, &playerPos);
        lastPlayerPos.xMin = playerPos.xMin;
        lastPlayerPos.xMax = playerPos.xMax;
    }
    if (hal_p->joystick.isPressedToLeft && playerPos.xMin > LEFT_BOUND){
        playerPos.xMin--;
        playerPos.xMax--;
        Erase_player(hal_p, &lastPlayerPos);
        Draw_player(hal_p, &playerPos);
        lastPlayerPos.xMin = playerPos.xMin;
        lastPlayerPos.xMax = playerPos.xMax;
    }
    if (hal_p->joystick.isTappedToTop && playerPos.yMin > TOP_BOUND){
           playerPos.yMin -=25;
           playerPos.yMax -=25;
           Erase_player(hal_p, &lastPlayerPos);
           Draw_player(hal_p, &playerPos);
           lastPlayerPos.yMin = playerPos.yMin;
           lastPlayerPos.yMax = playerPos.yMax;
       }
    if (hal_p->joystick.isTappedToBottom && playerPos.yMin < BOTTOM_BOUND){
               playerPos.yMin +=25;
               playerPos.yMax +=25;
               Erase_player(hal_p, &lastPlayerPos);
               Draw_player(hal_p, &playerPos);
               lastPlayerPos.yMin = playerPos.yMin;
               lastPlayerPos.yMax = playerPos.yMax;
           }

}

void Draw_player(HAL *hal_p,Graphics_Rectangle *playerPos){
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_LIME_GREEN);
    Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
    Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}

void Erase_player(HAL *hal_p,Graphics_Rectangle *playerPos){
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
       Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
       Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
       Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}





