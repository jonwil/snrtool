#include <string.h>
#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmreg.h>
#include "simex.h"
#include "eabits.h"
#include "fileio.h"
char simexerror[256];

void SIMEX_seterror(const char* error)
{
    strcpy_s(simexerror, error);
}

struct SimexFuncs
{
    int(* aboutfunc)(SABOUT* about);
    int(* idfunc)(const char* filename, long long offset, STDSTREAM* stream);
    int(* openfunc)(SINSTANCE* instance);
    int(* infofunc)(SINSTANCE* instance, SINFO** info, int element);
    int(* readfunc)(SINSTANCE* instance, SINFO* info, int element);
    int(* closefunc)(SINSTANCE* instance);
    int(* createfunc)(SINSTANCE* instance);
    int(* writefunc)(SINSTANCE* instance, SINFO* info, int element);
    int(* wclosefunc)(SINSTANCE* instance);
    int isvalid;
};

int waveabout(SABOUT* about)
{
    about->flags1 |= 3u;
    about->unk1 = -1;
    about->unk5 = -1;
    about->decodecodecs[3] = -1;
    about->unk10 = -1;
    about->unk14 = -1;
    about->encodecodecs[2] = -1;
    about->decodecodecs[0] = s24l_int;
    about->decodecodecs[1] = s16l_int;
    about->encodecodecs[0] = s16l_int;
    about->maxchannels = 64;
    about->decodecodecs[2] = u8_int;
    about->encodecodecs[1] = u8_int;
    about->unk17 = 1;
    about->unk18 = 1;
    strcpy_s(about->commandline, "wave");
    strcpy_s(about->name, "Microsoft Wave");
    return 1;
}

void gputi(unsigned char* ptr, unsigned int data, int bytes)
{
    for (int i = bytes; i; --i)
    {
        *ptr++ = data;
        data >>= 8;
    }
}

void gputm(unsigned char* ptr, unsigned int data, int bytes)
{
    for (unsigned int i = data << (8 * (4 - bytes)); bytes; --bytes)
    {
        *ptr++ = (unsigned char)(i >> 24);
        i <<= 8;
    }
}

unsigned int ggeti(unsigned char* src, int bytes)
{
    int shift = 0;
    unsigned int data = 0;
    for (int i = bytes; i; i--)
    {
        data += *src << shift;
        shift += 8;
        src++;
    }
    return data;
}

int waveid(const char* filename, long long offset, STDSTREAM* stream)
{
    if (!stream)
    {
        return 0;
    }
    char buf1[12];
    if (gread(stream, buf1, 12) && !strncmp(buf1, "RIFF", 4) && !strncmp(&buf1[8], "WAVE", 4))
    {
        unsigned char buf2[8];
        if (gread(stream, buf2, 8))
        {
            while (strncmp((char *)buf2, "fmt ", 4))
            {
                int a = ggeti(&buf2[4], 4);
                gseek(stream, a + stream->filepos);
                if (!gread(stream, buf2, 8))
                {
                    goto l1;
                }
            }
            gseek(stream, stream->filepos - 8);
        }
    l1:
        unsigned char buf3[24];
        if (gread(stream, buf3, 24))
        {
            int b = ggeti(&buf3[8], 2);
            if (b == WAVE_FORMAT_PCM)
            {
                return 100;
            }
            if (b == WAVE_FORMAT_EXTENSIBLE)
            {
                return 100;
            }
        }
    }
    return 0;
}

int waveopen(SINSTANCE* instance)
{
    return 1;
}

void initssound(SSOUND* sound)
{
    memset(sound, 0, sizeof(SSOUND));
    sound->filepos = 0xFFFFFFFFFFFFFFFFui64;
    sound->unk56 = 127;
    sound->unk58 = 127;
    sound->unk66 = 60;
    sound->unk60 = 64;
    sound->unk63 = 127;
    sound->unk70 = 0xFF;
    sound->unk65 = 127;
    sound->unk67 = 64;
    sound->codec = 0xFF;
    sound->loopstart = 0xFFFFFFFF;
    sound->loopend = 0xFFFFFFFF;
    sound->unk1 = 0x7FFFFFFF;
    sound->bitrate = 50;
    sound->unk2 = 127;
    sound->unk62 = 1;
    sound->unk69 = 1;
    sound->unk71 = 1;
    sound->unk19 = 0xFFFFFFFF;
    sound->unk20 = 0xFFFFFFFF;
    sound->unk21 = 0xFFFFFFFF;
    sound->unk22 = 0xFFFFFFFF;
    sound->unk23 = 0xFFFFFFFF;
    sound->unk24 = 0xFFFFFFFF;
    sound->chunkspersecond = -1.0;
    sound->unk279 = 0;
    sound->unk280 = 0;
    sound->unk281 = 0;
    sound->unk282 = 0;
    sound->unk283 = 0;
    sound->unk284 = 0;
    sound->percentcut = 0;
    sound->nochannelreorder = 0;
    sound->fftblocksamples = 1024;
    sound->version = 0xFFFFFFFF;
}

void initsinfo(SINFO* info)
{
    memset(info, 0, sizeof(SINFO));
    info->chunkrate = 15.0;
    info->chunkrateflag = true;
    info->soundcount = 1;
    info->sound[0] = new SSOUND;
    initssound(info->sound[0]);
    info->sound[1] = 0;
    info->sound[2] = 0;
    info->sound[3] = 0;
    info->sound[4] = 0;
    info->sound[5] = 0;
    info->sound[6] = 0;
    info->sound[7] = 0;
    info->sound[8] = 0;
    info->sound[9] = 0;
    info->sound[10] = 0;
    info->sound[11] = 0;
    info->sound[12] = 0;
    info->sound[13] = 0;
    info->sound[14] = 0;
    info->sound[15] = 0;
    info->sound[16] = 0;
    info->sound[17] = 0;
    info->sound[18] = 0;
    info->sound[19] = 0;
    info->sound[20] = 0;
    info->sound[21] = 0;
    info->sound[22] = 0;
    info->sound[23] = 0;
    info->sound[24] = 0;
    info->sound[25] = 0;
    info->sound[26] = 0;
    info->sound[27] = 0;
    info->sound[28] = 0;
    info->sound[29] = 0;
    info->sound[30] = 0;
    info->sound[31] = 0;
}

long long findchunk(STDSTREAM* file, const char* chunk, int* value)
{
    int len = file->filesize;
    long long pos = file->filepos;
    int i1 = 0;
    *value = 0;
    gseek(file, 0);
    unsigned char buf[8];
    while (i1 < len && gread(file, buf, 8) >= 8)
    {
        if (!strncmp((char *)buf, chunk, 4))
        {
            if (strncmp((char *)buf, "data", 4) || ggeti(&buf[4], 4))
            {
                gseek(file, i1);
                return i1;
            }
            i1 += 8;
            (*value)++;
            gseek(file, i1);
        }
        else
        {
            if (!strncmp((char *)buf, "RIFF", 4))
            {
                i1 += 12;
            }
            else
            {
                i1 += ggeti(&buf[4], 4) + 8;
            }
            gseek(file, i1);
        }
    }
    gseek(file, pos);
    return -1;
}

int waveinfo(SINSTANCE* instance, SINFO** info, int element)
{
    *info = 0;
    if (element)
    {
        SIMEX_seterror("An attempt to get info from an element other than 0 was made.");
        return 0;
    }
    gseek(instance->filestruct, 0i64);
    char buf1[12];
    if (!gread(instance->filestruct, buf1, 12))
    {
        SIMEX_seterror("Couldn't read from file.");
        return 0;
    }
    unsigned char buf2[8];
    if (gread(instance->filestruct, buf2, 8))
    {
        while (strncmp((char *)buf2, "fmt ", 4u))
        {
            int a = ggeti(&buf2[4], 4);
            gseek(instance->filestruct, a + instance->filestruct->filepos);
            if (!gread(instance->filestruct, buf2, 8))
            {
                goto l1;
            }
        }
        gseek(instance->filestruct, instance->filestruct->filepos - 8i64);
    }
l1:
    int filepos = instance->filestruct->filepos;
    unsigned char buf3[24];
    if (!gread(instance->filestruct, buf3, 24))
    {
        SIMEX_seterror("Couldn't read from file.");
        return 0;
    }
    int b = ggeti(&buf3[8], 2);
    if (b == WAVE_FORMAT_PCM)
    {
        goto l2;
    }
    if (b != WAVE_FORMAT_EXTENSIBLE)
    {
        SIMEX_seterror("Unrecognized wave format tag.");
        return 0;
    }
    gseek(instance->filestruct, filepos);
    if (gread(instance->filestruct, buf3, 48))
    {
    l2:
        SINFO *s = new SINFO;
        *info = s;
        initsinfo(s);
        SSOUND *sound = s->sound[0];
        sound->channel_config = (char)ggeti(&buf3[10], 2);
        int value = 0;
        long long smpl = findchunk(instance->filestruct, "smpl", &value);
        if (smpl >= 0)
        {
            gseek(instance->filestruct, smpl);
            unsigned char buf4[44];
            if (!gread(instance->filestruct, buf4, 44))
            {
                goto l3;
            }
            int c = ggeti(&buf4[36], 4);
            if (c)
            {
                unsigned char buf5[24];
                if (!gread(instance->filestruct, buf5, 24))
                {
                    goto l3;
                }
                sound->loopstart = ggeti(&buf5[8], 4);
                sound->loopend = ggeti(&buf5[12], 4);
            }
        }
        value = 0;
        long long data = findchunk(instance->filestruct, "data", &value);
        if (data < 0)
        {
            SIMEX_seterror("Couldn't find data chunk.");
            return 0;
        }
        gseek(instance->filestruct, data);
        if (gread(instance->filestruct, buf2, 8))
        {
            sound->sample_rate = ggeti(&buf3[12], 4);
            sound->num_samples = ggeti(&buf2[4], 4) / sound->channel_config;
            short bps = ggeti(&buf3[22], 2);
            if (bps == 8)
            {
                sound->codec = u8_int;
            }
            else
            {
                if (bps == 16)
                {
                    sound->codec = s16l_int;
                    sound->num_samples = sound->num_samples / 2;
                }
                else
                {
                    if (bps != 24)
                    {
                        SIMEX_seterror("Unsupported bits per sample in Wave importer.");
                        return 0;
                    }
                    sound->codec = s24l_int;
                    sound->num_samples = sound->num_samples / 3;
                }
            }
            sound->filepos = instance->filestruct->filepos;
            return 1;
        }
    }
l3:
    SIMEX_seterror("Couldn't read from file.");
    return 0;
}

void reorderchannels(SSOUND* sound)
{
    unsigned char numchannels = sound->channel_config;
    if (numchannels == 4)
    {
        short* p1 = sound->decodedsamples[2];
        sound->decodedsamples[2] = sound->decodedsamples[3];
        sound->decodedsamples[3] = p1;
    }
    if (numchannels == 6)
    {
        short* p2 = sound->decodedsamples[1];
        sound->decodedsamples[1] = sound->decodedsamples[2];
        short* p3 = sound->decodedsamples[5];
        sound->decodedsamples[2] = p2;
        short* p4 = sound->decodedsamples[3];
        sound->decodedsamples[3] = p3;
        sound->decodedsamples[5] = sound->decodedsamples[4];
        sound->decodedsamples[4] = p4;
    }
}

int allocateinputsamples(SSOUND* sound)
{
    for (int i = 0; i < sound->channel_config; i++)
    {
        sound->decodedsamples[i] = new short[sound->num_samples];
    }
    return 0;
}

int decode_s16l_int(STDSTREAM* file, int numchannels, int firstsample, int samples, short** outsamples)
{
    for (int i = firstsample; i <= samples; i++)
    {
        for (int j = 0; j < numchannels; j++)
        {
            unsigned char buf[2];
            if (!gread(file, &buf, 2))
            {
                SIMEX_seterror("Couldn't read 16-bit samples from file.");
                return 0;
            }
            outsamples[j][i] = ggeti(buf, 2);
        }
    }
    return 1;
}

int decode_u8_int(STDSTREAM* file, int numchannels, int firstsample, int samples, short** outsamples)
{
    for (int i = firstsample; i < samples; i++)
    {
        for (int j = 0; j < numchannels; j++)
        {
            unsigned char buf;
            if (!gread(file, &buf, 1))
            {
                SIMEX_seterror("Couldn't read 8-bit samples from file.");
                return 0;
            }
            outsamples[j][i] = (buf - 128) << 8;
        }
    }
    return 1;
}

int decode_s24l_int(STDSTREAM* file, int numchannels, int firstsample, int samples, short** outsamples)
{
    for (int i = 0; i <= samples; i++)
    {
        for (int j = 0; j < numchannels; j++)
        {
            unsigned char buf[3];
            if (!gread(file, &buf, 3))
            {
                SIMEX_seterror("Couldn't read 16-bit samples from file.");
                return 0;
            }
            outsamples[j][i] = buf[1] | (unsigned short)(buf[2] << 8);
        }
    }
    return 1;
}

