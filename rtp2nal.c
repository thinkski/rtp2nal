#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

struct __attribute__((packed)) RD_hdr_t {
    uint32_t tv_sec;
    uint32_t tv_usec;
    uint32_t source;
    uint32_t port;
};

struct __attribute__((packed)) RD_packet_t {
    uint16_t length;
    uint16_t plen;
    uint32_t offset;
};

struct __attribute__((packed)) rtphdr_t {
    uint8_t v  : 1;
    uint8_t p  : 1;
    uint8_t x  : 1;
    uint8_t cc : 4;
    uint8_t m  : 1;
    uint8_t pt : 7;
    uint16_t seq;
    uint32_t ts;
    uint32_t ssrc;
};

int main(int argc, char **argv)
{
    {
        char line[64];
        if (!fgets(line, sizeof(line), stdin)) {
            fprintf(stderr, "failed to read rtpplay file format header\n");
            exit(1);
        }

        if (strncmp("#!rtpplay", line, 9)) {
            fprintf(stderr, "error: does not look like an rtpdump file\n");
            exit(1);
        }
    }

    {
        struct RD_hdr_t hdr;

        // read rtpdump header struct
        if (1 != fread(&hdr, sizeof(hdr), 1, stdin)) {
            fprintf(stderr, "error: short header (malformed file?)\n");
            exit(1);
        }
    }

    int seq = -1;

    while (1) {
        struct RD_packet_t pkt;
        char payload[65536];

        // read rtpdump packet struct
        if (1 != fread(&pkt, sizeof(pkt), 1, stdin)) {
            fprintf(stderr, "error: short packet (malformed file?)\n");
            exit(1);
        }

        // read rtp packet
        if (1 != fread(&payload, htons(pkt.plen), 1, stdin)) {
            fprintf(stderr, "error: short payload (malformed file?)\n");
            exit(1);
        }

        struct rtphdr_t *hdr = (struct rtphdr_t *)payload;

//        printf("v=%i p=%i x=%i cc=%i m=%i pt=%i seq=%i ts=%i ssrc=0x%08x\n",
//            hdr->v, hdr->p, hdr->x, hdr->cc, hdr->m, hdr->pt, htons(hdr->seq), htonl(hdr->ts), htonl(hdr->ssrc));

        if (seq >= 0) {
            seq = (seq + 1) & 0xFFFF;
            if (seq != htons(hdr->seq)) {
                fprintf(stderr, "wrong sequence number. missing %i!\n", htons(hdr->seq) - seq);
                seq = htons(hdr->seq);
            }
        } else {
            seq = htons(hdr->seq);
            fprintf(stderr, "init sequence number %i!\n", seq);
        }

        unsigned char *nal = payload + sizeof(struct rtphdr_t);

//        printf("%i: %02x %02x %02x %02x %02x %02x\n", nal[0] & 0x1F, nal[1], nal[2], nal[3], nal[4], nal[5], nal[6]);
//
        int type = nal[0];

        // FU-A
        if (28 == (type & 0x1F)) {
            if (0x20 & nal[1]) {
                fprintf(stderr, "error: fu-a reserve bit not zero\n");
                exit(1);
            }
            if (0x80 & nal[1]) {
                // start bit
                unsigned char typ = (nal[0] & 0xE0) | (nal[1] & 0x1F);
                unsigned char prefix[5] = {0, 0, 0, 1, typ};
                fwrite(prefix, sizeof(prefix), 1, stdout);
            }
            if (0x40 & nal[1]) {
                // end bit
            }
            if (1 != fwrite(nal + 2, htons(pkt.plen) - sizeof(struct rtphdr_t) - 2, 1, stdout)) {
                fprintf(stderr, "error: short write\n");
                exit(1);
            }
        } else {
            unsigned char prefix[4] = {0, 0, 0, 1};
            fwrite(prefix, sizeof(prefix), 1, stdout);
            if (1 != fwrite(nal, htons(pkt.plen) - sizeof(struct rtphdr_t), 1, stdout)) {
                fprintf(stderr, "error: short write\n");
                exit(1);
            }
        }
    }

    return 0;
}
