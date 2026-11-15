#if defined _roleplay_infrastructure_adapters_configuration_included
    #endinput
#endif
#define _roleplay_infrastructure_adapters_configuration_included

#include "roleplay/application/contracts/configuration.inc"

native bool:Roleplay_Native_Config_Reload();
native bool:Roleplay_Native_Config_GetInt(const key[], &value);
native bool:Roleplay_Native_Config_GetFloat(const key[], &Float:value);
native bool:Roleplay_Native_Config_GetBool(const key[], &bool:value);
native bool:Roleplay_Native_Config_GetString(const key[], value[], maxlen);

stock bool:Roleplay_Config_Reload()
{
    return Roleplay_Native_Config_Reload();
}

stock bool:Roleplay_Config_GetInt(const key[], &value)
{
    return Roleplay_Native_Config_GetInt(key, value);
}

stock bool:Roleplay_Config_GetFloat(const key[], &Float:value)
{
    return Roleplay_Native_Config_GetFloat(key, value);
}

stock bool:Roleplay_Config_GetBool(const key[], &bool:value)
{
    return Roleplay_Native_Config_GetBool(key, value);
}

stock bool:Roleplay_Config_GetString(const key[], value[], maxlen)
{
    return Roleplay_Native_Config_GetString(key, value, maxlen);
}