int uncompressdata(STDSTREAM* file, long long* filepos, int firstsample, int numsamples, unsigned int numchannels, unsigned int codec, short** outsamples, SSOUND* sound)
{
    int samples = firstsample + numsamples - 1;
    if (samples <= 0)
    {
        return 1;
    }
    gseek(file, *filepos);
    switch (codec)
    {
    case s16l_int:
        return decode_s16l_int(file, numchannels, firstsample, samples, outsamples);
    case u8_int:
        return decode_u8_int(file, numchannels, firstsample, samples, outsamples);
    case s24l_int:
        return decode_s24l_int(file, numchannels, firstsample, samples, outsamples);
    }
    SIMEX_seterror("Unsupported sample import format.");
    return 0;
}

int waveread(SINSTANCE* instance, SINFO* info, int element)
{
    long long filepos = 0;
    int value = 0;
    SSOUND* sound = info->sound[0];
    if (allocateinputsamples(sound) >= 0)
    {
        value = 0;
        long long smpl = findchunk(instance->filestruct, "smpl", &value);
        if (smpl >= 0)
        {
            gseek(instance->filestruct, smpl);
            unsigned char buf1[44];
            if (!gread(instance->filestruct, buf1, 44))
            {
            l1:
                SIMEX_seterror("Couldn't read from file.");
                return 0;
            }
            unsigned int c = ggeti(&buf1[36], 4);
            if (c)
            {
                unsigned char buf2[24];
                if (!gread(instance->filestruct, buf2, 24))
                {
                    goto l1;
                }
                sound->loopstart = ggeti(&buf2[8], 4);
                sound->loopend = ggeti(&buf2[12], 4);
            }
        }
        value = 0;
        long long data = findchunk(instance->filestruct, "data", &value);
        if (data < 0)
        {
            SIMEX_seterror("Couldn't find data chunk.");
            return 0;
        }
        gseek(instance->filestruct, data);
        char buf3[8];
        if (!gread(instance->filestruct, buf3, 8))
        {
            goto l1;
        }
        filepos = instance->filestruct->filepos;
        bool reorder = sound->nochannelreorder == 0;
        SSOUND sound2;
        memcpy(&sound2, sound, sizeof(sound2));
        if (reorder)
        {
            reorderchannels(sound);
        }
        value = uncompressdata(instance->filestruct, &filepos, 0, sound->num_samples, sound->channel_config, sound->codec, sound->decodedsamples, sound);
        memcpy(sound, &sound2, sizeof(SSOUND));
    }
    return value;
}

int waveclose(SINSTANCE* instance)
{
    return 1;
}

int wavecreate(SINSTANCE* instance)
{
    instance->filestruct = gwopen(instance->filename);
    if (instance->filestruct)
    {
        return 1;
    }
    SIMEX_seterror("Error opening file in SIMEX_open().");
    return 0;
}

int encodes16l_int(SINSTANCE* instance, STDSTREAM* file, int firstsample, int samples, SSOUND* sound)
{
    for (int i = firstsample; i <= samples; i++)
    {
        for (int j = 0; j < sound->channel_config; j++)
        {
            unsigned char buf[2];
            gputi(buf, sound->decodedsamples[j][i], 2);
            gwrite(file, buf, 2);
        }
    }
    return -1;
}

int encodeu8_int(SINSTANCE* instance, STDSTREAM* file, int firstsample, int samples, SSOUND* sound)
{
    int i1 = firstsample;
    for (int i = firstsample; i1 <= samples; i = i1)
    {
        for (int j = 0; j < sound->channel_config; j++)
        {
            unsigned char buf;
            buf = (char)(sound->decodedsamples[j][i] >> 8);
            gwrite(file, &buf, 2);
        }
    }
    return -1;
}

int compressdata2(SINSTANCE* instance, STDSTREAM* file, int firstsample, int num_samples, SSOUND* sound)
{
    int result = 1;
    if (num_samples)
    {
        int samples = firstsample + num_samples - 1;
        switch (sound->codec)
        {
        case s16l_int:
            result = encodes16l_int(instance, file, firstsample, samples, sound);
            break;
        case u8_int:
            result = encodeu8_int(instance, file, firstsample, samples, sound);
            break;
        default:
            SIMEX_seterror("Unsupported samplerep in export samples.");
            result = 0;
        }
    }
    return result;
}

int compressdata(SINSTANCE* instance, STDSTREAM* file, int firstsample, int num_samples, SSOUND* sound)
{
    int result = compressdata2(instance, file, firstsample, num_samples, sound);
    if (result == -1)
    {
        result = 1;
    }
    return result;
}

int wavewrite(SINSTANCE* instance, SINFO* info, int element)
{
    int bps = 0;
    int samples = 0;
    SSOUND* sound = info->sound[0];
    if (element)
    {
        SIMEX_seterror("WAVE files can only contain element 0.");
        return samples;
    }
    gseek(instance->filestruct, 0i64);
    unsigned char buf1[12];
    if (!gwrite(instance->filestruct, buf1, 12))
    {
        goto l1;
    }
    unsigned char buf2[48];
    gputi(&buf2[8], 1, 2);
    gputi(&buf2[10], sound->channel_config, 2);
    gputi(&buf2[12], sound->sample_rate, 4);
    if (sound->codec)
    {
        if (sound->codec == u8_int)
        {
            bps = 8;
        }
        else
        {
            SIMEX_seterror("Unsupported bits per sample in Wave exporter.");
        }
    }
    else
    {
        bps = 16;
    }
    gputi(&buf2[16], sound->sample_rate * (bps >> 3) * sound->channel_config, 4);
    gputi(&buf2[20], (bps >> 3) * sound->channel_config, 2);
    gputi(&buf2[22], bps, 2);
    if (sound->channel_config <= 2)
    {
        if (gwrite(instance->filestruct, buf2, 24))
        {
            goto l3;
        }
    l1:
        SIMEX_seterror("Couldn't write to file stream.");
        return samples;
    }
    gputi(&buf2[8], WAVE_FORMAT_EXTENSIBLE, 2);
    gputi(&buf2[24], 22, 2);
    gputi(&buf2[26], bps, 2);
    switch (sound->channel_config)
    {
    case 8:
        gputi(&buf2[28], 255, 4);
        break;
    case 6:
        gputi(&buf2[28], 63, 4);
        break;
    case 4:
        gputi(&buf2[28], 51, 4);
        break;
    default:
        SIMEX_seterror("Invalid number of channels in Wave exporter.  Only 1, 2, 4 or 6 are allowed.");
        break;
    }
    buf2[35] = 0;
    buf2[37] = 0;
    buf2[42] = 0;
    buf2[44] = 0;
    if (*(short*)&buf2[26] == bps)
    {
        buf2[32] = 1;
        buf2[33] = 0;
        buf2[34] = 0;
        buf2[36] = 0;
        buf2[38] = 16;
        buf2[39] = 0;
        buf2[40] = 128;
        buf2[41] = 0;
        buf2[43] = 170;
        buf2[45] = 56;
        buf2[46] = 155;
        buf2[47] = 113;
    }
    else
    {
        buf2[32] = 113;
        buf2[33] = 155;
        buf2[34] = 56;
        buf2[36] = 170;
        buf2[38] = 0;
        buf2[39] = 128;
        buf2[40] = 0;
        buf2[41] = 16;
        buf2[43] = 0;
        buf2[45] = 0;
        buf2[46] = 0;
        buf2[47] = 1;
    }
    if (!gwrite(instance->filestruct, buf2, 48))
    {
        goto l1;
    }
l3:
    if (sound->loopstart >= 0 && sound->loopend >= 0)
    {
        unsigned char buf3[44];
        memcpy((char *)buf3, "smpl", 4);
        gputi(&buf3[4], 60, 4);
        gputi(&buf3[8], 0, 4);
        gputi(&buf3[12], 0, 4);
        long long i1 = (long long)(1.0 / (double)sound->sample_rate * 1000000000.0);
        gputi(&buf3[16], (int)i1, 4);
        gputi(&buf3[20], 60, 4);
        gputi(&buf3[24], 0, 4);
        gputi(&buf3[28], 0, 4);
        gputi(&buf3[32], 0, 4);
        gputi(&buf3[36], 1, 4);
        gputi(&buf3[40], 0, 4);
        if (!gwrite(instance->filestruct, buf3, 44))
        {
            goto l1;
        }
        unsigned char buf4[24];
        gputi(buf4, 0, 4);
        gputi(&buf4[4], 0, 4);
        gputi(&buf4[8], sound->loopstart, 4);
        gputi(&buf4[12], sound->loopend, 4);
        gputi(&buf4[16], 0, 4);
        gputi(&buf4[20], 0, 4);
        if (!gwrite(instance->filestruct, buf4, 24))
        {
            goto l1;
        }
    }
    unsigned char buf5[8];
    memcpy((char *)buf5, "data", 4);
    gputi(&buf5[4], (bps >> 3) * sound->num_samples * sound->channel_config, 4);
    if (!gwrite(instance->filestruct, buf5, 8))
    {
        goto l1;
    }
    bool reorder = sound->nochannelreorder == 0;
    SSOUND sound2;
    memcpy(&sound2, sound, sizeof(sound2));
    if (reorder)
    {
        reorderchannels(sound);
    }
    samples = compressdata(instance, instance->filestruct, 0, sound->num_samples, sound);
    memcpy(sound, &sound2, sizeof(SSOUND));
    if (samples)
    {
        int pos = instance->filestruct->filepos;
        gseek(instance->filestruct, 0i64);
        memcpy((char *)buf1, "RIFF", 4);
        gputi(&buf1[4], pos - 8, 4);
        memcpy((char *)&buf1[8], "WAVE", 4);
        if (!gwrite(instance->filestruct, buf1, 12)
            || ((memcpy((char *)buf5, "fmt ", 4), sound->channel_config <= 2) ? gputi(&buf5[4], 16, 4) : gputi(&buf5[4], 40, 4),
                !gwrite(instance->filestruct, buf5, 8)))
        {
            SIMEX_seterror("Couldn't write to file stream.");
            samples = 0;
        }
    }
    return samples;
}

int wavewclose(SINSTANCE* instance)
{
    int i = 1;
    if (instance->filestruct)
    {
        if (!gclose(instance->filestruct))
        {
            SIMEX_seterror("Error closing file in SIMEX_wclose().");
            i = 0;
        }
        instance->filestruct = 0;
    }
    return i;
}

int sndplayerabout(SABOUT* about)
{
    int result = 1;
    about->unk1 = 0;
    about->unk2 = 1;
    about->unk3 = -1;
    about->unk5 = -1;
    about->decodecodecs[0] = xas_int;
    about->decodecodecs[1] = xas0_int;
    about->decodecodecs[2] = ealayer3_int;
    about->decodecodecs[3] = ealayer30_int;
    about->decodecodecs[4] = s16b_int;
    about->decodecodecs[5] = eaxma;
    about->decodecodecs[6] = ealayer3pcm_int;
    about->decodecodecs[7] = ealayer3spike_int;
    about->decodecodecs[8] = gcadpcm;
    about->decodecodecs[9] = -1;
    about->unk10 = 0;
    about->unk11 = 1;
    about->unk12 = -1;
    about->unk14 = -1;
    about->encodecodecs[0] = xas_int;
    about->encodecodecs[1] = s16b_int;
    about->encodecodecs[2] = eaxma;
    about->encodecodecs[3] = gcadpcm;
    about->encodecodecs[4] = -1;
    about->encodecodecs[4] = ealayer3_int;
    about->encodecodecs[5] = ealayer3pcm_int;
    about->encodecodecs[6] = ealayer3spike_int;
    about->encodecodecs[7] = -1;
    about->flags1 |= 0x80900003;
    about->flags2 |= 3u;
    about->unk17 = 1;
    about->unk18 = 1;
    about->maxchannels = 64;
    strcpy_s(about->commandline, "sndplayer");
    strcpy_s(about->name, "SndPlayer Plug-In Asset");
    return result;
}

class BitGetter
{
public:
    BitGetter() {}
    ~BitGetter() {}
    void SetBitBuffer(void* pBuffer)
    {
        mpBitBuffer = (unsigned char*)pBuffer;
        mBitPosition = 0;
    }
    unsigned int GetBitPosition() { return mBitPosition; }
    unsigned int GetBits(unsigned int numBits)
    {
        unsigned int i1 = 0;
        if (!numBits)
        {
            return 0;
        }
        do
        {
            unsigned int i2 = mBitPosition & 7;
            unsigned int i3 = 8 - i2;
            if (8 - i2 > numBits)
            {
                i3 = numBits;
            }
            unsigned int i4 = mpBitBuffer[mBitPosition >> 3] >> (8 - i2 - i3);
            mBitPosition += i3;
            i1 = (i1 << i3) | ((1 << i3) - 1) & i4;
            numBits -= i3;
        } while (numBits);
        return i1;
    }
private:
    unsigned char* mpBitBuffer;
    unsigned int mBitPosition;
};

enum SndPlayerCodec
{
    SNDPLAYER_CODEC_XAS_INT = 0x0,
    SNDPLAYER_CODEC_EALAYER3_INT = 0x1,
    SNDPLAYER_CODEC_SIGN16BIG_INT = 0x2,
    SNDPLAYER_CODEC_EAXMA = 0x3,
    SNDPLAYER_CODEC_XAS1_INT = 0x4,
    SNDPLAYER_CODEC_EALAYER31_INT = 0x5,
    SNDPLAYER_CODEC_EALAYER32PCM_INT = 0x6,
    SNDPLAYER_CODEC_EALAYER32SPIKE_INT = 0x7,
    SNDPLAYER_CODEC_GCADPCM = 0x8,
    SNDPLAYER_CODEC_EASPEEX = 0x9,
    SNDPLAYER_CODEC_MAX = 0xA,
};

