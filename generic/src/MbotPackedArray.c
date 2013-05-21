/*
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
* 	   Revision history:
*
* 	   0.0.1   16.02.2010/Maz 	  First attempt
* 
*******************************************************************************/

//TODO: Atomic get/set state - if previous state is not X
//TODO: Allow user to handle array allocation / let user specify the pool where allocation is performed

#include <stdlib.h>
#include <string.h>
#include "MbotPackedArray.h"
#include "generic.h"

#define PACKEDARRAYTEST

    #include <unistd.h>


/* 2 state      => 1 bit 
 * 4 state      => 2 bits
 * 8 state      => 3 bits
 * 16 state     => 4 bits
 * 32 state     => 5 bits
 * 64 state     => 6 bits
 * 128 state    => 7 bits
 */
static const unsigned short int PACKEDARRAY_BITMASK_BIT[16]   ={1,2,4,8 ,16,32,64 ,128,256,512 ,1024,2048,4096,8192 ,16384,32768};
static const unsigned short int PACKEDARRAY_FIRST_BITS_SET[16]={1,3,7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};


#define PACKEDARRAY_ZEROBITMASKFOR8(startbit,bitamnt,mask) {\
    unsigned int PACKEDARRAY_BITMASK_CTR; \
    unsigned char PACKEDARRAY_BITMASK_tmp=0; \
    mask=0XFF; \
    for(PACKEDARRAY_BITMASK_CTR=0;PACKEDARRAY_BITMASK_CTR<bitamnt;PACKEDARRAY_BITMASK_CTR++) \
        PACKEDARRAY_BITMASK_tmp+=(unsigned char)PACKEDARRAY_BITMASK_BIT[startbit+PACKEDARRAY_BITMASK_CTR]; \
    mask=(unsigned char)(mask^(PACKEDARRAY_BITMASK_tmp)); \
}


#define PACKEDARRAY_ZEROBITMASKFOR16(startbit,bitamnt,mask) {\
    unsigned int PACKEDARRAY_BITMASK_CTR; \
    unsigned short PACKEDARRAY_BITMASK_tmp=0; \
    mask=0XFFFF; \
    for(PACKEDARRAY_BITMASK_CTR=0;PACKEDARRAY_BITMASK_CTR<bitamnt;PACKEDARRAY_BITMASK_CTR++) \
        PACKEDARRAY_BITMASK_tmp+=(unsigned short)PACKEDARRAY_BITMASK_BIT[startbit+PACKEDARRAY_BITMASK_CTR]; \
    mask=(unsigned short)(mask^PACKEDARRAY_BITMASK_tmp); \
}
#define PACKEDARRAY_MASK_N_BITS(bitamnt,mask) {\
    mask=0; \
    if((bitamnt)>0) \
        mask=PACKEDARRAY_FIRST_BITS_SET[(bitamnt)-1]; \
}
typedef struct  SPackedArray
{
    unsigned int bitsinitem;
    unsigned int items;
    size_t dbsize;
    unsigned char *database;
    unsigned char states;
    unsigned char hiawatha;
}SPackedArray;


static unsigned char mva_bitmap_read(SPackedArray *_this, unsigned char *slot,unsigned char offset);
static unsigned int mva_bitmap_write(SPackedArray *_this, unsigned char *slot,unsigned char offset, unsigned char value);
static char am_I_the_hiawatha(void);
#ifdef PACKEDARRAYTEST
    void MVaBitmapTest(void);
#endif
void MbotPackedArrayDestroy(SPackedArray **map)
{
    if(NULL==map || NULL==*map)
    {
        EPRINTC(PrintComp_PackAr,"NULL ptr in MbotPackedArrayDestroy()");
        return;
    }
    if(NULL!=(*map)->database)
        free((*map)->database);
    free(*map);
    *map=NULL;
}



static char am_I_the_hiawatha()
{
    unsigned short tmp=1;
    void *ptr;
    ptr=&tmp;
    return *(char *)ptr;
}

