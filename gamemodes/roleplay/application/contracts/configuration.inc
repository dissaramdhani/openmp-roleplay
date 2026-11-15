#if defined _roleplay_application_contracts_configuration_included
    #endinput
#endif
#define _roleplay_application_contracts_configuration_included

forward bool:Roleplay_Config_Reload();
forward bool:Roleplay_Config_GetInt(const key[], &value);
forward bool:Roleplay_Config_GetFloat(const key[], &Float:value);
forward bool:Roleplay_Config_GetBool(const key[], &bool:value);
forward bool:Roleplay_Config_GetString(const key[], value[], maxlen);
