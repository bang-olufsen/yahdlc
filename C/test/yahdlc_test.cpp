#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE yahdlc
#include <boost/test/unit_test.hpp>
#include "yahdlc.h"

BOOST_AUTO_TEST_CASE(yahdlcTest0To512BytesData) {
  int ret;
  unsigned int i, frame_length = 0, recv_length = 0;
  char send_data[512], frame_data[520], recv_data[520];
  struct yahdlc_control_t control_send, control_recv;

  // Initialize data to be send with random values
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) rand();
  }

  // Run through the different data sizes
  for (i = 0; i <= sizeof(send_data); i++) {
    // Initialize the control field structure
    control_send.frame = YAHDLC_FRAME_DATA;
    control_send.send_seq_no = i;
    control_send.recv_seq_no = i;

    // Create frame which must at least be 6 bytes more than data (escaped characters will increase the length)
    yahdlc_frame_data(control_send, send_data, i, frame_data, &frame_length);
    BOOST_CHECK(frame_length >= (i + 6));

    // Get data from frame. Bytes to be discarded should at least be one byte less than frame length
    ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                          &recv_length);
    BOOST_CHECK(ret >= (int )(frame_length - 1));
    BOOST_CHECK(recv_length == i);

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, i);
    BOOST_CHECK(ret == 0);
    BOOST_CHECK(control_send.frame == control_recv.frame);
    BOOST_CHECK(control_send.send_seq_no == control_recv.send_seq_no);
    BOOST_CHECK(control_send.recv_seq_no == control_recv.recv_seq_no);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTest5BytesFrame) {
  int ret;
  unsigned int recv_length = 0;
  struct yahdlc_control_t control;

  // Create an invalid frame with only one byte of FCS
  char recv_data[8], frame_data[] = { YAHDLC_FLAG_SEQUENCE, (char) 0xFF, 0x10,
      0x33,
      YAHDLC_FLAG_SEQUENCE };

  // Check that the decoded data will return invalid FCS error and 4 bytes to be discarded
  ret = yahdlc_get_data(&control, frame_data, sizeof(frame_data), recv_data,
                        &recv_length);
  BOOST_CHECK(ret == -2);
  BOOST_CHECK(recv_length == (sizeof(frame_data) - 1));
}

BOOST_AUTO_TEST_CASE(yahdlcTestDoubleStartFlagSequenceAndEmptyFrame) {
  int ret;
  char frame_data[8], recv_data[16];
  unsigned int frame_length, recv_length = 0;
  struct yahdlc_control_t control_send, control_recv;

  // Create empty data frame
  control_send.frame = YAHDLC_FRAME_ACK;
  yahdlc_frame_data(control_send, NULL, 0, frame_data, &frame_length);

  // Add an additional start flag sequence at the beginning
  memmove(&frame_data[1], &frame_data[0], frame_length);
  frame_data[0] = YAHDLC_FLAG_SEQUENCE;
  frame_length++;

  // Decoded data should return 6 bytes to be discarded and no bytes received
  ret = yahdlc_get_data(&control_recv, frame_data, frame_length, recv_data,
                        &recv_length);
  BOOST_CHECK(ret == ((int )frame_length - 1));
  BOOST_CHECK(recv_length == 0);
  BOOST_CHECK(control_send.frame == control_recv.frame);
}

BOOST_AUTO_TEST_CASE(yahdlcTestEndFlagSequenceInNewBuffer) {
  int ret;
  struct yahdlc_control_t control;
  unsigned int i, frame_length = -1, recv_length = -1;
  char send_data[16], frame_data[24], recv_data[24];

  // Initialize data to be send with random values
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) rand();
  }

  // Create frame which must at least be 6 bytes more than data (escaped characters will increase the length)
  control.frame = YAHDLC_FRAME_NACK;
  yahdlc_frame_data(control, send_data, sizeof(send_data), frame_data,
                    &frame_length);
  BOOST_CHECK(frame_length >= (sizeof(send_data) + 6));

  // Decode the data up to end flag sequence byte which should return no valid messages error
  ret = yahdlc_get_data(&control, frame_data, frame_length - 1, recv_data,
                        &recv_length);
  BOOST_CHECK(ret == -1);
  BOOST_CHECK(recv_length == 0);

  // Now decode the end flag sequence byte which should result in a decoded frame
  ret = yahdlc_get_data(&control, &frame_data[frame_length - 1], 1, recv_data,
                        &recv_length);
  BOOST_CHECK(ret == 0);
  BOOST_CHECK(recv_length == sizeof(send_data));

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK(ret == 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestFlagSequenceAndControlEscapeInData) {
  int ret;
  struct yahdlc_control_t control;
  unsigned int frame_length = 0, recv_length = 0;
  char send_data[] = { YAHDLC_FLAG_SEQUENCE, YAHDLC_CONTROL_ESCAPE },
      frame_data[16], recv_data[16];

  // Create the frame with the special flag sequence and control escape bytes
  yahdlc_frame_data(control, send_data, sizeof(send_data), frame_data,
                    &frame_length);
  // Length should be frame size (6) + 2 data bytes + 2 escaped characters = 10
  BOOST_CHECK(frame_length == 10);

  // Decode the frame
  ret = yahdlc_get_data(&control, frame_data, frame_length, recv_data,
                        &recv_length);
  BOOST_CHECK(ret == (int )(frame_length - 1));
  BOOST_CHECK(recv_length == sizeof(send_data));

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK(ret == 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestGetDataFromMultipleBuffers) {
  int ret;
  struct yahdlc_control_t control;
  char send_data[512], frame_data[520], recv_data[520];
  unsigned int i, frame_length = 0, recv_length = 0, buf_length = 16;

  // Initialize data to be send with random values
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) rand();
  }

  // Create frame which must at least be 4 bytes more than data (escaped characters will increase the length)
  yahdlc_frame_data(control, send_data, sizeof(send_data), frame_data,
                    &frame_length);
  BOOST_CHECK(frame_length >= (sizeof(send_data) + 4));

  // Run though the different buffers (simulating decode of buffers from UART)
  for (i = 0; i <= sizeof(send_data); i += buf_length) {
    // Decode the data
    ret = yahdlc_get_data(&control, &frame_data[i], buf_length, recv_data,
                          &recv_length);

    if (i < sizeof(send_data)) {
      BOOST_CHECK(ret == -1);
      BOOST_CHECK(recv_length == 0);
    } else {
      // The last chunk should at least give 3 bytes to be discarded (escaped characters will increase the length)
      BOOST_CHECK(ret >= 3);
      BOOST_CHECK(recv_length == sizeof(send_data));
    }
  }

  // Make sure that the data is valid
  ret = memcmp(send_data, recv_data, sizeof(send_data));
  BOOST_CHECK(ret == 0);
}

