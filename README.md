# rtp2nal

Tool for extracting an Annex B H.264 Network Abstraction Layer (NAL) unit
byte stream from a .rtp file exported by Wireshark

## Quickstart

Build using `gcc`/`clang`:
```
make
```

Capture an RTSP/RTP session in Wireshark, then select and export one of the streams:
![image](https://github.com/user-attachments/assets/6ee98e8e-2063-413e-b1a2-1bad961ba5ac)

Pipe the export through `rtp2nal` to extract the H.264 Annex B NAL byte stream:
```
cat 61459.rtp | ./rtp2nal > 61459.264
```

## Limitations

* Only H.264 (no H.265 support yet)
* Limited to one stream per .rtp file (export each stream as separate .rtp file)
* Only single NAL unit and Fragmentation Unit A (FU-A) payload structures are supported. STAP-A/B, MTAP16/24, FU-B not yet supported.
