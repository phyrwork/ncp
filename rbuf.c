#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbuf.h"

#define RBUF_DEFAULT_SIZE 4096

void
rbuf_init(rbuf_t *rbuf, uint8_t* buf, uint32_t size) {
    rbuf->size = size;
    rbuf->buf = buf;
    rbuf->available = rbuf->size - 1;
    rbuf->used = 0;
    rbuf->rfx = 0;
    rbuf->wfx = 0;
}

static void
rbuf_update_size(rbuf_t *rb) {
    if(rb->wfx == rb->rfx)
        rb->used = 0;
    else if(rb->wfx < rb->rfx)
        rb->used = rb->wfx+(rb->size-rb->rfx);
    else
        rb->used = rb->wfx-rb->rfx;

    rb->available = rb->size - rb->used - 1;
}

void
rbuf_set_mode(rbuf_t *rbuf, rbuf_mode_t mode)
{
    rbuf->mode = mode;
}

rbuf_mode_t
rbuf_mode(rbuf_t *rbuf)
{
    return rbuf->mode;
}

void
rbuf_skip(rbuf_t *rb, uint32_t size) {
    if(size >= rb->size) { // just empty the ringbuffer
        rb->rfx = rb->wfx;
    } else {
        if (size > rb->size-rb->rfx) {
            size -= rb->size-rb->rfx;
            rb->rfx = size;
        } else {
            rb->rfx+=size;
        }
    }
    rbuf_update_size(rb);
}

uint32_t
rbuf_read(rbuf_t *rb, uint8_t *out, uint32_t size) {
    uint32_t read_size = rb->used; // never read more than available data
    uint32_t to_end = rb->size - rb->rfx;

    // requested size is less than stored data, return only what has been requested
    if(read_size > size)
        read_size = size;

    if(read_size > 0) {
        // if the write pointer is beyond the read pointer or the requested read_size is
        // smaller than the number of octets between the read pointer and the end of the buffer,
        // than we can safely copy all the octets in a single shot
        if(rb->wfx > rb->rfx || to_end >= read_size) {
            memcpy(out, &rb->buf[rb->rfx], read_size);
            rb->rfx += read_size;
        }
        else { // otherwise we have to wrap around the buffer and copy octest in two times
            memcpy(out, &rb->buf[rb->rfx], to_end);
            memcpy(out+to_end, &rb->buf[0], read_size - to_end);
            rb->rfx = read_size - to_end;
        }
    }

    rbuf_update_size(rb);

    return read_size;
}

uint32_t
rbuf_write(rbuf_t *rb, uint8_t *in, uint32_t size) {
    uint32_t write_size = rb->available; // don't write more than available size

    if(!rb || !in || !size) // safety belt
        return 0;

    // if requested size fits the available space, use that
    if(write_size > size) {
        write_size = size;
    } else if (rb->mode == RBUF_MODE_OVERWRITE) {
        if (size > rb->size - 1) {
            // the provided buffer is bigger than the
            // ringbuffer itself. Since we are in overwrite mode,
            // only the last chunk will be actually stored.
            write_size = rb->size - 1;
            in = in + (size - write_size);
            rb->rfx = 0;
            memcpy(rb->buf, in, write_size);
            rb->wfx = write_size;
            rbuf_update_size(rb);
            return size;
        }
        // we are in overwrite mode, so let's make some space
        // for the new data by advancing the read offset
        uint32_t diff = size - write_size;
        rb->rfx += diff;
        write_size += diff;
        if (rb->rfx >= rb->size)
            rb->rfx -= rb->size;
    }

    if(rb->wfx >= rb->rfx) { // write pointer is ahead
        if(write_size <= rb->size - rb->wfx) {
            memcpy(&rb->buf[rb->wfx], in, write_size);
            rb->wfx+=write_size;
        } else { // and we have to wrap around the buffer
            uint32_t to_end = rb->size - rb->wfx;
            memcpy(&rb->buf[rb->wfx], in, to_end);
            memcpy(rb->buf, in+to_end, write_size - to_end);
            rb->wfx = write_size - to_end;
        }
    } else { // read pointer is ahead we can safely memcpy the entire chunk
        memcpy(&rb->buf[rb->wfx], in, write_size);
        rb->wfx+=write_size;
    }

    rbuf_update_size(rb);

    return write_size;
}

uint32_t
rbuf_used(rbuf_t *rb) {
    return rb->used;
}

uint32_t
rbuf_size(rbuf_t *rb) {
    return rb->size - 1;
}

uint32_t
rbuf_available(rbuf_t *rb) {
    return rb->available;
}

void
rbuf_clear(rbuf_t *rb) {
    rb->rfx = rb->wfx = 0;
    rbuf_update_size(rb);
}

