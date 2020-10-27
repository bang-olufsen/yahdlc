# yahdlc - Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.svg)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master) [![Scan_Coverity](https://img.shields.io/coverity/scan/12502.svg)](scan) [![License](https://img.shields.io/badge/license-MIT_License-blue.svg?style=flat)](LICENSE)

The Yet Another High-Level Data Link Control implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with Poll bit), ACK (S-frame Receive Ready with Final bit) and NACK (S-frame Reject with Final bit). All DATA frames must be acknowledged or negative acknowledged using the defined ACK and NACK frames. The Address and Control fields uses the 8-bit format which means that the highest sequence number is 7. The FCS field is 16-bit by default, but can be set to 32-bit by the definition of "CRC32".

Below are some examples on the usage:

```
Acknowledge of frames:
A ----> B   DATA [Seq No = 1]
A <---- B   DATA [Seq No = 4]
A <---- B    ACK [Seq No = 2]
A ----> B    ACK [Seq No = 5]

Negative acknowledge of frame:
A ----> B   DATA [Seq No = 1]
A <---- B   NACK [Seq No = 1]
A ----> B   DATA [Seq No = 1]

Acknowledge frame lost:
A ----> B   DATA [Seq No = 1]
A  X<-- B    ACK [Seq No = 2]
*Timeout*
A ----> B   DATA [Seq No = 1]
```

## Programming languages

Currently yahdlc supports C/C++ and Python. Python bindings for yahdlc has been implemented by SkypLabs and can be found here:

https://github.com/SkypLabs/python4yahdlc
