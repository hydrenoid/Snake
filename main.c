#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
 * Structure that defines each body part of the snake with an x and y pixel value
 */
typedef struct {
    int x;
    int y;
} Position;

/*
 * Structure that defines the whole snake, from list of body parts to size and index of head
 */
typedef struct {
    Position *parts;
    size_t used;
    size_t size;
    size_t score;
    size_t indexOfHead;
    int direction[2];

} Snake;

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
 * Takes in a string and displays it to the center of the LED Display
 */
void ledDisplayText(char text[])
{
    printf("Showing this message on LED: %s\n", text);
}

/*
 * Read in an integer and use that integer to place correct numbers into each LCD display 1-1000's place
 */
void lcdDisplay(int num)
{
    // set the 1000s spot
    printf("Display %d to 1000's LCD\n", num / 1000);
    num = num % 1000;
    // set the 100's spot
    printf("Display %d to 100's LCD\n", num / 100);
    num = num % 100;
    // set the 10's spot
    printf("Display %d to 10's LCD\n", num / 10);
    num = num % 10;
    // set the 1's spot
    printf("Display %d to 1's LCD\n", num / 1);
}

/*
 * Generate a random position and make sure it is within the width and height of screen and not inside snake body
 */
Position generateFood(Snake *snake)
{
    // set width and height
    int width = 100;
    int height = 100;

    Position temp;
    int condition = 0;
    while (condition == 0)
    {
        // generate random numbers between width and height
        temp.x = rand() % width - (width / 2);
        temp.y = rand() % height - (height / 2);

        // check to see if it matches any of the positions in the snake body
        for (int i = 0; i < snake->used; i++)
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
    for(int j = snake->indexOfHead + 1; j < snake->used; j++)
    {
        temp_parts[i] = snake->parts[j];
        i++;
    }
    for(int j = 0; j <= snake->indexOfHead ; j++)
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
    for (int i = 0; i < snake->used; i++)
    {
        if(snake->parts[i].x == temp.x && snake->parts[i].y == temp.y)
        {
            return 1;
        }
        else if(temp.x > 50 || temp.x < -50)
        {
            return 1;
        }
        else if(temp.y > 50 || temp.y < -50)
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

/*
 * Loop through the snake body and food then place all parts on screen
 */
void ledDisplayUpdate(Snake *snake, Position food)
{
    // for now just print the x and y values of each body part and food
    for (int i = 0; i < snake->used; i++)
    {
        printf("(%d,%d)\n", snake->parts[i].x, snake->parts[i].y);
        printf("-----------------------\n");
    }
    printf("food(x,y): (%d,%d)\n", food.x, food.y);
    printf("*****************************\n");
}

/*
 * Check if button 1 has been pressed, if it has return 1, if not return 0
 */
int checkButton1()
{
    return 0;
}

/*
 * Check if button 2 has been pressed, if it has return 1, if not return 0
 */
int checkButton2()
{
    return 0;
}

int main()
{
    // initialize srand
    srand(time(NULL));

    // TODO: display welcome text
    ledDisplayText("Welcome to Snake, press button 1 to turn left and button 2 to turn right, press any button to start");

    // TODO: set lcd display to 0
    lcdDisplay(0);

    // TODO: initialize snake, setting score to 0, direction to (1,0), and a array of parts 3 long
    Snake snake;
    initSnake(&snake, 3);  // initially 3 elements

    // TODO: insert the two tails and the head, indexOfHead is equal to 2, the head of the snake
    // Insert the first tail
    Position temp;
    temp.x = -2;
    temp.y = 0;
    insertArray(&snake, temp);

    // Insert the second tail
    temp.x = -1;
    temp.y = 0;
    insertArray(&snake, temp);

    // Insert the starting head
    temp.x = 0;
    temp.y = 0;
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
            ledDisplayText("GAME OVER\nPress 1 to end or 2 to play again");
            break;
        }

        // TODO: if the collision is -1 then it hit a food, generate a new food, update lcd and LED then continue
        if (collision == -1)
        {
            food = generateFood(&snake);
            lcdDisplay(snake.score);
            ledDisplayUpdate(&snake, food);
        }

        // TODO: if the collision is 0 then it did not hit anything, update LED display with new snake and continue
        if (collision == 0)
        {
            ledDisplayUpdate(&snake, food);
        }

        // TODO: check if either button 1 or button 2 has been pressed, if 1 then turn left, if 2 then turn right
        if(checkButton1() == 1)
        {
            printf("Turn the snake direction to left");
        }
        else if(checkButton2() == 1)
        {
            printf("Turn the snake direction to right");
        }
    }

    freeArray(&snake);

    return 0;
}
