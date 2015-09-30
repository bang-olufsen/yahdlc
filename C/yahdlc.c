#include "fcs16.h"
#include "yahdlc.h"

// HDLC Control type definitions
#define HDLC_CONTROL_TYPE_RECEIVE_READY 0x0
#define HDLC_CONTROL_TYPE_RECEIVE_NOT_READY 0x1
#define HDLC_CONTROL_TYPE_REJECT 0x2
#define HDLC_CONTROL_TYPE_SELECTIVE_REJECT 0x3

// HDLC Control byte structure
typedef union {
  struct {
    unsigned char s_or_u_frame: 1;
    unsigned char send_seq_no: 3;
    unsigned char poll: 1;
    unsigned char recv_seq_no: 3;
  } i_frame;

  struct {
    unsigned char s_or_u_frame: 1;
    unsigned char u_frame: 1;
    unsigned char type: 2;
    unsigned char poll: 1;
    unsigned char recv_seq_no: 3;
  } s_frame;

  unsigned char value;
} hdlc_control_t;

void yahdlc_escape_value(char value, char *dest, int *dest_index) {
  // Check and escape the value if needed
  if ((value == YAHDLC_FLAG_SEQUENCE) || (value == YAHDLC_CONTROL_ESCAPE)) {
    dest[(*dest_index)++] = YAHDLC_CONTROL_ESCAPE;
    value ^= 0x20;
  }

  // Add the value to the destination buffer and increment destination index
  dest[(*dest_index)++] = value;
}

yahdlc_control_t yahdlc_get_control_type(unsigned char value) {
  yahdlc_control_t control;
  hdlc_control_t hdlc_control;

  hdlc_control.value = value;

  if (hdlc_control.i_frame.s_or_u_frame) {
    if (hdlc_control.s_frame.type == HDLC_CONTROL_TYPE_RECEIVE_READY) {
      control.frame = YAHDLC_FRAME_ACK;
    } else {
      // Assume it is an NACK since Receive Not Ready, Selective Reject and U-frames are not supported
      control.frame = YAHDLC_FRAME_NACK;
    }

    // Add the receive sequence number from the S-frame (or U-frame)
    control.seq_no = hdlc_control.s_frame.recv_seq_no;
  } else {
    // It must be an I-frame so add the send sequence number (receive sequence number is not used)
    control.frame = YAHDLC_FRAME_DATA;
    control.seq_no = hdlc_control.i_frame.send_seq_no;
  }

  return control;
}

unsigned char yahdlc_frame_control_type(yahdlc_control_t *control) {
  hdlc_control_t hdlc_control;

  hdlc_control.value = 0;

  // For details see: https://en.wikipedia.org/wiki/High-Level_Data_Link_Control
  switch (control->frame) {
    case YAHDLC_FRAME_DATA:
      // Create the HDLC I-frame control byte with Poll bit set
      hdlc_control.i_frame.send_seq_no = control->seq_no;
      hdlc_control.i_frame.poll = 1;
      break;
    case YAHDLC_FRAME_ACK:
      // Create the HDLC Receive Ready S-frame control byte with Poll bit cleared
      hdlc_control.s_frame.recv_seq_no = control->seq_no;
      hdlc_control.s_frame.s_or_u_frame = 1;
      break;
    case YAHDLC_FRAME_NACK:
      // Create the HDLC Receive Ready S-frame control byte with Poll bit cleared
      hdlc_control.s_frame.recv_seq_no = control->seq_no;
      hdlc_control.s_frame.type = HDLC_CONTROL_TYPE_REJECT;
      hdlc_control.s_frame.s_or_u_frame = 1;
      break;
  }

  return hdlc_control.value;
}

