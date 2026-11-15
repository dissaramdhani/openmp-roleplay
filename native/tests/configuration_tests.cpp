#include "roleplay/configuration.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

void require(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

class TemporaryConfiguration
{
public:
    TemporaryConfiguration(const std::filesystem::path& sourceRoot, const std::string& environment)
    {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("roleplay-config-test-" + std::to_string(timestamp));
        std::filesystem::create_directories(root_);
        std::filesystem::copy(
            sourceRoot / environment,
            root_ / environment,
            std::filesystem::copy_options::recursive);
    }

    ~TemporaryConfiguration()
    {
        std::error_code error;
        std::filesystem::remove_all(root_, error);
    }

    const std::filesystem::path& root() const
    {
        return root_;
    }

    std::filesystem::path file(const std::string& environment, const std::string& category) const
    {
        return root_ / environment / (category + ".json");
    }

    void write(const std::string& environment, const std::string& category, const std::string& contents) const
    {
        std::ofstream output(file(environment, category), std::ios::binary | std::ios::trunc);
        require(static_cast<bool>(output), "Could not write temporary configuration fixture.");
        output << contents;
        require(static_cast<bool>(output), "Could not finish writing temporary configuration fixture.");
    }

private:
    std::filesystem::path root_;
};

void testDevelopmentDefaults(const std::filesystem::path& configRoot)
{
    roleplay::Configuration configuration(configRoot, "development");
    std::string error;
    require(configuration.load(error), error);

    std::string name;
    require(configuration.getString("server.name", name), "server.name was not available.");
    require(name == "OpenMP Roleplay Development", "Development server name was not loaded.");

    std::int64_t startingCash = 0;
    require(configuration.getInt("gameplay.starting_cash", startingCash), "starting_cash was not available.");
    require(startingCash == 500, "The safe starting_cash default was not applied.");

    std::int64_t poolSize = 0;
    require(configuration.getInt("database.pool_size", poolSize), "pool_size was not available.");
    require(poolSize == 5, "The safe pool_size default was not applied.");

    std::string username;
    require(configuration.getString("database.username", username), "Database username was not loaded.");
    require(username == "openmp_dev", "The development database username was not loaded.");

    double payoutMultiplier = 0;
    require(configuration.getFloat("jobs.payout_multiplier", payoutMultiplier), "payout_multiplier was not available.");
    require(payoutMultiplier == 1.0, "The safe payout_multiplier default was not applied.");

    bool logChat = true;
    require(configuration.getBool("logging.log_chat", logChat), "log_chat was not available.");
    require(!logChat, "The safe log_chat default was not applied.");

    require(!configuration.getInt("logging.log_chat", startingCash), "A boolean was returned as an integer.");
    require(!configuration.getInt("missing.key", startingCash), "An unknown key was returned.");
}

void testProductionEnvironment(const std::filesystem::path& configRoot)
{
    roleplay::Configuration configuration(configRoot, "production");
    std::string error;
    require(configuration.load(error), error);

    std::string name;
    require(configuration.getString("server.name", name), "Production server name was not available.");
    require(name == "OpenMP Roleplay", "Production environment was not selected explicitly.");
}

void testRejectsInvalidEnvironment(const std::filesystem::path& configRoot)
{
    roleplay::Configuration configuration(configRoot, "prod");
    std::string error;
    require(!configuration.load(error), "An unknown environment was accepted.");
    require(error.find("OPENMP_ENV") != std::string::npos, "Environment error was not actionable.");
}

void testRejectsMalformedJson(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration temporary(configRoot, "development");
    temporary.write("development", "gameplay", "{\"schema_version\": 1");

    roleplay::Configuration configuration(temporary.root(), "development");
    std::string error;
    require(!configuration.load(error), "Malformed JSON was accepted.");
    require(error.find("gameplay.json") != std::string::npos, "Malformed JSON error omitted the file.");
}

void testRejectsUnsupportedSchema(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration temporary(configRoot, "development");
    temporary.write("development", "server", "{\"schema_version\": 2, \"name\": \"Invalid\"}");

    roleplay::Configuration configuration(temporary.root(), "development");
    std::string error;
    require(!configuration.load(error), "An unsupported schema version was accepted.");
    require(error.find("schema_version 1") != std::string::npos, "Schema version error was not actionable.");
}

void testRejectsInvalidValues(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration wrongType(configRoot, "development");
    wrongType.write("development", "gameplay", "{\"schema_version\": 1, \"starting_cash\": \"500\"}");

    roleplay::Configuration typeConfiguration(wrongType.root(), "development");
    std::string error;
    require(!typeConfiguration.load(error), "A value with the wrong type was accepted.");
    require(error.find("starting_cash must be an integer") != std::string::npos, "Type error was not actionable.");

    TemporaryConfiguration outOfRange(configRoot, "development");
    outOfRange.write("development", "security", "{\"schema_version\": 1, \"max_login_attempts\": 0}");

    roleplay::Configuration rangeConfiguration(outOfRange.root(), "development");
    error.clear();
    require(!rangeConfiguration.load(error), "An out-of-range value was accepted.");
    require(error.find("max_login_attempts") != std::string::npos, "Range error omitted the key.");
}

void testRejectsUnknownKeysAndMissingFiles(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration unknownKey(configRoot, "development");
    unknownKey.write(
        "development",
        "gameplay",
        "{\"schema_version\": 1, \"starting_cash\": 500, \"unexpected\": true}");

    roleplay::Configuration unknownKeyConfiguration(unknownKey.root(), "development");
    std::string error;
    require(!unknownKeyConfiguration.load(error), "An unknown configuration key was accepted.");
    require(error.find("unexpected") != std::string::npos, "Unknown-key error omitted the key.");

    TemporaryConfiguration missingFile(configRoot, "development");
    std::filesystem::remove(missingFile.file("development", "vehicle"));

    roleplay::Configuration missingFileConfiguration(missingFile.root(), "development");
    error.clear();
    require(!missingFileConfiguration.load(error), "A missing category file was accepted.");
    require(error.find("vehicle.json") != std::string::npos, "Missing-file error omitted the file.");
}

void testRejectsInvalidSecretEnvironmentName(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration temporary(configRoot, "development");
    temporary.write(
        "development",
        "database",
        "{\"schema_version\": 1, \"host\": \"127.0.0.1\", \"port\": 3306, "
        "\"database\": \"openmp_roleplay_dev\", \"username\": \"openmp_dev\", "
        "\"password_env\": \"OPENMP-DATABASE-PASSWORD\"}");

    roleplay::Configuration configuration(temporary.root(), "development");
    std::string error;
    require(!configuration.load(error), "An invalid secret environment variable name was accepted.");
    require(error.find("uppercase environment variable name") != std::string::npos,
            "Secret environment variable error was not actionable.");
}

void testReloadIsAtomic(const std::filesystem::path& configRoot)
{
    TemporaryConfiguration temporary(configRoot, "development");
    roleplay::Configuration configuration(temporary.root(), "development");
    std::string error;
    require(configuration.load(error), error);

    temporary.write("development", "server", "{\"schema_version\": 1, \"name\": \"Reloaded\"}");
    require(configuration.reload(error), error);

    std::string name;
    require(configuration.getString("server.name", name), "Reloaded server name was not available.");
    require(name == "Reloaded", "A valid reload did not become active.");

    temporary.write("development", "server", "{\"schema_version\": 9, \"name\": \"Rejected\"}");
    error.clear();
    require(!configuration.reload(error), "An invalid reload was accepted.");
    require(configuration.getString("server.name", name), "Active configuration disappeared after a failed reload.");
    require(name == "Reloaded", "A failed reload replaced the active configuration.");
}

}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Expected the configuration root path as the only argument.\n";
        return 2;
    }

    const std::filesystem::path configRoot(argv[1]);
    const std::vector<std::pair<std::string, void (*)(const std::filesystem::path&)>> tests = {
        {"development defaults", testDevelopmentDefaults},
        {"production environment", testProductionEnvironment},
        {"invalid environment", testRejectsInvalidEnvironment},
        {"malformed JSON", testRejectsMalformedJson},
        {"unsupported schema", testRejectsUnsupportedSchema},
        {"invalid values", testRejectsInvalidValues},
        {"unknown keys and missing files", testRejectsUnknownKeysAndMissingFiles},
        {"invalid secret environment name", testRejectsInvalidSecretEnvironmentName},
        {"atomic reload", testReloadIsAtomic}};

    for (const auto& test : tests)
    {
        try
        {
            test.second(configRoot);
            std::cout << "[PASS] " << test.first << '\n';
        }
        catch (const std::exception& exception)
        {
            std::cerr << "[FAIL] " << test.first << ": " << exception.what() << '\n';
            return 1;
        }
    }

    return 0;
}
