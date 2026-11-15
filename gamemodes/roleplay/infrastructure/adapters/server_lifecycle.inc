#if defined _roleplay_infrastructure_adapters_server_lifecycle_included
    #endinput
#endif
#define _roleplay_infrastructure_adapters_server_lifecycle_included

#include "roleplay/application/contracts/server_lifecycle.inc"

stock Roleplay_Server_Shutdown()
{
    SendRconCommand("exit");
}
