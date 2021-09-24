#include "simex.h"

int main()
{
    SIMEX_init();
    int type = SIMEX_id("9.snr", 0);
    SINSTANCE* instance;
    SIMEX_open("9.snr", 0, type, &instance);
    SINSTANCE* instance2;
    SIMEX_create("9.wav", wave, &instance2);
    SINFO* info;
    SIMEX_info(instance, &info, 0);
    if (info)
    {
        SIMEX_read(instance, info, 0);
        info->sound[0]->codec = s16l_int;
        SIMEX_write(instance2, info, 0);
        SIMEX_freesinfo(info);
    }
    SIMEX_close(instance);
    SIMEX_wclose(instance2);
    SIMEX_shutdown();

    SIMEX_init();
    type = SIMEX_id("9.wav", 0);
    instance;
    SIMEX_open("9.wav", 0, type, &instance);
    instance2;
    SIMEX_create("9out", sndplayer, &instance2);
    info;
    SIMEX_info(instance, &info, 0);
    if (info)
    {
        SIMEX_read(instance, info, 0);
        info->sound[0]->codec = ealayer3_int;
        info->sound[0]->playloc = PLAYLOC_STREAM;
        SIMEX_write(instance2, info, 0);
        SIMEX_freesinfo(info);
    }
    SIMEX_close(instance);
    SIMEX_wclose(instance2);
    SIMEX_shutdown();
}
