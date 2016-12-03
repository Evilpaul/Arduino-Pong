/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define OLED_RESET 4

#define IDX_X 0
#define IDX_Y 1

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
const int PIXEL_SIZE = 8, WALL_WIDTH = 4, PADDLE_WIDTH = 4, BALL_SIZE = 4, SPEED = 3;
const int RESOLUTION[2] = { 128, 64 };
const int PLAYER_COLUMN = 0;
const int AI_COLUMN = RESOLUTION[IDX_X] - PADDLE_WIDTH;

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/
typedef enum
{
	HzDir_Left = -1,
	HzDir_Right = 1
} HzDir;

typedef enum
{
	VtDir_Up = -1,
	VtDir_Straight = 0,
	VtDir_Down = 1
} VtDir;

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct
{
	int score = 0;
	int position = 0;
} playerData;

typedef struct
{
	int position[2] = {20, (RESOLUTION[IDX_Y] / 2)};
	HzDir dirHori = HzDir_Right;
	VtDir dirVert = VtDir_Straight;
} ballData;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
Adafruit_SSD1306 display(OLED_RESET);

playerData player;
playerData ai;
ballData ball;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
void moveAi();
void drawScore();
void drawNet();
void drawBlock(int posX, int posY, int height, int width);
void drawPaddle(int column, int row);
void drawBall(int x, int y);

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void setup() {
	display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	display.clearDisplay();
	display.display();
}

void loop() {
	display.clearDisplay();

	if (ai.score > 9 || player.score > 9) {
		// somebody has won
		display.setTextSize(4);
		display.setTextColor(WHITE);
		display.setCursor(0, 0);

		// figure out who
		if (ai.score > player.score) {
			display.println("YOU  LOSE!");
		}
		else {
			display.println("YOU  WIN!");
		}
	}
	else {
		// move ball up diagonally
		ball.position[IDX_Y] = ball.position[IDX_Y] + (SPEED * ball.dirVert);
		if (ball.position[IDX_Y] <= 0) {
			// bounce the ball off the top
			ball.dirVert = VtDir_Down;
		}
		if (ball.position[IDX_Y] >= RESOLUTION[IDX_Y]) {
			// bounce the ball off the bottom
			ball.dirVert = VtDir_Up;
		}

		ball.position[IDX_X] = ball.position[IDX_X] + (SPEED * ball.dirHori); // move ball
		if (ball.dirHori == HzDir_Right) {
			if (ball.position[IDX_X] >= (RESOLUTION[IDX_X] - 6)) {
				// ball is at the AI edge of the screen
				if ((ai.position + 12) >= ball.position[IDX_Y] && (ai.position - 12) <= ball.position[IDX_Y]) {
					// ball hits AI paddle
					if (ball.position[IDX_Y] > (ai.position + 4)) {
						// deflect ball down
						ball.dirVert = VtDir_Down;
					} else if (ball.position[IDX_Y] < (ai.position - 4)) {
						// deflect ball up
						ball.dirVert = VtDir_Up;
					} else {
						// deflect ball straight
						ball.dirVert = VtDir_Straight;
					}
					// change ball direction
					ball.dirHori = HzDir_Left;
				} else {
					// GOAL!
					ball.position[IDX_X] = 6; // move ball to other side of screen
					ball.dirVert = VtDir_Straight; // reset ball to straight travel
					ball.position[IDX_Y] = RESOLUTION[IDX_Y] / 2; // move ball to middle of screen
					++player.score; // increase player score
				}
			}
		}

		if (ball.dirHori == HzDir_Left) {
			if (ball.position[IDX_X] <= 6) {
				// ball is at the player edge of the screen
				if ((player.position + 12) >= ball.position[IDX_Y]
						&& (player.position - 12) <= ball.position[IDX_Y]) {
					// ball hits player paddle
					if (ball.position[IDX_Y] > (player.position + 4)) {
						// deflect ball down
						ball.dirVert = VtDir_Down;
					} else if (ball.position[IDX_Y] < (player.position - 4)) {
						// deflect ball up
						ball.dirVert = VtDir_Up;
					} else {
						// deflect ball straight
						ball.dirVert = VtDir_Straight;
					}
					// change ball direction
					ball.dirHori = HzDir_Right;
				} else {
					ball.position[IDX_X] = RESOLUTION[IDX_X] - 6; // move ball to other side of screen
					ball.dirVert = VtDir_Straight; // reset ball to straight travel
					ball.position[IDX_Y] = RESOLUTION[IDX_Y] / 2; // move ball to middle of screen
					++ai.score; // increase AI score
				}
			}
		}
		drawBall(ball.position[IDX_X], ball.position[IDX_Y]);
		player.position = analogRead(A2); // read player potentiometer
		player.position = map(player.position, 0, 1023, 8, 54); // convert value from 0 - 1023 to 8 - 54
		drawPaddle(PLAYER_COLUMN, player.position);
		moveAi();
		drawPaddle(AI_COLUMN, ai.position);
		drawNet();
		drawScore();
	}

	display.display();
}

void moveAi() {
	// move the AI paddle
	if (ball.position[IDX_Y] > ai.position) {
		++ai.position;
	} else if (ball.position[IDX_Y] < ai.position) {
		--ai.position;
	}
}

void drawScore() {
	// draw AI and player scores
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(45, 0);
	display.println(player.score);

	display.setCursor(75, 0);
	display.println(ai.score);
}

void drawNet() {
	for (int i = 0; i < (RESOLUTION[IDX_Y] / WALL_WIDTH); ++i) {
		drawBlock(((RESOLUTION[IDX_X] / 2) - 1),
				i * (WALL_WIDTH) + (WALL_WIDTH * i), WALL_WIDTH, WALL_WIDTH);
	}
}

void drawBlock(int posX, int posY, int height, int width) {
	// draw group of pixels
	for (int i = posX; i < posX + width; i++)
	{
		display.drawFastVLine(i, posY, height, WHITE);
	}
}

void drawPaddle(int column, int row) {
	drawBlock(column, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH, PADDLE_WIDTH);
	drawBlock(column, row - PADDLE_WIDTH, PADDLE_WIDTH, PADDLE_WIDTH);
	drawBlock(column, row, PADDLE_WIDTH, PADDLE_WIDTH);
	drawBlock(column, row + PADDLE_WIDTH, PADDLE_WIDTH, PADDLE_WIDTH);
	drawBlock(column, row + (PADDLE_WIDTH * 2), PADDLE_WIDTH, PADDLE_WIDTH);
}

void drawBall(int x, int y) {
	display.drawCircle(x, y, BALL_SIZE, WHITE);
}
