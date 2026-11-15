#include "roleplay/configuration.hpp"

#include <spsdk/Logger.hh>
#include <spsdk/Plugin.hh>

#include <array>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <string>

namespace
{

constexpr std::size_t kKeyBufferSize = 128;
constexpr std::size_t kMaximumPawnStringLength = 256;

std::optional<std::string> readEnvironmentVariable(const char* name)
{
#ifdef _MSC_VER
    char* value = nullptr;
    std::size_t size = 0;
    const int result = _dupenv_s(&value, &size, name);
    std::unique_ptr<char, decltype(&std::free)> ownedValue(value, &std::free);
    if (result != 0)
    {
        spsdk::Logger::global.logLnF(
            spsdk::LogLevel::Error,
            "Could not read environment variable '%s'.",
            name);
        return std::nullopt;
    }

    if (value == nullptr)
    {
        return std::nullopt;
    }

    return std::string(value);
#else
    const char* value = std::getenv(name);
    return value == nullptr ? std::nullopt : std::optional<std::string>(value);
#endif
}

class RoleplayConfigurationPlugin final : public spsdk::IPlugin
{
public:
    std::string getPluginName() const override
    {
        return "Roleplay Configuration";
    }

    bool init() override
    {
        const auto environment = readEnvironmentVariable("OPENMP_ENV");
        const auto configuredRoot = readEnvironmentVariable("OPENMP_CONFIG_ROOT");
        const std::filesystem::path root =
            configuredRoot && !configuredRoot->empty()
                ? std::filesystem::path(*configuredRoot)
                : std::filesystem::path("config");

        configuration_ = roleplay::Configuration(root, environment.value_or(""));
        std::string error;
        if (!configuration_.load(error))
        {
            spsdk::Logger::global.logLnF(
                spsdk::LogLevel::Error,
                "Startup configuration rejected: %s",
                error.c_str());
            return true;
        }

        spsdk::Logger::global.logLnF(
            spsdk::LogLevel::Info,
            "Loaded schema v1 configuration for environment '%s'.",
            environment->c_str());
        return true;
    }

    void free() override
    {
        delete this;
    }

    int onAmxLoaded(AMX* amx) override
    {
        static AMX_NATIVE_INFO natives[] = {
            {"Roleplay_Native_Config_Reload", nativeReload},
            {"Roleplay_Native_Config_GetInt", nativeGetInt},
            {"Roleplay_Native_Config_GetFloat", nativeGetFloat},
            {"Roleplay_Native_Config_GetBool", nativeGetBool},
            {"Roleplay_Native_Config_GetString", nativeGetString},
            {nullptr, nullptr}};
        return amx_Register(amx, natives, -1);
    }

    int onAmxUnload(AMX*) override
    {
        return AMX_ERR_NONE;
    }

private:
    static bool readKey(AMX* amx, cell address, std::string& key)
    {
        cell* physicalAddress = nullptr;
        if (amx_GetAddr(amx, address, &physicalAddress) != AMX_ERR_NONE)
        {
            return false;
        }

        std::array<char, kKeyBufferSize> buffer {};
        if (amx_GetString(buffer.data(), physicalAddress, 0, buffer.size()) != AMX_ERR_NONE)
        {
            return false;
        }

        key = buffer.data();
        return !key.empty();
    }

    static bool writeCell(AMX* amx, cell address, cell value)
    {
        cell* physicalAddress = nullptr;
        if (amx_GetAddr(amx, address, &physicalAddress) != AMX_ERR_NONE)
        {
            return false;
        }

        *physicalAddress = value;
        return true;
    }

    static cell AMX_NATIVE_CALL nativeReload(AMX*, cell* params)
    {
        if (params[0] != 0)
        {
            return 0;
        }

        std::string error;
        if (!instance().configuration_.reload(error))
        {
            spsdk::Logger::global.logLnF(spsdk::LogLevel::Error, "Configuration reload rejected: %s", error.c_str());
            return 0;
        }

        spsdk::Logger::global.logLnF(spsdk::LogLevel::Info, "Configuration reloaded successfully.");
        return 1;
    }

    static cell AMX_NATIVE_CALL nativeGetInt(AMX* amx, cell* params)
    {
        if (params[0] != 2 * static_cast<cell>(sizeof(cell)))
        {
            return 0;
        }

        std::string key;
        std::int64_t value = 0;
        if (!readKey(amx, params[1], key) ||
            !instance().configuration_.getInt(key, value) ||
            value < std::numeric_limits<cell>::min() ||
            value > std::numeric_limits<cell>::max())
        {
            return 0;
        }

        return writeCell(amx, params[2], static_cast<cell>(value)) ? 1 : 0;
    }

    static cell AMX_NATIVE_CALL nativeGetFloat(AMX* amx, cell* params)
    {
        if (params[0] != 2 * static_cast<cell>(sizeof(cell)))
        {
            return 0;
        }

        std::string key;
        double value = 0;
        if (!readKey(amx, params[1], key) || !instance().configuration_.getFloat(key, value))
        {
            return 0;
        }

        return writeCell(amx, params[2], amx_ftoc(static_cast<float>(value))) ? 1 : 0;
    }

    static cell AMX_NATIVE_CALL nativeGetBool(AMX* amx, cell* params)
    {
        if (params[0] != 2 * static_cast<cell>(sizeof(cell)))
        {
            return 0;
        }

        std::string key;
        bool value = false;
        if (!readKey(amx, params[1], key) || !instance().configuration_.getBool(key, value))
        {
            return 0;
        }

        return writeCell(amx, params[2], value ? 1 : 0) ? 1 : 0;
    }

    static cell AMX_NATIVE_CALL nativeGetString(AMX* amx, cell* params)
    {
        if (params[0] != 3 * static_cast<cell>(sizeof(cell)) ||
            params[3] <= 0 ||
            static_cast<std::size_t>(params[3]) > kMaximumPawnStringLength)
        {
            return 0;
        }

        std::string key;
        std::string value;
        if (!readKey(amx, params[1], key) || !instance().configuration_.getString(key, value) ||
            value.size() >= static_cast<std::size_t>(params[3]))
        {
            return 0;
        }

        cell* physicalAddress = nullptr;
        if (amx_GetAddr(amx, params[2], &physicalAddress) != AMX_ERR_NONE)
        {
            return 0;
        }

        if (amx_SetString(physicalAddress, value.c_str(), 0, 0, static_cast<std::size_t>(params[3])) != AMX_ERR_NONE)
        {
            return 0;
        }

        return 1;
    }

    static RoleplayConfigurationPlugin& instance()
    {
        return *static_cast<RoleplayConfigurationPlugin*>(pluginInstance_);
    }

    static void setInstance(RoleplayConfigurationPlugin* plugin)
    {
        pluginInstance_ = plugin;
    }

    roleplay::Configuration configuration_ {"config", ""};
    static RoleplayConfigurationPlugin* pluginInstance_;

public:
    static RoleplayConfigurationPlugin* create()
    {
        auto* plugin = new RoleplayConfigurationPlugin();
        setInstance(plugin);
        return plugin;
    }
};

RoleplayConfigurationPlugin* RoleplayConfigurationPlugin::pluginInstance_ = nullptr;

}

SPSDK_GET_PLUGIN()
{
    return RoleplayConfigurationPlugin::create();
}
