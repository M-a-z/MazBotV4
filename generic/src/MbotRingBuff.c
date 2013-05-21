/** **************************************************************************************************** *
 *
 *  Lockless ringbuffer. Should work when there's only one reader / multiple writers.
 *  Prefers writer(s), Eg. allows overwrite unread data.
 *  Fixed size blocks only.
 *
 *  Revision History:
 *
 *  -0.0.1 06.10.2008/MVa   First Draft
 *
 * ******************************************************************************************************* */


#include "generic.h"
#include "MbotRingBuff.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define PUSKURIHEADER_SIZE 4
//#define DEBUG_PUSKURI
#define GUARD_LIMIT 0
#ifdef DEBUG_PUSKURI
    #define puskuridbg1(foo) printf(foo)
    #define puskuridbg2(foo,bar) printf(foo,bar)
    #define puskuridbg3(foo,bar,foobar) printf(foo,bar,foobar)
    #define puskuridbg4(foo,bar,foobar,barfoo) printf(foo,bar,foobar,barfoo)
#else
    #define puskuridbg1(foo) ((void)0)
    #define puskuridbg2(foo,bar) ((void)0)
    #define puskuridbg3(foo,bar,foobar) ((void)0)
    #define puskuridbg4(foo,bar,foobar,barfoo) ((void)0)
#endif
/*
   Reveals the puskurit for use
*/
Mbot_buffer *Undress_xxx(size_t slot_size,size_t mask)
{
    unsigned int tmp=0;
    Mbot_buffer *buff;
    buff=malloc(sizeof(Mbot_buffer));
    if(NULL==buff)
    {
        EPRINTC(PrintComp_ringbuff,"failed to allocate buffer %s:%d",__FILE__,__LINE__);
        return NULL;
    }
    //align
    if((tmp=(slot_size%4)))
    {
        slot_size+=tmp;
        WPRINTC(PrintComp_ringbuff,"Improperly aligned slot size given to Mbot_buffer");
    }
    buff->base=calloc(mask+1,slot_size+PUSKURIHEADER_SIZE);
    buff->slot_size=slot_size;
    //store unaligned size for correct memcpying
    buff->act_size=slot_size-tmp;
	buff->corrupted=MbotAtomic32Init();
	buff->rindex=MbotAtomic32Init();
    buff->windex=MbotAtomic32Init();
    buff->windex_writers=MbotAtomic32Init();
	if(NULL==buff->rindex||NULL==buff->windex||NULL==buff->windex_writers || NULL== buff->corrupted)
	{
		PPRINTC(PrintComp_ringbuff,"Alloc FAILED?!");
		return NULL;
	}
//    buff->windex_writers=0;
    puskuridbg1("Puskuri created\n");
    return buff;
}
unsigned int buff_xxx_getNreset_corrupt(Mbot_buffer  *buff)
{
    unsigned int corrupttmp;
    corrupttmp=(unsigned int)mbot_atomicGet(buff->corrupted);
    while((int)corrupttmp!=mbot_atomicCAS(buff->corrupted, corrupttmp, 0))
    {
        corrupttmp=(unsigned int)mbot_atomicGet(buff->corrupted);
    }
    return corrupttmp;
}

