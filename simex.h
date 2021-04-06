#pragma once
#include "fileio.h"
enum filetype
{
    aiff = 0,
    wave = 1,
    cdxa = 3,
    vag = 4,
    raw = 5,
    n64hybrid = 6,
    pcbank = 7,
    pcpatch = 8,
    pcstream = 9,
    psxbank = 10,
    psxpatch = 11,
    psxstream = 12,
    n64bank = 13,
    n64patch = 14,
    macbank = 15,
    macstream = 16,
    earsvagstream = 17,
    nbapssstream = 18,
    ps2bank = 19,
    ps2stream = 20,
    gcbank = 21,
    gcstream = 22,
    xboxbank = 23,
    xboxstream = 24,
    quicktime = 30,
    mpeg = 34,
    sndstream = 35,
    xenonbank = 36,
    pspbank = 37,
    xenonstream = 38,
    sndplayer = 39,
    pspstream = 40,
    impluseresponse = 41,
    pcbankint = 42,
    pcstreamint = 43,
    xenonbankint = 44,
    xenonstreamint = 45,
    pspbankint = 46,
    pspstreamint = 47,
    ps3bank = 48,
    ps3stream = 49,
    ps3bankint = 50,
    ps3streamint = 51,
    wiibank = 52,
    wiistream = 53,
    wiibankint = 54,
    wiistreamint = 55,
};

enum playlocation
{
    PLAYLOC_MAINCPU = 0x4,
    PLAYLOC_SPU = 0x8,
    PLAYLOC_DS3DHW = 0x10,
    PLAYLOC_IOPCPU = 0x100,
    PLAYLOC_DSP = 0x200,
    PLAYLOC_DS2DHW = 0x400,
    PLAYLOC_RAM = 0x800,
    PLAYLOC_STREAM = 0x1000,
    PLAYLOC_GIGASAMPLE = 0x2000,
};

enum filtertype
{
    loopstart = 30,
    loopend = 31,
    ifnoloopstart = 32,
    ifnoloopend = 33,
    marktouser = 34,
    seekable = 35,
    fileformatversion = 36,
    chunkspersecond = 37,
    cbr = 44,
    vbr = 46,
    playloc = 50,
    priority = 60,
    gigasampleperiod = 61,
    irfsamplesperfftblock = 63,
    irfpercentfreqcut = 64,
    embeduserdata = 92,
    embedtimestretch = 94,
    removeuserdatabyid = 95,
    removeuserdatabyval = 96,
    removealluserdata = 97,
    resample = 120,
    downsampletofit = 122,
    alignloop = 124,
    padloop = 126,
    scaleamplitude = 130,
    fadein = 140,
    fadeout = 142,
    crop = 150,
    nochannelreordering = 166,
    remix = 169,
    copychan = 174,
    cutchan = 175,
    distort = 180,
    reverb = 190,
    resamplefilter = 220
};

struct SABOUT
{
    char unk1;
    char unk2;
    char unk3;
    int unk4;
    char unk5;
    char unk6;
    char unk7;
    int unk8;
    char decodecodecs[12];
    int unk9;
    char unk10;
    char unk11;
    char unk12;
    int unk13;
    char unk14;
    char unk15;
    int unk16;
    char encodecodecs[16];
    int unk17;
    int unk18;
    int maxchannels;
    char commandline[16];
    char name[24];
    int unk19;
    int unk20;
    int unk21;
    int unk22;
    int flags1;
    int flags2;
    int unk23;
};

struct SINSTANCE
{
    int numsounds;
    int filetype;
    int unk1;
    int version;
    STDSTREAM* filestruct;
    STDSTREAM* filestruct2;
    char filename[1024];
    long long fileoffset;
    int* writebuffer;
    int unk2;
    int unk3;
    int unk4;
    int unk5;
    int unk6;
    int unk7;
    int unk8;
};

struct siinfoaiff
{
    int id;
    int position;
    int stringsize;
    char* string;
};

struct markerstruct
{
    int markercount;
    siinfoaiff* markers[200];
};

