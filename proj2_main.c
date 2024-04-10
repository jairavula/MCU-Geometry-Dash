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

// Initialize some constants
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
 // image data structure
extern const Graphics_Image explosion8BPP_UNCOMP;
extern const Graphics_Image titleScreen8BPP_UNCOMP;

// declare functions
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
void Player_jumpLogic(HAL* hal_p, Gamesettings *game, PlayerState *currentState);
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
    Application app; // creates app object, reused from Proj1
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
    Screen_manager(hal_p, app_p); // Main entry point of application


}

void Screen_manager(HAL *hal_p, Application *app_p)
{ // Initializes the game settings for a new game
   static Gamesettings game = {splashScreen, false, false, false, false, false, {50, 45, 55, 50},{50, 45, 55, 50}, 3, 0,{0} };
 //Randomization logic from joystick inputs used, consists of taking readings of joystick, storing in randADC, and performing
  //bitwise operations for further randomization
   int joystickInput = ((hal_p->joystick.x << 4) | (hal_p->joystick.y)) & 0xFF;
   game.randADC ^= joystickInput; // XOR with the joystick input to introduce variability
   game.randADC = (game.randADC << 5) | (game.randADC >> (32 - 5)); // Rotate left by 5 bits (for a 32-bit number)
   game.randADC ^= joystickInput; // XOR again to mix in more of the joystick input

    switch (game.screenState) // FSM logic for each screen setting
    {
    // entry point of app
    case splashScreen:
        Splash_screen(hal_p, &game);
        if (SWTimer_expired(&app_p->timer)){
            game.screenState = mainMenuScreen;
            game.loadTitleScreen = true;
            Graphics_clearDisplay(&hal_p->g_sContext);
        }
        break;
    case mainMenuScreen: // main menu, leads to other screens based on button clicks defined in title_screen()
        Title_screen(hal_p, &game);
        if (game.loadInstructionsScreen == true){
            game.screenState = instructionsScreen;
            Graphics_clearDisplay(&hal_p->g_sContext); // clear graphics before screen change
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
    case gameScreen:  // State transition logic is defined within the screen's function
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
// only graphics on this page
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Spring 2024 Project", -1, 5, 5, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "ECE Surfers", -1, 5, 13, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Jai Ravula", -1, 5, 21, true);
}

void Title_screen(HAL *hal_p, Gamesettings *game){
// holds current position of cursor and debounces joystick
    static int cursorPos = TITLE_SCREEN_PLAY_CURSOR_POS;
    static int joystickTimer;

    Graphics_Rectangle topBar = {0,0,128,30}; // makes space for text

    if (game->loadTitleScreen){ // Load graphics only once
        Graphics_drawImage(&hal_p->g_sContext,  &titleScreen8BPP_UNCOMP, 0, 0);
        Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
           Graphics_fillRectangle(&hal_p->g_sContext, &topBar);
           Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Play ECE Surfers", -1, 5, 5, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Instructions", -1, 5, 13, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "View High Scores", -1, 5, 21, true);
         Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, cursorPos , true);
         game->loadTitleScreen = false;
    }

