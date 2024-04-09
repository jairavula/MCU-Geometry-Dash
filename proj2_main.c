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

#define TITLE_SCREEN_PLAY_CURSOR_POS 5
#define TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS 13
#define TITLE_SCREEN_SCORES_CURSOR_POS 21
#define TITLE_SCREEN_CURSOR_OFFSET 8
#define JOYSTICK_COOLDOWN 3
#define MAX_LIVES 3

#define LEFT_BOUND 0
#define RIGHT_BOUND 128
#define TOP_BOUND 50
#define BOTTOM_BOUND 95

#define DEFAULT_SCORE 0

extern const Graphics_Image explosion8BPP_UNCOMP;
extern const Graphics_Image titleScreen8BPP_UNCOMP;

void Screen_manager(HAL *hal_p, Application *app_p);
void Splash_screen(HAL *hal_p, Gamesettings *game);
void Title_screen(HAL *hal_p, Gamesettings *game);
void Instructions_screen(HAL* hal_p, Gamesettings* game);
void Highscores_screen(HAL* hal_p, Gamesettings* game);
void Game_screen(HAL* hal_p, Gamesettings* game);
void Draw_player(HAL *hal_p,Graphics_Rectangle *playerPos);
void Erase_player(HAL *hal_p,Graphics_Rectangle *playerPos);
void Player_movementLogic(HAL* hal_p, Gamesettings *game);
void Game_screenGraphics(HAL* hal_p, Gamesettings *game);
void ClearLivesDisplay(HAL* hal_p, const Graphics_Rectangle* area);
void AddObstacle(Gamesettings* game, Graphics_Rectangle* rect);
void SpawnObstacles(HAL* hal_p, Gamesettings* game);
void UpdateAndDrawObstacles(HAL* hal_p, Gamesettings* game);
bool CheckCollision(Graphics_Rectangle* playerRect, Graphics_Rectangle* obstacleRect);
void CheckAndHandleCollisions(HAL* hal_p, Gamesettings* game);
void Game_overScreen(HAL* hal_p, Gamesettings* game);
void Player_jumpLogic(HAL* hal_p, Gamesettings *game);
void UpdateHighScores(Gamesettings *game);
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
    static Gamesettings game = {splashScreen, false, false, false, false, false, {50, 45, 55, 50},{50, 45, 55, 50}, 3, 0,{0} };
    switch (game.screenState) // FSM logic for each screen setting
    {
    // entry point of game, leads to game settings or instructions screen through button taps
    case splashScreen:
        Splash_screen(hal_p, &game);
        if (SWTimer_expired(&app_p->timer)){
            game.screenState = mainMenuScreen;
            game.loadTitleScreen = true;
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
        if (game.loadGameOverScreen){
            game.screenState = gameOverScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
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
    case gameOverScreen:
        Game_overScreen(hal_p, &game);
        if (game.loadTitleScreen){
            game.screenState = mainMenuScreen;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
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

    Graphics_Rectangle topBar = {0,0,128,30};

    if (game->loadTitleScreen){
        Graphics_drawImage(&hal_p->g_sContext,  &titleScreen8BPP_UNCOMP, 0, 0);
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
           Graphics_fillRectangle(&hal_p->g_sContext, &topBar);
           Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Play ECE Surfers", -1, 5, 5, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Instructions", -1, 5, 13, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "View High Scores", -1, 5, 21, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 13 , true);
         game->loadTitleScreen = false;
    }


    if (hal_p->joystick.isTappedToBottom && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 5, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 13, true);
        cursorPos += TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToBottom && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 13, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 21, true);
        cursorPos += TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToTop && cursorPos == TITLE_SCREEN_SCORES_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 21, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 13, true);
        cursorPos -= TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }
    if (hal_p->joystick.isTappedToTop && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer >  JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 13, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 5, true);
        cursorPos -= TITLE_SCREEN_CURSOR_OFFSET;
        joystickTimer = 0;
    }

    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadGameScreen = true;
       // game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadInstructionsScreen = true;
       // game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isPressed(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_SCORES_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadHighScoresScreen = true;
       // game->loadTitleScreen = false;
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

void Highscores_screen(HAL* hal_p, Gamesettings* game) {
    static int buttonTimer = 0;
    int i;
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "HIGH SCORES", -1, 5, 5, true);

    char scoreStr[20]; // Buffer to hold the score string
    int yPosition = 20; // Starting y position for the first high score

    // Loop through the highScores array and print each score
    for ( i = 0; i < MAX_HIGH_SCORES; i++) {
        sprintf(scoreStr, "%d: %d", i + 1, game->highScores[i]); // Format: "1: 100"
        Graphics_drawString(&hal_p->g_sContext, (int8_t*)scoreStr, -1, 5, yPosition, true);
        yPosition += 10; // Move down for the next score; adjust spacing as needed
    }

    // Button logic to return to the main menu
    if (Button_isPressed(&hal_p->boosterpackJS) && buttonTimer > JOYSTICK_COOLDOWN) {
        game->loadHighScoresScreen = false;
        game->loadTitleScreen = true;
        buttonTimer = 0;
    }
    buttonTimer++;
}