static unsigned int mva_bitmap_write(SPackedArray *_this, unsigned char *slot,unsigned char offset, unsigned char value)
{
    unsigned int bitsinslot;
//    unsigned short *tmp;
    unsigned char mask;
    bitsinslot=8-offset;


    if(bitsinslot<_this->bitsinitem)
    {
        unsigned short mask;
        unsigned short *state;
        unsigned short tmpval=value;

        DPRINTC
        (
            PrintComp_PackAr,"Writable bits spreaded over char boundary => using short (startbit is %u, bit amnt=%u)",
            offset,
            _this->bitsinitem
        );
        if(!_this->hiawatha)
        {
            state=(unsigned short *)(slot-1);
        }
        else
           state = (unsigned short *)slot;
        DPRINTC(PrintComp_PackAr,"Writing bitmap at position %p",state);
        PACKEDARRAY_ZEROBITMASKFOR16(offset,_this->bitsinitem,mask);
        /* Set the state to zero */
//        *state=(*state&(mask^0xFFFF));
        DPRINTC(PrintComp_PackAr,"Old 16bit stateslot: 0x%x",*state);
        DPRINTC(PrintComp_PackAr,"Zeroing statebits with mask 0x%x",mask);
        *state=(*state&mask);
        DPRINTC(PrintComp_PackAr,"16bit stateslot after zeroing: 0x%x",*state);
        tmpval<<=offset;
        DPRINTC(PrintComp_PackAr,"Writing new bits 0x%x",tmpval);
//        mask=0;
        *state=(*state|tmpval);
        DPRINTC(PrintComp_PackAr,"new 16bit stateslot 0x%x",*state);
//        *state=(*state|mask);
        return 0;
    }
    DPRINTC(PrintComp_PackAr,"Writing bitmap at position %p",slot);
    DPRINTC
    (
        PrintComp_PackAr,"Writable bits not crossing char boundary => using char (startbit is %u, bit amnt=%u)",
        offset,
        _this->bitsinitem
    );
   PACKEDARRAY_ZEROBITMASKFOR8(offset,_this->bitsinitem,mask);
    DPRINTC(PrintComp_PackAr,"Old 8bit stateslot: 0x%x",*slot);
    DPRINTC(PrintComp_PackAr,"Zeroing statebits with mask 0x%x",mask);
    *slot=(*slot&mask);
    DPRINTC(PrintComp_PackAr,"8bit stateslot after zeroing: 0x%x",*slot);
//    value<<offset;
    DPRINTC(PrintComp_PackAr,"Writing new bits 0x%x",value<<offset);
    *slot=(*slot|(unsigned char)(value<<offset));
    return 0;
}
static unsigned char mva_bitmap_read(SPackedArray *_this, unsigned char *slot,unsigned char offset)
{
    unsigned int bitsinslot;
    //unsigned short *tmp;
    unsigned char statebits;
    unsigned char mask;
    bitsinslot=8-offset;
    if(bitsinslot<_this->bitsinitem)
    {
        /* Damn. The result has spreaded over byte boundary */
        unsigned short mask;
        unsigned short statebits;
        if(!_this->hiawatha)
        {
            statebits=*(unsigned short *)(slot-1);
        }
        else
            statebits=*(unsigned short *)slot;
        statebits>>=offset;
        PACKEDARRAY_MASK_N_BITS(_this->bitsinitem,mask);
        DPRINTC(PrintComp_PackAr,"I should return %u bits, starting from index %u, using mask 0x%x",_this->bitsinitem,offset,mask);
        return (unsigned char)(statebits&mask);
    }
    /* All resultbits should be in this slot */
    statebits=*slot;
    statebits>>=offset;
    PACKEDARRAY_MASK_N_BITS(_this->bitsinitem,mask);
    DPRINTC(PrintComp_PackAr,"I should return %u bits, starting from index %u, using mask 0x%x",_this->bitsinitem,offset,mask);
    return (statebits&mask);
}

