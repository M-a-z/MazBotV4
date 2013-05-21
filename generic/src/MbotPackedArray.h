/**
********************************************************************************
*      Implementation of a packed array written in C        			
*      Written by  Maz (2010)                              			
*      http://maz-programmersdiary.blogspot.com/           			
*                                                          			
*      You're free to use this piece of code.              			
*      You can also modify it freely, but if you           			
*      improve this, you must write the improved code      			
*      in comments at:                                     			
*      http://maz-programmersdiary.blogspot.com/           			
*      or at:                                              			
*      http://c-ohjelmoijanajatuksia.blogspot.com/         			
*      or mail the corrected version to me at              			
*      Mazziesaccount@gmail.com                            			
*
*
* 	   The packed array can be used as normal array, 
* 	   to store values. However, this array will only use 
* 	   as many bits/item as needed. 
* 	   During the array initialization, maximum value of stored 
* 	   item is specified. The array should support using 1-7 bits 
* 	   for storing the value, Eg. values 1-127 are supported maximums. 
* 	   However, if only needed values are 1 and 0, the functionality 
* 	   will be same as with simple bitset. 
* 	   However, the implementation of packed array is heavier than 
* 	   implementation of a bitset - thus it is not good to use this 
* 	   for such purpose.
*
* 	   O value: (Yep, I have no idea how the O notation is correctly 
* 	   written :] ). So the speed for accessing the items should stay constant, 
* 	   regardless the amount of items. However, when the amount of 
* 	   possible states (amount of bits needed to represent a value 
* 	   stored in array) increases, the speed of operations is *slightly* 
* 	   reduced. This reduction however happens only when amount 
* 	   of bits/item is increased, thus the speed is same if we use 
* 	   33 or 60 as maximum values. (Numbers from 32 to 64 require 
* 	   same amount of bits to be represented.)
*
* 	   And finally, please note this code is not really portable.
* 	   It relies on ints being 32 bit wide....
*
* 	   Revision history:
*
* 	   0.0.1   16.02.2010/Maz    First draft 
* 
*******************************************************************************/

#ifndef PACKEDARRAY_H
#define PACKEDARRAY_H

#include "generic.h"
#ifdef __cplusplus
extern "C"{
#endif
//This can be uncommented to generate loads of prints :]
//#define PACKEDARRAYPRINTALOT
struct SPackedArray;

/**
 * @brief Destroys the packed array
 *
 * @param map pointer to array handle returned by MbotPackedArrayInit(). Shall be NULLed upon return.
 */
void MbotPackedArrayDestroy(struct SPackedArray **map);
/**
 * @brief Initializes desired size of packed array
 *
 * @param states amount of possible states for one array index. can be 1-127
 * @param items maximum amount of "packed items", eg if we compare to traditional array, this is the size of an array
 * @return PackedArray handle, NULL on error
 */
struct SPackedArray * MbotPackedArrayInit(unsigned char states, unsigned int items);

/**
 * @brief Sets the value for array item. 
 *
 * @param map, handle to array returned by MbotPackedArrayInit().
 * @param item, index of array (NOTE: indexing starts from 1, not from 0!)
 * @param state, value wished to be written to the array.
 * @return 0 on success, negative value upon error
 * @warning Operation is not atomic (TODO: make it atomic)!
 */
int MbotPackedArraySetState(struct SPackedArray *map, unsigned int item, unsigned char state);
/**
 * @brief Reads the value set for current index.
 *
 * @param map, handle to array returned by MbotPackedArrayInit().
 * @param item, index of array (NOTE: indexing starts from 1, not from 0!)
 * @return value stored in specified index.
 * @warning Operation is not atomic (TODO: make it atomic)!
 */
unsigned char MbotPackedArrayGetState(struct SPackedArray *map,unsigned int item);
#ifdef __cplusplus
}
#endif

#endif //PACKEDARRAY_H