// Logic to handle state changes of the cursor based on where the cursor is currently and if the joystick debounce time has passed
    if (hal_p->joystick.isTappedToBottom && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) " ", -1, 115, 5, true); // replace cursor with empty space
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "x", -1, 115, 13, true); // draw new cursor in new position
        cursorPos += TITLE_SCREEN_CURSOR_OFFSET; // adjust cursor position
        joystickTimer = 0; // restart the debounce time
    } // All other statements here are similar
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

    if(Button_isTapped(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_PLAY_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadGameScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isTapped(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_INSTRUCTIONS_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadInstructionsScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    if(Button_isTapped(&hal_p->boosterpackJS) && cursorPos == TITLE_SCREEN_SCORES_CURSOR_POS && joystickTimer > JOYSTICK_COOLDOWN){
        game->loadHighScoresScreen = true;
        game->loadTitleScreen = false;
        joystickTimer = 0;
    }
    joystickTimer++;
}



void Instructions_screen(HAL* hal_p, Gamesettings* game){
    static int buttonTimer = 0; // debounce variable

    if (game->loadInstructionsScreen){ // Load graphics only once
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "HOW TO PLAY", -1, 5, 5, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Avoid all the ", -1, 5, 13, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "obstacles, move left ", -1, 5, 21, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "or right, up a lane", -1, 5, 29, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "or down a lane, or", -1, 5, 37, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "jump over obstacles.", -1, 5, 45, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Survive for as long", -1, 5, 54, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "as possible,", -1, 5, 62, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "and get the highest", -1, 5, 70, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "score! Move Joystick ", -1, 5, 78, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "to move", -1, 5, 86, true);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Press BB1 to jump", -1, 5, 94, true);
        game->loadInstructionsScreen = false;
    }



// go back to title screen if button is tapped and debounce time has passed
    if(Button_isTapped(&hal_p->boosterpackJS) && buttonTimer > JOYSTICK_COOLDOWN){
        game->loadTitleScreen = true;
        buttonTimer = 0;
    }
    buttonTimer++; // debounce logic
}

void Highscores_screen(HAL* hal_p, Gamesettings* game) {
    static int buttonTimer = 0; // debounce logic
    int i; // used for iteration
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "HIGH SCORES", -1, 5, 5, true);

    char scoreStr[20]; // Buffer to hold the score string
    int yPosition = 20; // Starting y position for the first high score

    // Loop through the highScores array and print each score
    for ( i = 0; i < MAX_HIGH_SCORES; i++) {
        // Formats the high scores like this for example: "1: 100"
        sprintf(scoreStr, "%d: %d", i + 1, game->highScores[i]);
        Graphics_drawString(&hal_p->g_sContext, (int8_t*)scoreStr, -1, 5, yPosition, true); // print score
        yPosition += 10; // Move down for the next score; adjust spacing as needed
    }

    // Button logic to return to the main menu, similar to instructions screen
    if (Button_isTapped(&hal_p->boosterpackJS) && buttonTimer > JOYSTICK_COOLDOWN) {
        game->loadHighScoresScreen = false;
        game->loadTitleScreen = true;
        buttonTimer = 0;
    }
    buttonTimer++;
}

void Game_screen(HAL* hal_p, Gamesettings* game){

    char scoreStr[20]; // Buffer to hold the score string
    if (game->loadGameScreen){ // start the score timer
        game->scoreTimer = SWTimer_construct(ADD_SCORE_TIME);
        game->loadGameScreen = false;
    }

    if (SWTimer_expired(&game->scoreTimer)) {
            SWTimer_start(&game->scoreTimer); // Reset the timer
        }
// Print score at top of screen
    sprintf(scoreStr, "Score: %d", game->currentScore);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*)scoreStr, -1, 30, 5, true);
// Logic for player movement and graphics of the game
    Player_movementLogic(hal_p, game);
    Game_screenGraphics(hal_p, game);
// end game if out of lives
    if (game->lives == 0){
        game->loadGameOverScreen = true;
        UpdateHighScores(game);
    }
}

// Updates the list of high scores
void UpdateHighScores(Gamesettings *game) {
    int i;
    int j;
    for (i = 0; i < MAX_HIGH_SCORES; i++) {
        if (game->currentScore > game->highScores[i]) { // if score is higher than one in the list
            // Shift lower scores down by one position
            for ( j = MAX_HIGH_SCORES - 1; j > i; j--) {
                game->highScores[j] = game->highScores[j - 1];
            }
            // Then insert the current score at the correct position
            game->highScores[i] = game->currentScore;
            break; // Exit loop
        }
    }
}

void Game_overScreen(HAL* hal_p, Gamesettings* game){
    char scoreStr[12];
    sprintf(scoreStr, "%d", game->currentScore); // Shows the score of the game
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "GAME OVER", -1, 30, 40, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Score:", -1, 30, 55, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) scoreStr, -1, 70, 55, true);
// Various graphics
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "(Press JSB to return)", -1, 0, 112, true);
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "(to the main menu)", -1, 5, 120, true);
 // Returns to title screen and resets game logic variables
    if (Button_isTapped(&hal_p->boosterpackJS)){
        game->lives = 3;
        game->currentScore = 0;
        game->loadGameScreen = false;
        game->loadGameOverScreen = false;
        game->loadTitleScreen = true;
    }


}

void Game_screenGraphics(HAL* hal_p, Gamesettings *game){
    // draws the lanes of the game
   static int i;
    Graphics_Rectangle bar1 = {0, 25, 128, 25};
    Graphics_Rectangle bar2 = {0, 50, 128, 50};
    Graphics_Rectangle bar3 = {0, 75, 128, 75};
    Graphics_Rectangle bar4 = {0, 100, 128, 100};
    static int numLives = 3;

// Initialies the area and boxes for lives counter
    static Graphics_Rectangle lives = {64, 110, 68, 114};
    Graphics_Rectangle livesArea = {64, 110, 90, 114};

// draws the lives stuff
    Graphics_drawString(&hal_p->g_sContext, (int8_t*) "Lives: ", -1, 20, 110, true);


    Graphics_drawRectangle(&hal_p->g_sContext, &bar1);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar2);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar3);
    Graphics_drawRectangle(&hal_p->g_sContext, &bar4);