BOOST_AUTO_TEST_CASE(yahdlcTestMultipleFramesWithSingleFlagSequence) {
  int ret, frame_index = 0;
  struct yahdlc_control_t control;
  char send_data[32], frame_data[512], recv_data[512];
  unsigned int i, frame_length = 0, recv_length = 0, frames = 10;

  // Initialize data to be send with random values
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) rand();
  }

  // Run through the number of frames to be send
  for (i = 0; i < frames; i++) {
    // Create frame which must at least be 4 bytes more than data (escaped characters will increase the length)
    yahdlc_frame_data(control, send_data, sizeof(send_data),
                      &frame_data[frame_index], &frame_length);
    BOOST_CHECK(frame_length >= (sizeof(send_data) + 4));

    // Remove the end flag sequence byte as there must only be one flag sequence byte between frames
    frame_index += frame_length - 1;
  }

  // For the last frame we need the end flag sequence byte
  frame_length = frame_index + 1;
  frame_index = 0;

  // Now decode all the frames
  for (i = 0; i < frames; i++) {
    // Get the data from the frame. Bytes to be discarded should at least be 3 more than send byte size
    ret = yahdlc_get_data(&control, &frame_data[frame_index],
                          frame_length - frame_index, recv_data, &recv_length);
    BOOST_CHECK(ret >= (int )(sizeof(send_data) + 3));
    BOOST_CHECK(recv_length == sizeof(send_data));

    // Increment the number of bytes to be discarded from the frame data (source) buffer
    frame_index += ret;

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, sizeof(send_data));
    BOOST_CHECK(ret == 0);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestMultipleFramesWithDoubleFlagSequence) {
  int ret, frame_index = 0;
  struct yahdlc_control_t control;
  char send_data[32], frame_data[512], recv_data[512];
  unsigned int i, frame_length = 0, recv_length = 0, frames = 10;

  // Initialize data to be send with random values
  for (i = 0; i < sizeof(send_data); i++) {
    send_data[i] = (char) rand();
  }

  // Run through the number of frames to be send
  for (i = 0; i < frames; i++) {
    // Create frame which must at least be 4 bytes more than data (escaped characters will increase the length)
    yahdlc_frame_data(control, send_data, sizeof(send_data),
                      &frame_data[frame_index], &frame_length);
    BOOST_CHECK(frame_length >= (sizeof(send_data) + 4));

    // Do not remove end flag sequence to test the silent discard of this additional byte
    frame_index += frame_length;
  }

  frame_length = frame_index;
  frame_index = 0;

  // Now decode all the frames
  for (i = 0; i < frames; i++) {
    // Get the data from the frame. Bytes to be discarded should at least be 3 more than send byte size
    ret = yahdlc_get_data(&control, &frame_data[frame_index],
                          frame_length - frame_index, recv_data, &recv_length);
    BOOST_CHECK(ret >= (int )(sizeof(send_data) + 3));
    BOOST_CHECK(recv_length == sizeof(send_data));

    // Increment the number of bytes to be discarded from the frame data (source) buffer
    frame_index += ret;

    // Compare the send and received bytes
    ret = memcmp(send_data, recv_data, sizeof(send_data));
    BOOST_CHECK(ret == 0);
  }
}

BOOST_AUTO_TEST_CASE(yahdlcTestFramesWithBitErrors) {
  int ret;
  struct yahdlc_control_t control;
  unsigned int i, frame_length = 0, recv_length = 0;
  char send_data[] = { 0x55 }, frame_data[8], recv_data[8];

  // Run through the bytes in a frame with a single byte of data
  for (i = 0; i < (sizeof(send_data) + 6); i++) {
    // Create the frame
    yahdlc_frame_data(control, send_data, sizeof(send_data), frame_data,
                      &frame_length);
    BOOST_CHECK(frame_length == (sizeof(send_data) + 6));

    // Generate a single bit error in each byte in the frame
    frame_data[i] ^= 1;

    // The first and last buffer will return no stop/end flag sequence. The other data will return invalid FCS
    ret = yahdlc_get_data(&control, frame_data, frame_length, recv_data,
                          &recv_length);
    if ((i == 0) || (i == (frame_length - 1))) {
      BOOST_CHECK(ret == -1);
      BOOST_CHECK(recv_length == 0);
    } else {
      BOOST_CHECK(ret == -2);
      BOOST_CHECK(recv_length == 6);
    }
  }
}
