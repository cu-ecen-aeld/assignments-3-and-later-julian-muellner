/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

int aesd_circular_buffer_find_fpos_for_entry_offset(struct aesd_circular_buffer *buffer,
            uint32_t buffer_offset, uint32_t entry_offset, uint64_t *fpos) {
    
    if(buffer->out_offs == buffer->in_offs && !buffer->full) // only when empty
        return -1;
    
    uint64_t size = 0;
    uint32_t entry_no = 0;

    uint8_t index = buffer->out_offs;
    do {
        

        if(entry_no == buffer_offset) {
            if(buffer->entry[index].size <= entry_offset) {
                return -1;
            }
            *fpos = size + entry_offset;
            return 0;
        }

        entry_no++;
        size += buffer->entry[index].size;

        index = (index + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    } while(index != buffer->in_offs);
    
    return -1;
}

uint64_t aesd_buffer_size(struct aesd_circular_buffer *buffer) {
    uint64_t size = 0;
    
    if(buffer->out_offs == buffer->in_offs && !buffer->full) // only when empty
        return 0;

    uint8_t index = buffer->out_offs;
    do {
        size += buffer->entry[index].size;
        index = (index + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    } while(index != buffer->in_offs);
    
    return size;
}

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    if(buffer->out_offs == buffer->in_offs && !buffer->full) // only when empty
        return NULL;

    size_t bytes_consumed = 0, bytes_consumed_new = 0;
    uint8_t index = buffer->out_offs;
    do {
        bytes_consumed_new = bytes_consumed + buffer->entry[index].size;

        if(bytes_consumed_new > char_offset) { // this is the block we are looking for
            *entry_offset_byte_rtn = char_offset - bytes_consumed;
            return &buffer->entry[index];
        }

        bytes_consumed = bytes_consumed_new;

        index = (index + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    } while(index != buffer->in_offs);
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
const char* aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    const char* retVal = buffer->full ? buffer->entry[buffer->in_offs].buffptr : NULL;

    buffer->entry[buffer->in_offs] = *add_entry;
    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

    if(buffer->full) {
        buffer->out_offs = (buffer->out_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    }

    if(buffer->in_offs == buffer->out_offs) 
        buffer->full = true;

    return retVal;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
