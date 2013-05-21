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
*      This is a simple bitset. Basically this can be 
*      seen as an array with one bit wide items
*
*      Revision history:
*
*      0.0.1   17.03.2010/Maz    First draft 
* 
*******************************************************************************/



#include "MbotBitset.h"

static unsigned int AABITSET_ZEROMASK32[32] = 
{
    0xFFFFFFFE,0xFFFFFFFD,0xFFFFFFFB,0xFFFFFFF7,
    0xFFFFFFEF,0xFFFFFFDF,0xFFFFFFBF,0xFFFFFF7F,
    0xFFFFFEFF,0xFFFFFDFF,0xFFFFFBFF,0xFFFFF7FF,
    0xFFFFEFFF,0xFFFFDFFF,0xFFFFBFFF,0xFFFF7FFF,
    0xFFFEFFFF,0xFFFDFFFF,0xFFFBFFFF,0xFFF7FFFF,
    0xFFEFFFFF,0xFFDFFFFF,0xFFBFFFFF,0xFF7FFFFF,
    0xFEFFFFFF,0xFDFFFFFF,0xFBFFFFFF,0xF7FFFFFF,
    0xEFFFFFFF,0xDFFFFFFF,0xBFFFFFFF,0x7FFFFFFF
};

static unsigned int AABITSET_ONEMASK32[32] = 
{
    0x1,        0x2,        0x4,        0x8,
    0x10,       0x20,       0x40,       0x80,
    0x100,      0x200,      0x400,      0x800,
    0x1000,     0x2000,     0x4000,     0x8000,
    0x10000,    0x20000,    0x40000,    0x80000,
    0x100000,   0x200000,   0x400000,   0x800000,
    0x1000000,  0x2000000,  0x4000000,  0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000
};

static unsigned long long int AABITSET_ONEMASK64[64] =
{
    0x1LL,                0x2LL,                0x4LL,                0x8LL,
    0x10LL,               0x20LL,               0x40LL,               0x80LL,
    0x100LL,              0x200LL,              0x400LL,              0x800LL,
    0x1000LL,             0x2000LL,             0x4000LL,             0x8000LL,
    0x10000LL,            0x20000LL,            0x40000LL,            0x80000LL,
    0x100000LL,           0x200000LL,           0x400000LL,           0x800000LL,
    0x1000000LL,          0x2000000LL,          0x4000000LL,          0x8000000LL,
    0x10000000LL,         0x20000000LL,         0x40000000LL,         0x80000000LL,
    0x100000000LL,        0x200000000LL,        0x400000000LL,        0x800000000LL,
    0x1000000000LL,       0x2000000000LL,       0x4000000000LL,       0x8000000000LL,
    0x10000000000LL,      0x20000000000LL,      0x40000000000LL,      0x80000000000LL,
    0x100000000000LL,     0x200000000000LL,     0x400000000000LL,     0x800000000000LL,
    0x1000000000000LL,    0x2000000000000LL,    0x4000000000000LL,    0x8000000000000LL,
    0x10000000000000LL,   0x20000000000000LL,   0x40000000000000LL,   0x80000000000000LL,
    0x100000000000000LL,  0x200000000000000LL,  0x400000000000000LL,  0x800000000000000LL,
    0x1000000000000000LL, 0x2000000000000000LL, 0x4000000000000000LL, 0x8000000000000000LL
};

