/**
 * @file yahdlc.h
 */

#ifndef YAHDLC_H
#define YAHDLC_H

/** HDLC start/end flag sequence */
#define YAHDLC_FLAG_SEQUENCE 0x7D

/** HDLC control escape value */
#define YAHDLC_CONTROL_ESCAPE 0x7E

/** HDLC all station address */
#define YAHDLC_ALL_STATION_ADDR 0xFF

/** Supported HDLC frame types */
enum yahdlc_frame_t {
  YAHDLC_FRAME_DATA,
  YAHDLC_FRAME_ACK,
  YAHDLC_FRAME_NACK,
};

/** Control field information */
struct yahdlc_control_t {
  yahdlc_frame_t frame;
  unsigned char seq_no :3;
};

/**
 * Retrieves data from specified buffer containing the HDLC frame. Frames can be
 * parsed from multiple buffers e.g. when received via UART.
 *
 * @param[out] control Control field structure with frame type and sequence number
 * @param[in] src Source buffer with frame
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be able to contain max frame size)
 * @param[out] dest_len Destination buffer length
 * @retval >=0 Success (size of returned value should be discarded from source buffer)
 * @retval -1 Invalid parameter
 * @retval -2 Invalid message
 * @retval -3 Invalid FCS (size of dest_len should be discarded from source buffer)
 */
int yahdlc_get_data(struct yahdlc_control_t *control, const char *src,
                    unsigned int src_len, char *dest, unsigned int *dest_len);

/**
 * Creates HDLC frame with specified data buffer.
 *
 * @param[in] control Control field structure with frame type and sequence number
 * @param[in] src Source buffer with data
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be bigger than source buffer)
 * @param[out] dest_len Destination buffer length
 * @retval 0 Success
 * @retval -1 Invalid parameter
 */
int yahdlc_frame_data(struct yahdlc_control_t *control, const char *src,
                      unsigned int src_len, char *dest, unsigned int *dest_len);

#endif