void Game_screen(HAL* hal_p, Gamesettings* game){

    char scoreStr[20]; // Buffer to hold the score string, adjust size if needed

    if (game->loadGameScreen){
        game->scoreTimer = SWTimer_construct(ADD_SCORE_TIME);
        game->loadGameScreen = false;
    }

    if (SWTimer_expired(&game->scoreTimer)) {
            SWTimer_start(&game->scoreTimer); // Reset the timer
        }

    sprintf(scoreStr, "Score: %d", game->currentScore);
       Graphics_drawString(&hal_p->g_sContext, (int8_t*)scoreStr, -1, 30, 5, true);

    Player_movementLogic(hal_p, game);
    Game_screenGraphics(hal_p, game);

    if (game->lives == 0){
        game->loadGameOverScreen = true;
        UpdateHighScores(game);
    }
}

void UpdateHighScores(Gamesettings *game) {
    int i;
    int j;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        if (game->currentScore > game->highScores[i]) {
            // Shift lower scores down by one position
            for ( j = MAX_HIGH_SCORES - 1; j > i; j--) {
                game->highScores[j] = game->highScores[j - 1];
            }
            // Insert the current score at the correct position
            game->highScores[i] = game->currentScore;
            break; // Exit the loop after updating the high score list
        }
    }
}

void Game_overScreen(HAL* hal_p, Gamesettings* game){
    char scoreStr[12];
    sprintf(scoreStr, "%d", game->currentScore);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "GAME OVER", -1, 30, 40, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Score:", -1, 30, 55, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) scoreStr, -1, 70, 55, true);

    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "(Press JSB to return)", -1, 0, 112, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "(to the main menu)", -1, 5, 120, true);

    if (Button_isTapped(&hal_p->boosterpackJS)){
        game->lives = 3;
        game->currentScore = 0;
        game->loadGameScreen = false;
        game->loadGameOverScreen = false;
        game->loadTitleScreen = true;
    }


}

void Game_screenGraphics(HAL* hal_p, Gamesettings *game){
   static int i;
    Graphics_Rectangle bar1 = {0, 25, 128, 25};
    Graphics_Rectangle bar2 = {0, 50, 128, 50};
    Graphics_Rectangle bar3 = {0, 75, 128, 75};
    Graphics_Rectangle bar4 = {0, 100, 128, 100};
    static int numLives = 3;


    static Graphics_Rectangle lives = {64, 110, 68, 114};
    Graphics_Rectangle livesArea = {64, 110, 90, 114};


    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Lives: ", -1, 20, 110, true);


    Graphics_drawRectangle(&hal_p->g_sContext, &bar1);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar2);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar3);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar4);

    SpawnObstacles(hal_p, game);
    UpdateAndDrawObstacles(hal_p, game);

    CheckAndHandleCollisions(hal_p, game);

    for (i=0; i < game->lives; i++){
        Graphics_drawRectangle(&hal_p->g_sContext, &lives);
        lives.xMin += 10;
        lives.xMax += 10;
    }
    lives.xMin = 64;
    lives.xMax = 68;
    if (numLives > game->lives){
        ClearLivesDisplay(hal_p, &livesArea);
        numLives--;
        if (numLives == 0){
            numLives = 3;
        }
    }
}

void ClearLivesDisplay(HAL* hal_p, const Graphics_Rectangle* area) {
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&hal_p->g_sContext, area);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}