enum SndPlayerPlayType
{
    SNDPLAYER_PLAYTYPE_RAM = 0x0,
    SNDPLAYER_PLAYTYPE_STREAM = 0x1,
    SNDPLAYER_PLAYTYPE_GIGASAMPLE = 0x2,
    SNDPLAYER_PLAYTYPE_MAX = 0x3,
};

struct SndPlayerHeader
{
    int version;
    int codec;
    int channel_config;
    int sample_rate;
    int playloc;
    int loop_flag;
    int num_samples;
    int loop_start;
    int prefech_samples;
    int loop_offset;
    SndPlayerHeader() : version(-1), codec(SNDPLAYER_CODEC_XAS_INT), channel_config(0), sample_rate(0), playloc(SNDPLAYER_PLAYTYPE_RAM), loop_flag(0), num_samples(0), loop_start(-1), prefech_samples(-1), loop_offset(-1)
    {
    }
    void parse(unsigned char* buffer)
    {
        BitGetter b;
        b.SetBitBuffer(buffer);
        version = b.GetBits(4);
        codec = b.GetBits(4);
        channel_config = b.GetBits(6) + 1;
        sample_rate = b.GetBits(18);
        playloc = b.GetBits(2);
        loop_flag = b.GetBits(1);
        num_samples = b.GetBits(29);
        if (loop_flag)
        {
            loop_start = b.GetBits(32);
        }
        else
        {
            loop_start = -1;
        }
        if (playloc == SNDPLAYER_PLAYTYPE_GIGASAMPLE)
        {
            prefech_samples = b.GetBits(32);
        }
        if (loop_flag)
        {
            if (playloc == SNDPLAYER_PLAYTYPE_STREAM || playloc == SNDPLAYER_PLAYTYPE_GIGASAMPLE && prefech_samples <= loop_start)
            {
                loop_offset = b.GetBits(32);
            }
            else
            {
                loop_offset = 0;
            }
        }
    }
    int calculateoffset()
    {
        int offset = 8;
        if (loop_flag)
        {
            offset = 12;
        }
        if (playloc == SNDPLAYER_PLAYTYPE_STREAM)
        {
            if (loop_flag)
            {
                offset += 4;
            }
        }
        else if (playloc == SNDPLAYER_PLAYTYPE_GIGASAMPLE)
        {
            offset += 4;
            if (prefech_samples <= loop_start)
            {
                offset += 4;
            }
        }
        return offset;
    }
    bool validate()
    {
        int result = 1;
        if (version < 0 || version > 1)
        {
            result = 0;
        }
        if (codec < 0 || codec >= 9)
        {
            result = 0;
        }
        if (channel_config < 1 || channel_config > 64)
        {
            result = 0;
        }
        if (sample_rate <= 0 || sample_rate > 200000)
        {
            result = 0;
        }
        if (playloc && playloc != 1 && playloc != 2)
        {
            result = 0;
        }
        if (loop_flag && loop_flag != 1)
        {
            result = 0;
        }
        if (num_samples <= 0)
        {
            result = 0;
        }
        if (loop_flag)
        {
            if (loop_start < 0 || num_samples <= loop_start)
            {
                result = 0;
            }
        }
        if (playloc == 1)
        {
            if (!loop_flag)
            {
                return result;
            }
            if (version)
            {
                if (loop_offset <= 0)
                {
                    result = 0;
                }
                return result;
            }
            goto l1;
        }
        if (playloc == 2)
        {
            if (version)
            {
                result = 0;
            }
            if (prefech_samples <= 0)
            {
                result = 0;
            }
            if (loop_flag)
            {
            l1:
                if (loop_offset < 0)
                {
                    result = 0;
                }
                return result;
            }
        }
        return result;
    }
};

void readheader(STDSTREAM* stream, SndPlayerHeader* header)
{
    unsigned char buffer[64];
    long long pos = stream->filepos;
    gseek(stream, 0);
    int read = gread(stream, buffer, 64);
    gseek(stream, pos);
    bool b = false;
    unsigned char* buf2 = buffer;
    if (buffer[0] == 72)
    {
        b = true;
        buf2 = &buffer[4];
    }
    header->parse(buf2);
    int loop_flag = header->loop_flag;
    int i1 = 8;
    if (loop_flag)
    {
        i1 = 12;
    }
    int playloc = header->playloc;
    if (playloc == SNDPLAYER_PLAYTYPE_STREAM)
    {
        if (!loop_flag)
        {
            goto l1;
        }
        goto l2;
    }
    if (playloc == SNDPLAYER_PLAYTYPE_GIGASAMPLE)
    {
        i1 += 4;
        if (header->prefech_samples <= header->loop_start)
        {
        l2:
            i1 += 4;
            goto l1;
        }
    }
l1:
    if (b)
    {
        read = read - 4;
    }
    if (read < i1)
    {
        header->loop_offset = -1;
        header->prefech_samples = -1;
        header->loop_start = -1;
        header->num_samples = 0;
        header->loop_flag = 0;
        header->playloc = SNDPLAYER_PLAYTYPE_RAM;
        header->sample_rate = 0;
        header->channel_config = 0;
        header->codec = SNDPLAYER_CODEC_XAS_INT;
        header->version = -1;
    }
}

char extrafilename[256];

int sndplayerid(const char* filename, long long offset, STDSTREAM* stream)
{
    if (!stream)
    {
        return 0;
    }
    if (stream->filesize < 8)
    {
        return 0;
    }
    SndPlayerHeader header;
    readheader(stream, &header);
    if (!header.validate())
    {
        return 0;
    }
    char* fname = (char*)filename;
    int result = 50;
    int len = strlen(filename);
    if (len < 4 || filename[len - 4] != '.')
    {
        return result;
    }
    const char* ext = &filename[len - 3];
    if (!header.version)
    {
        if (!strcmp(ext, "snr"))
        {
            result = 90;
            if (header.playloc == SNDPLAYER_PLAYTYPE_STREAM
                || header.playloc == SNDPLAYER_PLAYTYPE_GIGASAMPLE && header.prefech_samples < header.num_samples)
            {
                char c;
                do
                {
                    c = *fname;
                    fname[&extrafilename[3] - filename] = *fname;
                    ++fname;
                } while (c);
                strcpy_s(&extrafilename[len], 4, "sns");
                if (!fileexists(&extrafilename[3]))
                {
                    return 0;
                }
            }
        }
        return result;
    }
    if (strcmp(ext, "sph"))
    {
        if (!strcmp(&filename[len - 3], "sps"))
        {
            result = 90;
        }
        return result;
    }
    char c;
    do
    {
        c = *fname;
        fname[&extrafilename[3] - filename] = *fname;
        ++fname;
    } while (c);
    strcpy_s(&extrafilename[len], 4, "sps");
    return fileexists(&extrafilename[3]) ? 90 : 0;
}

int sndplayeropen(SINSTANCE* instance)
{
    SndPlayerHeader header;
    readheader(instance->filestruct, &header);
    if (!header.validate())
    {
        return 0;
    }
    instance->version = header.version;
    return 1;
}

int sndplayerinfo(SINSTANCE* instance, SINFO** info, int element)
{
    int result;
    *info = 0;
    if (element)
    {
        SIMEX_seterror("An attempt to get info from a patch element other than 0 was made.");
        result = 0;
    }
    else
    {
        SINFO* s = new SINFO;
        *info = s;
        initsinfo(s);
        SSOUND* sound = (*info)->sound[0];
        SndPlayerHeader header;
        readheader(instance->filestruct, &header);
        if (header.validate())
        {
            bool loop = header.loop_flag == 0;
            char version = header.version;
            char channel_config = header.channel_config;
            float sampleperiod = (float)header.prefech_samples / (float)header.sample_rate;
            int sample_rate = header.sample_rate;
            instance->version = header.version;
            sound->fileversion = version;
            int num_samples = header.num_samples;
            sound->num_samples = header.num_samples;
            sound->channel_config = channel_config;
            sound->sample_rate = sample_rate;
            sound->gigasampleperiod = sampleperiod;
            if (!loop)
            {
                sound->loopstart = header.loop_start;
                sound->loopend = num_samples - 1;
            }
            if (header.playloc)
                sound->playloc = header.playloc != 1 ? PLAYLOC_GIGASAMPLE : PLAYLOC_STREAM;
            else
                sound->playloc = PLAYLOC_RAM;
            switch (header.codec)
            {
            case SNDPLAYER_CODEC_XAS_INT:
                sound->bitrate = -1;
                sound->codec = xas0_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_EALAYER3_INT:
                sound->bitrate = -1;
                sound->codec = ealayer30_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_SIGN16BIG_INT:
                sound->bitrate = -1;
                sound->codec = 1;
                result = 1;
                break;
            case SNDPLAYER_CODEC_EAXMA:
                sound->bitrate = -1;
                sound->codec = eaxma;
                result = 1;
                break;
            case SNDPLAYER_CODEC_XAS1_INT:
                sound->bitrate = -1;
                sound->codec = xas_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_EALAYER31_INT:
                sound->bitrate = -1;
                sound->codec = ealayer3_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_EALAYER32PCM_INT:
                sound->bitrate = -1;
                sound->codec = ealayer3pcm_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_EALAYER32SPIKE_INT:
                sound->bitrate = -1;
                sound->codec = ealayer3spike_int;
                result = 1;
                break;
            case SNDPLAYER_CODEC_GCADPCM:
                sound->bitrate = -1;
                sound->codec = gcadpcm;
                result = 1;
                break;
            default:
                sound->codec = -1;
                sound->bitrate = -1;
                result = 1;
                break;
            }
        }
        else
        {
            SIMEX_seterror("Unable to read a valid header from input file.");
            result = 0;
        }
    }
    return result;
}

rw::audio::core::DecoderExtended* sndplayercreatedecoder(SSOUND* sound, unsigned int channels)
{
    unsigned int guid = 0;
    switch (sound->codec)
    {
    case s16b_int:
        guid = 'P6B0';
        break;
    case gcadpcm:
        SIMEX_seterror("Error creating GcAdpcm decoder. Not supported on this platform");
        break;
    case xas0_int:
        SIMEX_seterror("Error creating Xas0 decoder. Not supported on this platform");
        break;
    case ealayer30_int:
        SIMEX_seterror("Error creating El30 decoder. Not supported on this platform");
        break;
    case eaxma:
        SIMEX_seterror("Error creating EaXma decoder. Not supported on this platform");
        break;
    case xas_int:
        guid = 'Xas1';
        break;
    case ealayer3_int:
        guid = 'EL31';
        break;
    case ealayer3pcm_int:
        guid = 'L32P';
        break;
    case ealayer3spike_int:
        guid = 'L32S';
        break;
    default:
        break;
    }
    rw::audio::core::DecoderExtended* result = nullptr;
    if (guid)
    {
        rw::audio::core::System::GetInstance()->Lock();
        rw::audio::core::DecoderRegistry *registry = rw::audio::core::System::GetInstance()->GetDecoderRegistry();
        void *handle = registry->GetDecoderHandle(guid);
        result = registry->DecoderExtendedFactory(handle, channels, 1, rw::audio::core::System::GetInstance());
        rw::audio::core::System::GetInstance()->Unlock();
    }
    return result;
}

int sndplayerdecode(SndPlayerHeader* info, rw::audio::core::DecoderExtended* decoder, unsigned char* data, int size, short** samples)
{
    unsigned char* buf = data;
    int decodedsamples = 0;
    unsigned char* buf2 = &data[size];
    short* outsamples[64];
    int i2;
    int i3;
    if (data < &data[size])
    {
        for (;;)
        {
            for (int i = 0; i < info->channel_config; i++)
            {
                outsamples[i] = &samples[i][decodedsamples];
            }
            i2 = 0;
            if (info->version == 0)
            {
                break;
            }
            i3 = (buf[3] << 0) | (buf[2] << 8) | (buf[1] << 16);
            if (*buf == 68)
            {
                i2 = (buf[7] << 0) | (buf[6] << 8) | (buf[5] << 16) | (buf[4] << 24);
            l1:
                if (!i3)
                {
                    return decodedsamples;
                }
                bool b2 = decodedsamples && decodedsamples != info->loop_start;
                decoder->Feed(buf + 8, i2, b2 ? rw::audio::core::Decoder::FEEDTYPE_CONTINUE : rw::audio::core::Decoder::FEEDTYPE_NEW);
                decodedsamples += decoder->Decode(outsamples, i2);
            }
            buf += i3;
            if (buf >= buf2)
            {
                return decodedsamples;
            }
        }
        gputm((unsigned char*)&i3, *(unsigned int*)buf, 4);
        gputm((unsigned char*)&i2, *(unsigned int*)&buf[4], 4);
        i3 &= 0x7FFFFFFF;
        goto l1;
    }
    return 0;
}

