#if defined _roleplay_application_startup_included
    #endinput
#endif
#define _roleplay_application_startup_included

#include "roleplay/application/contracts/configuration.inc"
#include "roleplay/application/contracts/logging.inc"

stock Roleplay_Application_Start()
{
    new serverName[81];
    if (!Roleplay_Config_GetString("server.name", serverName, sizeof(serverName)))
    {
        Roleplay_Log("[roleplay] Startup aborted: required server.name configuration is unavailable.");
        SendRconCommand("exit");
        return 0;
    }

    new message[128];
    format(message, sizeof(message), "[roleplay] Application started: %s", serverName);
    Roleplay_Log(message);
    return 1;
}
