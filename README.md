# yahdlc - Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master) [![License](https://img.shields.io/github/license/mashape/apistatus.svg)](LICENSE)

The Yet Another High-Level Data Link Control implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with Poll bit), ACK (S-frame Receive Ready with Final bit) and NACK (S-frame Reject with Final bit). All DATA frames must be acknowledged or negative acknowledged using the defined ACK and NACK frames. The Address and Control fields uses the 8-bit format which means that the highest sequence number is 7. The FCS field is 16-bit in size.

Below are some examples on the usage:

```
Acknowledged frame:
A ----> B   DATA [Send sequence number]
A <---- B    ACK [Receive sequence number]

Negative acknowledged frame:
A ----> B   DATA [Send sequence number]
A <---- B   NACK [Receive sequence number]
A ----> B   DATA [Send sequence number]

Acknowledge frame lost:
A ----> B   DATA [Send sequence number]
A  X<-- B    ACK [Receive sequence number]
*Timeout*
A ----> B   DATA [Send sequence number]
```