// spawns the obstacles and moves them
    SpawnObstacles(hal_p, game);
    UpdateAndDrawObstacles(hal_p, game);
// checks for a collision right after moving an obstacle
    CheckAndHandleCollisions(hal_p, game);
// draws the lives after checking for collision
    for (i=0; i < game->lives; i++){
        Graphics_drawRectangle(&hal_p->g_sContext, &lives);
        lives.xMin += 10;
        lives.xMax += 10;
    }
    lives.xMin = 64;
    lives.xMax = 68;
    if (numLives > game->lives){ // Delete a life box if one is lost
        ClearLivesDisplay(hal_p, &livesArea); // reset lives display
        numLives--;
        if (numLives == 0){
            numLives = 3; // set back to 3 if game ends
        }
    }
}

void ClearLivesDisplay(HAL* hal_p, const Graphics_Rectangle* area) {
    // draws black box over lives for it to be redrawn
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_fillRectangle(&hal_p->g_sContext, area);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}

void Player_movementLogic(HAL* hal_p, Gamesettings *game) {
    static PlayerState currentState = GROUND; // state variable for where the player is

// Moves player to right if joystick is tilted and within the game area
    if (hal_p->joystick.isTiltToRight && game->playerPos.xMax < RIGHT_BOUND) {
           game->playerPos.xMin++; // decrease the x pos of the player
           game->playerPos.xMax++;
           Erase_player(hal_p, &game->lastPlayerPos); // erase player and redraw it
           Draw_player(hal_p, &game->playerPos);
           game->lastPlayerPos = game->playerPos; // store the player pos for future erasing
       }
    // Similar to above but for left
    if (hal_p->joystick.isTiltToLeft && game->playerPos.xMin > LEFT_BOUND) {
            game->playerPos.xMin--;
            game->playerPos.xMax--;
            Erase_player(hal_p, &game->lastPlayerPos);
            Draw_player(hal_p, &game->playerPos);
            game->lastPlayerPos = game->playerPos;
        }
    // Similar to above, but if fully pressed, move the player faster
    if (hal_p->joystick.isPressedToRight && game->playerPos.xMax < RIGHT_BOUND) {
        game->playerPos.xMin+=2;
        game->playerPos.xMax+=2;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    // similar to above
    if (hal_p->joystick.isPressedToLeft && game->playerPos.xMin > LEFT_BOUND) {
        game->playerPos.xMin-=2;
        game->playerPos.xMax-=2;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    // Change lanes to the one above if the player is within the bounds and is not in a jump state
    if (hal_p->joystick.isTappedToTop && game->playerPos.yMin > TOP_BOUND && currentState == GROUND) {
        game->playerPos.yMin -= 25;
        game->playerPos.yMax -= 25;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    // similar to above, but for going down, still ensure player is not in a jump state
    if (hal_p->joystick.isTappedToBottom && game->playerPos.yMin < BOTTOM_BOUND && currentState == GROUND) {
        game->playerPos.yMin += 25;
        game->playerPos.yMax += 25;
        Erase_player(hal_p, &game->lastPlayerPos);
        Draw_player(hal_p, &game->playerPos);
        game->lastPlayerPos = game->playerPos;
    }
    // calls logic for jumping
    Player_jumpLogic(hal_p, game, &currentState);
}

void Player_jumpLogic(HAL* hal_p, Gamesettings *game, PlayerState * currentState) {
    static int ascentHeight = 0;

    switch (*currentState) {
        case GROUND:
            // Check if button 1 is pressed to start the jump
            if (Button_isPressed(&hal_p->boosterpackS1)) {
                *currentState = ASCENDING;
            }
            break;
            // keep increasing y position of player for 20 units
        case ASCENDING:
            if (ascentHeight < 20) {
                game->playerPos.yMin--;
                game->playerPos.yMax--;
                ascentHeight++;
            } else {
                *currentState = PEAK; // player is at peak if 20 units have been ascended
            }
            break;

        case PEAK: // change state to descending
            *currentState = DESCENDING;
            break;

        case DESCENDING: // same logic as going up, but for going down
            if (ascentHeight > 0) {
                game->playerPos.yMin++;
                game->playerPos.yMax++;
                ascentHeight--;
            } else { // reset player to ground
                Erase_player(hal_p, &game->lastPlayerPos);
                Draw_player(hal_p, &game->playerPos);
                game->lastPlayerPos = game->playerPos;
                *currentState = GROUND;
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

void Draw_player(HAL *hal_p, Graphics_Rectangle *playerPos) { // draws the player square
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_LIME_GREEN);
    Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
    Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}
//
void Erase_player(HAL *hal_p, Graphics_Rectangle *playerPos) { // draws black over the player square position
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
    Graphics_drawRectangle(&hal_p->g_sContext, playerPos);
    Graphics_fillRectangle(&hal_p->g_sContext, playerPos);
    Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
}






void AddObstacle(Gamesettings* game, Graphics_Rectangle* rect) {
    int i;
    // If the specified array position is empty, add an obstacle of specified dimensions to it, make it active, and not collided with yet
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
        if (game->obstacles[i].isActive) { // if the obstable is actively in the game

            Graphics_Rectangle oldRect = game->obstacles[i].rect; // store last pos of obstacle

            game->obstacles[i].rect.xMin--; // shift obstacle left
            game->obstacles[i].rect.xMax--;


            Graphics_Rectangle eraseRect = { // create a obstacle used to erase the last pos
                .xMin = oldRect.xMax - 1,
                .xMax = oldRect.xMax,
                .yMin = oldRect.yMin,
                .yMax = oldRect.yMax,
            };

            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_drawRectangle(&hal_p->g_sContext, &eraseRect); // erase last pos

            Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawRectangle(&hal_p->g_sContext, &game->obstacles[i].rect);// draw new obstacle

            if (game->obstacles[i].rect.xMax < 0) {
                game->obstacles[i].isActive = false; // if an obstacle reaches the end, set it inactive and add 500 to score
                game->currentScore +=500;
            }
        }
    }
}

void SpawnObstacles(HAL* hal_p, Gamesettings* game) {
    // 9 different types of obstacles in the game, each with their own position/ dimensions
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
    if (firstLoad) { // construct start the obstacle timer on first load
        game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN);
        firstLoad = false;
        SWTimer_start(&game->timer);
    }

    if (SWTimer_expired(&game->timer)) {
        int randIndex = game->randADC % 9; // choose a random obstacle based on the randomized value in the randADC variable
        Graphics_Rectangle newRect; // holder for obstacle to be used

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

        AddObstacle(game, &newRect); // add the obstacle to the game

        if(game->currentScore < 10000){ // lowest spawn setting
           game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN);
           SWTimer_start(&game->timer); // Reset the timer
        }

        else if(game->currentScore > 10000 && game->currentScore < 25000){ // faster spawn setting
            game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN_FAST);
            SWTimer_start(&game->timer); // Reset the timer
        }
        else if (game->currentScore >= 25000){ // fastest spawn setting
            game->timer = SWTimer_construct(SPAWN_OBSTACLE_COOLDOWN_FASTEST);
            SWTimer_start(&game->timer); // Reset the timer
        }
        else { // set it back to the slowest spawn setting
            SWTimer_start(&game->timer); // Reset the timer

        }
    }
}

void CheckAndHandleCollisions(HAL* hal_p, Gamesettings* game) {
    int i=0;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (game->obstacles[i].isActive && !game->obstacles[i].hasCollided) { // if an obstacle is active and hasnt had a collision yet
            if (CheckCollision(&game->playerPos, &game->obstacles[i].rect)) { // check for a collision
                    // used to erase animation
                Graphics_Rectangle eraseAnimation = {game->playerPos.xMax, game->playerPos.yMin+14, game->playerPos.xMax+24, game->playerPos.yMin-14};
                game->obstacles[i].hasCollided = true; // Mark as collided
                game->lives--; // decrease lives
                for (i = 0; i< 50; i++){ // draw explosion
                    Graphics_drawImage(&hal_p->g_sContext, &explosion8BPP_UNCOMP, game->playerPos.xMin, game->playerPos.yMin - 16);

                }
                Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_BLACK);
                Graphics_fillRectangle(&hal_p->g_sContext, &eraseAnimation); // erase the explosion and continue game
                Graphics_setForegroundColor(&hal_p->g_sContext, GRAPHICS_COLOR_WHITE);

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
    //  check if there has been a collision by checking intersection between
    // the player and the object. Sees is the player is "overlapping" the object
    // in any position.
}