int sndplayerread(SINSTANCE* instance, SINFO* info, int element)
{
    STDSTREAM* extra = nullptr;
    SSOUND* sound = info->sound[0];
    rw::audio::core::DecoderExtended* decoder = nullptr;
    int result = 0;
    SndPlayerHeader header;
    if (allocateinputsamples(sound) < 0)
    {
        goto l1;
    }
    readheader(instance->filestruct, &header);
    if (!header.validate())
    {
        SIMEX_seterror("Unable to read a valid SndPlayer header from the input file.");
        goto l1;
    }
    decoder = sndplayercreatedecoder(sound, header.channel_config);
    int decodesize;
    unsigned char* buf;
    unsigned char* buf2;
    int extrasize;
    int size;
    if (header.version != 1)
    {
        if (header.version)
        {
            SIMEX_seterror("Unable to import asset of this version. Please use a newer version of Simex.");
            goto l1;
        }
        extrasize = 0;
        size = instance->filestruct->filesize;
        if (!strcmp(&extrafilename[3], ""))
        {
            goto l2;
        }
        extra = gopen(&extrafilename[3]);
        if (!extra)
        {
            goto l3;
        }
        extrasize = extra->filesize;
        if (extrasize > 0)
        {
        l2:
            buf = new unsigned char[size + extrasize];
            gseek(instance->filestruct, 0i64);
            gread(instance->filestruct, buf, size);
            if (extrasize)
            {
                gread(extra, &buf[size], extrasize);
                gclose(extra);
                extra = 0;
            }
            int offset = header.calculateoffset();
            decodesize = size + extrasize - offset;
            buf2 = &buf[offset];
            goto l6;
        }
    l5:
        SIMEX_seterror("Invalid SndPlayer stream file detected.");
        gclose(extra);
        goto l1;
    }
    if (!strcmp(&extrafilename[3], ""))
    {
        decodesize = instance->filestruct->filesize;
        if (decodesize <= 0)
        {
            SIMEX_seterror("Invalid SndPlayer stream file detected.");
            goto l1;
        }
        buf = new unsigned char[instance->filestruct->filesize];
        gseek(instance->filestruct, 0i64);
        gread(instance->filestruct, buf, decodesize);
        buf2 = buf;
        goto l6;
    }
    extra = gopen(&extrafilename[3]);
    if (extra)
    {
        decodesize = extra->filesize;
        if (decodesize > 0)
        {
            buf = new unsigned char[extra->filesize];
            gread(extra, buf, decodesize);
            gclose(extra);
            extra = 0;
            buf2 = buf;
        l6:
            if (sndplayerdecode(&header, decoder, buf2, decodesize, sound->decodedsamples) == header.num_samples)
            {
                result = 1;
            }
            else
            {
                SIMEX_seterror("Couldn't import all data from SndPlayer files.");
            }
            if (buf)
            {
                delete[] buf;
            }
            goto l4;
        }
        goto l5;
    }
l3:
    SIMEX_seterror("Couldn't open SndPlayer stream file.");
l4:
    if (extra)
    {
        gclose(extra);
    }
l1:
    if (decoder)
    {
        decoder->Release();
    }
    return result;
}

int sndplayerclose(SINSTANCE* instance)
{
    return 1;
}

int sndplayercreate(SINSTANCE* instance)
{
    return 1;
}

class BitPutter {
public:
    void addbits(unsigned int data, int numBits)
    {
        unsigned int i1 = data << (32 - numBits) >> (32 - numBits);
        unsigned int i2 = data << (32 - numBits) >> (32 - numBits);
        if (numBits > 0)
        {
            for (;;)
            {
                unsigned int i3 = mBitPosition & 7;
                int i4 = 8 - i3;
                unsigned int i5 = this->mBitPosition >> 3;
                if (i3 == 0)
                {
                    mpBitBuffer[i5] = 0;
                }
                unsigned int i6 = 8 - i3;
                if (numBits <= i4)
                {
                    i6 = numBits;
                }
                numBits -= i6;
                mpBitBuffer[i5] |= (unsigned char)(i1 >> numBits) << (i4 - i6);
                mBitPosition += i6;
                if (numBits <= 0)
                {
                    break;
                }
                i1 = i2;
            }
        }
    }
    BitPutter() : mBitPosition(0) {}
    ~BitPutter()
    {
        assert(mBitPosition == 0);
    }
    unsigned int GetBitsWritten() { return mBitPosition; }
    int GetBitBuffer(char* buf)
    {
        int i1 = mBitPosition >> 3;
        int i2 = mBitPosition >> 3;
        if (mBitPosition >> 3 >= 64)
        {
            i2 = 64;
        }
        memcpy(buf, mpBitBuffer, i2);
        memmove(mpBitBuffer, &mpBitBuffer[i2], i1 - i2 + 1);
        mBitPosition -= 8 * i2;
        return i2;
    }
private:
    unsigned char mpBitBuffer[1028];
    unsigned int mBitPosition = 0;
};

class EncoderHelper
{
private:
    rw::audio::core::EncoderRegistry* mpEncoderRegistry;
    rw::audio::core::EncoderExtended* mpEncoder;
    unsigned char* mpEncodeBuffer;
    char* mpSeekBuffer;
    unsigned char* mpEncodePtr;
    char* mpSeekPtr;
    int mEncodeSize;
    int mSeekSize;
    bool mHasEncodedData;
public:
    bool IsSeekable() { return mpEncoder->IsSeekable(); }
    void SetIsChunked(bool isChunked) { mpEncoder->SetIsChunked(isChunked); }
    void Free()
    {
        mpEncoder->Release();
        if (mpEncodeBuffer)
        {
            delete[] mpEncodeBuffer;
        }
        if (mpSeekBuffer)
        {
            delete[] mpSeekBuffer;
        }
        delete this;
    }
    float GetAverageDataRate() { return mpEncoder->GetAverageDataRate(); }
    int EncodeBlock(short** sampledata, unsigned char** encodebuffer, int numsamples, int* encodedbytes, SSOUND* sound, void** seekbuffer, int* seeksize)
    {
        int newencodesize = mpEncoder->GetEncodeMemoryRequired(numsamples);
        if (mEncodeSize < newencodesize)
        {
            if (mEncodeSize)
            {
                delete[] mpEncodeBuffer;
            }
            mpEncodeBuffer = new unsigned char[newencodesize];
            mEncodeSize = newencodesize;
        }
        int newseeksize = mpEncoder->GetSeekMemoryRequired(numsamples);
        if (mSeekSize < newseeksize)
        {
            if (mSeekSize)
            {
                delete[] mpSeekBuffer;
            }
            mpSeekBuffer = new char[newseeksize];
            mSeekSize = newseeksize;
        }
        if (sound->bitrate < 0)
        {
            mpEncoder->SetVbrQuality(0.89999998f);
        }
        else if (sound->bitrate > 100)
        {
            mpEncoder->SetCbrRate(sound->bitrate);
        }
        else
        {
            mpEncoder->SetVbrQuality((float)sound->bitrate * 0.0099999998f);
        }
        int result = mpEncoder->Encode(sampledata, mpEncodeBuffer, numsamples, encodedbytes, mpSeekBuffer, seeksize, 0);
        *encodebuffer = mpEncodeBuffer;
        *seekbuffer = mpSeekBuffer;
        mpEncodePtr = &mpEncodeBuffer[*encodedbytes];
        if (mpSeekBuffer)
        {
            mpSeekPtr = &mpSeekBuffer[*seeksize];
        }
        mHasEncodedData = true;
        return result;
    }
    int Flush(unsigned char** encodedata, int* encodedbytes, void** seekdata, int* seeksize)
    {
        int result;
        if (mHasEncodedData)
        {
            result = mpEncoder->Flush(mpEncodePtr, encodedbytes, mpSeekPtr, seeksize, 0);
            *encodedata = mpEncodePtr;
            *seekdata = mpSeekPtr;
            mHasEncodedData = 0;
        }
        else
        {
            SIMEX_seterror("EncoderHelper::Flush - Can only be called after some data is encoded.");
            result = -1;
        }
        return result;
    }
    EncoderHelper(SSOUND* sound, int guid) : mpEncoderRegistry(nullptr), mpEncodeBuffer(nullptr), mpEncodePtr(nullptr), mEncodeSize(0), mpSeekBuffer(nullptr), mpSeekPtr(nullptr), mSeekSize(0), mHasEncodedData(false)
    {
        rw::audio::core::System::GetInstance()->Lock();
        mpEncoderRegistry = rw::audio::core::System::GetInstance()->GetEncoderRegistry();
        mpEncoder = mpEncoderRegistry->EncoderExtendedFactory(mpEncoderRegistry->GetEncoderHandle(guid), sound->channel_config, sound->sample_rate, rw::audio::core::System::GetInstance());
        rw::audio::core::System::GetInstance()->Unlock();
        if (mpEncoder->IsLoopInfoRequired() && sound->loopend > 0)
        {
            mpEncoder->SetLoopInfo(sound->loopstart, sound->loopend);
        }
    }
};

EncoderHelper* CreateEncoderHelper(SSOUND *sound, int guid)
{
    return new EncoderHelper(sound, guid);
}

int truncateloopend(SSOUND* sound)
{
    if (!sound->unk71)
    {
        return 1;
    }
    int loopend = sound->loopend;
    if (loopend <= 0)
    {
        return 1;
    }
    unsigned char codec = sound->codec;
    int codecval;
    switch (codec)
    {
    case vag_blk:
        codecval = 28;
        break;
    case gcadpcm:
        codecval = 28;
        break;
    case xboxadpcm:
        codecval = 64;
        break;
    default:
        codecval = codec != atrac3_int && codec != atrac3plus_int;
        break;
    }
    if (sound->num_samples <= loopend + 2 * codecval + 1)
    {
        return 1;
    }
    int loopval = loopend + codecval + 1;
    int loopval2;
    if (codecval && loopval % codecval)
    {
        loopval2 = loopval - loopval % codecval;
    }
    else
    {
        loopval2 = loopend + 1;
    }
    int loopval3 = codecval + loopval2;
    int bufsize = 2 * loopval3;
    short** decodedsamples = sound->decodedsamples;
    for (int i = 0; i < sound->channel_config; i++)
    {
        short* buffer = new short[bufsize];
        memcpy(buffer, *decodedsamples, bufsize);
        delete[] * decodedsamples;
        *decodedsamples = buffer;
        decodedsamples++;
    }
    sound->num_samples = loopval3;
    return 1;
}

class SeekHelper;

struct SeekStruct
{
    SeekStruct* next;
    void* data;
    int unk1;
    int unk2;
    int unk3;
};

int WriteSnrBlock(const char* filename, STDSTREAM** file, SSOUND* sound, EncoderHelper* helper, SeekHelper* seekhelper, int sampleoffset, int numsamplesin, int isstream, int shouldflush, bool unk1, int unk2, int* numsamplesout, int* unk3, char* fpath, int* unk4)
{
    int result = numsamplesin;
    *numsamplesout = 0;
    STDSTREAM* file2 = *file;
    int i1 = 0;
    int i2 = sampleoffset;
    int i3 = numsamplesin;
    if (numsamplesin > 0)
    {
        short** sampledata = new short *[sound->channel_config];
        if (sound->codec == eaxma)
        {
            short** s = sound->decodedsamples;
            for (int i = 0; i < sound->channel_config; i++)
            {
                sampledata[i] = &(*s++)[sampleoffset];
            }
            int blockseeksize = (sound->channel_config + 1) / 2;
            unsigned char* encodebuffer = 0;
            int encodedBytes = 0;
            SeekStruct *seekbuffer = 0;
            int seeksize = 0;
            int encodedsamples = helper->EncodeBlock(sampledata, &encodebuffer, numsamplesin, &encodedBytes, sound, (void **)&seekbuffer, &seeksize);
            encodedsamples += helper->Flush(&encodebuffer, &encodedBytes, (void **)&seekbuffer, &seeksize);
            SeekStruct* seekbuffer2 = seekbuffer;
            int i4 = 0;
            if ((sound->playloc & PLAYLOC_GIGASAMPLE) == 0)
            {
                *unk4 = 0;
            }
            else
            {
                int offset = (int)((double)sound->sample_rate * sound->gigasampleperiod);
                if (sampleoffset < offset)
                {
                    int offset2 = offset - sampleoffset;
                    if (offset - sampleoffset >= encodedsamples)
                    {
                        *unk4 = offset;
                    }
                    else
                    {
                        char buf2[2048];
                        memset(buf2, -1, sizeof(buf2));
                        long long filepos = file2->filepos;
                        int i5 = 8;
                        int i6 = 8;
                        int i7 = -1;
                        unsigned char buf[8];
                        gwrite(file2, buf, 8);
                        for (int i = 0; i < blockseeksize; i++)
                        {
                            long long pos = file2->filepos;
                            int j = 0;
                            int i9 = 0;
                            gwrite(file2, &i9, 4);
                            SeekStruct* seekbuffer3 = seekbuffer;
                            unsigned char* encodebuffer2 = encodebuffer;
                            i1 = 4;
                            int i10 = 0;
                            if (offset2 > 0)
                            {
                                do
                                {
                                    unsigned char* encodebuffer3 = encodebuffer2;
                                    unsigned int len;
                                    gputm((unsigned char*)&len, *(unsigned int*)encodebuffer2, 4);
                                    len >>= 2;
                                    for (int k = i; k; k--)
                                    {
                                        gputm((unsigned char*)&len, *(unsigned int*)&encodebuffer3[len], 4);
                                        encodebuffer3 += len;
                                        len >>= 2;
                                    }
                                    int i11 = len - 4;
                                    gwrite(file2, encodebuffer3 + 4, len - 4);
                                    j += seekbuffer3->unk1;
                                    int i12 = 0;
                                    if (j >= offset2)
                                    {
                                        gputm((unsigned char*)&i10, *(unsigned int*)&encodebuffer3[len], 4);
                                        i10 &= 3u;
                                    }
                                    else
                                    {
                                        i12 = ((i11 + 2047) & 0xFFFFF800) - i11;
                                        gwrite(file2, buf2, i12);
                                    }
                                    encodebuffer2 += seekbuffer3->unk2;
                                    i1 += i11 + i12;
                                    seekbuffer3 = seekbuffer3->next;
                                } while (j < offset2);
                            }
                            unsigned int i13 = (4 * i1) | i10 & 3;
                            gputm((unsigned char*)&i9, i13, 4);
                            long long pos3 = file2->filepos;
                            gseek(file2, pos);
                            gwrite(file2, &i9, 4);
                            gseek(file2, pos3);
                            i7 = j;
                            i5 = i1 + i6;
                            i6 += i1;
                        }
                        int i14 = i7 + sampleoffset;
                        int i15 = 0;
                        for (int i = i7; i;)
                        {
                            i -= seekbuffer2->unk1;
                            i15 += seekbuffer2->unk2;
                            seekbuffer2 = seekbuffer2->next;
                        }
                        i4 = i15;
                        gputm(buf, i5, 4);
                        gputm(&buf[4], i7, 4);
                        long long pos3 = file2->filepos;
                        gseek(file2, filepos);
                        gwrite(file2, buf, 8);
                        gseek(file2, pos3);
                        *unk4 = i14;
                        i1 = i5;
                        isstream = 1;
                        char s1[1024];
                        buildfilename(s1, 1024, filename, 0, "snr");
                        char s2[1024];
                        buildfilename(s2, 1024, filename, 0, "sns");
                        buildfilename(fpath, 1024, filename, "H", "snr");
                        gclose(file2);
                        renamefile(fpath, s1);
                        buildfilename(fpath, 1024, filename, "S", "sns");
                        wchar_t outpath[1024];
                        getpath(fpath, outpath, 1024);
                        _wremove(outpath);
                        file2 = gwopen(fpath);
                        *file = file2;
                        *unk3 = 1;
                    }
                }
            }
            while (seekbuffer2)
            {
                int i16 = 0;
                int i17 = seekbuffer2->unk2 + 8;
                int i18 = 0;
                if (!seekbuffer2->next)
                {
                    if (isstream)
                    {
                        i16 = 0x80000000;
                    }
                    if ((sound->playloc & (PLAYLOC_GIGASAMPLE | PLAYLOC_STREAM)) != 0)
                    {
                        i18 = ((seekbuffer2->unk2 + 71) & 0xFFFFFFC0) - (seekbuffer2->unk2 + 8);
                        i17 = (seekbuffer2->unk2 + 71) & 0xFFFFFFC0;
                    }
                }
                unsigned char buf3[8];
                gputm(buf3, i17 | i16, 4);
                gputm(&buf3[4], seekbuffer2->unk1, 4);
                gwrite(file2, buf3, 8);
                gwrite(file2, &encodebuffer[i4], seekbuffer2->unk2);
                if (i18 > 0)
                {
                    char buf4[64];
                    memset(buf4, 0, sizeof(buf4));
                    gwrite(file2, buf4, i18);
                }
                if (seekhelper)
                {
                    //seek tables not supported
                }
                int i19 = seekbuffer2->unk2;
                seekbuffer2 = seekbuffer2->next;
                i1 += i17;
                i4 += i19;
            }
            *numsamplesout += encodedsamples;
        }
        else
        {
            do
            {
                if (isstream)
                {
                    i3 = (int)((double)sound->sample_rate * (2040.0 / helper->GetAverageDataRate()));
                    if (i3 > numsamplesin)
                    {
                        i3 = numsamplesin;
                    }
                }
                short** samples = sound->decodedsamples;
                for (int i = 0; i < sound->channel_config; i++)
                {
                    sampledata[i] = &samples[i][i2];
                }
                unsigned char* encodebuffer4 = nullptr;
                unsigned char* encodeptr = nullptr;
                int encodedbytes = 0;
                int encodedbytes2 = 0;
                void* seekbuffer4 = nullptr;
                int blockseeksize = 0;
                int blockseeksize2 = 0;
                int i21 = helper->EncodeBlock(sampledata, &encodebuffer4, i3, &encodedbytes, sound, &seekbuffer4, &blockseeksize);
                if (i21 <= 0 && !shouldflush)
                {
                    break;
                }
                numsamplesin -= i3;
                i2 += i3;
                int i22 = 0;
                if (encodedbytes > 0 || shouldflush)
                {
                    int i23;
                    unsigned int i24;
                    int i25;
                    if (numsamplesin > 0)
                    {
                        i23 = encodedbytes + 8;
                        i24 = encodedbytes + 8;
                        i25 = encodedbytes + 8;
                    }
                    else
                    {
                        if (shouldflush)
                        {
                            i21 += helper->Flush(&encodeptr, &encodedbytes2, &seekbuffer4, &blockseeksize2);
                        }
                        i23 = encodedbytes2 + encodedbytes + 8;
                        i24 = i23;
                        i25 = i23;
                        if (isstream)
                        {
                            i23 |= 0x80000000;
                        }
                    }
                    i1 += encodedbytes + encodedbytes2;
                    bool b3 = i24 > 8;
                    if (unk2 && i24 > 8)
                    {
                        i1 += 8;
                        if (numsamplesin <= 0 && isstream)
                        {
                            i24 += 64 - i1 % 64;
                            i23 += 64 - i1 % 64;
                            i22 = 64 - i1 % 64;
                            i25 = i24;
                            i1 += i22;
                        }
                        unsigned char buf[8];
                        gputm(buf, i23, 4);
                        gputm(&buf[4], i21, 4);
                        gwrite(file2, buf, 8);
                    }
                    if (seekhelper && b3)
                    {
                        //seek tables not supported
                    }
                }
                *numsamplesout += i21;
                gwrite(file2, encodebuffer4, encodedbytes);
                if (numsamplesin <= 0 && shouldflush)
                {
                    gwrite(file2, encodeptr, encodedbytes2);
                }
                if (i22)
                {
                    unsigned char buf4[64];
                    memset(buf4, 0, sizeof(buf4));
                    gwrite(file2, buf4, i22);
                }
            } while (numsamplesin);
        }
        delete[] sampledata;
        result = i1;
    }
    return result;
}

int WriteSnrHeader(int prefetch_samples, SSOUND* sound, STDSTREAM* file, int loop_offset)
{
    BitPutter bits;
    bits.addbits(0, 4);
    unsigned char codec = sound->codec;
    switch (codec)
    {
    case xas_int:
        bits.addbits(SNDPLAYER_CODEC_XAS1_INT, 4);
        break;
    case xas0_int:
        SIMEX_seterror("Error writing SndPlayer header for XAS samplerep.");
        return -1;
    case ealayer3pcm_int:
        bits.addbits(SNDPLAYER_CODEC_EALAYER32PCM_INT, 4);
        break;
    case ealayer3spike_int:
        bits.addbits(SNDPLAYER_CODEC_EALAYER32SPIKE_INT, 4);
        break;
    case ealayer3_int:
        bits.addbits(SNDPLAYER_CODEC_EALAYER31_INT, 4);
        break;
    case ealayer30_int:
        SIMEX_seterror("Error writing SndPlayer header for SND_SR_EALAYER3_INT samplerep.");
        return -1;
    case eaxma:
        SIMEX_seterror("Error writing SndPlayer header for EAXMA samplerep.");
        return -1;
    case gcadpcm:
        SIMEX_seterror("Error writing SndPlayer header for GCADPCM samplerep.");
        return -1;
    default:
        bits.addbits(SNDPLAYER_CODEC_SIGN16BIG_INT, 4);
        break;
    }
    bits.addbits(sound->channel_config - 1, 6);
    bits.addbits(sound->sample_rate, 18);
    if ((sound->playloc & PLAYLOC_RAM) != 0 || !sound->playloc)
    {
        bits.addbits(SNDPLAYER_PLAYTYPE_RAM, 2);
    }
    else if ((sound->playloc & PLAYLOC_STREAM) != 0)
    {
        bits.addbits(SNDPLAYER_PLAYTYPE_STREAM, 2);
    }
    else
    {
        bits.addbits(SNDPLAYER_PLAYTYPE_GIGASAMPLE, 2);
    }
    bits.addbits(sound->loopstart >= 0, 1);
    if (sound->loopstart < 0)
    {
        bits.addbits(sound->num_samples, 29);
    }
    else
    {
        bits.addbits(sound->loopend + 1, 29);
    }
    if (sound->loopstart >= 0)
    {
        bits.addbits(sound->loopstart, 32);
    }
    if ((sound->playloc & PLAYLOC_STREAM) != 0 && sound->loopstart >= 0)
    {
        bits.addbits(loop_offset, 32);
    }
    if ((sound->playloc & PLAYLOC_GIGASAMPLE) != 0)
    {
        bits.addbits(prefetch_samples, 32);
        if (sound->loopstart >= prefetch_samples)
        {
            if (sound->loopstart == prefetch_samples)
            {
                bits.addbits(0, 32);
            }
            else
            {
                bits.addbits(loop_offset, 32);
            }
        }
    }
    char buf[64];
    int i2 = bits.GetBitBuffer(buf);
    if (!gwrite(file, buf, i2))
    {
        SIMEX_seterror("Error writing SndPlayer header.");
        return -1;
    }
    return 0;
}

void printwarning(const char* format, ...)
{
}

int EncodeVersion0Snr(const char* filename, SSOUND* sound)
{
    int ret = 0;
    int loop_offset = -1;
    int prefech_samples = 0;
    SeekHelper* seekhelper = nullptr;
    int codec = sound->codec;
    EncoderHelper* helper = nullptr;
    switch (codec)
    {
    case xas_int:
        helper = CreateEncoderHelper(sound, 'Xas1');
        break;
    case xas0_int:
        SIMEX_seterror("Error creating Deprecated XAS encoder.");
        if (seekhelper)
        {
            //seek tables not supported
        }
        return ret;
    case s16b_int:
        helper = CreateEncoderHelper(sound, 'P6B0');
        break;
    case eaxma:
        SIMEX_seterror("EA XMA not supported");
        break;
    case ealayer3pcm_int:
        helper = CreateEncoderHelper(sound, 'L32P');
        break;
    case ealayer3spike_int:
        helper = CreateEncoderHelper(sound, 'L32S');
        break;
    case ealayer3_int:
        helper = CreateEncoderHelper(sound, 'EL31');
        break;
    case ealayer30_int:
        SIMEX_seterror("Error creating Deprecated EALayer3 encoder.");
        if (seekhelper)
        {
            //seek tables not supported
        }
        return ret;
    case gcadpcm:
        SIMEX_seterror("GameCube ADPCM not supported");
        if (seekhelper)
        {
            //seek tables not supported
        }
        return ret;
    }
    bool buildseektable;
    if (!sound->createseektable || (sound->playloc & PLAYLOC_GIGASAMPLE) != 0 || sound->loopstart >= 0)
    {
        goto l2;
    }
    buildseektable = 1;
    if (!helper->IsSeekable())
    {
        printwarning("The chosen codec does not yet support seeking. No seek table will be created.\n");
    l2:
        buildseektable = 0;
    }
    char fname[1024];
    buildfilename(fname, 1024, filename, 0, "snr");
    wchar_t outpath[1024];
    getpath(fname, outpath, 1024);
    _wremove(outpath);
    char path[1024];
    buildfilename(path, 1024, filename, 0, "sns");
    getpath(path, outpath, 1024);
    _wremove(outpath);
    char filename2[1024];
    buildfilename(filename2, 1024, filename, "H", "snr");
    getpath(filename2, outpath, 1024);
    _wremove(outpath);
    STDSTREAM* file1 = gwopen(filename2);
    STDSTREAM* file2 = 0;
    if (buildseektable)
    {
        //seek tables not supported
    }
    if ((sound->playloc & (PLAYLOC_GIGASAMPLE | PLAYLOC_STREAM)) != 0)
    {
        helper->SetIsChunked(true);
    }
    else
    {
        helper->SetIsChunked(false);
    }
    int sampleoffset = 0;
    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    bool b = 1;
    int i6;
    if (sound->loopstart < 0)
    {
        i6 = sound->num_samples;
    }
    else
    {
        i6 = sound->loopend + 1;
    }
    short playloc = sound->playloc;
    int i7 = i6;
    if ((playloc & PLAYLOC_RAM) != 0 || !playloc || (playloc & PLAYLOC_STREAM) != 0)
    {
        if (sound->gigasampleperiod > 0.0f)
        {
            printwarning("    NOTE: This is not a Gigasample file, therefore the period of %f is ignored.\n", sound->gigasampleperiod);
        }
    }
    else
    {
        if (sound->gigasampleperiod <= 0.0f)
        {
            printwarning("    NOTE: No Gigasample in-RAM period specified. Using default of 1 second.\n");
            sound->gigasampleperiod = 1.0f;
        }
        prefech_samples = (int)((double)sound->sample_rate * sound->gigasampleperiod);
        if (prefech_samples > i6)
        {
            prefech_samples = i6;
        }
    }
    if (i6 > 0)
    {
        int isstream;
        int shouldflush;
        int numsamplesin;
        for (;;)
        {
            if (!i3 && (sound->playloc == PLAYLOC_STREAM || i4 && (sound->playloc & PLAYLOC_GIGASAMPLE) != 0))
            {
                gclose(file1);
                renamefile(filename2, fname);
                buildfilename(filename2, 1024, filename, "S", "sns");
                getpath(filename2, outpath, 1024);
                _wremove(outpath);
                file1 = gwopen(filename2);
                i3 = 1;
            }
            playloc = sound->playloc;
            if ((playloc & PLAYLOC_RAM) != 0 || !playloc || (sound->playloc & PLAYLOC_STREAM) != 0)
            {
                break;
            }
            if (sound->codec == eaxma)
            {
                isstream = 0;
                shouldflush = 1;
                if (sound->loopstart < 0)
                {
                    numsamplesin = i7;
                }
                else if (sampleoffset >= sound->loopstart)
                {
                    numsamplesin = sound->loopend - sound->loopstart + 1;
                    helper->SetIsChunked(true);
                    if (prefech_samples <= sampleoffset)
                    {
                        isstream = 1;
                    }
                }
                else
                {
                    numsamplesin = sound->loopstart;
                    if (prefech_samples >= loopstart)
                    {
                        helper->SetIsChunked(false);
                    }
                    else
                    {
                        isstream = 1;
                        i2 = 1;
                        helper->SetIsChunked(true);
                    }
                }
            }
            else if (loopstart >= 0)
            {
                numsamplesin = prefech_samples;
                if (sampleoffset >= prefech_samples)
                {
                    if (sampleoffset >= loopstart)
                    {
                        isstream = 1;
                        numsamplesin = sound->loopend - sampleoffset + 1;
                        i2 = 0;
                        shouldflush = 1;
                    }
                    else
                    {
                        shouldflush = 1;
                        numsamplesin = loopstart - sampleoffset;
                        i2 = 1;
                        isstream = 1;
                    }
                }
                else
                {
                    if (loopstart >= prefech_samples)
                    {
                        isstream = 0;
                        i2 = 1;
                        shouldflush = 0;
                        goto l3;
                    }
                    if (sampleoffset >= loopstart)
                    {
                        numsamplesin = prefech_samples - loopstart;
                    }
                    else
                    {
                        numsamplesin = sound->loopstart;
                    }
                    shouldflush = sampleoffset < loopstart;
                    i2 = shouldflush;
                    if (numsamplesin + sampleoffset - 1 == sound->loopend)
                    {
                        goto l4;
                    }
                }
            }
            else if (sampleoffset >= prefech_samples)
            {
                isstream = 1;
                numsamplesin = i7 - sampleoffset;
                shouldflush = 1;
            }
            else
            {
                numsamplesin = i7;
                if (i7 >= prefech_samples)
                {
                    numsamplesin = prefech_samples;
                }
                isstream = 0;
                shouldflush = i7 <= prefech_samples;
            }
        l3:
            __int64 filepos = file1->filepos;
            int samplesout;
            int out = WriteSnrBlock(filename, &file1, sound, helper, seekhelper, sampleoffset, numsamplesin, isstream, shouldflush, b, 1, &samplesout, &i3, filename2, &prefech_samples);
            ret += out;
            b = shouldflush != 0;
            if (loop_offset < 0)
            {
                if (i2 && isstream)
                {
                    loop_offset = file1->filepos - (int)filepos;
                }
                if (!sound->loopstart)
                {
                    loop_offset = 0;
                }
            }
            sampleoffset += numsamplesin;
            i1 += samplesout;
            if (samplesout < numsamplesin)
            {
                if ((sound->playloc & PLAYLOC_GIGASAMPLE) == 0)
                {
                    SIMEX_seterror("Unexpected behavior: Encoded less frames than requested in non-gigasample data export.");
                    if (seekhelper)
                    {
                        //seek tables not supported
                    }
                    return ret;
                }
                int i8 = 0;
                int numsamplesout = 0;
                int i9 = samplesout == 0;
                do
                {
                    if (sampleoffset >= i7)
                    {
                        break;
                    }
                    if (++sampleoffset == sound->loopstart)
                    {
                        shouldflush = 1;
                    }
                    out += WriteSnrBlock(filename, &file1, sound, helper, seekhelper, sampleoffset, 1, isstream, shouldflush, b, i9, &numsamplesout, &i3, filename2, &prefech_samples);
                    i8 = numsamplesout;
                    ret += out;
                    b = shouldflush != 0;
                } while (numsamplesout <= 0);
                i1 += i8;
                prefech_samples = i1;
                if (!i9)
                {
                    __int64 filepos2 = file1->filepos;
                    gseek(file1, filepos);
                    unsigned char buf[4];
                    gputm(buf, out, 4);
                    gwrite(file1, buf, 4);
                    unsigned char buf2[4];
                    gputm(buf2, i8 + samplesout, 4);
                    gwrite(file1, buf2, 4);
                    gseek(file1, filepos2);
                }
            }
            if ((sound->playloc & PLAYLOC_GIGASAMPLE) != 0 && sampleoffset >= prefech_samples)
            {
                i4 = 1;
            }
            if (sampleoffset >= i7)
            {
                goto l5;
            }
        }
        isstream = playloc == PLAYLOC_STREAM;
        if (sound->loopstart < 0)
        {
            numsamplesin = sound->num_samples;
        }
        else if (sound->codec == gcadpcm)
        {
            truncateloopend(sound);
            numsamplesin = sound->num_samples;
            i7 = numsamplesin;
        }
        else if (sampleoffset >= sound->loopstart)
        {
            numsamplesin = sound->loopend - sound->loopstart + 1;
        }
        else
        {
            numsamplesin = sound->loopstart;
            i2 = 1;
        }
    l4:
        shouldflush = 1;
        goto l3;
    }
l5:
    int i10 = 0;
    gseek(file1, 0);
    if (!file1->filesize)
    {
        i10 = 1;
    }
    gclose(file1);
    if (i10)
    {
        removefile(filename2);
    }
    else if (i3)
    {
        renamefile(filename2, path);
    }
    else
    {
        renamefile(filename2, fname);
    }
    helper->Free();
    STDSTREAM* file3 = gopen(fname);
    if (file3)
    {
        int size = file3->filesize;
        char *filebuf = 0;
        if (size)
        {
            filebuf = new char[file3->filesize];
            gread(file3, filebuf, size);
        }
        gclose(file3);
        removefile(fname);
        buildfilename(filename2, 1024, filename, "H", "snr");
        removefile(filename2);
        STDSTREAM* file4 = gwopen(filename2);
        if (WriteSnrHeader(prefech_samples, sound, file4, loop_offset) >= 0)
        {
            if (size)
            {
                gwrite(file4, filebuf, size);
                delete[] filebuf;
            }
            gclose(file4);
            renamefile(filename2, fname);
            if (buildseektable)
            {
                //seek tables not supported
            }
        }
        else
        {
            SIMEX_seterror("Error writing the header file.");
            ret = 0;
        }
    }
    else
    {
        SIMEX_seterror("Error reopening SndPlayer SNR file for header update.");
        ret = 0;
    }
    if (seekhelper)
    {
        //seek tables not supported
    }
    return ret;
}

int sndplayerwrite(SINSTANCE* instance, SINFO* info, int element)
{
    if (element)
    {
        SIMEX_seterror("TBD files can only contain element 0.");
        return 0;
    }
    SSOUND *sound = info->sound[0];
    const char *filename = instance->filename;
    if (info->sound[0]->codec == gcadpcm)
    {
        sound->version = 0;
    }
    if (sound->version == 1)
    {
        sound->playloc = PLAYLOC_STREAM;
    }
    if (sound->version)
    {
        if (sound->version >= 1)
        {
            //version 1 not supported
            return 0;
        }
        sound->version = 0;
    }
    return EncodeVersion0Snr(filename, sound);
}

int sndplayerwclose(SINSTANCE* instance)
{
    int i = 1;
    if (instance->filestruct)
    {
        if (!gclose(instance->filestruct))
        {
            SIMEX_seterror("Error closing file in SIMEX_wclose().");
            i = 0;
        }
        instance->filestruct = 0;
    }
    return i;
}

SimexFuncs simexfuncs[56] = {
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //aiff 1
    { waveabout, waveid, waveopen, waveinfo, waveread, waveclose, wavecreate, wavewrite, wavewclose, 1 }, //wave 2
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 3
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //cdxa 4
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //vag 5
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //raw 6
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //n64hybrid 7
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pcbank 8
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pcpatch 9
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pcstream 10
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //psxbank 11
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //psxpatch 12
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //psxstream 13
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //n64bank 14
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //n64patch 15
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //macbank 16
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //macstream 17
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //earsvagstream 18
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //nbapssstream 19
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps2bank 20
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps2stream 21
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //gcbank 22
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //gcstream 23
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xboxbank 24
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xboxstream 25
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 26
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 27
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 28
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 29
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 30
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //quicktime 31
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 32
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 33
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //unk 34
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //mpeg 35
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //sndstream 36
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xenonbank 37
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pspbank 38
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xenonstream 39
    { sndplayerabout, sndplayerid, sndplayeropen, sndplayerinfo, sndplayerread, sndplayerclose, sndplayercreate, sndplayerwrite, sndplayerwclose, 1 }, //sndplayer 40
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pspstream 41
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //impulseresponse 42
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pcbankint 43
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pcstreamint 44
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xenonbankint 45
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //xenonstreamint 46
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pspbankint 47
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //pspstreamint 48
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps3bank 48
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps3stream 49
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps3bankint 50
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //ps3streamint 51
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //wiibank 52
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //wiistream 53
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //wiibankint 54
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0 }, //wiistreamint 55
    };

typedef int(*filterfunc)(SSOUND* sound, SIMEXFILTERPARAM *param);
int filterresample(SSOUND* sound, SIMEXFILTERPARAM* param);
void* AllocatorAlloc(int size)
{
    return new char[size];
}
int AllocatorFree(void* ptr)
{
    delete[] ptr;
    return 1;
}
void progressbar(int progress)
{
}
filterfunc filterfuncs[300] = {
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterloopstart
nullptr, //filterloopend
nullptr, //filterifnoloopstart
nullptr, //filterifnoloopend
nullptr, //filtermarktouser
nullptr, //filterseekable
nullptr, //filterfileformatversion
nullptr, //filterchunkspersecond
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filtercbr
nullptr,
nullptr, //filtercbr
nullptr,
nullptr,
nullptr,
nullptr, //filterplayloc
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterpriority
nullptr, //filtergigasampleperiod
nullptr,
nullptr, //filterirfsamplesperfftblock
nullptr, //filterirfpercentfreqcut
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterembeduserdata
nullptr,
nullptr, //filterembedtimestretch
nullptr, //filterremoveuserdatabyid
nullptr, //filterremoveuserdatabyval
nullptr, //filterremovealluserdata
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
filterresample, //filterresample
nullptr,
nullptr, //filterdownsampletofit
nullptr,
nullptr, //filteralignloop
nullptr,
nullptr, //filterpadloop
nullptr,
nullptr,
nullptr,
nullptr, //filterscaleamplitude
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterfadein
nullptr,
nullptr, //filterfadeout
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filtercrop
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filternochannelreordering
nullptr,
nullptr,
nullptr, //filterremix
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filtercopychan
nullptr, //filtercutchan
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterdistort
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterreverb
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr, //filterresamplefilter
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr,
nullptr
};

SIMEXFILTERABOUTPARAM resamplefilterparam = { "Sample Rate", "rate", "Specified as new sample rate in Hertz.", 400.0, 96000.0, nullptr, 0};

SIMEXFILTERABOUT filterabout[300] = {
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterloopstart
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterloopend
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterifnoloopstart
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterifnoloopend
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtermarktouser
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterseekable
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterfileformatversion
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterchunkspersecond
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtercbr
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtercbr
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterplayloc
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterpriority
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtergigasampleperiod
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterirfsamplesperfftblock
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterirfpercentfreqcut
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterembeduserdata
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterembedtimestretch
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterremoveuserdatabyid
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterremoveuserdatabyval
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterremovealluserdata
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ "Resample", "rs", "Resample to a new rate.", 1, 0, 1, &resamplefilterparam }, //filterresample
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterdownsampletofit
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filteralignloop
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterpadloop
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterscaleamplitude
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterfadein
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterfadeout
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtercrop
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filternochannelreordering
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterremix
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtercopychan
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filtercutchan
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterdistort
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterreverb
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }, //filterresamplefilter
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr },
{ nullptr, nullptr, nullptr, 0, 0, 0, nullptr }
};

SIMEXFILTERABOUT* SIMEX_filterabout(int type)
{
    if (type <= 299)
    {
        return (filterabout[type].name != 0 ? &filterabout[type] : 0);
    }
    SIMEX_seterror("Filter type is out of range.");
    return 0;
}

int SIMEX_filterssound(SSOUND* sound, int filter, SIMEXFILTERPARAM* param)
{
    if (filterfuncs[filter])
    {
        return filterfuncs[filter](sound, param);
    }
    SIMEX_seterror("Filter type is not implemented.");
    return -1;
}

const char* SIMEX_getlasterr()
{
    return simexerror;
}