static unsigned long long int AABITSET_ZEROMASK64[64] =
{
    0xFFFFFFFFFFFFFFFELL,0xFFFFFFFFFFFFFFFDLL,0xFFFFFFFFFFFFFFFBLL,0xFFFFFFFFFFFFFFF7LL,
    0xFFFFFFFFFFFFFFEFLL,0xFFFFFFFFFFFFFFDFLL,0xFFFFFFFFFFFFFFBFLL,0xFFFFFFFFFFFFFF7FLL,
    0xFFFFFFFFFFFFFEFFLL,0xFFFFFFFFFFFFFDFFLL,0xFFFFFFFFFFFFFBFFLL,0xFFFFFFFFFFFFF7FFLL,
    0xFFFFFFFFFFFFEFFFLL,0xFFFFFFFFFFFFDFFFLL,0xFFFFFFFFFFFFBFFFLL,0xFFFFFFFFFFFF7FFFLL,
    0xFFFFFFFFFFFEFFFFLL,0xFFFFFFFFFFFDFFFFLL,0xFFFFFFFFFFFBFFFFLL,0xFFFFFFFFFFF7FFFFLL,
    0xFFFFFFFFFFEFFFFFLL,0xFFFFFFFFFFDFFFFFLL,0xFFFFFFFFFFBFFFFFLL,0xFFFFFFFFFF7FFFFFLL,
    0xFFFFFFFFFEFFFFFFLL,0xFFFFFFFFFDFFFFFFLL,0xFFFFFFFFFBFFFFFFLL,0xFFFFFFFFF7FFFFFFLL,
    0xFFFFFFFFEFFFFFFFLL,0xFFFFFFFFDFFFFFFFLL,0xFFFFFFFFBFFFFFFFLL,0xFFFFFFFF7FFFFFFFLL,
    0xFFFFFFFEFFFFFFFFLL,0xFFFFFFFDFFFFFFFFLL,0xFFFFFFFBFFFFFFFFLL,0xFFFFFFF7FFFFFFFFLL,
    0xFFFFFFEFFFFFFFFFLL,0xFFFFFFDFFFFFFFFFLL,0xFFFFFFBFFFFFFFFFLL,0xFFFFFF7FFFFFFFFFLL,
    0xFFFFFEFFFFFFFFFFLL,0xFFFFFDFFFFFFFFFFLL,0xFFFFFBFFFFFFFFFFLL,0xFFFFF7FFFFFFFFFFLL,
    0xFFFFEFFFFFFFFFFFLL,0xFFFFDFFFFFFFFFFFLL,0xFFFFBFFFFFFFFFFFLL,0xFFFF7FFFFFFFFFFFLL,
    0xFFFEFFFFFFFFFFFFLL,0xFFFDFFFFFFFFFFFFLL,0xFFFBFFFFFFFFFFFFLL,0xFFF7FFFFFFFFFFFFLL,
    0xFFEFFFFFFFFFFFFFLL,0xFFDFFFFFFFFFFFFFLL,0xFFBFFFFFFFFFFFFFLL,0xFF7FFFFFFFFFFFFFLL,
    0xFEFFFFFFFFFFFFFFLL,0xFDFFFFFFFFFFFFFFLL,0xFBFFFFFFFFFFFFFFLL,0xF7FFFFFFFFFFFFFFLL,
    0xEFFFFFFFFFFFFFFFLL,0xDFFFFFFFFFFFFFFFLL,0xBFFFFFFFFFFFFFFFLL,0x7FFFFFFFFFFFFFFFLL
};

SMbotBitsSet * MbotBitsetInit(unsigned int numofitems)
{
    size_t needed_space=0;
    SMbotBitsSet *_this;
    if(0==numofitems)
        return NULL;
    _this=calloc(1,sizeof(SMbotBitsSet));
    if(NULL==_this)
    {
        /* Error */
        return NULL;
    }
    _this->numofitems=numofitems;
    if(4==sizeof(size_t))
    {
        _this->shiftwidth=5;
        _this->modmask=(size_t)0x1F;
        _this->onemask=(size_t *)AABITSET_ONEMASK32;
        _this->zeromask=(size_t *)AABITSET_ZEROMASK32;
    }
    else if(8==sizeof(size_t))
    {
        _this->shiftwidth=6;
        _this->modmask=(size_t)0x3FLL;
        _this->onemask=(size_t *)AABITSET_ONEMASK64;
        _this->zeromask=(size_t *)AABITSET_ZEROMASK64;
    }
    else
    {
        EPRINTC(PrintComp_bitset,"Architecture seems to not be 32 or 64 bits, rewrite me! %s:%d",__FILE__,__LINE__);
        return NULL;
    }
    needed_space=numofitems/8; /* One bit to represent one item */
    needed_space+=sizeof(size_t);            /* Add one slot for floored bits, don't bother to calculate" */
    _this->bitch=calloc(1,needed_space);
    if(NULL==_this->bitch)
    {
        /* Error */
        free(_this);
        _this=NULL;
    }
    return _this;
}

void MbotBitsetUninit(SMbotBitsSet **_this_)
{
    if(NULL==_this_ || NULL == *_this_)
    {
        /* error terror */
        return;
    }
    free(*_this_);
    *_this_=NULL;
}


