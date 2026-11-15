#if defined _roleplay_application_startup_included
    #endinput
#endif
#define _roleplay_application_startup_included

#include "roleplay/application/contracts/logging.inc"

stock Roleplay_Application_Start()
{
    Roleplay_Log("[roleplay] Application started.");
    return 1;
}