int SIMEX_about(int type, SABOUT* about)
{
    if (about)
    {
        if (type > 55)
        {
            SIMEX_seterror("The file format passed to SIMEX_about is not supported.");
            return 0;
        }
        else if (simexfuncs[type].aboutfunc)
        {
            memset(about, 0, sizeof(SABOUT));
            return simexfuncs[type].aboutfunc(about);
        }
        else
        {
            SIMEX_seterror("The file format passed to SIMEX_about is not supported.");
            return 0;
        }
    }
    else
    {
        SIMEX_seterror("A NULL SABOUT was passed to SIMEX_about.");
        return 0;
    }
}

int SIMEX_id(const char* filename, long long offset)
{
    int i1 = -1;
    int i2 = 0;
    int i3 = 0;
    STDSTREAM *stream = gopen(filename);
    if (stream)
    {
        gseek(stream, offset);
    }
    int i4 = 0;
    int i5[1];
    i5[0] = mpeg;
    int i6 = 0;
    int i = 0;
    while (2)
    {
        int i7 = 0;
        while (i4 != i5[i7])
        {
            if (++i7 >= 1)
            {
                if (simexfuncs[i].idfunc && simexfuncs[i].isvalid)
                {
                    int i8 = simexfuncs[i].idfunc(filename, offset, stream);
                    i3 = i8;
                    if (i8 > i2)
                    {
                        i2 = i8;
                        i1 = i6;
                    }
                    if (stream)
                    {
                        gseek(stream, offset);
                    }
                }
                if (i3 == 100)
                {
                    goto l1;
                }
                i4 = i6;
                break;
            }
        }
        ++i4;
        ++i;
        i6 = i4;
        if (i < 56)
        {
            continue;
        }
        break;
    }
l1:
    if (i2 != 100)
    {
        int i9 = 0;
        for (i = 0; i < 1; ++i)
        {
            int i10 = i5[i9];
            if (simexfuncs[i10].idfunc && simexfuncs[i10].isvalid)
            {
                int i12 = simexfuncs[i10].idfunc(filename, offset, stream);
                i3 = i12;
                if (i12 > i2)
                {
                    i2 = i12;
                    i1 = i10;
                }
                if (stream)
                {
                    gseek(stream, offset);
                }
            }
            if (i3 == 100)
            {
                break;
            }
            i9 = i + 1;
        }
    }
    if (stream)
    {
        gclose(stream);
    }
    return i1;
}

int SIMEX_open(const char* filename, long long fileoffset, int filetype, SINSTANCE** instance)
{
    int i1 = 0;
    if (filetype <= 55)
    {
        bool b = simexfuncs[filetype].openfunc == 0;
        if (!b)
        {
            *instance = new SINSTANCE;
            if (*instance)
            {
                memset(*instance, 0, sizeof(SINSTANCE));
                (*instance)->numsounds = 1;
                (*instance)->fileoffset = fileoffset;
                (*instance)->filetype = filetype;
                const char *c1 = filename;
                char *c2 = (*instance)->filename;
                char c;
                do
                {
                    c = *c1;
                    *c2++ = *c1++;
                } while (c);
                (*instance)->filestruct = gopen(filename);
                if ((*instance)->filestruct)
                {
                    gseek((*instance)->filestruct, fileoffset);
                }
                i1 = simexfuncs[filetype].openfunc(*instance);
                if (i1 > 0)
                {
                    return i1;
                }
            }
            if ((*instance)->filestruct)
            {
                gclose((*instance)->filestruct);
            }
            if (*instance)
            {
                delete (*instance);
                *instance = 0;
            }
            return i1;
        }
    }
    SIMEX_seterror("The file format passed to SIMEX_open is not supported.");
    return 0;
}

int SIMEX_info(SINSTANCE* instance, SINFO** info, int element)
{
    int filetype = instance->filetype;
    if (filetype >= 0 && filetype < 56 && simexfuncs[filetype].infofunc)
    {
        gseek(instance->filestruct, instance->fileoffset);
        return simexfuncs[instance->filetype].infofunc(instance, info, element);
    }
    else
    {
        *info = 0;
        SIMEX_seterror("The file format passed to SIMEX_info is not supported.");
        return 0;
    }
}

int SIMEX_read(SINSTANCE* instance, SINFO* info, int element)
{
    if (info)
    {
        int filetype = instance->filetype;
        if (filetype >= 0 && filetype < 56 && simexfuncs[filetype].readfunc)
        {
            gseek(instance->filestruct, instance->fileoffset);
            return simexfuncs[instance->filetype].readfunc(instance, info, element);
        }
        else
        {
            SIMEX_seterror("The file format passed to SIMEX_read is not supported.");
            return 0;
        }
    }
    else
    {
        SIMEX_seterror("A NULL SINFO structure was passed to SIMEX_read.");
        return 0;
    }
}

int SIMEX_close(SINSTANCE* inst)
{
    int filetype = inst->filetype;
    int i1 = 0;
    if (filetype >= 0 && filetype < 56 && (simexfuncs[filetype].closefunc) != 0)
    {
        if (simexfuncs[filetype].closefunc(inst))
        {
            i1 = 1;
        }
    }
    else
    {
        SIMEX_seterror("The file format passed to SIMEX_close is not supported.");
    }
    if (inst->filestruct && !gclose(inst->filestruct))
    {
        if (i1)
        {
            SIMEX_seterror("Problem closing Simex::FileHandle.");
        }
        i1 = 0;
    }
    if (inst->filestruct2 && !gclose(inst->filestruct2))
    {
        if (i1)
        {
            SIMEX_seterror("Problem closing Simex::FileHandle.");
        }
        i1 = 0;
    }
    delete inst;
    return i1;
}

int SIMEX_create(const char* filename, unsigned int filetype, SINSTANCE** instance)
{
    *instance = 0;
    if (filetype <= 55 && simexfuncs[filetype].createfunc)
    {
        *instance = new SINSTANCE;
        if (*instance)
        {
            memset(*instance, 0, sizeof(SINSTANCE));
            char *c1 = (*instance)->filename;
            char c2;
            do
            {
                c2 = *filename;
                *c1++ = *filename++;
            } while (c2);
            (*instance)->filetype = filetype;
            if (simexfuncs[filetype].createfunc(*instance) > 0)
            {
                return 1;
            }
        }
    }
    else
    {
        SIMEX_seterror("The file format passed to SIMEX_create is not supported.");
    }
    if (*instance)
    {
        delete (*instance);
    }
    *instance = 0;
    return 0;
}

int SIMEX_wclose(SINSTANCE* instance)
{
    int filetype = instance->filetype;
    int result;
    if (filetype >= 0 && filetype < 56 && (simexfuncs[filetype].wclosefunc) != 0)
    {
        result = simexfuncs[filetype].wclosefunc(instance);
        delete instance;
    }
    else
    {
        SIMEX_seterror("The file format passed to SIMEX_wclose is not supported.");
        result = 0;
    }
    return result;
}

int codecmatch(int filetype, int codec)
{
    SABOUT about;
    if (!SIMEX_about(filetype, &about))
    {
        return 0;
    }
    char c = about.encodecodecs[0];
    if (about.encodecodecs[0] < 0)
    {
        return 0;
    }
    char *codecs = about.encodecodecs;
    while (codec != c)
    {
        c = *++codecs;
        if (c < 0)
        {
            return 0;
        }
    }
    return 1;
}

int playlocmatch(int a1, SSOUND* a2)
{
    SABOUT about;
    int result = SIMEX_about(a1, &about);
    if (result)
    {
        int i1 = (about.flags1 >> 29) & 1;
        int i2 = (about.flags1 >> 25) & 1;
        int i3 = (about.flags1 >> 26) & 1;
        int i4 = (about.flags1 >> 27) & 1;
        int i5 = (about.flags1 >> 28) & 1;
        int i6 = (about.flags1 >> 30) & 1;
        int i7 = (about.flags2 >> 1) & 1;
        if ((i7 + i6 + i1 + i5 + i4 + i3 + i2 + (about.flags1 >> 31) + (about.flags2 & 1) + ((about.flags1 >> 8) & 1)) > 1
            && (((a2->playloc & PLAYLOC_SPU) != 0) && !i3
                || (a2->playloc & PLAYLOC_MAINCPU) != 0 && !i2
                || (a2->playloc & PLAYLOC_IOPCPU) != 0 && !i4
                || (a2->playloc & PLAYLOC_DSP) != 0 && !i6
                || (a2->playloc & PLAYLOC_DS2DHW) != 0 && !i5
                || (a2->playloc & PLAYLOC_DS3DHW) != 0 && !i1
                || (a2->playloc & PLAYLOC_RAM) != 0 && about.flags1 >= 0
                || (a2->playloc & PLAYLOC_STREAM) != 0 && (about.flags2 & 1) == 0
                || (a2->playloc & PLAYLOC_GIGASAMPLE) != 0 && !i7))
        {
            SIMEX_seterror("The specified playloc is not supported by this exporter.");
            result = 0;
        }
        else
        {
            result = 1;
        }
    }
    return result;
}

int freessound(SINFO* info, int soundid)
{
    SSOUND *sound = info->sound[soundid];
    if (!sound)
    {
        return 0;
    }
    if (sound->scalingtable1)
    {
        delete[] sound->scalingtable1;
    }
    if (sound->scalingtable2)
    {
        delete[] sound->scalingtable2;
    }
    if (sound->scalingtable3)
    {
        delete[] sound->scalingtable3;
    }
    if (sound->scalingtable4)
    {
        delete[] sound->scalingtable4;
    }
    for (int i = 0; i < 4; i++)
    {
        delete[] sound->userdata[i];
    }
    for (int i = 0; i < sound->markercount; i++)
    {
        if (sound->markers[i])
        {
            for (int j = 0; j < sound->markers[i]->markercount; j++)
            {
                if (sound->markers[i]->markers[j])
                {
                    if (sound->markers[i]->markers[j]->stringsize)
                    {
                        delete[] sound->markers[i]->markers[j]->string;
                    }
                    delete sound->markers[i]->markers[j];
                    sound->markers[i]->markers[j] = 0;
                }
            }
            delete sound->markers[i];
        }
    }
    for (int i = 0; i < sound->channel_config; i++)
    {
        if (sound->decodedsamples[i])
        {
            delete[] sound->decodedsamples[i];
        }
    }
    for (int i = 0; i < 6; i++)
    {
        if (sound->timestretchvalues[i])
        {
            delete[] sound->timestretchvalues[i];
        }
        if (sound->unk31[i])
        {
            delete[] sound->unk31[i];
        }
        if (sound->unk43[i])
        {
            delete[] sound->unk43[i];
        }
    }
    delete sound;
    int count = info->soundcount-- == 1;
    int i1 = 0;
    int i2 = 0;
    if (!count)
    {
        do
        {
            if (!info->sound[i2])
            {
                for (SSOUND **s = &info->sound[i1] + i2; !*s; ++i1)
                    ++s;
                info->sound[i2] = *(&info->sound[i1] + i2);
                *(&info->sound[i1] + i2) = 0;
            }
            ++i2;
        } while (i2 < info->soundcount);
    }
    return 1;
}

int SIMEX_freesinfo(SINFO* info)
{
    if (!info)
    {
        return 0;
    }
    char i = info->soundcount;
    if (i)
    {
        while (freessound(info, i - 1))
        {
            i = info->soundcount;
            if (!i)
                goto l1;
        }
        return 0;
    }
l1:
    delete info;
    return 1;
}

int copysinfo(SINFO* dest, SINFO* src)
{
    memcpy(dest, src, sizeof(SINFO));
    for (int i = 0; i < src->soundcount; i++)
    {
        SSOUND* sound = src->sound[i];
        SSOUND* destsound = new SSOUND;
        memcpy(destsound, sound, sizeof(SSOUND));
        dest->sound[i] = destsound;
        for (int j = 0; j < 4; j++)
        {
            destsound->userdatasize[j] = sound->userdatasize[j];
            destsound->userdata[j] = new char[sound->userdatasize[j]];
            memcpy(destsound->userdata[j], sound->userdata[j], sound->userdatasize[j]);
        }
        for (int j = 0; j < 200; j++)
        {
            if (sound->markers[j])
            {
                destsound->markers[j] = new markerstruct;
                memset(destsound->markers[j], 0, sizeof(markerstruct));
                destsound->markers[j]->markercount = sound->markers[j]->markercount;
                for (int k = 0; k < sound->markers[j]->markercount; k++)
                {
                    if (sound->markers[j]->markers[k])
                    {
                        destsound->markers[j]->markers[k] = new siinfoaiff;
                        destsound->markers[j]->markers[k]->id = sound->markers[j]->markers[k]->id;
                        destsound->markers[j]->markers[k]->position = sound->markers[j]->markers[k]->position;
                        destsound->markers[j]->markers[k]->stringsize = sound->markers[j]->markers[k]->stringsize;
                        if (sound->markers[j]->markers[k]->stringsize)
                        {
                            destsound->markers[j]->markers[k]->string = new char[sound->markers[j]->markers[k]->stringsize];
                            memcpy(destsound->markers[j]->markers[k]->string, sound->markers[j]->markers[k]->string, sound->markers[j]->markers[k]->stringsize);
                        }
                    }
                    else
                    {
                        destsound->markers[j]->markers[k] = nullptr;
                    }
                }
            }
            else
            {
                sound->markers[j] = nullptr;
            }
        }
        if (sound->scalingtable1)
        {
            destsound->scalingtable1 = new char[128];
            memcpy(destsound->scalingtable1, sound->scalingtable1, 128);
        }
        if (sound->scalingtable2)
        {
            destsound->scalingtable2 = new char[128];
            memcpy(destsound->scalingtable2, sound->scalingtable2, 128);
        }
        if (sound->scalingtable3)
        {
            destsound->scalingtable3 = new char[128];
            memcpy(destsound->scalingtable3, sound->scalingtable3, 128);
        }
        if (sound->scalingtable4)
        {
            destsound->scalingtable4 = new char[128];
            memcpy(destsound->scalingtable4, sound->scalingtable4, 128);
        }
        for (int j = 0; j < sound->channel_config; j++)
        {
            if (sound->decodedsamples[j])
            {
                destsound->decodedsamples[j] = new short[sound->num_samples];
                memcpy(destsound->decodedsamples[j], sound->decodedsamples[j], 2 * sound->num_samples);
            }
        }
        for (int j = 0; j < 6; j++)
        {
            if (sound->timestretchvalues[j])
            {
                destsound->timestretchvalues[j] = new char[sound->timestretch];
                memcpy(destsound->timestretchvalues[j], sound->timestretchvalues[j], sound->timestretch);
            }
            if (sound->unk31[j])
            {
                destsound->unk31[j] = new char[sound->unk37[j]];
                memcpy(destsound->unk31[j], sound->unk31[j], sound->unk37[j]);
            }
            if (sound->unk43[j])
            {
                destsound->unk43[j] = new char[sound->unk49[j]];
                memcpy(destsound->unk43[j], sound->unk43[j], sound->unk49[j]);
            }
        }
    }
    return 1;
}

