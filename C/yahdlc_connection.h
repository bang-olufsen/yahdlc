/**
 * @file yahdlc_link.h
 */

#ifndef YAHDLC_LINK_H
#define YAHDLC_LINK_H

#include <errno.h>

typedef int (*yahdlc_write_cb_t)(const void *buf, unsigned int count);
typedef int (*yahdlc_read_cb_t)(const void *buf, unsigned int count);

#ifdef __cplusplus
extern "C" {
#endif

void yahdlc_set_write_cb(yahdlc_write_cb_t cb);
void yahdlc_set_read_cb(yahdlc_read_cb_t cb);

int yahdlc_write(const void *buf, unsigned int count);
int yahdlc_read(const void *buf, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif
