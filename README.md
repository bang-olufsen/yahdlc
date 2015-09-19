# Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master) [![MIT license](http://img.shields.io/badge/license-MIT-brightgreen.svg)](http://opensource.org/licenses/MIT)

The Yet Another High Level Data Link Control (yahdlc) implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with Poll bit), ACK (S-frame Receive Ready with Final bit) and NACK (S-frame Reject with Final bit). All DATA frames must be acknowledged or negative acknowledged using the defined ACK and NACK frames. Below are some examples:

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

## Repository Layout

Each directory contains source code for parsing the protocol in a specific
language.