int MbotPackedArraySetState(SPackedArray *_this, unsigned int item, unsigned char state)
{
    unsigned int slot;
    unsigned int index=item-1;
    unsigned char offset;
    if(NULL==_this || _this->items<item || 0==item || state > _this->states)
    {
        EPRINTC(PrintComp_PackAr,"invalid param given to MbotPackedArraySetState()");
        return -1;
    }
    DPRINTC(PrintComp_PackAr,"Setting statebits to 0x%x for item %u (states %u, items %u)",state,item,_this->states,_this->items);
    if(_this->hiawatha)
    {
        slot=(unsigned int)((unsigned long long)_this->bitsinitem*index/(unsigned long long)8);
    }
    else
    {
        slot=
        _this->dbsize-
        (unsigned int)
        (
            (unsigned long long)_this->bitsinitem*index
            /
            (unsigned long long)8
        );
    }
    DPRINTC(PrintComp_PackAr,"Slot where index belongs=%u",slot);
    offset=(unsigned char)(((unsigned long long)_this->bitsinitem*(unsigned long long)index)%8);
    DPRINTC(PrintComp_PackAr,"State starts from %u th bit",offset);
    return mva_bitmap_write(_this,_this->database+(size_t)slot,offset,state);
}
unsigned char MbotPackedArrayGetState(SPackedArray *_this, unsigned int item)
{
    unsigned int slot;
    unsigned int index=item-1;
    unsigned char offset;
    if(NULL==_this || _this->items<item || 0==item )
    {
        EPRINTC(PrintComp_PackAr,"invalid param given to MbotPackedArrayGetState()");
        return 0xFF;
    }
    if(_this->hiawatha)
    {
        slot=(unsigned int)((unsigned long long)_this->bitsinitem*index/(unsigned long long)8);
    }
    else
    {
        slot=
        _this->dbsize-
        (unsigned int)
        (
            (unsigned long long)_this->bitsinitem*index
            /
            (unsigned long long)8
        );
    }
    offset=(unsigned char)(((unsigned long long)_this->bitsinitem*(unsigned long long)index)%8);
    DPRINTC(PrintComp_PackAr,"Reading item %u using index %u, bitoffset %u",item,slot,offset);

//    slot=(unsigned long long)_this->bitsinitem*index/(unsigned long long)8;
    return mva_bitmap_read(_this,_this->database+(size_t)slot,offset);
}

SPackedArray * MbotPackedArrayInit(unsigned char states, unsigned int items)
{
    SPackedArray *_this;
    unsigned int i;
    unsigned long long reqspace;
    if(0==states || states > 127)
    {
        EPRINTC(PrintComp_PackAr,"Invalid state given to MbotPackedArrayInit()");
        return NULL;
    }
    if(0==items)
    {
        EPRINTC(PrintComp_PackAr,"invalid amount of items in MbotPackedArrayInit()");
        return NULL;
    }
    _this=malloc(sizeof(SPackedArray));
    if(NULL==_this)
    {
        EPRINTC(PrintComp_PackAr,"Malloc FAILED at MbotPackedArrayInit(), out of mem?");
        return NULL;
    }
    memset(_this,0,sizeof(SPackedArray));
    _this->states=states;
    _this->items=items;
    /* calc amnt of bits for one item */
    for(i=states-1;i;i>>=1)
        _this->bitsinitem++;
    if(0==_this->bitsinitem)
        _this->bitsinitem++;
    /* Amount of bits */
    reqspace=(unsigned long long )_this->bitsinitem*(unsigned long long)items;
    DPRINTC(PrintComp_PackAr,"SpaceNeeded for %llu bits",reqspace);
//    reqspace=reqspace*(unsigned long long)items;
    if(reqspace%8)
        /* Alignment needed */
        reqspace+=(unsigned long long)8;
    reqspace=reqspace/(unsigned long long)8;
     DPRINTC(PrintComp_PackAr,"after alignment, allocating %u bytes",(unsigned int)reqspace);
    DPRINTC(PrintComp_PackAr,"Asked map with %u states and %u items, allocating %u bytes",states,items,(unsigned int)reqspace);
    _this->dbsize=(size_t)reqspace;
    _this->database=calloc(_this->dbsize,sizeof(char));
    if(NULL==_this->database)
    {
        EPRINTC(PrintComp_PackAr,"Calloc FAILED at MbotPackedArrayInit(), out of mem?");
        free(_this);
        return NULL;
    }
    _this->hiawatha=am_I_the_hiawatha();
    return _this;
}

