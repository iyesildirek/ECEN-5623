/*========================================================================
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
* @file ring.c
* @brief This source file contains a c program to implement a circular ring buffer.
*
* @author Ismail Yesildirek
* @date March 18 2019
* @version 1.0
*
*/

/*
* Reference "Implementing Circular Buffer in C" for implementation strategy. 
* https://embedjournal.com/implementing-circular-buffer-embedded-c/
*/

/****Header Files****/
#include <ring.h>

int bufferSize = 4;  //ok for camera void data
unsigned char data[4] = { "    " };
int queueFull = 0;

/****Main Code****/
/*
int main(void)
{
	ring_t *buffer_struct = malloc(sizeof(ring_t));
	int insertStatus = 0;
	int rmStatus =0;
	int entryStatus =0;
	unsigned char userInput = ' ';
	unsigned char charInput[4];

	buffer_struct = init(bufferSize);
	printf("The size of buffer is: %d and length is: %d\n",sizeof(buffer_struct->Buffer), sizeof(buffer_struct->Length));


	if ((buffer_struct == NULL) || (buffer_struct->Buffer == NULL) || (buffer_struct->circularQueue == NULL))
	{
		printf("NULL pointer provided\n");
		return 0;
	}
		printf("This program creates a circular buffer of size %d\n", bufferSize);
		while (1)
		{
			printf("Please enter 0 to insert, 1 to remove, 2 to check the number of entries in the circular buffer or 3 to exit\n");
			scanf(" %u", &userInput);
			fflush(stdin);                 /
			if (userInput == '0')
			{
				printf("Please enter a char for the circular buffer: ");
				scanf(" %u", &charInput);
				insertStatus = insert(buffer_struct, charInput);
				if (insertStatus == 1)
				{
					printf("Circular Buffer is Full. Remove an item with the rm()\n");
				}
				else if (insertStatus == -1)
				{
					printf("NULL pointer provided\n");
				}
				else
				{
					printf("Buffer head is now at index: %d\n",(buffer_struct->Ini)-1);
				}
			}
			else if (userInput == '1')
			{
				rmStatus = rm(buffer_struct,(buffer_struct->Buffer));
				//add "&" to avoid seg fault or re-write funct to accept char **
				if (rmStatus == 1)
				{
					printf("Circular Buffer is Empty. Add an item with the insert()\n");
				}
				else if (rmStatus == -1)
				{
					printf("NULL pointer provided\n");
				}
				else
				{
					printf("Buffer tail is now at index: %d\n",buffer_struct->Outi);
				}
			}
			else if (userInput == '2')
			{
				entryStatus = entries(buffer_struct);
				printf("The number of entries are %d\n", entryStatus);
			}
			else if (userInput == '3')
			{
				printf("Good Bye!\n");
				return 0;
			}
			else
			{
				printf("Enter a valid command (0, 1, 2, or 3)\n");
			}
		}
}
*/
/****Function Definitions****/

ring_t* init(int length)
{
	ring_t *temp = malloc(sizeof(ring_t));
	temp->Length = length;
	temp->Buffer_s = "a  ";
	temp->Ini = 0;
	temp->Outi = 0;
	temp->count = 0;
	//temp->circularQueue[length];
	return temp;
}

int insert(ring_t *ring, unsigned char data)
{
	if ((ring != NULL) || (ring->Buffer_s != NULL) || (ring->circularQueue != NULL))
	{
		/*Next head index*/
		int nextIndex = 0;
		if (ring->count == 0)
		{
			nextIndex = 1;
		}
		else
		{
			nextIndex = (ring->Ini + 1) & (ring->Length-1);
		}

		/*Check if buffer is full*/
		if ((nextIndex == ring->Outi) &&(ring->count !=0))
		{
		return 1;
		}

		/*Check if head index needs to circle back in buffer*/
		else if (nextIndex >= ring->Length)
		{
			ring->circularQueue[ring->Ini] = data; /*Add data to queue position Ini*/
			ring->count++;	/*number of insert()*/
			ring->Ini = 0;
		}

		else
		{
			if (ring->Ini == ring->Length)
			{
				ring->Ini = 0;
			}
			ring->circularQueue[ring->Ini] = data; /*Add data to queue position Ini*/
			ring->count++;	/*number of insert()*/
			ring->Ini++; /*increment queue index*/
		}

		return 0;
}
	else
	{
		printf("NULL pointer provided\n");
		return -1;
	}
}

int rm(ring_t *ring, unsigned char *data)
{
	if ((ring != NULL) || (ring->Buffer_s != NULL) || (ring->circularQueue != NULL))
	{
		/*Next tail index*/
		int nextIndex = ring->Outi + 1;
		
		/*Check if buffer is empty*/
		if (ring->Ini == ring->Outi)
		{
			return 1;
		}

		/*Check if tail index needs to circle back in buffer*/
		else if (nextIndex >= ring->Length)
		{
			nextIndex = 0;
		}
		else
		{
			//do nothing
		}

		/*Read char and store in *Buffer prior to clearing buffer[Ouit]*/
		//*data = ring->circularQueue[ring->Outi];
		ring->circularQueue[ring->Outi] = '0'; /*clear buffer*/
		ring->Outi++;
		ring->count--;	/*Reduce number of insert()*/
		return 0;
	}
	else
	{
		return -1;
	}
}

int entries(ring_t *ring)
{
	if ((ring != NULL) || (ring->Buffer_s != NULL) || (ring->circularQueue != NULL))
	{
		int temp;
		temp = ring->count;
		for (int i = 0; i < ring->Length; i++)
		{
			printf("Array Index[%d] has char: %c\n", i, ring->circularQueue[i]);
		}
		return temp;
	}
	else
	{
		printf("NULL pointer provided\n");
		return -1;
	}
}

