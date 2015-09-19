# Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master) [![License](https://img.shields.io/github/license/mashape/apistatus.svg)](LICENSE)

The Yet Another High-Level Data Link Control (yahdlc) implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with Poll bit), ACK (S-frame Receive Ready with Final bit) and NACK (S-frame Reject with Final bit). All DATA frames must be acknowledged or negative acknowledged using the defined ACK and NACK frames. The Control field is 8-bit which mean that the highest sequence number is 7.

Below are some examples on the usage:

```
Acknowledged frame:
A ----> B   DATA [Send Seq No]
A <---- B    ACK [Recv Seq No]

Negative acknowledged frame:
A ----> B   DATA [Send Seq No]
A <---- B   NACK [Recv Seq No]
A ----> B   DATA [Send Seq No]

Acknowledge frame lost:
A ----> B   DATA [Send Seq No]
A  X<-- B    ACK [Recv Seq No]
 Timeout
A ----> B   DATA [Send Seq No]
```
