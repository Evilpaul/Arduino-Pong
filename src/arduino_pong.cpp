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
const int resolution[2] = { 128, 64 };

/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct
{
	int score = 0;
	int position = 0;
} playerData;

/******************************************************************************/
/*------------------------------Global variables------------------------------*/
/******************************************************************************/
Adafruit_SSD1306 display(OLED_RESET);

char ballDirectionHori = 'R', ballDirectionVerti = 'S';

playerData player;
playerData ai;

/******************************************************************************/
/*-------------------------Function Prototypes--------------------------------*/
/******************************************************************************/
void moveAi();
void drawScore();
void drawNet();
void drawPixel(int posX, int posY, int dimensions);
void drawPlayerPaddle(int row);
void drawAiPaddle(int row);
void drawBall(int x, int y);

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void setup() {
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
	display.clearDisplay();
	display.display();
}

void loop() {
	display.clearDisplay();

	if (ai.score > 9 || player.score > 9) {
		// somebody has won
		display.clearDisplay();
		display.setTextSize(4);
		display.setTextColor(WHITE);
		display.setCursor(0, 0);
		// figure out who
		if (ai.score > player.score) {
			display.println("YOU  LOSE!");
		} else if (player.score > ai.score) {
			display.println("YOU  WIN!");
		}
	}
	else {
		if (ballDirectionVerti == 'U') {
			// move ball up diagonally
			ball[IDX_Y] = ball[IDX_Y] - SPEED;
		}

		if (ballDirectionVerti == 'D') {
			// move ball down diagonally
			ball[IDX_Y] = ball[IDX_Y] + SPEED;
		}

		if (ball[IDX_Y] <= 0) {
			// bounce the ball off the top
			ballDirectionVerti = 'D';
		}
		if (ball[IDX_Y] >= resolution[IDX_Y]) {
			// bounce the ball off the bottom
			ballDirectionVerti = 'U';
		}

		if (ballDirectionHori == 'R') {
			ball[IDX_X] = ball[IDX_X] + SPEED; // move ball
			if (ball[IDX_X] >= (resolution[IDX_X] - 6)) {
				// ball is at the AI edge of the screen
				if ((ai.position + 12) >= ball[IDX_Y] && (ai.position - 12) <= ball[IDX_Y]) {
					// ball hits AI paddle
					if (ball[IDX_Y] > (ai.position + 4)) {
						// deflect ball down
						ballDirectionVerti = 'D';
					} else if (ball[IDX_Y] < (ai.position - 4)) {
						// deflect ball up
						ballDirectionVerti = 'U';
					} else {
						// deflect ball straight
						ballDirectionVerti = 'S';
					}
					// change ball direction
					ballDirectionHori = 'L';
				} else {
					// GOAL!
					ball[IDX_X] = 6; // move ball to other side of screen
					ballDirectionVerti = 'S'; // reset ball to straight travel
					ball[IDX_Y] = resolution[IDX_Y] / 2; // move ball to middle of screen
					++player.score; // increase player score
				}
			}
		}

		if (ballDirectionHori == 'L') {
			ball[IDX_X] = ball[IDX_X] - SPEED; // move ball
			if (ball[IDX_X] <= 6) {
				// ball is at the player edge of the screen
				if ((player.position + 12) >= ball[IDX_Y]
						&& (player.position - 12) <= ball[IDX_Y]) {
					// ball hits player paddle
					if (ball[IDX_Y] > (player.position + 4)) {
						// deflect ball down
						ballDirectionVerti = 'D';
					} else if (ball[IDX_Y] < (player.position - 4)) {
						// deflect ball up
						ballDirectionVerti = 'U';
					} else {
						// deflect ball straight
						ballDirectionVerti = 'S';
					}
					// change ball direction
					ballDirectionHori = 'R';
				} else {
					ball[IDX_X] = resolution[IDX_X] - 6; // move ball to other side of screen
					ballDirectionVerti = 'S'; // reset ball to straight travel
					ball[IDX_Y] = resolution[IDX_Y] / 2; // move ball to middle of screen
					++ai.score; // increase AI score
				}
			}
		}
		drawBall(ball[IDX_X], ball[IDX_Y]);
		player.position = analogRead(A2); // read player potentiometer
		player.position = map(player.position, 0, 1023, 8, 54); // convert value from 0 - 1023 to 8 - 54
		drawPlayerPaddle(player.position);
		moveAi();
		drawAiPaddle(ai.position);
		drawNet();
		drawScore();
	}

	display.display();
}

void moveAi() {
	// move the AI paddle
	if (ball[IDX_Y] > ai.position) {
		++ai.position;
	} else if (ball[IDX_Y] < ai.position) {
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
	for (int i = 0; i < (resolution[IDX_Y] / WALL_WIDTH); ++i) {
		drawPixel(((resolution[IDX_X] / 2) - 1),
				i * (WALL_WIDTH) + (WALL_WIDTH * i), WALL_WIDTH);
	}
}

void drawPixel(int posX, int posY, int dimensions) {
	// draw group of pixels
	for (int x = 0; x < dimensions; ++x) {
		for (int y = 0; y < dimensions; ++y) {
			display.drawPixel((posX + x), (posY + y), WHITE);
		}
	}
}

void drawPlayerPaddle(int row) {
	drawPixel(0, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
	drawPixel(0, row - PADDLE_WIDTH, PADDLE_WIDTH);
	drawPixel(0, row, PADDLE_WIDTH);
	drawPixel(0, row + PADDLE_WIDTH, PADDLE_WIDTH);
	drawPixel(0, row + (PADDLE_WIDTH + 2), PADDLE_WIDTH);
}

void drawAiPaddle(int row) {
	int column = resolution[IDX_X] - PADDLE_WIDTH;
	drawPixel(column, row - (PADDLE_WIDTH * 2), PADDLE_WIDTH);
	drawPixel(column, row - PADDLE_WIDTH, PADDLE_WIDTH);
	drawPixel(column, row, PADDLE_WIDTH);
	drawPixel(column, row + PADDLE_WIDTH, PADDLE_WIDTH);
	drawPixel(column, row + (PADDLE_WIDTH * 2), PADDLE_WIDTH);
}

void drawBall(int x, int y) {
	display.drawCircle(x, y, BALL_SIZE, WHITE);
}
