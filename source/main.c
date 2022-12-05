#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "hardware.h"
#include "structures.h"

#define WIDTH 10

/*
 * Initialize the snake with initialSize allocating memory and declaring the direction, used, size, and score
 */
void initSnake(Snake *a, size_t initialSize) {
    a->parts = malloc(initialSize * sizeof(Position));
    a->used = 0;
    a->size = initialSize;
    a->direction[0] = 1;
    a->direction[1] = 0;
    a->score = 0;
    a->indexOfHead = 2;
}

/*
 * Insert an element into the end of the array, if it runs out of space (used == space) then
 * double the size and reallocate the memory
 */
void insertArray(Snake *a, Position element) {
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size
    if (a->used == a->size) {
        a->size *= 2;
        a->parts = realloc(a->parts, a->size * sizeof(Position));
    }
    a->parts[a->used++] = element;
}

/*
 * Free the memory that the array is taking part and set it pointing to NULL, reset used and size
 */
void freeArray(Snake *a) {
    free(a->parts);
    a->parts = NULL;
    a->used = a->size = 0;
}


/*
 * Generate a random position and make sure it is within the width and height of screen and not inside snake body
 */
Position generateFood(Snake *snake)
{
    // set width and height
    int width = 41;
    int height = 20;

    Position temp;
    int condition = 0;
    while (condition == 0)
    {
        // generate random numbers between width and height
        temp.x = rand() % width;
        temp.y = rand() % height;



        // check to see if it matches any of the positions in the snake body
        int i = 0;
        for (i = 0; i < snake->used; i++)
        {
            if(snake->parts[i].x == temp.x && snake->parts[i].y == temp.y)
            {
                continue;
            }
        }
        condition = 1;
    }
    return temp;
}

/*
 * Using the current head and the current direction, add them together and generate a new head
 */
Position generateNewHead(Snake *snake)
{
    // Create a temp position from the old head, generate new x and y values by adding them to direction, then return
    // as new head
    Position temp = snake->parts[snake->indexOfHead];
    temp.x = temp.x + snake->direction[0];
    temp.y = temp.y + snake->direction[1];

    return temp;
}

/*
 * Replace the oldest tail with the new head of the snake, essentially popping off tail and inserting head
 */
void noFoodUpdate(Snake *snake)
{
    // generate a newHead
    Position temp = generateNewHead(snake);

    // check if indexOfHead + 1 > used, if it is then start back at parts[0], get rid of tail and replace with new head
    if(snake->indexOfHead + 1 == snake->used)
    {
        snake->parts[0] = temp;
        snake->indexOfHead = 0;
    }
    // otherwise set the new head as the oldest tail (one past indexOfHead)
    else
    {
        snake->parts[snake->indexOfHead + 1] = temp;
        snake->indexOfHead++;
    }
}

/*
 * If the snake head hit food then we need to create a new array pointer with one more memory allocated to it,
 * copy the snake body into the temporary array, free the memory from the old body, set the snake body pointing to the
 * new body, add the new head, increment score, and update array variables
 */
void foodUpdate(Snake *snake)
{
    // create a temporary pointer of positions with memory allocated of 1 more than current used
    Position *temp_parts = malloc((snake->used + 1) * sizeof(Position));

    // iterate through snake body, start at the oldest tail and work toward the head, when you reach head then add the
    // new head on top of it
    int i = 0;
    int j;
    for(j = snake->indexOfHead + 1; j < snake->used; j++)
    {
        temp_parts[i] = snake->parts[j];
        i++;
    }
    for(j = 0; j <= snake->indexOfHead ; j++)
    {
        temp_parts[i] = snake->parts[j];
        i++;
    }

    temp_parts[snake->used] = generateNewHead(snake);

    // set temp array as the new snake body array, increase used and size by 1
    // free the memory from the old array and set snake->parts pointing to the temp array, set the indexOfHead
    free(snake->parts);
    snake->parts = temp_parts;
    snake->used++;
    snake->size++;
    snake->indexOfHead = snake->used - 1;

    // increase score by 1
    snake->score++;
}

/*
 * Check if the snake head hits the walls or itself, if so return 1, if it hits food then increase length and return -1,
 * if it hits nothing then move in direction and return 0
 */
int checkForCollision(Snake *snake, Position food)
{
    // generate a temporary new head
    Position temp = generateNewHead(snake);

    // check if snake head hit itself or hit a wall
    int i;
    for (i = 0; i < snake->used; i++)
    {
        if(snake->parts[i].x == temp.x && snake->parts[i].y == temp.y)
        {
            return 1;
        }
        else if(temp.x > 41 || temp.x < 1)
        {
            return 1;
        }
        else if(temp.y > 20 || temp.y < 1)
        {
            return 1;
        }
    }

    // check if snake head hit the food use the foodUpdate function to increase length and generate a new food
    if(temp.x == food.x && temp.y == food.y)
    {
        foodUpdate(snake);
        return -1;
    }
    // otherwise update with noFoodUpdate keeping same length
    else
    {
        noFoodUpdate(snake);
        return 0;
    }

}


