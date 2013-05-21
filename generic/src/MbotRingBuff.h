/** **************************************************************************************************** *
 *
 * Lockless ringbuffer. Should work when there's only one reader / multiple writers.
 *  Prefers writer(s), Eg. allows overwrite unread data.
 *  Fixed size blocks only.
 *
 *  Revision History:
 *
 * 	- 0.0.2 15.08.2009/Maz  Fixed documentation and function xxx_cloth => _cloth						
 *  -0.0.1 06.10.2008/Maz   First Draft
 *
 ******************************************************************************************************** */


#ifndef MBOTPUSKURIT_H
#define MBOTPUSKURIT_H
#include <unistd.h>
#include "generic.h"
#include "helpers.h"

#define GUARD_LIMIT 0
/**
 * @brief Allocates and prepares the buffer for 255 items
 * @params slot_size Size of items to be stored (size_t).
 * @return returns void* type handle to allocated buffer. NULL if fails.
 *
 * @see buff255_cloth,Undress4095
 */
#define Undress255(slot_size) Undress_xxx(slot_size,0x000000FF)
/**
 * @brief retrive next stored item.
 * @params buff handle to buffer returned by Undress255
 * @return returns stored item or NULL if there's no unread items, or if an error occurred.
 * @warning There's a bug which can cause reader to read from slot which is not yet written if there's
 * more than one writer, and more than one reader thread. To make this less propable, readers are gaining data only when theres
 * {GUARD_LIMIT} or more unread writings in buffer. Thiss will however cause last {GUARD_LIMIT} writes to be always lost.
 * This bug will not occur when there is only one reader.
 * @see Undress255, buff255_grope, buff4095_peep
 */
#define buff255_peep(buff) buff_xxx_peep(buff,0x000000FF)
/**
 * @brief write item to buffer
 * @params buff handle to buffer returned by Undress255
 * @params data pointer to data which is going to be written.
 * @warning size of data to be written must be same as specified size in initialization.
 * @warning This is by no means safe. It relies upon the fact that reader has time to catch write
 * before writer loops over and reaches reader's index again. There's no protection preventing
 * writer from writing the slot reader is reading from. Nor is there any blocking mechanism preventing
 * writer from writing over unread slots. If writer is too fast, data WILL BE corrupted.
 * * @warning There's a bug which can cause reader to read from slot which is not yet written - if there's
 * more than one writer, and more than one reader thread. To make this less propable, readers are gaining data only when theres
 * {GUARD_LIMIT} or more unread writings in buffer. Thiss will however cause last {GUARD_LIMIT} writes to be always lost.
 * This bug will not occur when there is only one reader.
 * @see Undress255, buff255_peep, buff4095_grope
 */
#define buff255_grope(buff,data) buff_xxx_grope(buff,data,0x000000FF)
/**
 * @brief Free buffer and release resources
 * @param pointer to buff handle returned by Undress255 (type Mbot_buffer **)
 * @warning after call to buff255_cloth the buffer should not be read/written anymore!
 * @see Undress255, buff4095_cloth
 */
#define buff255_cloth(foo) buff_cloth(foo)

/**
 * @brief Allocates and prepares the buffer for 4096 items
 * @see Undress255
 */
#define Undress4095(foo) Undress_xxx(foo,0x00000FFF)
/**
 * @brief retrive next stored item.
 * @see buff255_peep
 */
#define buff4095_peep(foo) buff_xxx_peep(foo,0x000000FFF)
/**
 * @brief write item to buffer
 * @see buff255_grope
 */
#define buff4095_grope(foo,bar) buff_xxx_grope(foo,bar,0x000000FFF)
/**
 * @brief Free buffer and release resources
 * @see buff255_cloth
 */
#define buff4095_cloth(foo) buff_cloth(foo)


typedef struct Mbot_puskuriIntData
{
    int valid_to_read;
    char userdata[1];
}Mbot_puskuriIntData;

typedef struct Mbot_buffer
{
    void *base;
    size_t slot_size;
    size_t act_size;
	MbotAtomic32 *corrupted;
    MbotAtomic32 *windex;
    MbotAtomic32 *windex_writers;
//    unsigned int windex_writers;
    MbotAtomic32 *rindex;
}Mbot_buffer;
unsigned int buff_xxx_getNreset_corrupt(Mbot_buffer *buff);

/**
    @brief Reveals the puskurit for use - internal please use macros instead.

*/
Mbot_buffer *Undress_xxx(size_t slot_size,size_t mask);
/**
    @brief peep the puskuri to obtain (and remove) data - internal please use macros instead.
*/
void *buff_xxx_peep(Mbot_buffer *buff, unsigned int mask);
/**
   @brief Stuff things in - internal please use macros instead.
*/
void buff_xxx_grope(Mbot_buffer *buff,void *data, unsigned int mask);
/**
   @brief Free resources- internal please use macros instead.
   Release the puskurit and bury all evidences of what you've been doing 
   Quickly, before your wife/gf finds out!  - internal please use macros instead.
*/
void buff_cloth(Mbot_buffer **buff);

#endif