uint32_t
rbuf_find(rbuf_t *rb, uint8_t octet) {
    uint32_t i;
    uint32_t to_read = rbuf_used(rb);
    if (to_read == 0)
        return -1;

    if(rb->wfx > rb->rfx) {
        for (i = rb->rfx; i < rb->wfx; i++) {
            if(rb->buf[i] == octet)
                return(i-rb->rfx);
        }
    } else {
        for (i = rb->rfx; i < rb->size; i++) {
            if(rb->buf[i] == octet)
                return(i-rb->rfx);
        }
        for (i = 0; i < rb->wfx; i++) {
            if(rb->buf[i] == octet)
                return((rb->size-rb->rfx)+i);
        }
    }
    return -1;
}

uint32_t
rbuf_read_until(rbuf_t *rb, uint8_t octet, uint8_t *out, uint32_t maxsize)
{
    uint32_t i;
    uint32_t size = rbuf_used(rb);
    uint32_t to_read = size;
    uint32_t found = 0;
    for (i = rb->rfx; i < rb->size; i++) {
        to_read--;
        if(rb->buf[i] == octet)  {
            found = 1;
            break;
        } else if ((size-to_read) == maxsize) {
            break;
        } else {
            out[i] = rb->buf[i];
        }
    }
    if(!found) {
        for (i = 0; to_read > 0 && (size-to_read) < maxsize; i++) {
            to_read--;
            if(rb->buf[i] == octet) {
                break;
            }
            else {
                out[i] = rb->buf[i];
            }
        }
    }
    rbuf_skip(rb, (size - to_read));
    return (size-to_read);
}

static uint32_t
rbuf_copy_uint32_ternal(rbuf_t *src, rbuf_t *dst, uint32_t len, uint32_t move)
{
    if (!src || !dst || !len)
        return 0;

    uint32_t to_copy = rbuf_available(dst);
    if (len < to_copy)
        to_copy = len;

    uint32_t available = rbuf_used(src);
    if (available < to_copy)
        to_copy = available;

    uint32_t contiguous = (dst->wfx > dst->rfx)
                  ? dst->size - dst->wfx
                  : dst->rfx - dst->wfx;

    if (contiguous >= to_copy) {
        if (move) {
            rbuf_read(src, &dst->buf[dst->wfx], to_copy);
        } else {
            if (src->rfx < src->wfx) {
                memcpy(&dst->buf[dst->wfx], &src->buf[src->rfx], to_copy);
            } else {
                uint32_t to_end = src->size - src->rfx;
                memcpy(&dst->buf[dst->wfx], &src->buf[src->rfx], to_end);
                dst->wfx += to_end;
                memcpy(&dst->buf[dst->wfx], &src->buf[0], to_copy - to_end);
            }
        }
        dst->wfx += to_copy;
    } else {
        uint32_t remainder = to_copy - contiguous;
        if (move) {
            rbuf_read(src, &dst->buf[dst->wfx], contiguous);
            rbuf_read(src, &dst->buf[0], remainder);
        } else {
            if (src->rfx < src->wfx) {
                memcpy(&dst->buf[dst->wfx], &src->buf[src->rfx], contiguous);
                memcpy(&dst->buf[0], &src->buf[src->rfx + contiguous], remainder);
            } else {
                uint32_t to_end = src->size - src->rfx;
                if (to_end > contiguous) {
                    memcpy(&dst->buf[dst->wfx], &src->buf[dst->rfx], contiguous);
                    uint32_t diff = to_end - contiguous;
                    if (diff > remainder) {
                        memcpy(&dst->buf[0], &src->buf[dst->rfx + contiguous], remainder);
                    } else {
                        memcpy(&dst->buf[0], &src->buf[dst->rfx + contiguous], diff);
                        memcpy(&dst->buf[diff], &src->buf[0], remainder - diff);
                    }
                } else {
                    memcpy(&dst->buf[dst->wfx], &src->buf[dst->rfx], to_end);
                    uint32_t diff = contiguous - to_end;
                    if (diff) {
                        memcpy(&dst->buf[dst->wfx + to_end], &src->buf[0], diff);
                        memcpy(&dst->buf[0], &src->buf[diff], remainder);
                    }
                }
            }
        }
        dst->wfx = remainder;
    }
    rbuf_update_size(dst);
    return to_copy;
}

uint32_t
rbuf_move(rbuf_t *src, rbuf_t *dst, uint32_t len)
{
    return rbuf_copy_uint32_ternal(src, dst, len, 1);
}

uint32_t
rbuf_copy(rbuf_t *src, rbuf_t *dst, uint32_t len)
{
    return rbuf_copy_uint32_ternal(src, dst, len, 0);
}

// vim: tabstop=4 shiftwidth=4 expandtab:
/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
