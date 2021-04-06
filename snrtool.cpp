#include "simex.h"

int main()
{
    SIMEX_init();
    int type = SIMEX_id("c:\\temp\\9tennis.wav", 0);
    SINSTANCE* instance;
    SIMEX_open("c:\\temp\\9tennis.wav", 0, type, &instance);
    SINSTANCE* instance2;
    SIMEX_create("c:\\temp\\testx", sndplayer, &instance2);
    SINFO* info;
    SIMEX_info(instance, &info, 0);
    if (info)
    {
        SIMEX_read(instance, info, 0);
        info->sound[0]->codec = ealayer3pcm_int;
        SIMEXFILTERPARAM param;
        param.intparam = 48000;
        SIMEX_filterssound(info->sound[0], resample, &param);
        SIMEX_write(instance2, info, 0);
        SIMEX_freesinfo(info);
    }
    SIMEX_close(instance);
    SIMEX_wclose(instance2);
    SIMEX_shutdown();
}
