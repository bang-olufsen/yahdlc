#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yahdlc
#include <boost/test/unit_test.hpp>
#include "yahdlc.h"

BOOST_AUTO_TEST_CASE(yahdlcTestFrameDataInvalidInputs) {
  int ret;
  yahdlc_control_t control;
  unsigned int frame_length = 0;
  char send_data[8], frame_data[8];

  // Check invalid control field parameter
  ret = yahdlc_frame_data(NULL, send_data, sizeof(send_data), frame_data,
                          &frame_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid source buffer parameter (positive source buffer length)
  ret = yahdlc_frame_data(&control, NULL, 1, frame_data, &frame_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check that it is possible to create an empty frame
  ret = yahdlc_frame_data(&control, NULL, 0, frame_data, &frame_length);
  BOOST_CHECK_EQUAL(ret, 0);

  // Check invalid destination buffer parameter
  ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), NULL,
                          &frame_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid destination buffer length parameter
  ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
  NULL);
  BOOST_CHECK_EQUAL(ret, -EINVAL);
}

BOOST_AUTO_TEST_CASE(yahdlcTestGetDataInvalidInputs) {
  int ret;
  yahdlc_control_t control;
  unsigned int recv_length = 0;
  char frame_data[8], recv_data[8];

  ret = yahdlc_get_data_with_state(NULL, &control, frame_data, sizeof(frame_data),
                        recv_data, &recv_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid control field parameter
  ret = yahdlc_get_data(NULL, frame_data, sizeof(frame_data), recv_data,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid source buffer parameter
  ret = yahdlc_get_data(&control, NULL, sizeof(frame_data), recv_data,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid destination buffer parameter
  ret = yahdlc_get_data(&control, frame_data, sizeof(frame_data), NULL,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  // Check invalid destination buffer length parameter
  ret = yahdlc_get_data(&control, frame_data, sizeof(frame_data), recv_data,
  NULL);
  BOOST_CHECK_EQUAL(ret, -EINVAL);
}

BOOST_AUTO_TEST_CASE(yahdlcTestSetGetState) {
  int ret;
  yahdlc_state_t state, yahdlc_state;

  state.fcs = 123;

  ret = yahdlc_set_state(NULL);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  ret = yahdlc_set_state(&state);
  BOOST_CHECK_EQUAL(ret, 0);

  ret = yahdlc_get_state(NULL);
  BOOST_CHECK_EQUAL(ret, -EINVAL);

  ret = yahdlc_get_state(&yahdlc_state);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(state.control_escape, yahdlc_state.control_escape);
  BOOST_CHECK_EQUAL(state.fcs, yahdlc_state.fcs);
  BOOST_CHECK_EQUAL(state.start_index, yahdlc_state.start_index);
  BOOST_CHECK_EQUAL(state.end_index, yahdlc_state.end_index);
  BOOST_CHECK_EQUAL(state.src_index, yahdlc_state.src_index);
  BOOST_CHECK_EQUAL(state.dest_index, yahdlc_state.dest_index);
}

BOOST_AUTO_TEST_CASE(yahdlcTestGetDataReset) {
  yahdlc_state_t state, yahdlc_state;

  yahdlc_get_data_reset();
  yahdlc_get_data_reset_with_state(&state);
  yahdlc_get_state(&yahdlc_state);
  BOOST_CHECK_EQUAL(state.control_escape, yahdlc_state.control_escape);
  BOOST_CHECK_EQUAL(state.fcs, yahdlc_state.fcs);
  BOOST_CHECK_EQUAL(state.start_index, yahdlc_state.start_index);
  BOOST_CHECK_EQUAL(state.end_index, yahdlc_state.end_index);
  BOOST_CHECK_EQUAL(state.src_index, yahdlc_state.src_index);
  BOOST_CHECK_EQUAL(state.dest_index, yahdlc_state.dest_index);
}

BOOST_AUTO_TEST_CASE(yahdlcTestDataFrameControlField) {
  int ret;
  char frame_data[8], recv_data[8];
  unsigned int i, frame_length = 0, recv_length = 0;
  yahdlc_control_t control_send, control_recv;

  // Run through the supported sequence numbers (3-bit)
  for (i = 0; i <= 7; i++) {
    // Initialize the control field structure with frame type and sequence number
    control_send.frame = YAHDLC_FRAME_DATA;
    control_send.seq_no = i;

    // Create an empty frame with the control field information
    ret = yahdlc_frame_data(&control_send, NULL, 0, frame_data, &frame_length);
    BOOST_CHECK_EQUAL(ret, 0);

    // Get the data from the frame
    ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                          &recv_length);

    // Result should be frame_length minus start flag to be discarded and no bytes received
    BOOST_CHECK_EQUAL(ret, ((int )frame_length - 1));
    BOOST_CHECK_EQUAL(recv_length, 0);

    // Verify the control field information
    BOOST_CHECK_EQUAL(control_send.frame, control_recv.frame);
    BOOST_CHECK_EQUAL(control_send.seq_no, control_recv.seq_no);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestAckFrameControlField) {
  int ret;
  char frame_data[8], recv_data[8];
  unsigned int i, frame_length = 0, recv_length = 0;
  yahdlc_control_t control_send, control_recv;

  // Run through the supported sequence numbers (3-bit)
  for (i = 0; i <= 7; i++) {
    // Initialize the control field structure with frame type and sequence number
    control_send.frame = YAHDLC_FRAME_ACK;
    control_send.seq_no = i;

    // Create an empty frame with the control field information
    ret = yahdlc_frame_data(&control_send, NULL, 0, frame_data, &frame_length);
    BOOST_CHECK_EQUAL(ret, 0);

    // Get the data from the frame
    ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                          &recv_length);

    // Result should be frame_length minus start flag to be discarded and no bytes received
    BOOST_CHECK_EQUAL(ret, ((int )frame_length - 1));
    BOOST_CHECK_EQUAL(recv_length, 0);

    // Verify the control field information
    BOOST_CHECK_EQUAL(control_send.frame, control_recv.frame);
    BOOST_CHECK_EQUAL(control_send.seq_no, control_recv.seq_no);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestNackFrameControlField) {
  int ret;
  char frame_data[8], recv_data[8];
  unsigned int i, frame_length = 0, recv_length = 0;
  yahdlc_control_t control_send, control_recv;

  // Run through the supported sequence numbers (3-bit)
  for (i = 0; i <= 7; i++) {
    // Initialize the control field structure with frame type and sequence number
    control_send.frame = YAHDLC_FRAME_NACK;
    control_send.seq_no = i;

    // Create an empty frame with the control field information
    ret = yahdlc_frame_data(&control_send, NULL, 0, frame_data, &frame_length);
    BOOST_CHECK_EQUAL(ret, 0);

    // Get the data from the frame
    ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                          &recv_length);

    // Result should be frame_length minus start flag to be discarded and no bytes received
    BOOST_CHECK_EQUAL(ret, ((int )frame_length - 1));
    BOOST_CHECK_EQUAL(recv_length, 0);

    // Verify the control field information
    BOOST_CHECK_EQUAL(control_send.frame, control_recv.frame);
    BOOST_CHECK_EQUAL(control_send.seq_no, control_recv.seq_no);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTest0To512BytesData) {
  int ret;
  unsigned int i, frame_length = 0, recv_length = 0;
  yahdlc_control_t control_send, control_recv;
  char send_data[512], frame_data[520], recv_data[520];

  // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) (rand() % 0x70);
  }

  // Run through the different data sizes
  for (i = 0; i <= sizeof(send_data); i++) {
    // Initialize control field structure and create frame
    control_send.frame = YAHDLC_FRAME_DATA;
    ret = yahdlc_frame_data(&control_send, send_data, i, frame_data,
                            &frame_length);

    // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
    BOOST_CHECK(frame_length <= ((i + 6) + 2));
    BOOST_CHECK_EQUAL(ret, 0);

    // Get the data from the frame
    ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                          &recv_length);

    // Bytes to be discarded should be one byte less than frame length
    BOOST_CHECK_EQUAL(ret, (frame_length - 1));
    BOOST_CHECK_EQUAL(recv_length, i);

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, i);
    BOOST_CHECK_EQUAL(ret, 0);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTest5BytesFrame) {
  int ret;
  unsigned int recv_length = 0;
  yahdlc_control_t control;

  // Create an invalid frame with only one byte of FCS
  char recv_data[8], frame_data[] = { YAHDLC_FLAG_SEQUENCE,
      (char) YAHDLC_ALL_STATION_ADDR, 0x10, 0x33,
      YAHDLC_FLAG_SEQUENCE };

  // Now decode the frame
  ret = yahdlc_get_data(&control, frame_data, sizeof(frame_data), recv_data,
                        &recv_length);

  // Check that the decoded data will return invalid FCS error and 4 bytes to be discarded
  BOOST_CHECK_EQUAL(ret, -EIO);
  BOOST_CHECK_EQUAL(recv_length, (sizeof(frame_data) - 1));
}

BOOST_AUTO_TEST_CASE(yahdlcTestDoubleStartFlagSequenceAndEmptyFrame) {
  int ret;
  char frame_data[8], recv_data[16];
  unsigned int frame_length, recv_length = 0;
  yahdlc_control_t control_send, control_recv;

  // Initialize control field structure
  control_send.frame = YAHDLC_FRAME_DATA;

  // Create an empty frame
  ret = yahdlc_frame_data(&control_send, NULL, 0, frame_data, &frame_length);
  BOOST_CHECK_EQUAL(ret, 0);

  // Add an additional start flag sequence at the beginning
  memmove(&frame_data[1], &frame_data[0], frame_length++);
  frame_data[0] = YAHDLC_FLAG_SEQUENCE;

  // Decode the frame
  ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                        &recv_length);

  // Result should be frame_length minus start flag to be discarded and no bytes received
  BOOST_CHECK_EQUAL(ret, ((int )frame_length - 1));
  BOOST_CHECK_EQUAL(recv_length, 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestEndFlagSequenceInNewBuffer) {
  int ret;
  yahdlc_control_t control;
  char send_data[16], frame_data[24], recv_data[24];
  unsigned int i, frame_length = 0, recv_length = 0;

  // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) (rand() % 0x70);
  }

  // Initialize control field structure and create frame
  control.frame = YAHDLC_FRAME_DATA;
  ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
                          &frame_length);

  // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
  BOOST_CHECK(frame_length <= ((sizeof(send_data) + 6) + 2));
  BOOST_CHECK_EQUAL(ret, 0);

  // Decode the data up to end flag sequence byte which should return no valid messages error
  ret = yahdlc_get_data(&control, frame_data, frame_length - 1, recv_data,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, -ENOMSG);
  BOOST_CHECK_EQUAL(recv_length, 0);

  // Now decode the end flag sequence byte which should result in a decoded frame
  ret = yahdlc_get_data(&control, &frame_data[frame_length - 1], 1, recv_data,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, 0);
  BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestFlagSequenceAndControlEscapeInData) {
  int ret;
  yahdlc_control_t control;
  unsigned int frame_length = 0, recv_length = 0;
  char send_data[] = { YAHDLC_FLAG_SEQUENCE, 0x11, YAHDLC_CONTROL_ESCAPE },
      frame_data[16], recv_data[16];

  // Initialize control field structure and create frame
  control.frame = YAHDLC_FRAME_DATA;
  ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
                          &frame_length);

  // Length should be frame size (6) + 3 data bytes + 2 escaped characters = 11
  BOOST_CHECK_EQUAL(frame_length, 11);
  BOOST_CHECK_EQUAL(ret, 0);

  // Decode the frame
  ret = yahdlc_get_data(&control, frame_data, frame_length, recv_data,
                        &recv_length);
  BOOST_CHECK_EQUAL(ret, (int )(frame_length - 1));
  BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestGetDataFromMultipleBuffers) {
  int ret;
  yahdlc_control_t control;
  char send_data[512], frame_data[530], recv_data[530];
  unsigned int i, frame_length = 0, recv_length = 0, buf_length = 16;

  // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) (rand() % 0x70);
  }

  // Initialize control field structure and create frame
  control.frame = YAHDLC_FRAME_DATA;
  ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
                          &frame_length);

  // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
  BOOST_CHECK(frame_length <= ((sizeof(send_data) + 6) + 2));
  BOOST_CHECK_EQUAL(ret, 0);

  // Run though the different buffers (simulating decode of buffers from UART)
  for (i = 0; i <= sizeof(send_data); i += buf_length) {
    // Decode the data
    ret = yahdlc_get_data(&control, &frame_data[i], buf_length, recv_data,
                          &recv_length);

    if (i < sizeof(send_data)) {
      // All chunks until the last should return no message and zero length
      BOOST_CHECK_EQUAL(ret, -ENOMSG);
      BOOST_CHECK_EQUAL(recv_length, 0);
    } else {
      // The last chunk should return max 6 frame bytes - 1 start flag sequence byte + 2 byte for the possible
      // escaped FCS = 7 bytes
      BOOST_CHECK(ret <= 7);
      BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));
    }
  }

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK_EQUAL(ret, 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestMultipleFramesWithSingleFlagSequence) {
  int ret, frame_index = 0;
  yahdlc_control_t control;
  char send_data[32], frame_data[512], recv_data[512];
  unsigned int i, frame_length = 0, recv_length = 0, frames = 10;

  // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) (rand() % 0x70);
  }

  // Run through the number of frames to be send
  for (i = 0; i < frames; i++) {
    // Initialize control field structure and create frame
    control.frame = YAHDLC_FRAME_DATA;
    ret = yahdlc_frame_data(&control, send_data, sizeof(send_data),
                            &frame_data[frame_index], &frame_length);

    // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
    BOOST_CHECK(frame_length <= ((sizeof(send_data) + 6) + 2));
    BOOST_CHECK_EQUAL(ret, 0);

    // Remove the end flag sequence byte as there must only be one flag sequence byte between frames
    frame_index += frame_length - 1;
  }

  // For the last frame we need the end flag sequence byte
  frame_length = frame_index + 1;
  frame_index = 0;

  // Now decode all the frames
  for (i = 0; i < frames; i++) {
    // Get the data from the frame
    ret = yahdlc_get_data(&control, &frame_data[frame_index],
                          frame_length - frame_index, recv_data, &recv_length);

    // Check that frame length (minus 1 due to missing end byte sequence) to is max 2 bytes larger than data
    // due to escape of FCS value
    BOOST_CHECK(ret <= (int )((sizeof(send_data) + 5) + 2));
    BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));

    // Increment the number of bytes to be discarded from the frame data (source) buffer
    frame_index += ret;

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, sizeof(send_data));
    BOOST_CHECK_EQUAL(ret, 0);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestMultipleFramesWithDoubleFlagSequence) {
  int ret, frame_index = 0;
  yahdlc_control_t control;
  char send_data[32], frame_data[512], recv_data[512];
  unsigned int i, frame_length = 0, recv_length = 0, frames = 10;

  // Initialize data to be send with random values (up to 0x70 to keep below the values to be escaped)
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) (rand() % 0x70);
  }

  // Run through the number of frames to be send
  for (i = 0; i < frames; i++) {
    // Initialize control field structure and create frame
    control.frame = YAHDLC_FRAME_DATA;
    ret = yahdlc_frame_data(&control, send_data, sizeof(send_data),
                            &frame_data[frame_index], &frame_length);

    // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
    BOOST_CHECK(frame_length <= ((sizeof(send_data) + 6) + 2));
    BOOST_CHECK_EQUAL(ret, 0);

    // Do not remove end flag sequence to test the silent discard of this  additional byte
    frame_index += frame_length;
  }

  frame_length = frame_index;
  frame_index = 0;

  // Now decode all the frames
  for (i = 0; i < frames; i++) {
    // Get the data from the frame
    ret = yahdlc_get_data(&control, &frame_data[frame_index],
                          frame_length - frame_index, recv_data, &recv_length);

    // Check that frame length is maximum 2 bytes larger than data due to escape of FCS value
    BOOST_CHECK(ret <= (int )((sizeof(send_data) + 6) + 2));
    BOOST_CHECK_EQUAL(recv_length, sizeof(send_data));

    // Increment the number of bytes to be discarded from the frame data buffer
    frame_index += ret;

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, sizeof(send_data));
    BOOST_CHECK_EQUAL(ret, 0);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestFramesWithBitErrors) {
  int ret;
  yahdlc_control_t control;
  unsigned int i, frame_length = 0, recv_length = 0;
  char send_data[] = { 0x55 }, frame_data[8], recv_data[8];

  // Run through the bytes in a frame with a single byte of data
  for (i = 0; i < (sizeof(send_data) + 6); i++) {
    // Initialize control field structure
    control.frame = YAHDLC_FRAME_DATA;

    // Create the frame
    ret = yahdlc_frame_data(&control, send_data, sizeof(send_data), frame_data,
                            &frame_length);
    BOOST_CHECK_EQUAL(frame_length, (sizeof(send_data) + 6));
    BOOST_CHECK_EQUAL(ret, 0);

    // Generate a single bit error in each byte in the frame
    frame_data[i] ^= 1;

    // Now decode the frame
    ret = yahdlc_get_data(&control, frame_data, frame_length, recv_data,
                          &recv_length);

    // The first and last buffer will return no message. The other data will return invalid FCS
    if ((i == 0) || (i == (frame_length - 1))) {
      BOOST_CHECK_EQUAL(ret, -ENOMSG);
      BOOST_CHECK_EQUAL(recv_length, 0);
    } else {
      BOOST_CHECK_EQUAL(ret, -EIO);
      BOOST_CHECK_EQUAL(recv_length, 6);
    }
  }
}
