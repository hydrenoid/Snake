//
// Created by Jonathon Moore on 11/1/2022.
//

#ifndef SWE_350MILESTONE2_STRUCTURES_H

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

#define SWE_350MILESTONE2_STRUCTURES_H

#endif //SWE_350MILESTONE2_STRUCTURES_H