struct SSOUND
{
    int unk1;
    int unk2;
    int unk3;
    int unk4;
    int unk5;
    int unk6;
    int unk7;
    int unk8;
    int unk9;
    int unk10;
    int unk11;
    int unk12;
    int unk13;
    int unk14;
    int unk15;
    int unk16;
    long long filepos;
    int unk17;
    int unk18;
    int unk19;
    int unk20;
    int unk21;
    int unk22;
    int unk23;
    int unk24;
    short* decodedsamples[64];
    char* timestretchvalues[6];
    char* unk31[6];
    int unk37[6];
    char* unk43[6];
    int unk49[6];
    char unk55;
    char unk56;
    char unk57;
    char unk58;
    int unk59;
    char unk60;
    char unk61;
    char unk62;
    char unk63;
    char fileversion;
    char unk64;
    char unk65;
    char unk66;
    char unk67;
    char unk68;
    char unk69;
    unsigned char unk70;
    char priority;
    unsigned char codec;
    char channel_config;
    char unk71;
    char* scalingtable1;
    char* scalingtable2;
    int unk74;
    short unk75;
    short playloc;
    char* scalingtable3;
    char* scalingtable4;
    int bitrate;
    int sample_rate;
    int num_samples;
    int loopstart;
    int loopend;
    int timestretch;
    float gigasampleperiod;
    short fftblocksamples;
    short percentcut;
    char* userdata[4];
    int userdatasize[4];
    markerstruct* markers[200];
    int markercount;
    int unk279;
    int unk280;
    int unk281;
    int unk282;
    int unk283;
    int unk284;
    int unk285;
    int unk286;
    int unk287;
    int createseektable;
    float chunkspersecond;
    int version;
    char nochannelreorder;
    char unk288;
    char unk289;
    char unk290;
};

struct SINFO
{
    SSOUND* sound[32];
    float chunkrate;
    short masterrandomdetune;
    char soundcount;
    bool chunkrateflag;
};

enum codectype
{
    s16l_int = 0,
    s16b_int = 1,
    s8_int = 2,
    eaxa_int = 3,
    mt_blk = 4,
    vag_blk = 5,
    n64_blk = 6,
    s16b_blk = 7,
    s16l_blk = 8,
    s8_blk = 9,
    eaxa_blk = 10,
    u8_int = 11,
    cdxacodec = 12,
    dvi_int = 13,
    layer1 = 14,
    layer2 = 15,
    layer3 = 16,
    gcadpcm = 18,
    s24l_int = 19,
    xboxadpcm = 20,
    s24b_int = 21,
    mt5_blk = 22,
    ealayer3 = 23,
    xas0_int = 24,
    ealayer30_int = 25,
    atrac3_int = 26,
    atrac3plus_int = 27,
    eaxma = 28,
    xas_int = 29,
    ealayer3_int = 30,
    ealayer3pcm_int = 31,
    ealayer3spike_int = 32,
};

struct SIMEXFILTERABOUTPARAM
{
    const char* name;
    const char* param;
    const char* description;
    double min;
    double max;
    const char** paramstrings;
    int type;
};

struct SIMEXFILTERABOUT
{
    const char* name;
    const char* param;
    const char* description;
    int numparams;
    char unk1;
    char unk2;
    SIMEXFILTERABOUTPARAM* params;
};

struct SIMEXFILTERPARAM
{
    int intparam;
    double doubleparam;
    char stringparam[28];
    void* pointerparam;
    int pointerparamsize;
    int unk;
};

int SIMEX_close(SINSTANCE* inst);
int SIMEX_create(const char* filename, unsigned int filetype, SINSTANCE** instance);
SIMEXFILTERABOUT* SIMEX_filterabout(int type);
int SIMEX_filterssound(SSOUND* sound, int filter, SIMEXFILTERPARAM* param);
int SIMEX_freesinfo(SINFO* info);
const char* SIMEX_getlasterr();
const char* SIMEX_getsamplerepname(unsigned int codec);
int SIMEX_id(const char* filename, long long offset);
int SIMEX_info(SINSTANCE* instance, SINFO** info, int element);
int SIMEX_open(const char* filename, long long fileoffset, int filetype, SINSTANCE** instance);
int SIMEX_read(SINSTANCE* instance, SINFO* info, int element);
int SIMEX_wclose(SINSTANCE* instance);
int SIMEX_write(SINSTANCE* instance, SINFO* info, int element);
