# yahdlc - Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master) [![License](https://img.shields.io/badge/license-MIT_License-blue.svg?style=flat)](LICENSE)

The Yet Another High-Level Data Link Control implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with Poll/Final bit depending on valid receive sequence number), ACK (S-frame Receive Ready with Final bit) and NACK (S-frame Reject with Final bit). The Address and Control fields uses the 8-bit format which means that the highest sequence number is 7. The FCS field is 16-bit in size.

Below are some examples on the usage:

```
Acknowledge using ACK frame:
A ----> B   DATA [Poll, Send Seq No]
A <---- B    ACK [Final, Recv Seq No]

Acknowledge using DATA frame:
A ----> B   DATA [Poll, Send Seq No]
A <---- B   DATA [Final, Send Seq No, Recv Seq No]

Negative acknowledge using NACK frame:
A ----> B   DATA [Poll, Send Seq No]
A <---- B   NACK [Final, Recv Seq No]
A ----> B   DATA [Poll, Send Seq No]

Acknowledge frame lost:
A ----> B   DATA [Poll, Send Seq No]
A  X<-- B    ACK [Final, Recv Seq No]
*Timeout*
A ----> B   DATA [Poll, Send Seq No]
```