int yahdlc_get_data(yahdlc_control_t *control, const char *src,
                    unsigned int src_len, char *dest, unsigned int *dest_len) {
  unsigned int i;
  static unsigned short fcs = FCS16_INIT_VALUE;
  static unsigned char control_escape = 0, value = 0;
  static int ret = -1, start_index = -1, end_index = -1, src_index = 0,
      dest_index = 0;

  // Make sure that all parameters are valid
  if (!control || !src || !dest || !dest_len) {
    return -EINVAL;
  }

  // Run through the data bytes
  for (i = 0; i < src_len; i++) {
    // First find the start flag sequence
    if (start_index < 0) {
      if (src[i] == YAHDLC_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present
        if ((i < (src_len - 1)) && (src[i + 1] == YAHDLC_FLAG_SEQUENCE)) {
          // Just loop again to silently discard it (accordingly to HDLC)
          continue;
        }

        start_index = src_index;
      }
    } else {
      // Check for end flag sequence
      if (src[i] == YAHDLC_FLAG_SEQUENCE) {
        // Check if an additional flag sequence byte is present or earlier received
        if (((i < (src_len - 1)) && (src[i + 1] == YAHDLC_FLAG_SEQUENCE))
            || ((start_index + 1) == src_index)) {
          // Just loop again to silently discard it (accordingly to HDLC)
          continue;
        }

        end_index = src_index;
        break;
      } else if (src[i] == YAHDLC_CONTROL_ESCAPE) {
        control_escape = 1;
      } else {
        // Update the value based on any control escape received
        if (control_escape) {
          control_escape = 0;
          value = src[i] ^ 0x20;
        } else {
          value = src[i];
        }

        // Now update the FCS value
        fcs = fcs16(fcs, value);

        if (src_index == start_index + 2) {
          // Control field is the second byte after the start flag sequence
          *control = yahdlc_get_control_type(value);
        } else if (src_index > (start_index + 2)) {
          // Start adding the data values after the Control field to the buffer
          dest[dest_index++] = value;
        }
      }
    }
    src_index++;
  }

  // Check for invalid frame (no start or end flag sequence)
  if ((start_index < 0) || (end_index < 0)) {
    // Return no message and make sure destination length is 0
    *dest_len = 0;
    ret = -ENOMSG;
  } else {
    // A frame is at least 4 bytes in size and has a valid FCS value
    if ((end_index < (start_index + 4)) || (fcs != FCS16_GOOD_VALUE)) {
      // Return FCS error and indicate that data up to end flag sequence in buffer should be discarded
      *dest_len = i;
      ret = -EIO;
    } else {
      // Return success and indicate that data up to end flag sequence in buffer should be discarded
      *dest_len = dest_index - sizeof(fcs);
      ret = i;
    }

    // Reset values for next frame
    fcs = FCS16_INIT_VALUE;
    src_index = dest_index = 0;
    start_index = end_index = -1;
  }

  return ret;
}

int yahdlc_frame_data(yahdlc_control_t *control, const char *src,
                      unsigned int src_len, char *dest, unsigned int *dest_len) {
  unsigned int i;
  int dest_index = 0;
  unsigned char value = 0;
  unsigned short fcs = FCS16_INIT_VALUE;

  // Make sure that all parameters are valid
  if (!control || (!src && (src_len > 0)) || !dest || !dest_len) {
    return -EINVAL;
  }

  // Start by adding the start flag sequence
  dest[dest_index++] = YAHDLC_FLAG_SEQUENCE;

  // Add the all-station address from HDLC (broadcast)
  fcs = fcs16(fcs, YAHDLC_ALL_STATION_ADDR);
  yahdlc_escape_value(YAHDLC_ALL_STATION_ADDR, dest, &dest_index);

  // Add the framed control field value
  value = yahdlc_frame_control_type(control);
  fcs = fcs16(fcs, value);
  yahdlc_escape_value(value, dest, &dest_index);

  // Only DATA frames should contain data
  if (control->frame == YAHDLC_FRAME_DATA) {
    // Calculate FCS and escape data
    for (i = 0; i < src_len; i++) {
      fcs = fcs16(fcs, src[i]);
      yahdlc_escape_value(src[i], dest, &dest_index);
    }
  }

  // Invert the FCS value accordingly to the specification
  fcs ^= 0xFFFF;

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
