/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    From the linux kernel

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ROCK_H
#define ROCK_H 1

/* These structs are used by the system-use-sharing protocol, in which the
   Rock Ridge extensions are embedded.  It is quite possible that other
   extensions are present on the disk, and this is fine as long as they
   all use SUSP */

struct SU_SP {
    unsigned char magic[2];
    unsigned char skip;
};

struct SU_CE {
    char extent[8];
    char offset[8];
    char size[8];
};

struct SU_ER {
    unsigned char len_id;
    unsigned char len_des;
    unsigned char len_src;
    unsigned char ext_ver;
    char data[1];
};

struct RR_RR {
    char flags[1];
};

struct RR_PX {
    char mode[8];
    char n_links[8];
    char uid[8];
    char gid[8];
    char serno[8];
};

struct RR_PN {
    char dev_high[8];
    char dev_low[8];
};


struct SL_component {
    unsigned char flags;
    unsigned char len;
    char text[1];
};

struct RR_SL {
    unsigned char flags;
    struct SL_component link;
};

struct RR_NM {
    unsigned char flags;
    char name[1];
};

struct RR_CL {
    char location[8];
};

struct RR_PL {
    char location[8];
};

struct stamp {
    char time[7];
};

struct RR_TF {
    char flags;
    struct stamp times[1];  /* Variable number of these beasts */
};

/* Linux-specific extension for transparent decompression */
struct RR_ZF {
    char algorithm[2];
    char parms[2];
    char real_size[8];
};

/* These are the bits and their meanings for flags in the TF structure. */
#define TF_CREATE 1
#define TF_MODIFY 2
#define TF_ACCESS 4
#define TF_ATTRIBUTES 8
#define TF_BACKUP 16
#define TF_EXPIRATION 32
#define TF_EFFECTIVE 64
#define TF_LONG_FORM 128

struct rock_ridge {
    char signature[2];
    char len; /* 711 */
    char version; /* 711 */
    union {
        struct SU_SP SP;
        struct SU_CE CE;
        struct SU_ER ER;
        struct RR_RR RR;
        struct RR_PX PX;
        struct RR_PN PN;
        struct RR_SL SL;
        struct RR_NM NM;
        struct RR_CL CL;
        struct RR_PL PL;
        struct RR_TF TF;
        struct RR_ZF ZF;
    } u;
};

#define RR_PX 1   /* POSIX attributes */
#define RR_PN 2   /* POSIX devices */
#define RR_SL 4   /* Symbolic link */
#define RR_NM 8   /* Alternate Name */
#define RR_CL 16  /* Child link */
#define RR_PL 32  /* Parent link */
#define RR_RE 64  /* Relocation directory */
#define RR_TF 128 /* Timestamps */

#endif /* ROCK_H */