void Player_movementLogic(HAL* hal_p, Gamesettings *game) {
    if (hal_p->joystick.isPressedToRight && game->playerPos.xMax < RIGHT_BOUND) {
        game->playerPos.xMin++;
        game->playerPos.xMax++;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    if (hal_p->joystick.isPressedToLeft && game->playerPos.xMin > LEFT_BOUND) {
        game->playerPos.xMin--;
        game->playerPos.xMax--;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    if (hal_p->joystick.isTappedToTop && game->playerPos.yMin > TOP_BOUND) {
        game->playerPos.yMin -= 25;
        game->playerPos.yMax -= 25;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    if (hal_p->joystick.isTappedToBottom && game->playerPos.yMin < BOTTOM_BOUND) {
        game->playerPos.yMin += 25;
        game->playerPos.yMax += 25;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    Player_jumpLogic(hal_p, game);
}

void Player_jumpLogic(HAL* hal_p, Gamesettings *game) {
    static PlayerState currentState = GROUND;
    static int ascentHeight = 0;

    switch (currentState) {
        case GROUND:
            // Check if button 1 is pressed to start the jump
            if (Button_isPressed(&hal_p->boosterpackS1)) {
                currentState = ASCENDING;
            }
            break;

        case ASCENDING:
            if (ascentHeight < 20) {
                game->playerPos.yMin--;
                game->playerPos.yMax--;
                ascentHeight++;
            } else {
                currentState = PEAK;
            }
            break;

        case PEAK:
            currentState = DESCENDING;
            break;

        case DESCENDING:
            if (ascentHeight > 0) {
                game->playerPos.yMin++;
                game->playerPos.yMax++;
                ascentHeight--;
            } else {
                Erase_player(hal_p, &game->lastPlayerPos);
                Draw_player(hal_p, &game->playerPos);
                game->lastPlayerPos = game->playerPos;
                currentState = GROUND;
            }
            break;
    }

    // Always erase and draw player after position update, except when on ground
    if (currentState != GROUND) {
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
}

void Draw_player(HAL *hal_p, Graphics_Rectangle *playerPos) {
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_LIME_GREEN);
    Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
    Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}
//
void Erase_player(HAL *hal_p, Graphics_Rectangle *playerPos) {
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
    Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}






void AddObstacle(Gamesettings* game, Graphics_Rectangle* rect) {
    int i;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (!game->obstacles[i].isActive) {
            game->obstacles[i].rect = *rect;
            game->obstacles[i].isActive = true;
            game->obstacles[i].hasCollided = false;
            break;
        }
    }
}

void UpdateAndDrawObstacles(HAL* hal_p, Gamesettings* game) {
    int i;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].isActive) {

            Graphics_Rectangle oldRect = game->obstacles[i].rect;

            game->obstacles[i].rect.xMin--;
            game->obstacles[i].rect.xMax--;


            Graphics_Rectangle eraseRect = {
                .xMin = oldRect.xMax - 1,
                .xMax = oldRect.xMax,
                .yMin = oldRect.yMin,
                .yMax = oldRect.yMax,
            };

            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_drawRectangle(&hal_p->g_sContext, &eraseRect);

            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawRectangle(&hal_p->g_sContext, &game->obstacles[i].rect);

            if (game->obstacles[i].rect.xMax < 0) {
                game->obstacles[i].isActive = false;
                game->currentScore +=500;
            }
        }
    }
}

void SpawnObstacles(HAL* hal_p, Gamesettings* game) {
    static Graphics_Rectangle lowObstacleR1 = {128, 89, 134, 99};
    static Graphics_Rectangle lowObstacleR2 = {128, 65, 134, 75};
    static Graphics_Rectangle lowObstacleR3 = {128, 40, 134, 50};

    static Graphics_Rectangle highObstacleR1 = {128, 76, 134, 85}; // Bottom row
    static Graphics_Rectangle highObstacleR2 = {128, 51, 134, 65};   // Middle row
    static Graphics_Rectangle highObstacleR3 = {128, 26, 134, 35};   // Top row

    static Graphics_Rectangle fullObstacleR1 = {128, 76, 134, 99}; // Bottom row
    static Graphics_Rectangle fullObstacleR2 = {128, 51, 134, 74};  // Middle row
    static Graphics_Rectangle fullObstacleR3 = {128, 26, 134, 49};  // Top row

    static bool firstLoad = true;
    if (firstLoad) {
        game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN);
        srand(time(NULL)); // Seed the random number generator
        firstLoad = false;
        SWTimer_start(&game->timer);
    }

    if (SWTimer_expired(&game->timer)) {
        int randIndex = rand() % 9; // Now randomizing among 9 options
        Graphics_Rectangle newRect;

        // Determine the obstacle based on randIndex
        switch (randIndex) {
            case 0: newRect = lowObstacleR1; break;
            case 1: newRect = lowObstacleR2; break;
            case 2: newRect = lowObstacleR3; break;
            case 3: newRect = highObstacleR1; break;
            case 4: newRect = highObstacleR2; break;
            case 5: newRect = highObstacleR3; break;
            case 6: newRect = fullObstacleR1; break;
            case 7: newRect = fullObstacleR2; break;
            case 8: newRect = fullObstacleR3; break;
        }

        AddObstacle(game, &newRect);

        if(game->currentScore > 10000 && game->currentScore < 25000){
            game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN_FAST);
            SWTimer_start(&game->timer); // Reset the timer
        }
        else if (game->currentScore >= 25000){
            game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN_FASTEST);
            SWTimer_start(&game->timer); // Reset the timer
        }
        else {
            SWTimer_start(&game->timer); // Reset the timer

        }
    }
}

void CheckAndHandleCollisions(HAL* hal_p, Gamesettings* game) {
    int i=0;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].isActive && !game->obstacles[i].hasCollided) {
            if (CheckCollision(&game->playerPos, &game->obstacles[i].rect)) {
                game->obstacles[i].hasCollided = true; // Mark as collided
                game->lives--;
                Graphics_drawImage(&hal_p->g_sContext, &explosion8BPP_UNCOMP, game->playerPos.xMin, game->playerPos.yMin - 16);
                break;
            }
        }
    }
}

bool CheckCollision(Graphics_Rectangle* playerRect, Graphics_Rectangle* obstacleRect) {
    return (playerRect->xMin < obstacleRect->xMax &&
            playerRect->xMax > obstacleRect->xMin &&
            playerRect->yMin < obstacleRect->yMax &&
            playerRect->yMax > obstacleRect->yMin);
}















