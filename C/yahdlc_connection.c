#include "yahdlc.h"
#include "yahdlc_connection.h"

static char yahdlc_write_buffer[1000];
static char yahdlc_read_buffer[1000];

static yahdlc_write_cb_t yahdlc_write_cb;
static yahdlc_read_cb_t yahdlc_read_cb;

static yahdlc_control_t yahdlc_write_control;
static yahdlc_control_t yahdlc_read_control;

void yahdlc_set_write_cb(yahdlc_write_cb_t cb)
{
    yahdlc_write_cb = cb;
}

void yahdlc_set_read_cb(yahdlc_read_cb_t cb)
{
    yahdlc_read_cb = cb;
}

int yahdlc_write(const void *buf, unsigned int count)
{
    unsigned int len;
    yahdlc_frame_data(&yahdlc_write_control, buf, count, yahdlc_write_buffer, &len);
    return yahdlc_write_cb(yahdlc_write_buffer, len);
}

int yahdlc_read(const void *buf, unsigned int count)
{
    int ret;
    unsigned int len;

    if (!buf || count <)

    if ((ret = yahdlc_read_cb(yahdlc_read_buffer, sizeof(yahdlc_read_buffer))) < 0)
        return ret;

    if ((ret = yahdlc_get_data(&yahdlc_read_control, yahdlc_read_buffer, ret, buf, len)) < 0)
        return ret;

    switch (yahdlc_read_control.frame) {
    case YAHDLC_FRAME_DATA:
        return ret;
    case YAHDLC_FRAME_ACK:
        break;
    case YAHDLC_FRAME_NACK:
        break;
    }
}