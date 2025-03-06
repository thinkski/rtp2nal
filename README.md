# rtp2nal

Tool for extracting an Annex B H.264 Network Abstraction Layer (NAL) unit
byte stream from a .rtp file exported by Wireshark

## Quickstart

```
cat stream0.rtp | ./rtp2nal > stream0.264
```
