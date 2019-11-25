#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "logger.h"
#include "memoryStructure.h"

/*
 * Prototypes
*/
void nextinit(size_t size);

void* nextMalloc(size_t requested);
void nextFree(void* block);
void printMemory();
Element* allocateBlock(Element* space, size_t size);
void freeElement(Element* element);
void mergeForward(Element* element);
void mergeBackwards(Element* element);

/*
 * Implementations
 */
void nextinit(size_t size)
{
    // Check parameter
    if (size < 1)
    {
        printf("ERROR: Trying to initialize with less 1 byte size!\n");
        return;
    }

    // Check if memory is already initialized
    if (memory.memPool.memStart != NULL)
    {
        printf("\nWARNING: Initializing memory again!\n");
        clean();
    }

    // Create memory pool
    memory.memPool.size = size;
    memory.memPool.memStart = malloc(size);

    // Start linked list
    memory.head = (Element*) malloc( sizeof(Element) );
    memory.head->size = size;
    memory.bytesFree  = size;
    memory.head->alloc = 0;
    memory.head->ptr = memory.memPool.memStart;
    memory.tail = memory.head;
    memory.next = memory.head;

    /*
    //TODO: Remove
    char buffer[100];
    sprintf(buffer, "Initializing %lu bytes\n", size);
    writeLog(buffer);
    */
}

/**
 * Takes care of allocating memory using the next fit
 * strategy. It takes the size of the desired memory in bytes, 
 * and returns the pointer to the memory. It will return NULL 
 * if no memory is available.
 * @param requested Size in bytes
 * @return Pointer to memory
 */
void* nextMalloc(size_t requested) 
{
    // Check if there's space
    if (memory.bytesFree < requested)
        return NULL;

    Element* allocated;

    // Get current position
    Element* element = memory.next;

    // Find space
    do
    {
        // Check NULL
        if (element == NULL)
        {
            element = memory.tail;
        }

        // Check space and alloc
        if (element->alloc == 0 && element->size >= requested)
        {
            allocated = allocateBlock(element, requested);
            memory.next = allocated->next;
            memory.bytesFree -= allocated->size;
            break;
        }

        // Get next element
        element = element->next;

    } while (element != memory.next);

    /*
    //TODO: Remove
    char buffer[100];
    sprintf(buffer, "\nAllocating %lu bytes\n\n", requested);
    writeLog(buffer);
    */
    
    return allocated->ptr;
}

void nextFree(void* block) 
{
    // Find the correct element
    Element* element = findByAddress(block);

    // Check for error
    if (element->alloc == 0)
    {
        printf("WARNING: Memory was already free!\n");
        return;
    }
    /*
    //TODO: Remove
    char buffer[100];
    sprintf(buffer, "\nFreeing %d bytes\n", element->size);
    */

    // Free it
    freeElement(element);
}

/**
 * 
 */
void printMemory()
{
    // Get first element
    unsigned int count = 0;
    Element* element = memory.tail;

    while (element != NULL)
    {
        // Print
        printf("Element %u\n\tSize: %d\n\tAllocated: %d\n\tPointer: %p\n", count, element->size, element->alloc, element->ptr);

        // Get next element
        count++;
        element = element->next;
    }

    // Make extra spacing
    puts("");
}

/*
 * Support Methods
 */

/**
 * This method allocates the memory, and takes care of 
 * splitting Elements, head and tail.
 * @param space Pointer to Element with enough space
 * @param size Space needed in bytes
 * @return Pointer to the allocated block
 */
Element* allocateBlock(Element* space, size_t size)
{
    // Space is bigger
    if (space->size > size)
    {
        // Split the block of space in two
        Element* newElement = (Element*) malloc( sizeof(Element) );
        newElement->next = space;
        newElement->prev = space->prev;
        newElement->ptr  = space->ptr;
        newElement->size = size;
        newElement->alloc= 1;

        space->prev  = newElement;
        space->ptr  += size;
        space->size -= size;

        if (newElement->prev != NULL)
            newElement->prev->next = newElement;

        // Take care of tail
        if (newElement->prev == NULL)
            memory.tail = newElement;

        return newElement;
    }

    // Space is equal
    else if (space->size == size)
    {
        // Just allocate space, and return
        space->alloc = 1; return space;
    }

    // Error occurred
    else
    {
        // Write error to log
        printf("ERROR: allocateBlock() is called, when there isn't not enough space:\n");
        printf("\tRequested Size: %lu\n\tSize of Block: %u\n", size, space->size);
        return NULL;
    }
}

/**
 * This takes care of freeing the memory, and takes 
 * care of merging elements. It will also handle head, 
 * tail and next pointers.
 * @param element Pointer to element to be freed
 */
void freeElement(Element* element)
{
    // Free the block
    element->alloc = 0;
    memory.bytesFree += element->size;

    // Merge forward
    if (element->next != NULL)
    {
        if (element->next->alloc == 0)
        { mergeForward(element); }
    }

    // Merge backwards
    if (element->prev != NULL)
    {
        if (element->prev->alloc == 0)
        { mergeBackwards(element); }
    }
}

/**
 * This merges the element with it's next element.
 * This isn't taking allocation into consideration, 
 * but makes sure head, tail and next is correct after 
 * the merge.
 * @param element The growing element
 */
void mergeForward(Element* element)
{
    Element* next = element->next;

    // Set next and prev ptrs
    element->next       = next->next;
    if (next->next != NULL)
        next->next->prev = element;

    // Correct size
    element->size += next->size;

    // Check head
    if (element->next == NULL)
        memory.head = element;

    // Check next
    if (memory.next == next)
        memory.next = element;

    // Free memory
    free(next);
}

/**
 * This merges the element with it's prev element. 
 * This isn't taking allocation into consideration, 
 * but makes sure head, tail and next is correct after 
 * the merge.
 * @param element The growing element
 */
void mergeBackwards(Element* element)
{
    Element* previous = element->prev;

    // Set next and prev pointers
    element->prev = previous->prev;
    if (element->prev != NULL)
        element->prev->next = element;

    // Memory pool pointer
    element->ptr = previous->ptr;

    // Correct size
    element->size += previous->size;

    // Check tail
    if (element->prev == NULL)
        memory.tail = element;

    // Free memory
    free(previous);
}