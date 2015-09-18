# yahdlc

[![Build Status](https://travis-ci.org/bang-olufsen/yahdlc.png)](https://travis-ci.org/bang-olufsen/yahdlc) [![Coverage Status](https://coveralls.io/repos/bang-olufsen/yahdlc/badge.svg?branch=master&service=github)](https://coveralls.io/github/bang-olufsen/yahdlc?branch=master)

A framing protocol optimized for embedded communication with a simple and 
efficient encoding and decoding (single pass for each operation). The data
integrity is covered by a 16-bit Frame Check Sequence (CRC) value.


## Repository Layout

Each directory contains source code for parsing the protocol in a specific
language.


## Protocol

The protocol is a simplified version of [RFC 1662](https://tools.ietf.org/html/rfc1662) 
where the Address, Control and Protocol fields have been removed. The frame
format is:

<pre>
| Start Byte | Escaped Data | Escaped 16-bit CRC | End Byte |
</pre>


Each frame starts and ends with the flag sequence value 0x7D. Data is 
immediately following the start flag sequence. After the data is a two 
byte CRC. All data and CRC bytes are escaped.

The control escape value is 0x7E. Whenever the value 0x7E or 0x7D occur 
in the message the value is prefixed by the control escape value and the 
original value XOR with 0x20. As an example, 0x7E in a message would 
become 0x7E 0x5E.
