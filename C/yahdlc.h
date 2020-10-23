/**
 * @file yahdlc.h
 */

#ifndef YAHDLC_H
#define YAHDLC_H

#include "fcs.h"
#include <errno.h>

/** HDLC start/end flag sequence */
#define YAHDLC_FLAG_SEQUENCE 0x7E

/** HDLC control escape value */
#define YAHDLC_CONTROL_ESCAPE 0x7D

/** HDLC all station address */
#define YAHDLC_ALL_STATION_ADDR 0xFF

/** Supported HDLC frame types */
typedef enum {
  YAHDLC_FRAME_DATA,
  YAHDLC_FRAME_ACK,
  YAHDLC_FRAME_NACK,
} yahdlc_frame_t;

/** Control field information */
typedef struct {
  yahdlc_frame_t frame;
  unsigned char seq_no :3;
} yahdlc_control_t;

/** Variables used in yahdlc_get_data and yahdlc_get_data_with_state
 * to keep track of received buffers
 */
typedef struct {
  char control_escape;
  FCS_SIZE fcs;
  int start_index;
  int end_index;
  int src_index;
  int dest_index;
} yahdlc_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the yahdlc state
 *
 * @param[in] state The new yahdlc state to be used
 * @retval 0 Success
 * @retval -EINVAL Invalid parameter
 */
int yahdlc_set_state(yahdlc_state_t *state);

/**
 * Get current yahdlc state
 *
 * @param[out] state Current yahdlc state
 * @retval 0 Success
 * @retval -EINVAL Invalid parameter
 */
int yahdlc_get_state(yahdlc_state_t *state);

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
 * @retval -EINVAL Invalid parameter
 * @retval -ENOMSG Invalid message
 * @retval -EIO Invalid FCS (size of dest_len should be discarded from source buffer)
 *
 * @see yahdlc_get_data_with_state
 */
int yahdlc_get_data(yahdlc_control_t *control, const char *src,
                    unsigned int src_len, char *dest, unsigned int *dest_len);

/**
 * Retrieves data from specified buffer containing the HDLC frame. Frames can be
 * parsed from multiple buffers e.g. when received via UART.
 *
 * This function is a variation of @ref yahdlc_get_data
 * The difference is only in first argument: yahdlc_state_t *state
 * Data under that pointer is used to keep track of internal buffers.
 *
 * @see yahdlc_get_data
 */
int yahdlc_get_data_with_state(yahdlc_state_t *state, yahdlc_control_t *control, const char *src,
                               unsigned int src_len, char *dest, unsigned int *dest_len);


/**
 * Resets values used in yahdlc_get_data function to keep track of received buffers
 */
void yahdlc_get_data_reset();

/**
 * This is a variation of @ref yahdlc_get_data_reset
 * Resets state values that are under the pointer provided as argument
 *
 * This function need to be called before the first call to yahdlc_get_data_with_state
 * when custom state storage is used.
 *
 * @see yahdlc_get_data_reset
 */
void yahdlc_get_data_reset_with_state(yahdlc_state_t *state);

/**
 * Creates HDLC frame with specified data buffer.
 *
 * @param[in] control Control field structure with frame type and sequence number
 * @param[in] src Source buffer with data
 * @param[in] src_len Source buffer length
 * @param[out] dest Destination buffer (should be bigger than source buffer)
 * @param[out] dest_len Destination buffer length
 * @retval 0 Success
 * @retval -EINVAL Invalid parameter
 */
int yahdlc_frame_data(yahdlc_control_t *control, const char *src,
                      unsigned int src_len, char *dest, unsigned int *dest_len);

#ifdef __cplusplus
}
#endif

#endif