int SIMEX_write(SINSTANCE* instance, SINFO* info, int element)
{
    SSOUND* sound = nullptr;
    SINFO* info2 = nullptr;
    int ret = 1;
    SABOUT about;
    if (!SIMEX_about(instance->filetype, &about))
    {
        return 0;
    }
    int playloc = (about.flags2 & 1)
        + (about.flags1 >> 31)
        + ((about.flags1 >> 8) & 1)
        + ((about.flags1 >> 25) & 1)
        + ((about.flags1 >> 26) & 1)
        + ((about.flags1 >> 27) & 1)
        + ((about.flags1 >> 28) & 1)
        + ((about.flags1 >> 29) & 1)
        + ((about.flags1 >> 30) & 1)
        + ((about.flags2 >> 1) & 1);
    for (int soundcount = 0; soundcount < info->soundcount; soundcount++)
    {
        int sample_rate;
        sound = info->sound[soundcount];
        if (sound->sample_rate <= 0)
        {
            SIMEX_seterror("Invalid sample rate detected.");
            return 0;
        }
        if (sound->channel_config <= 0)
        {
            SIMEX_seterror("Invalid number of channels detected.");
            return 0;
        }
        if (sound->num_samples < 0)
        {
            SIMEX_seterror("Invalid length detected.");
            return 0;
        }
        if (sound->bitrate == -1)
        {
            sound->bitrate = 50;
        }
        if (sound->bitrate < 0)
        {
            SIMEX_seterror("Invalid bit rate detected.");
            return 0;
        }
        if (sound->channel_config > about.maxchannels)
        {
            SIMEX_seterror("This exporter is not capable of writing a file with this many channels in it.");
            return 0;
        }
        if (!codecmatch(instance->filetype, sound->codec))
        {
            SIMEX_seterror("This exporter does not support the sample representation specified.");
            return 0;
        }
        if (!playlocmatch(instance->filetype, sound))
        {
            return 0;
        }
        if (!playloc)
        {
            sound->playloc = 0;
        }
        char codec = sound->codec;
        if (codec == layer1)
        {
            int sample_rate = sound->sample_rate;
            if (sample_rate != 32000 && sample_rate != 44100 && sample_rate != 48000)
            {
                SIMEX_seterror("Samples compressed with MPEG Layers 1/2 must be 32000, 44100, or 48000 Hertz sources.");
                return 0;
            }
        }
        if (codec == layer2)
        {
            sample_rate = sound->sample_rate;
            if (sample_rate != 16000
                && sample_rate != 22050
                && sample_rate != 24000
                && sample_rate != 32000
                && sample_rate != 44100
                && sample_rate != 48000)
            {
                SIMEX_seterror("Samples compressed with MPEG Layer 2 must be 16000, 22050, 24000, 32000, 44100, or 48000 Hertz sources.");
                return 0;
            }
        }
        if (codec == eaxma)
        {
            int sample_rate = info->sound[soundcount]->sample_rate;
            if (sound->sample_rate > 48000)
            {
                info2 = new SINFO;
                if (!copysinfo(info2, info))
                {
                    return ret;
                }
                SIMEXFILTERPARAM param;
                param.intparam = 48000;
                SIMEX_filterssound(info->sound[soundcount], resample, &param);
                printwarning(
                    "Out-of-range EA-XMA sample rate of %i detected.  Max is 48000. Resampling to %i...\n",
                    sample_rate,
                    param.intparam);
            }
        }
        codec = sound->codec;
        if (codec != layer3)
        {
            if (codec == ealayer3)
            {
                goto l1;
            }
            if (codec != ealayer30_int && codec != ealayer3_int && codec != ealayer3pcm_int && codec != ealayer3spike_int)
            {
                goto l2;
            }
        }
        if (codec == ealayer30_int || codec == ealayer3_int || codec == ealayer3pcm_int || codec == ealayer3spike_int)
        {
        l1:
            if (sound->timestretch > 0)
            {
                SIMEX_seterror("Time stretch cannot be applied to EALayer3 compressed samples.");
                return 0;
            }
        }
        sample_rate = info->sound[soundcount]->sample_rate;
        if (codec == ealayer3
            || codec == ealayer30_int
            || codec == ealayer3_int
            || codec == ealayer3pcm_int
            || codec == ealayer3spike_int)
        {
            int sample_rate2 = sound->sample_rate;
            if (sample_rate2 != 16000
                && sample_rate2 != 22050
                && sample_rate2 != 24000
                && sample_rate2 != 32000
                && sample_rate2 != 44100
                && sample_rate2 != 48000)
            {
                SINFO *info2 = new SINFO;
                if (!copysinfo(info2, info))
                {
                    return ret;
                }
                SIMEXFILTERPARAM param;
                if (sample_rate >= 16000)
                {
                    if ((sample_rate - 16001) > 6048)
                    {
                        if ((sample_rate - 22051) > 1948)
                        {
                            if ((sample_rate - 24001) > 7998)
                            {
                                param.intparam = (sample_rate - 32001) > 12098 ? 48000 : 44100;
                            }
                            else
                            {
                                param.intparam = 32000;
                            }
                        }
                        else
                        {
                            param.intparam = 24000;
                        }
                    }
                    else
                    {
                        param.intparam = 22050;
                    }
                }
                else
                {
                    param.intparam = 16000;
                }
                SIMEX_filterssound(info->sound[soundcount], resample, &param);
                printwarning("Non-standard MPEG sample rate of %i detected.  Resampling to %i...\n", sample_rate, param.intparam);
            }
        }
        if (sound->codec == layer3)
        {
            int sample_rate = sound->sample_rate;
            if (sample_rate != 8000
                && sample_rate != 11025
                && sample_rate != 12000
                && sample_rate != 16000
                && sample_rate != 22050
                && sample_rate != 24000
                && sample_rate != 32000
                && sample_rate != 44100
                && sample_rate != 48000)
            {
                info2 = new SINFO;
                if (!copysinfo(info2, info))
                    return ret;
                SIMEXFILTERPARAM param;
                if (sample_rate >= 8000)
                {
                    if ((sample_rate - 8001) > 3023)
                    {
                        if ((sample_rate - 11026) > 973)
                        {
                            if ((sample_rate - 12001) > 3998)
                            {
                                if ((sample_rate - 16001) > 6048)
                                {
                                    if ((sample_rate - 22051) > 1948)
                                    {
                                        if ((sample_rate - 24001) > 7998)
                                            param.intparam = (sample_rate - 32001) > 12098 ? 48000 : 44100;
                                        else
                                            param.intparam = 32000;
                                    }
                                    else
                                    {
                                        param.intparam = 24000;
                                    }
                                }
                                else
                                {
                                    param.intparam = 22050;
                                }
                            }
                            else
                            {
                                param.intparam = 16000;
                            }
                        }
                        else
                        {
                            param.intparam = 12000;
                        }
                    }
                    else
                    {
                        param.intparam = 11025;
                    }
                }
                else
                {
                    param.intparam = 8000;
                }
                SIMEX_filterssound(info->sound[soundcount], resample, &param);
                printwarning(
                    "Non-standard MPEG sample rate of %i detected.  Resampling to %i...\n",
                    sample_rate,
                    param.intparam);
            }
        }
    l2:
        if (sound->loopstart >= 0 && sound->loopend < 0)
        {
            SIMEX_seterror("Malformed loop points, only start point set.");
            return 0;
        }
        if (sound->loopend >= 0)
        {
            if (sound->loopstart < 0)
            {
                SIMEX_seterror("Malformed loop points, only end point set.");
                return 0;
            }
            if (sound->loopend >= sound->num_samples)
            {
                SIMEX_seterror("Malformed loop points, end point past sample data.");
                return 0;
            }
        }
        if (sound->loopstart >= 0 && sound->loopend >= 0)
        {
            if (sound->loopstart > sound->loopend)
            {
                SIMEX_seterror("Malformed loop points, start point located after end point.");
                return 0;
            }
            if (sound->timestretch > 0)
            {
                SIMEX_seterror("Time stretch cannot be applied to samples that contain loop points.");
                return 0;
            }
        }
        if (sound->timestretch > 0 && sound->channel_config >= 2)
        {
            SIMEX_seterror("Time stretch can only be applied to mono samples.");
            return 0;
        }
    }
    if (info->chunkrateflag && instance->filetype == sndstream)
    {
        info->chunkrateflag = 0;
        info->chunkrate = 5.0;
    }
    SINFO* info3 = new SINFO;
    for (int i = 0; i < sound->markercount; i++)
    {
        for (int j = 0; j < sound->markers[i]->markercount; j++)
        {
            if (sound->markers[i]->markers[j])
            {
                if (sound->markers[i]->markers[j]->position > sound->num_samples || sound->markers[i]->markers[j]->position < 0)
                {
                    SIMEX_seterror("Bad Marker detected.");
                }
            }
        }
    }
    memcpy(info3, info, sizeof(SINFO));
    int filetype = instance->filetype;
    if (filetype >= 0 && filetype < 56 && simexfuncs[filetype].writefunc)
    {
        ret = simexfuncs[filetype].writefunc(instance, info, element);
        if (memcmp(info3, info, sizeof(SINFO)))
        {
            SIMEX_seterror("INTERNAL ERROR: Source SINFO was modified by exporter.");
            ret = 0;
        }
        if (!info2)
        {
            goto l4;
        }
        copysinfo(info, info2);
        SIMEX_freesinfo(info2);
    }
    else
    {
        SIMEX_seterror("The file format passed to SIMEX_write is not supported.");
        ret = 0;
    }
    if (info2)
    {
        int i2 = info3->soundcount;
        if (i2)
        {

            while (freessound(info3, i2 - 1))
            {
                i2 = info3->soundcount;
                if (!i2)
                {
                    goto l4;
                }
            }
            return ret;
        }
    }
l4:
    delete info3;
    return ret;
}

const char* codecnames[33] = {
    "Signed 16-Bit Little-Endian Interleaved",
    "Signed 16-Bit Big-Endian Interleaved",
    "Signed 8-Bit Interleaved",
    "EA-XA Interleaved (3.7:1)",
    "MicroTalk Block (10:1)",
    "Sony VAG Block (3.5:1)",
    "N64 Block (3.5:1)",
    "Signed 16-Bit Big-Endian Block",
    "Signed 16-Bit Little-Endian Block",
    "Signed 8-Bit Block",
    "EA-XA Block (3.7:1)",
    "Unsigned 8-Bit Interleaved",
    "CD-XA Interleaved (3.5:1)",
    "DVI Interleaved (4:1)",
    "MPEG Layer 1",
    "MPEG Layer 2",
    "MPEG Layer 3",
    nullptr,
    "GameCube ADPCM (3.5:1)",
    "Signed 24-Bit Little-Endian Interleaved",
    "Xbox ADPCM (3.5:1)",
    "Signed 24-Bit Big-Endian Interleaved",
    "MicroTalk Block (5:1)",
    "EALayer3 (Modified MPEG Layer 3)",
    "XAS Interleaved (XA Seekable)",
    "EALayer3 Interleaved (Modified MP3)",
    "ATRAC3 Interleaved ",
    "ATRAC3plus Interleaved ",
    "EA-XMA ",
    "XAS Interleaved (Version 1)",
    "EALayer3 Interleaved (Version 1)",
    "EALayer3 Interleaved (Version 2) PCM",
    "EALayer3 Interleaved (Version 2) Spike"
};

const char* SIMEX_getsamplerepname(unsigned int codec)
{
    if (codec > 32)
    {
        return "Unknown";
    }
    return codecnames[codec];
}