int main()
{
    initializeHardware();

    // initialize srand
    srand(time(NULL));

    int replay = 1;
    Snake snake;

    // open entire loop for replaying
    while(replay == 1)
    {
        // TODO: display welcome text
        lcdDisplayText("Welcome to Snake, press button 1 to turn left and button 2 to turn right, press any button to start");

        // TODO: set lcd display to 0
        ledDisplay(0);

        // create a loop that waits for user input to continue
        while(checkButton1() != 1 && checkButton2() != 1)
        {
            continue;
        }

        // TODO: initialize snake, setting score to 0, direction to (1,0), and a array of parts 3 long

        initSnake(&snake, 3);  // initially 3 elements

        // TODO: insert the two tails and the head, indexOfHead is equal to 2, the head of the snake
        // Insert the first tail
        Position temp;
        temp.x = 10;
        temp.y = 10;
        insertArray(&snake, temp);

        // Insert the second tail
        temp.x = 11;
        temp.y = 10;
        insertArray(&snake, temp);

        // Insert the starting head
        temp.x = 12;
        temp.y = 10;
        insertArray(&snake, temp);

        // TODO: create a piece of food somewhere on the screen
        Position food = generateFood(&snake);

        int isAlive = 1;

        while(isAlive == 1) {
            /*
             * TODO: check for collisions, if it hits a food increase length, if not pop tail and add new head, if it hits wall
             * or self then end game
            */
            int collision = checkForCollision(&snake, food);

            // TODO: if the collision is 1 then it hit the wall or itself, break the game loop and display losing message
            if (collision == 1)
            {
                lcdDisplayText("GAME OVER\nPress 1 to end or 2 to play again");

                while(checkButton1() != 1 && checkButton2() != 1)
                {
                    continue;
                }

                if(checkButton1() == 1)
                {
                    replay = 0;
                }
                break;
            }

            // TODO: if the collision is -1 then it hit a food, generate a new food, update lcd and LED then continue
            if (collision == -1)
            {
                food = generateFood(&snake);
                ledDisplay(snake.score);
                lcdDisplayUpdate(&snake, food);
            }

            // TODO: if the collision is 0 then it did not hit anything, update LCD display with new snake and continue
            if (collision == 0)
            {
                lcdDisplayUpdate(&snake, food);
            }

            // TODO: check if either button 1 or button 2 has been pressed, if 1 then turn left, if 2 then turn right
            if(checkButton1() == 1)
            {
                printf("Turn the snake direction left\n");

                // snake is going right, turn left to up
                if(snake.direction[0] == 1 && snake.direction[1] == 0)
                {
                    snake.direction[0] = 0;
                    snake.direction[1] = 1;
                }

                // snake is going up, turn left to left
                else if(snake.direction[0] == 0 && snake.direction[1] == 1)
                {
                    snake.direction[0] = -1;
                    snake.direction[1] = 0;
                }

                // snake is going left, turn left to down
                else if(snake.direction[0] == -1 && snake.direction[1] == 0)
                {
                    snake.direction[0] = 0;
                    snake.direction[1] = -1;
                }

                // snake is going down, turn left to right
                else if(snake.direction[0] == 0 && snake.direction[1] == -1)
                {
                    snake.direction[0] = 1;
                    snake.direction[1] = 0;
                }

            }
            else if(checkButton2() == 1)
            {
                printf("Turn the snake direction to right");
                // snake is going right, turn right to down
                if(snake.direction[0] == 1 && snake.direction[1] == 0)
                {
                    snake.direction[0] = 0;
                    snake.direction[1] = -1;
                }

                    // snake is going up, turn right to right
                else if(snake.direction[0] == 0 && snake.direction[1] == 1)
                {
                    snake.direction[0] = 1;
                    snake.direction[1] = 0;
                }

                    // snake is going left, turn right to up
                else if(snake.direction[0] == -1 && snake.direction[1] == 0)
                {
                    snake.direction[0] = 0;
                    snake.direction[1] = 1;
                }

                    // snake is going down, turn right to left
                else if(snake.direction[0] == 0 && snake.direction[1] == -1)
                {
                    snake.direction[0] = -1;
                    snake.direction[1] = 0;
                }
            }
        }

    }

    freeArray(&snake);

    cleanupHardware();

    return 0;
}
