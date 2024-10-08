#include "faker-cxx/system.h"

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/algo_helper.h"
#include "common/format_helper.h"
#include "faker-cxx/helper.h"
#include "faker-cxx/internet.h"
#include "faker-cxx/number.h"
#include "faker-cxx/string.h"
#include "faker-cxx/word.h"
#include "system_data.h"

namespace faker::system
{
namespace
{
const std::unordered_map<FileType, std::string_view> fileTypeToStringMapping{
    {FileType::Video, "video"},
    {FileType::Audio, "audio"},
    {FileType::Image, "image"},
    {FileType::Text, "text"},
    {FileType::Application, "application"},
};

std::string_view extension(std::string_view mimeType)
{
    const auto it = mimeTypesExtensions.find(mimeType);

    if (it == mimeTypesExtensions.end())
    {
        auto pos = mimeType.find_last_of('/');

        return mimeType.substr(pos + 1);
    }

    return it->second;
}
}

std::string fileName()
{
    const auto baseName = word::words();

    return baseName + "." + fileExtension(std::nullopt);
}

std::string fileExtension(const std::optional<FileType>& mimeType)
{
    if (mimeType.has_value())
    {
        const auto& mimeTypeName = fileTypeToStringMapping.at(mimeType.value());

        std::vector<std::string> extensions;

        for (const auto& mime : mimeTypes)
        {
            size_t pos = mime.find_first_of('/');

            const auto mt = mime.substr(0, pos);

            if (mimeTypeName == mt)
            {
                extensions.emplace_back(mime.substr(pos + 1));
            }
        }

        return helper::randomElement(extensions);
    }
    else
    {
        std::set<std::string_view> extensionSet;

        for (const auto& mimeTypeName : mimeTypes)
        {
            extensionSet.insert(extension(mimeTypeName));
        }

        std::vector<std::string> extensions(extensionSet.begin(), extensionSet.end());

        return helper::randomElement(extensions);
    }
}

std::string commonFileName()
{
    const auto baseName = word::words();

    return baseName + "." + std::string{commonFileExtension()};
}

std::string_view commonFileExtension()
{
    auto mimeTypeStr = helper::randomElement(commonMimeTypes);

    return extension(mimeTypeStr);
}

std::string_view mimeType()
{
    std::vector<std::string_view> mimeTypeKeys;
    mimeTypeKeys.reserve(mimeTypes.size());

    std::copy(mimeTypes.begin(), mimeTypes.end(), std::back_inserter(mimeTypeKeys));

    return helper::randomElement(mimeTypeKeys);
}

std::string_view fileType()
{
    return helper::randomElement(commonFileTypes);
}

std::string_view directoryPath()
{
    return helper::randomElement(directoryPaths);
}

std::string filePath()
{
    return std::string{directoryPath()} + fileName();
}

std::string semver()
{
    const int major = number::integer(9);
    const int minor = number::integer(9);
    const int patch = number::integer(9);

    return common::format("{}.{}.{}", major, minor, patch);
}

std::string networkInterface(const std::optional<NetworkInterfaceOptions>& options)
{
    const auto defaultInterfaceType = helper::randomElement(commonInterfaceTypes);
    const auto defaultInterfaceSchema = std::string(helper::objectKey(commonInterfaceSchemas));

    auto interfaceType = std::string(defaultInterfaceType);
    auto interfaceSchema = defaultInterfaceSchema;

    if (options.has_value())
    {
        if (options->interfaceType.has_value() && !options->interfaceType.value().empty())
        {
            interfaceType = options->interfaceType.value();
        }

        if (options->interfaceSchema.has_value() && !options->interfaceSchema.value().empty())
        {
            interfaceSchema = options->interfaceSchema.value();
        }
    }

    std::string suffix;

    std::string prefix;

    auto digit = []() { return string::numeric(); };

    if (interfaceSchema == "index")
    {
        suffix = digit();
    }
    else if (interfaceSchema == "slot")
    {
        suffix = helper::maybe<std::string>([&]() { return "f" + digit(); });
        suffix += helper::maybe<std::string>([&]() { return "d" + digit(); });
    }
    else if (interfaceSchema == "mac")
    {
        suffix = internet::mac("");
    }
    else if (interfaceSchema == "pci")
    {
        prefix = helper::maybe<std::string>([&]() { return "P" + digit(); });
        suffix = digit() + "s" + digit();
        suffix += helper::maybe<std::string>([&]() { return "f" + digit(); });
        suffix += helper::maybe<std::string>([&]() { return "d" + digit(); });
    }

    return prefix + interfaceType + std::string(commonInterfaceSchemas.at(interfaceSchema)) + suffix;
}

std::string cron(const CronOptions& options)
{
    std::vector<std::string> minutes = {std::to_string(number::integer(59)), "*"};

    std::vector<std::string> hours = {std::to_string(number::integer(23)), "*"};

    std::vector<std::string> days = {std::to_string(number::integer(1, 31)), "*", "?"};

    std::vector<std::string> months = {std::to_string(number::integer(1, 12)), "*"};

    std::vector<std::string> daysOfWeek = {
        std::to_string(number::integer(6)),
        std::string(
            cronDayOfWeek[static_cast<unsigned long>(number::integer(0, static_cast<int>(cronDayOfWeek.size() - 1)))]),
        "*", "?"};

    const auto years = options.includeYear ? std::vector<std::string>{std::to_string(number::integer(1970, 2099))} :
                                             std::vector<std::string>{std::to_string(number::integer(1970, 2099)), "*"};
    const auto minute = helper::randomElement(minutes);
    const auto hour = helper::randomElement(hours);
    const auto day = helper::randomElement(days);
    const auto month = helper::randomElement(months);
    const auto dayOfWeek = helper::randomElement(daysOfWeek);
    const auto year = helper::randomElement(years);

    std::string standardExpression = minute + " " + hour + " " + day + " " + month + " " + dayOfWeek;

    if (options.includeYear)
    {
        standardExpression += " " + year;
    }

    const auto nonStandardExpressions = {"@annually", "@daily", "@hourly", "@monthly", "@reboot", "@weekly", "@yearly"};

    return options.includeNonStandard ? helper::randomElement(nonStandardExpressions) : standardExpression;
}
}
