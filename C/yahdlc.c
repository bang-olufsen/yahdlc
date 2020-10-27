#include "yahdlc.h"

// HDLC Control field bit positions
#define YAHDLC_CONTROL_S_OR_U_FRAME_BIT 0
#define YAHDLC_CONTROL_SEND_SEQ_NO_BIT 1
#define YAHDLC_CONTROL_S_FRAME_TYPE_BIT 2
#define YAHDLC_CONTROL_POLL_BIT 4
#define YAHDLC_CONTROL_RECV_SEQ_NO_BIT 5

// HDLC Control type definitions
#define YAHDLC_CONTROL_TYPE_RECEIVE_READY 0
#define YAHDLC_CONTROL_TYPE_RECEIVE_NOT_READY 1
#define YAHDLC_CONTROL_TYPE_REJECT 2
#define YAHDLC_CONTROL_TYPE_SELECTIVE_REJECT 3

static yahdlc_state_t yahdlc_state = {
  .control_escape = 0,
  .fcs = FCS_INIT_VALUE,
  .start_index = -1,
  .end_index = -1,
  .src_index = 0,
  .dest_index = 0,
};

int yahdlc_set_state(yahdlc_state_t *state) {
  if (!state) {
    return -EINVAL;
  }

  yahdlc_state = *state;
  return 0;
}


int yahdlc_get_state(yahdlc_state_t *state) {
  if (!state) {
    return -EINVAL;
  }

  *state = yahdlc_state;
  return 0;
}

void yahdlc_escape_value(char value, char *dest, int *dest_index) {
  // Check and escape the value if needed
  if ((value == YAHDLC_FLAG_SEQUENCE) || (value == YAHDLC_CONTROL_ESCAPE)) {
    dest[(*dest_index)++] = YAHDLC_CONTROL_ESCAPE;
    value ^= 0x20;
  }

  // Add the value to the destination buffer and increment destination index
  dest[(*dest_index)++] = value;
}

yahdlc_control_t yahdlc_get_control_type(unsigned char control) {
  yahdlc_control_t value;

  // Check if the frame is a S-frame (or U-frame)
  if (control & (1 << YAHDLC_CONTROL_S_OR_U_FRAME_BIT)) {
    // Check if S-frame type is a Receive Ready (ACK)
    if (((control >> YAHDLC_CONTROL_S_FRAME_TYPE_BIT) & 0x3)
        == YAHDLC_CONTROL_TYPE_RECEIVE_READY) {
      value.frame = YAHDLC_FRAME_ACK;
    } else {
      // Assume it is an NACK since Receive Not Ready, Selective Reject and U-frames are not supported
      value.frame = YAHDLC_FRAME_NACK;
    }

    // Add the receive sequence number from the S-frame (or U-frame)
    value.seq_no = (control >> YAHDLC_CONTROL_RECV_SEQ_NO_BIT);
  } else {
    // It must be an I-frame so add the send sequence number (receive sequence number is not used)
    value.frame = YAHDLC_FRAME_DATA;
    value.seq_no = (control >> YAHDLC_CONTROL_SEND_SEQ_NO_BIT);
  }

  return value;
}

unsigned char yahdlc_frame_control_type(yahdlc_control_t *control) {
  unsigned char value = 0;

  // For details see: https://en.wikipedia.org/wiki/High-Level_Data_Link_Control
  switch (control->frame) {
    case YAHDLC_FRAME_DATA:
      // Create the HDLC I-frame control byte with Poll bit set
      value |= (control->seq_no << YAHDLC_CONTROL_SEND_SEQ_NO_BIT);
      value |= (1 << YAHDLC_CONTROL_POLL_BIT);
      break;
    case YAHDLC_FRAME_ACK:
      // Create the HDLC Receive Ready S-frame control byte with Poll bit cleared
      value |= (control->seq_no << YAHDLC_CONTROL_RECV_SEQ_NO_BIT);
      value |= (1 << YAHDLC_CONTROL_S_OR_U_FRAME_BIT);
      break;
    case YAHDLC_FRAME_NACK:
      // Create the HDLC Receive Ready S-frame control byte with Poll bit cleared
      value |= (control->seq_no << YAHDLC_CONTROL_RECV_SEQ_NO_BIT);
      value |= (YAHDLC_CONTROL_TYPE_REJECT << YAHDLC_CONTROL_S_FRAME_TYPE_BIT);
      value |= (1 << YAHDLC_CONTROL_S_OR_U_FRAME_BIT);
      break;
  }

  return value;
}

void yahdlc_get_data_reset() {
  yahdlc_get_data_reset_with_state(&yahdlc_state);
}

void yahdlc_get_data_reset_with_state(yahdlc_state_t *state) {
  state->fcs = FCS_INIT_VALUE;
  state->start_index = state->end_index = -1;
  state->src_index = state->dest_index = 0;
  state->control_escape = 0;
}