/*
   peep the puskuri to obtain (and remove) data
   NOTE! returns copy of the data which can, and should be freed by caller.
*/
void *buff_xxx_peep(Mbot_buffer *buff,unsigned int mask)
{
    unsigned int rindextmp;
    unsigned int windextmp;
    int tmpcomp;
    void *tmp;
    Mbot_puskuriIntData *slot;

    rindextmp=(unsigned int)mbot_atomicGet(buff->rindex);
    windextmp=(unsigned int)mbot_atomicGet(buff->windex);
    tmpcomp=(windextmp&mask) - (rindextmp&mask);
    /*If write index is at 'forbidden range' => do not allow read.
     *This is a safety measure, required by insufficient thread safety when too fast writer overwrites
     *not yet read values. This also minimizes possibility to get into 'read not yet written value => bad header flag'
     *situations when multible simultaneous writers do writings, and writer who obtained larger index finishes before 
     *writer with smaller index => hasty reader might read unread values, since finished writer has increased read index 
     *to point at slower (unfinished) writer's slot.
     */
    if(( tmpcomp >= 0 && tmpcomp <= GUARD_LIMIT ) || ( tmpcomp <= 0 && tmpcomp >= GUARD_LIMIT ) )
    {
//Note, this print is printed A LOT...
//puskuridbg3("Yay! We're at guarded zone, won't read windex=%d, rindex=%d\n",windextmp,rindextmp);
        return NULL;
    }
    rindextmp=mbot_atomicIncIfNequal(buff->rindex,1,windextmp-GUARD_LIMIT);
    if(rindextmp==windextmp-GUARD_LIMIT)
        return NULL; //No new data
    puskuridbg3("readingPuskuri, windex=%d,rindex=%d\n",windextmp,rindextmp);
    puskuridbg2("Added rindex=%d\n",rindextmp+1);
    slot=(Mbot_puskuriIntData *) 
    (
        ((char *)buff->base)+ 
        (size_t)
        (
            ( (size_t) ((rindextmp+1)&mask) )*
            (
                (size_t)(buff->slot_size+PUSKURIHEADER_SIZE)
            )
        )
    );
    tmp=malloc(buff->slot_size);
    if(NULL==tmp)
    {
        EPRINTC(PrintComp_ringbuff,"failed to allocate buffer %s:%d",__FILE__,__LINE__);
        return NULL;
    }

    memcpy(tmp,(void *)slot->userdata,buff->slot_size);

    if(0==slot->valid_to_read)
    {
        /*
         * So only thing here left to catch is that writer has gone past readers, and this data is invalid => reject copied data.
         */
        free(tmp);

        WPRINTC(PrintComp_ringbuff,"Failed to read slot %d, no valid_to_read flag set!\n",rindextmp+1);
        return NULL;
    }
    //                      base +                      slot_index      *       (slot_size+internalHeaderSize)
    //mark as invalid to read:
    slot->valid_to_read=0;
    //memcpy(tmp,(void *)(((char *)buff->base)+(((size_t)((rindextmp+1)&mask))*((size_t)buff->slot_size+(size_t)PUSKURIHEADER_SIZE)+PUSKURIHEADER_SIZE)),buff->slot_size);
    puskuridbg4("puskuriRead, first bytes: 0x%08x 0x%08x 0x%08x\n",*(unsigned int *)tmp,*( ((unsigned int *)tmp)+1),*( ((unsigned int *)tmp)+2) );
    return tmp;
}
/*
   Stuff things in
*/
void buff_xxx_grope(Mbot_buffer *buff,void *data,unsigned int mask)
{
    unsigned int windextmp;
    unsigned int wrn = 0;
    Mbot_puskuriIntData *slot;
    puskuridbg1("adding data to puskuri\n");
    //Increment write index which tells other writers to select next slot.
    windextmp=(unsigned int)mbot_atomicAdd(buff->windex_writers,1);
	//mbot_atomicAdd returns value before addition => add 1 to tmp var
	windextmp++;
// We should atomically check if check if buffer is full, and if it is, increase read buffer by one as if overwritten slot was read.
    //Between these two calls there is a small slot of time for reader to get the index where we're going to write... TODO fix this.
    slot=(Mbot_puskuriIntData *)
    (
        ((char *)buff->base)+
        (size_t)
        (
            ((size_t)(windextmp&mask))*(

                ((size_t)buff->slot_size)+
                PUSKURIHEADER_SIZE
            )
        ) 
    );
/* If we're overwriting data => increase read index */
    wrn=mbot_atomicIncIfEqual(buff->rindex,windextmp-mask,GUARD_LIMIT+1);
    if(wrn==windextmp-mask)
    {
		/* If the readindex was increased, mark the data in this slot to be invalid */   
		slot->valid_to_read=0;
        EPRINTC(
            PrintComp_ringbuff,
            "Writer writing over unread data, please increase buffer or fasten readers to avoid data loss/corrupted values!! (GUARD_LIMIT=%d, windex=%d,rindex=%d",
            GUARD_LIMIT,
            windextmp,
            mbot_atomicGet(buff->rindex)
        );
		mbot_atomicAdd(buff->corrupted,1);
    }
	else
	{
    slot->valid_to_read=1;
    memcpy((void *)slot->userdata,data,buff->act_size);
    puskuridbg2("windex_writers added: windex_writers=%d\n",windextmp);
    puskuridbg4("puskuriWrite data     first bytes: 0x%08x 0x%08x 0x%08x\n",*(unsigned int *)data,*(((unsigned int *)data)+1),*(((unsigned int *)data)+2));

    puskuridbg4("puskuriWrite written, first bytes: 0x%08x 0x%08x 0x%08x\n",*(unsigned int *)slot->userdata,*(((unsigned int *)slot->userdata)+1),*(((unsigned int *)slot->userdata)+2));

    windextmp=(unsigned int)mbot_atomicAdd(buff->windex,1);
    puskuridbg2("windex added: windex=%d\n",windextmp+1);
	}
}
/*
   release the puskurit
*/
void buff_cloth(Mbot_buffer **buff)
{
    free((*buff)->base);
    free(*buff);
    *buff=NULL;
}

