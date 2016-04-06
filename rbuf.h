/**
 * @file rbuf.h
 *
 * @brief Ring buffers
 *
 * Ringbuffer implementation store/access arbitrary binary data
 *
 * @todo allow to register i/o filters to be executed at read/write time
 *
 */

#ifndef __RBUF_H__
#define __RBUF_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    RBUF_MODE_BLOCKING = 0,
    RBUF_MODE_OVERWRITE
} rbuf_mode_t;

typedef struct {
    uint8_t *buf;            // the buffer
    uint32_t size;           // buffer size
    uint32_t available;      // buffer size
    uint32_t used;           // used size
    uint32_t rfx;            // read offset
    uint32_t wfx;            // write offset
    uint32_t mode;           // the ringbuffer mode (blocking/overwrite)
} rbuf_t;

/**
 * @brief Initialize a ringbuffer
 * @param sbuf : A pointer to the rbuf control struct
 * @param buf  : A pointer to the storage array
 * @param size : The size of the ringbuffer (in bytes)
 */
void rbuf_init(rbuf_t *rbuf, uint8_t *buf, uint32_t size);

void rbuf_set_mode(rbuf_t *rbuf, rbuf_mode_t mode);
rbuf_mode_t rbuf_mode(rbuf_t *rbuf);

/**
 * @brief Skip the specified amount of bytes
 * @param rbuf  : A valid pointer to a rbuf_t structure
 * @param size  : The number of bytes to skip
 */
void rbuf_skip(rbuf_t *rbuf, uint32_t size);

/**
 * @brief Read the specified amount of bytes from the ringbuffer
 * @param rbuf : A valid pointer to a rbuf_t structure
 * @param out  : A valid pointer initialized to store the read data 
 * @param size : The amount of bytes to read and copy to the memory
 *               pointed by 'out'
 * @return     : The amount of bytes actually read from the ringbuffer
 */
uint32_t rbuf_read(rbuf_t *rbuf, uint8_t *out, uint32_t size);

/**
 * @brief Write the specified amount of bytes into the ringbuffer
 * @param rbuf : A valid pointer to a rbuf_t structure
 * @param in   : A pointer to the data to copy into the ringbuffer
 * @param size : The amount of bytes to be copied
 * @return     : The amount of bytes actually copied into the ringbuffer
 * @note       : The ringbuffer may not fit the entire buffer to copy so 
 *               the returned value might be less than the input 'size'.
 *               The caller should check for the returned value and retry
 *               writing the remainder once the ringbuffer has been emptied
 *               sufficiently
 */
uint32_t rbuf_write(rbuf_t *rbuf, uint8_t *in, uint32_t size);

/**
 * @brief Returns the total size of the ringbuffer (specified at creation time)
 * @param rbuf  : A valid pointer to a rbuf_t structure
 * @return the total amount of bytes that can be stored in the rbuf
 */
uint32_t rbuf_size(rbuf_t *rbuf);

/**
 * @brief Returns the amount of bytes stored into the ringbuffer
 *        and available for reading
 * @param rbuf  : A valid pointer to a rbuf_t structure
 * @return the amount of bytes stored into the ringbuffer
 *         and available for reading
 */
uint32_t rbuf_used(rbuf_t *rbuf);

/**
 * @brief Returns the amount of space left in the ringbuffer for writing
 * @note equivalent to: rbuf_size() - rbuf_used()
 * @param rbuf  : A valid pointer to a rbuf_t structure
 * @return the amount of bytes which is still possible to write into the ringbuffer
 *         until some data is consumed by a rbuf_read() operation
 */
uint32_t rbuf_available(rbuf_t *rbuf);

/**
 * @brief Scan the ringbuffer untill the specific byte is found
 * @param rbuf   : A valid pointer to a rbuf_t structure
 * @param octet : The byte to search into the ringbuffer
 * @return the offset to the specified byte, -1 if not found
 */
uint32_t rbuf_find(rbuf_t *rbuf, uint8_t octet);

/**
 * @brief Read until a specific byte is found or maxsize is reached
 * @param rbuf     : A valid pointer to a rbuf_t structure
 * @param octet   : The byte to look for before stopping
 * @param out     : A valid pointer initialized to store the read data 
 * @param maxsize : The maximum amount of bytes that can be copied to
 *                  the memory pointed by 'out'
 * @return        : The amount of bytes actually read from the ringbuffer
 */
uint32_t rbuf_read_until(rbuf_t *rbuf, uint8_t octet, uint8_t *out, uint32_t maxsize);

uint32_t rbuf_move(rbuf_t *src, rbuf_t *dst, uint32_t len);
uint32_t rbuf_copy(rbuf_t *src, rbuf_t *dst, uint32_t len);

/**
 * @brief Clear the ringbuffer by eventually skipping all the unread bytes (if any)
 * @param rbuf : A valid pointer to a rbuf_t structure
 */
void rbuf_clear(rbuf_t *rbuf);

#ifdef __cplusplus
}
#endif

#endif

// vim: tabstop=4 shiftwidth=4 expandtab:
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
