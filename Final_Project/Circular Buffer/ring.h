/*========================================================================
** ring.h
** Circular buffer
** ECEN5813
**========================================================================*/

/*****************************************************************************
* Copyright (C) 2019 by Ismail Yesildirek
*
* Redistribution, modification or use of this software in source or binary
* forms is permitted as long as the files maintain this copyright. Users are
* permitted to modify this and use it to learn about the field of embedded
* software. Ismail Yesildirek and the University of Colorado are not
* liable for any misuse of this material.
*
*****************************************************************************/

/**
* @file ring.h
* @brief This header file provides the ring buffer prototypes.
*
* @author Ismail Yesildirek
* @date March 18 2019
* @version 1.0
*
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifndef RING_H
#define RING_H

/****Global Variables****/
#define TRUE 1
#define FALSE 0

extern int bufferSize; 
extern unsigned char data[4];
extern int queueFull;

typedef struct
{
	unsigned char *Buffer;
	int Length; /*Max size*/
	int Ini; /*head*/
	int Outi; /*tail*/
	int count; /*# of char buffer*/
	unsigned char circularQueue[100];
} ring_t;

extern ring_t *buffer_struct;
/****Function Prototypes****/

ring_t* init( int length );

/*Return 0 for success and -1 for failure*/
int insert( ring_t *ring, unsigned char data );
int rm( ring_t *ring, unsigned char *data );
/*Remove() renamed to rm() to avoid name issue*/

/* Entries should return the number of elements present in the circular buffer*/
/* The number of elements that are entered but not removed from the circular list*/
int entries( ring_t *ring );

#endif