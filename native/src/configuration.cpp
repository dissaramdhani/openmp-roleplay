#include "roleplay/configuration.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace roleplay
{
namespace
{

using Json = nlohmann::json;

constexpr int kSchemaVersion = 1;
constexpr std::uintmax_t kMaximumFileSize = 64 * 1024;

enum class ValueType
{
    String,
    Integer,
    Number,
    Boolean
};

struct FieldSpec
{
    std::string name;
    ValueType type;
    bool required;
    std::optional<ConfigurationValue> defaultValue;
    double minimum;
    double maximum;
    std::size_t minimumLength;
    std::size_t maximumLength;
    std::vector<std::string> allowedValues;
};

struct CategorySpec
{
    std::string name;
    std::vector<FieldSpec> fields;
};

FieldSpec stringField(
    std::string name,
    bool required,
    std::optional<std::string> defaultValue,
    std::size_t minimumLength,
    std::size_t maximumLength,
    std::vector<std::string> allowedValues = {})
{
    std::optional<ConfigurationValue> value;
    if (defaultValue)
    {
        value = *defaultValue;
    }

    return {
        std::move(name),
        ValueType::String,
        required,
        std::move(value),
        0,
        0,
        minimumLength,
        maximumLength,
        std::move(allowedValues)};
}

FieldSpec integerField(
    std::string name,
    bool required,
    std::optional<std::int64_t> defaultValue,
    std::int64_t minimum,
    std::int64_t maximum)
{
    std::optional<ConfigurationValue> value;
    if (defaultValue)
    {
        value = *defaultValue;
    }

    return {
        std::move(name),
        ValueType::Integer,
        required,
        std::move(value),
        static_cast<double>(minimum),
        static_cast<double>(maximum),
        0,
        0,
        {}};
}

FieldSpec numberField(
    std::string name,
    bool required,
    std::optional<double> defaultValue,
    double minimum,
    double maximum)
{
    std::optional<ConfigurationValue> value;
    if (defaultValue)
    {
        value = *defaultValue;
    }

    return {
        std::move(name),
        ValueType::Number,
        required,
        std::move(value),
        minimum,
        maximum,
        0,
        0,
        {}};
}

FieldSpec booleanField(std::string name, bool required, std::optional<bool> defaultValue)
{
    std::optional<ConfigurationValue> value;
    if (defaultValue)
    {
        value = *defaultValue;
    }

    return {
        std::move(name),
        ValueType::Boolean,
        required,
        std::move(value),
        0,
        0,
        0,
        0,
        {}};
}

const std::vector<CategorySpec>& schema()
{
    static const std::vector<CategorySpec> categories = {
        {"server",
         {stringField("name", true, std::nullopt, 1, 80),
          stringField("language", false, std::string("id"), 1, 16)}},
        {"database",
         {stringField("host", true, std::nullopt, 1, 253),
          integerField("port", true, std::nullopt, 1, 65535),
          stringField("database", true, std::nullopt, 1, 64),
          stringField("username", true, std::nullopt, 1, 64),
          stringField("password_env", true, std::nullopt, 1, 64),
          integerField("pool_size", false, std::int64_t(5), 1, 50)}},
        {"gameplay",
         {integerField("starting_cash", false, std::int64_t(500), 0, 2000000000),
          integerField("max_characters", false, std::int64_t(3), 1, 10)}},
        {"economy",
         {stringField("currency_symbol", false, std::string("$"), 1, 4),
          integerField("maximum_cash", false, std::int64_t(2000000000), 1, 2000000000)}},
        {"vehicle",
         {integerField("max_personal_vehicles", false, std::int64_t(5), 0, 100),
          numberField("max_vehicle_speed", false, 200.0, 1.0, 400.0)}},
        {"jobs",
         {integerField("max_active_jobs", false, std::int64_t(1), 1, 10),
          numberField("payout_multiplier", false, 1.0, 0.1, 10.0)}},
        {"security",
         {integerField("max_login_attempts", false, std::int64_t(5), 1, 20),
          integerField("login_lockout_seconds", false, std::int64_t(900), 0, 86400)}},
        {"authentication",
         {integerField("bcrypt_cost", false, std::int64_t(12), 4, 16),
          integerField("password_min_length", false, std::int64_t(8), 8, 128)}},
        {"logging",
         {stringField("level", false, std::string("info"), 1, 8, {"debug", "info", "warn", "error"}),
          booleanField("log_chat", false, false)}},
        {"performance",
         {integerField("timer_interval_ms", false, std::int64_t(1000), 50, 60000),
          integerField("worker_threads", false, std::int64_t(2), 1, 32)}}};

    return categories;
}

bool validateField(
    const Json& value,
    const FieldSpec& field,
    ConfigurationValue& result,
    const std::string& displayPath,
    std::string& error)
{
    const std::string key = displayPath + "." + field.name;

    switch (field.type)
    {
    case ValueType::String:
    {
        if (!value.is_string())
        {
            error = key + " must be a string.";
            return false;
        }

        const std::string text = value.get<std::string>();
        if (text.size() < field.minimumLength || text.size() > field.maximumLength)
        {
            error = key + " must contain between " + std::to_string(field.minimumLength) +
                    " and " + std::to_string(field.maximumLength) + " characters.";
            return false;
        }

        if (field.name == "password_env")
        {
            const auto isValidCharacter = [](char character) {
                return (character >= 'A' && character <= 'Z') ||
                       (character >= '0' && character <= '9') ||
                       character == '_';
            };
            const bool startsWithLetter =
                (text.front() >= 'A' && text.front() <= 'Z') || text.front() == '_';
            if (!startsWithLetter ||
                !std::all_of(text.begin(), text.end(), isValidCharacter))
            {
                error = key + " must be an uppercase environment variable name.";
                return false;
            }
        }

        if (!field.allowedValues.empty() &&
            std::find(field.allowedValues.begin(), field.allowedValues.end(), text) == field.allowedValues.end())
        {
            error = key + " must be one of: ";
            for (std::size_t index = 0; index < field.allowedValues.size(); ++index)
            {
                if (index > 0)
                {
                    error += ", ";
                }
                error += field.allowedValues[index];
            }
            error += ".";
            return false;
        }

        result = text;
        return true;
    }
    case ValueType::Integer:
    {
        if (!value.is_number_integer())
        {
            error = key + " must be an integer.";
            return false;
        }

        const long double number = value.is_number_unsigned()
                                       ? static_cast<long double>(value.get<std::uint64_t>())
                                       : static_cast<long double>(value.get<std::int64_t>());
        if (number < field.minimum || number > field.maximum)
        {
            error = key + " must be between " + std::to_string(static_cast<std::int64_t>(field.minimum)) +
                    " and " + std::to_string(static_cast<std::int64_t>(field.maximum)) + ".";
            return false;
        }

        result = static_cast<std::int64_t>(number);
        return true;
    }
    case ValueType::Number:
    {
        if (!value.is_number())
        {
            error = key + " must be a number.";
            return false;
        }

        const double number = value.get<double>();
        if (!std::isfinite(number) || number < field.minimum || number > field.maximum)
        {
            error = key + " must be a finite number between " + std::to_string(field.minimum) +
                    " and " + std::to_string(field.maximum) + ".";
            return false;
        }

        result = number;
        return true;
    }
    case ValueType::Boolean:
        if (!value.is_boolean())
        {
            error = key + " must be a boolean.";
            return false;
        }
        result = value.get<bool>();
        return true;
    }

    error = key + " has an unsupported schema type.";
    return false;
}

}

Configuration::Configuration(std::filesystem::path root, std::string environment)
    : root_(std::move(root))
    , environment_(std::move(environment))
{
}

bool Configuration::createSnapshot(std::shared_ptr<const Snapshot>& snapshot, std::string& error) const
{
    if (environment_ != "development" && environment_ != "production")
    {
        error = "OPENMP_ENV must be exactly 'development' or 'production'.";
        return false;
    }

    auto candidate = std::make_shared<Snapshot>();

    for (const CategorySpec& category : schema())
    {
        const std::filesystem::path filePath = root_ / environment_ / (category.name + ".json");
        const std::string displayPath = environment_ + "/" + category.name + ".json";
        std::error_code fileSizeError;
        const std::uintmax_t fileSize = std::filesystem::file_size(filePath, fileSizeError);
        if (fileSizeError)
        {
            error = "Cannot read required configuration file " + displayPath + ".";
            return false;
        }
        if (fileSize > kMaximumFileSize)
        {
            error = "Configuration file " + displayPath + " exceeds the 64 KiB size limit.";
            return false;
        }

        std::ifstream input(filePath, std::ios::binary);
        if (!input)
        {
            error = "Cannot open required configuration file " + displayPath + ".";
            return false;
        }

        const std::string contents {
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()};
        if (input.bad())
        {
            error = "Failed while reading configuration file " + displayPath + ".";
            return false;
        }

        Json document;
        try
        {
            document = Json::parse(contents);
        }
        catch (const Json::parse_error& exception)
        {
            error = "Invalid JSON in " + displayPath + ": " + exception.what();
            return false;
        }

        if (!document.is_object())
        {
            error = displayPath + " must contain a JSON object.";
            return false;
        }

        const auto version = document.find("schema_version");
        bool supportedVersion = false;
        if (version != document.end() && version->is_number_unsigned())
        {
            supportedVersion = version->get<std::uint64_t>() == kSchemaVersion;
        }
        else if (version != document.end() && version->is_number_integer())
        {
            supportedVersion = version->get<std::int64_t>() == kSchemaVersion;
        }

        if (!supportedVersion)
        {
            error = displayPath + " must declare supported schema_version " +
                    std::to_string(kSchemaVersion) + ".";
            return false;
        }

        std::set<std::string> allowedKeys = {"schema_version"};
        for (const FieldSpec& field : category.fields)
        {
            allowedKeys.insert(field.name);
        }

        for (auto value = document.begin(); value != document.end(); ++value)
        {
            if (allowedKeys.find(value.key()) == allowedKeys.end())
            {
                error = displayPath + " contains unknown key '" + value.key() + "'.";
                return false;
            }
        }

        for (const FieldSpec& field : category.fields)
        {
            const auto value = document.find(field.name);
            if (value == document.end())
            {
                if (field.required || !field.defaultValue)
                {
                    error = displayPath + " is missing required key '" + field.name + "'.";
                    return false;
                }

                candidate->values.emplace(
                    category.name + "." + field.name,
                    *field.defaultValue);
                continue;
            }

            ConfigurationValue validatedValue;
            if (!validateField(*value, field, validatedValue, displayPath, error))
            {
                return false;
            }

            candidate->values.emplace(category.name + "." + field.name, std::move(validatedValue));
        }
    }

    snapshot = std::move(candidate);
    return true;
}

std::shared_ptr<const Configuration::Snapshot> Configuration::currentSnapshot() const
{
    return std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
}

bool Configuration::load(std::string& error)
{
    std::shared_ptr<const Snapshot> candidate;
    if (!createSnapshot(candidate, error))
    {
        return false;
    }

    std::atomic_store_explicit(&snapshot_, std::move(candidate), std::memory_order_release);
    return true;
}

bool Configuration::reload(std::string& error)
{
    if (!currentSnapshot())
    {
        error = "Configuration cannot be reloaded before the initial load succeeds.";
        return false;
    }

    std::shared_ptr<const Snapshot> candidate;
    if (!createSnapshot(candidate, error))
    {
        return false;
    }

    std::atomic_store_explicit(&snapshot_, std::move(candidate), std::memory_order_release);
    return true;
}

bool Configuration::getInt(const std::string& key, std::int64_t& value) const
{
    const auto snapshot = currentSnapshot();
    if (!snapshot)
    {
        return false;
    }

    const auto item = snapshot->values.find(key);
    if (item == snapshot->values.end())
    {
        return false;
    }

    const auto typedValue = std::get_if<std::int64_t>(&item->second);
    if (!typedValue)
    {
        return false;
    }

    value = *typedValue;
    return true;
}

bool Configuration::getFloat(const std::string& key, double& value) const
{
    const auto snapshot = currentSnapshot();
    if (!snapshot)
    {
        return false;
    }

    const auto item = snapshot->values.find(key);
    if (item == snapshot->values.end())
    {
        return false;
    }

    const auto typedValue = std::get_if<double>(&item->second);
    if (!typedValue)
    {
        return false;
    }

    value = *typedValue;
    return true;
}

bool Configuration::getBool(const std::string& key, bool& value) const
{
    const auto snapshot = currentSnapshot();
    if (!snapshot)
    {
        return false;
    }

    const auto item = snapshot->values.find(key);
    if (item == snapshot->values.end())
    {
        return false;
    }

    const auto typedValue = std::get_if<bool>(&item->second);
    if (!typedValue)
    {
        return false;
    }

    value = *typedValue;
    return true;
}

bool Configuration::getString(const std::string& key, std::string& value) const
{
    const auto snapshot = currentSnapshot();
    if (!snapshot)
    {
        return false;
    }

    const auto item = snapshot->values.find(key);
    if (item == snapshot->values.end())
    {
        return false;
    }

    const auto typedValue = std::get_if<std::string>(&item->second);
    if (!typedValue)
    {
        return false;
    }

    value = *typedValue;
    return true;
}

}