#ifdef PACKEDARRAYTEST
static void dumpbitmap(SPackedArray *map)
{
    size_t mapsize;
    unsigned int i;
    size_t alignment_needed_bits;
    DPRINTC(PrintComp_PackAr,"Dumping info for map");
    DPRINTC
    (
        PrintComp_PackAr,
        "map->items=%u, map->states=%u, map->bitsinitem=%u, map->database=%p",
        map->items, 
        map->states,
        map->bitsinitem, 
        map->database
    );
    mapsize=(size_t)(((unsigned long long)map->items*(unsigned long long)map->bitsinitem)/8);
    DPRINTC(PrintComp_PackAr,"Calculated (cropped) mapsize: %u bytes",mapsize);
    alignment_needed_bits=(size_t)(((unsigned long long)map->items*(unsigned long long)map->bitsinitem)%8);
    DPRINTC(PrintComp_PackAr,"Bits requiring alignement: %u",alignment_needed_bits);
    mapsize+=(alignment_needed_bits)?1:0;
    DPRINTC(PrintComp_PackAr,"Aligned mapsize: %u bytes",mapsize);
    for(i=0;i<mapsize;i++)
    {
        DPRINTC(PrintComp_PackAr,"0x%x",*((map->database)+i));
    }
}

void MVaBitmapTest()
{
    SPackedArray *map;
    unsigned char newstate;
    int i;
    unsigned char state;
    unsigned int items=22;
    for(state=2;state<128;state<<=1)
    {
        unsigned int tmp=0;
        DPRINTC(PrintComp_PackAr,"Creating %u state bitmap with %u items",(unsigned int)state,(unsigned int)items);
        map=MbotPackedArrayInit(state, items);
        if(NULL==map)
        {
            EPRINTC(PrintComp_PackAr,"NoooOoOOoOooOoo! bitmapInit returned NULL!");
            return;
        }
        DPRINTC(PrintComp_PackAr,"Reading initial states:");
        sleep(1);
        for(i=1;(unsigned int)i<=items;i++)
        {
            DPRINTC(PrintComp_PackAr,"item %u state %u",i,MbotPackedArrayGetState(map,i));
            sleep(1);
            for(newstate=(unsigned char)1;newstate<state;newstate=(unsigned char)((newstate==1)?2:(newstate<<1)-1))
            {
                DPRINTC(PrintComp_PackAr,"Setting state to %u",(unsigned int)newstate);
                MbotPackedArraySetState(map,i,newstate);
                tmp=(unsigned int)newstate;
                DPRINTC(PrintComp_PackAr,"item %u new state %u",i,(unsigned int)MbotPackedArrayGetState(map,i));
                sleep(1);
            }
        }
        for(i=1;(unsigned int)i<=items;i++)
        {
            if((unsigned char)tmp!=(unsigned char)MbotPackedArrayGetState(map,i))
            {
                DPRINTC
                (
                    PrintComp_PackAr,
                    "OhNo! read and written states differ for item %u. Read 0x%x, exp 0x%x",
                    i,
                    (unsigned int)MbotPackedArrayGetState
                    (
                        map,
                        i
                    ),
                    tmp
                );
            }
        }
        DPRINTC(PrintComp_PackAr,"Dumping bitmapdata");
        sleep(5);
        dumpbitmap(map);
        sleep(10);
        MbotPackedArrayDestroy(&map);
    }
    DPRINTC(PrintComp_PackAr,"Ending bitmaptest");
    DPRINTC(PrintComp_PackAr,"|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
    sleep(30);
}
#endif

