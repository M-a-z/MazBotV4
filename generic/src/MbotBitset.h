/**********************************************************************************
*
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
* 	   This is a simple bitset. Basically this can 
* 	   be seen as an array with one bit wide items
*      I still haven't learned the O notation :) 
*      But anyways, the size of the bitset should not
*      really affect to the performance.
*
*      Revision history:
*
*      0.0.1   17.03.2010/Maz    First draft 
* 
*******************************************************************************/

#ifndef MBOTBITSET_H
#define MBOTBITSET_H
#include "generic.h"
#include <stdlib.h>



/**
 * @brief for internal use only
 */
typedef struct SMbotBitsSet
{
    unsigned int numofitems;
    unsigned int shiftwidth;
    size_t modmask;
    size_t *onemask;
    size_t *zeromask;
    size_t *bitch;
}SMbotBitsSet;

/**
 * @brief for internal use only
 */
static unsigned const int G_bitsinslot=sizeof(size_t)*8;
/**
 * @brief turn on (1) the bit indicated by index number - starting from 0
 *
 * @param _this pointer to internal struct returned by MbotBitsetInit()
 * @param index Index number for array item (bit) to be set. Starts from 0
 * @returns 0 if successfull, negative number if parameters are invalid
 */
static __inline__ unsigned int MbotBitsetSet(SMbotBitsSet *_this,unsigned int index)
{
    unsigned int slot;
    unsigned int bitpos;
    if(index>=_this->numofitems)
    {
        return -1;
    }
    slot=index>>_this->shiftwidth;
    bitpos=index&_this->modmask;
    _this->bitch[slot] |= /*(1<<bitpos)*/ _this->onemask[bitpos];
    return 0;
}
/**
 * @brief turn off (0) the bit indicated by index number - starting from 0
 *
 * @param _this pointer to internal struct returned by MbotBitsetInit()
 * @param index Index number for array item (bit) to be set. Starts from 0
 * @returns 0 if successfull, negative number if parameters are invalid
 */

static __inline__ unsigned int MbotBitsetUnSet(SMbotBitsSet *_this,unsigned int index)
{
    unsigned int slot;
    unsigned int bitpos;
    if(index>=_this->numofitems)
    {
        return -1;
    }
    slot=index>>_this->shiftwidth;
    bitpos=index&_this->modmask;
    _this->bitch[slot] &= _this->zeromask[bitpos];
    return 0;
}

/**
 * @brief Check the state of bit indicated by index number - starting from 0
 *
 * The function shall return a value which represents the state of the bit.
 * If bit is unset (0), function returns 0. Othervice non zero value is returned.
 * However user must not trust the returned value shall be 1 if bit is set. Only 
 * acceptaple way for checking the bit state, is to check if returned is zero or
 * 0xFFFFFFFF! NOTE: Return type is void * only to ensure things are handled
 * with chuncks which size is natural to the architecture. The returned number 
 * however IS NOT A POINTER TO VALUE.
 *
 * @param _this pointer to internal struct returned by MbotBitsetInit()
 * @param index Index number for array item (bit) to be get. Starts from 0
 * @returns 0 if bit is not set. 0xFFFFFFFF if an error occurred, and some other non zero value if bit was 1
 */

static __inline__ size_t MbotBitsetGet(SMbotBitsSet *_this,unsigned int index)
{
    unsigned int slot;
    unsigned int bitpos;
    if(index>=_this->numofitems)
    {
        return 0xFFFFFFFF;
    }
    slot=index>>_this->shiftwidth;
    bitpos=index&_this->modmask;
    return _this->bitch[slot]&_this->onemask[bitpos];
}

/**
 * @brief Initialize bitset
 *
 * @param numofitems number of bits whiched to be in array.
 * @returns pointer to initialized bitset handle, or NULL on error.
 */

SMbotBitsSet * MbotBitsetInit(unsigned int numofitems);

/**
 * @brief release resources reserved by the bitset.
 *
 * @param _this_ pointer to bitset handle returned by MbotBitsetInit. Shall be NULLed upon SUCCESSFULL uninitialization.
 */
void MbotBitsetUninit(SMbotBitsSet **_this_);

#endif
