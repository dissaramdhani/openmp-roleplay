#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace roleplay
{

using ConfigurationValue = std::variant<std::int64_t, double, bool, std::string>;

class Configuration
{
public:
    Configuration(std::filesystem::path root, std::string environment);

    bool load(std::string& error);
    bool reload(std::string& error);

    bool getInt(const std::string& key, std::int64_t& value) const;
    bool getFloat(const std::string& key, double& value) const;
    bool getBool(const std::string& key, bool& value) const;
    bool getString(const std::string& key, std::string& value) const;

private:
    struct Snapshot
    {
        std::unordered_map<std::string, ConfigurationValue> values;
    };

    bool createSnapshot(std::shared_ptr<const Snapshot>& snapshot, std::string& error) const;
    std::shared_ptr<const Snapshot> currentSnapshot() const;

    std::filesystem::path root_;
    std::string environment_;
    std::shared_ptr<const Snapshot> snapshot_;
};

}