int yahdlc_get_data(yahdlc_control_t *control, const char *src,
                    unsigned int src_len, char *dest, unsigned int *dest_len) {
  return yahdlc_get_data_with_state(&yahdlc_state, control, src, src_len, dest, dest_len);
}

int yahdlc_get_data_with_state(yahdlc_state_t *state, yahdlc_control_t *control, const char *src,
                    unsigned int src_len, char *dest, unsigned int *dest_len) {
  int ret;
  char value;
  unsigned int i;

  // Make sure that all parameters are valid
  if (!state || !control || !src || !dest || !dest_len) {
    return -EINVAL;
  }

  // Run through the data bytes
  for (i = 0; i < src_len; i++) {
    // First find the start flag sequence
    if (state->start_index < 0) {
      if (src[i] == YAHDLC_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present
        if ((i < (src_len - 1)) && (src[i + 1] == YAHDLC_FLAG_SEQUENCE)) {
          // Just loop again to silently discard it (accordingly to HDLC)
          continue;
        }

        state->start_index = state->src_index;
      }
    } else {
      // Check for end flag sequence
      if (src[i] == YAHDLC_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present or earlier received
        if (((i < (src_len - 1)) && (src[i + 1] == YAHDLC_FLAG_SEQUENCE))
            || ((state->start_index + 1) == state->src_index)) {
          // Just loop again to silently discard it (accordingly to HDLC)
          continue;
        }

        state->end_index = state->src_index;
        break;
      } else if (src[i] == YAHDLC_CONTROL_ESCAPE) {
        state->control_escape = 1;
      } else {
        // Update the value based on any control escape received
        if (state->control_escape) {
          state->control_escape = 0;
          value = src[i] ^ 0x20;
        } else {
          value = src[i];
        }

        // Now update the FCS value
        state->fcs = calc_fcs(state->fcs, value);

        if (state->src_index == state->start_index + 2) {
          // Control field is the second byte after the start flag sequence
          *control = yahdlc_get_control_type(value);
        } else if (state->src_index > (state->start_index + 2)) {
          // Start adding the data values after the Control field to the buffer
          dest[state->dest_index++] = value;
        }
      }
    }
    state->src_index++;
  }

  // Check for invalid frame (no start or end flag sequence)
  if ((state->start_index < 0) || (state->end_index < 0)) {
    // Return no message and make sure destination length is 0
    *dest_len = 0;
    ret = -ENOMSG;
  } else {
    // A frame is at least 4 bytes in size and has a valid FCS value
    if ((state->end_index < (state->start_index + 4))
        || (state->fcs != FCS_GOOD_VALUE)) {
      // Return FCS error and indicate that data up to end flag sequence in buffer should be discarded
      *dest_len = i;
      ret = -EIO;
    } else {
      // Return success and indicate that data up to end flag sequence in buffer should be discarded
      *dest_len = state->dest_index - sizeof(state->fcs);
      ret = i;
    }

    // Reset values for next frame
    yahdlc_get_data_reset_with_state(state);
  }

  return ret;
}

int yahdlc_frame_data(yahdlc_control_t *control, const char *src,
                      unsigned int src_len, char *dest, unsigned int *dest_len) {
  unsigned int i;
  int dest_index = 0;
  unsigned char value = 0;
  FCS_SIZE fcs = FCS_INIT_VALUE;

  // Make sure that all parameters are valid
  if (!control || (!src && (src_len > 0)) || !dest || !dest_len) {
    return -EINVAL;
  }

  // Start by adding the start flag sequence
  dest[dest_index++] = YAHDLC_FLAG_SEQUENCE;

  // Add the all-station address from HDLC (broadcast)
  fcs = calc_fcs(fcs, YAHDLC_ALL_STATION_ADDR);
  yahdlc_escape_value(YAHDLC_ALL_STATION_ADDR, dest, &dest_index);

  // Add the framed control field value
  value = yahdlc_frame_control_type(control);
  fcs = calc_fcs(fcs, value);
  yahdlc_escape_value(value, dest, &dest_index);

  // Only DATA frames should contain data
  if (control->frame == YAHDLC_FRAME_DATA) {
    // Calculate FCS and escape data
    for (i = 0; i < src_len; i++) {
      fcs = calc_fcs(fcs, src[i]);
      yahdlc_escape_value(src[i], dest, &dest_index);
    }
  }

  // Invert the FCS value accordingly to the specification
  fcs ^= FCS_INVERT_MASK;

  // Run through the FCS bytes and escape the values
  for (i = 0; i < sizeof(fcs); i++) {
    value = ((fcs >> (8 * i)) & 0xFF);
    yahdlc_escape_value(value, dest, &dest_index);
  }

  // Add end flag sequence and update length of frame
  dest[dest_index++] = YAHDLC_FLAG_SEQUENCE;
  *dest_len = dest_index;

  return 0;
}
