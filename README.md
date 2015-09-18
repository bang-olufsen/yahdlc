# yahdlc - Yet Another HDLC

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master)

The Yet Another HDLC (yahdlc) implementation is a framing protocol optimized for embedded communication with single pass operations. It uses the HDLC asynchronous framing format. For more information see:

https://en.wikipedia.org/wiki/High-Level_Data_Link_Control

The supported frames are limited to DATA (I-frame with poll bit), ACK (S-frame Receive Ready (RR) with final bit) and NACK (S-frame Reject (REJ) with final bit). All DATA frames must be acknowledged or negative acknowledged using the defined ACK and NACK frames. 

## Repository Layout

Each directory contains source code for parsing the protocol in a specific
language.
