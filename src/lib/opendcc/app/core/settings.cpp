// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/opendcc.h"
#include "opendcc/app/core/settings.h"

#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/fileUtils.h>
#include <pxr/base/tf/error.h>
#include <fstream>

#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN

namespace
{
    static const std::string session_prefix = "session";

    bool path_starts_with(const std::string& path, const std::string& prefix)
    {
        if (path.size() < prefix.size())
            return false;

        for (size_t i = 0; i < prefix.size(); i++)
        {
            if (path[i] != prefix[i])
                return false;
        }

        return (path.size() == prefix.size()) || (path[prefix.size()] == Settings::get_separator());
    }
};

PXR_NAMESPACE_USING_DIRECTIVE

Settings::Settings()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
#define REGISTER_VALUE_TYPE(val_type, cond, extract)                                                                                              \
    Settings::register_type<val_type>([](const nonstd::any& val) { return Json::Value(static_cast<val_type>(nonstd::any_cast<val_type>(val))); }, \
                                      [](const Json::Value& val) {                                                                                \
                                          if (cond)                                                                                               \
                                              return nonstd::make_any<val_type>(static_cast<val_type>(extract));                                  \
                                          return nonstd::any();                                                                                   \
                                      })
#define REGISTER_VECTOR_TYPE(val_type, cond, extract)                          \
    Settings::register_type<std::vector<val_type>>(                            \
        [](const nonstd::any& val) {                                           \
            const auto vector = nonstd::any_cast<std::vector<val_type>>(val);  \
            Json::Value result(Json::ValueType::arrayValue);                   \
            result.resize(vector.size());                                      \
            for (auto i = 0; i < vector.size(); i++)                           \
                result[i] = static_cast<val_type>(vector[i]);                  \
            return result;                                                     \
        },                                                                     \
        [](const Json::Value& json_val) {                                      \
            if (!json_val.isArray())                                           \
                return nonstd::any();                                          \
            if (!json_val.empty())                                             \
            {                                                                  \
                const auto& val = json_val[0];                                 \
                if (!cond)                                                     \
                    return nonstd::any();                                      \
            }                                                                  \
            std::vector<val_type> result(json_val.size());                     \
            for (auto i = 0; i < json_val.size(); i++)                         \
            {                                                                  \
                const auto& val = json_val[i];                                 \
                result[i] = static_cast<val_type>(extract);                    \
            }                                                                  \
            return nonstd::make_any<std::vector<val_type>>(std::move(result)); \
        })

#define REGISTER_TYPE(val_type, cond, extract)    \
    REGISTER_VALUE_TYPE(val_type, cond, extract); \
    REGISTER_VECTOR_TYPE(val_type, cond, extract);
        REGISTER_TYPE(bool, val.isBool(), val.asBool());
        REGISTER_TYPE(uint8_t, val.isUInt(), val.asUInt());
        REGISTER_TYPE(uint16_t, val.isUInt(), val.asUInt());
        REGISTER_TYPE(uint32_t, val.isUInt(), val.asUInt());
        REGISTER_TYPE(uint64_t, val.isUInt(), val.asUInt());
        REGISTER_TYPE(int8_t, val.isInt(), val.asInt());
        REGISTER_TYPE(int16_t, val.isInt(), val.asInt());
        REGISTER_TYPE(int32_t, val.isInt(), val.asInt());
        REGISTER_TYPE(int64_t, val.isInt(), val.asInt());
        REGISTER_TYPE(float, val.isDouble(), val.asDouble());
        REGISTER_TYPE(double, val.isDouble(), val.asDouble());
        REGISTER_TYPE(std::string, val.isString(), val.asString());
#undef REGISTER_TYPE
#undef REGISTER_VALUE_TYPE
#undef REGISTER_VECTOR_TYPE
    });
}

Settings::Settings(const std::string& settings_path)
    : Settings()
{
    m_settings_file = settings_path;
    Json::Value root;
    std::ifstream file(m_settings_file);
    if (!file.is_open())
    {
        TF_WARN("Failed to open application settings file. The settings file will be recreated.");
        return;
    }
    Json::CharReaderBuilder builder;
    std::string errs;
    if (!parseFromStream(builder, file, &m_json_root, &errs) || !errs.empty())
    {
        TF_RUNTIME_ERROR("Settings parse error: %s", errs.c_str());
        m_settings_file.clear();
        return;
    }
    deserialize();
}

Settings::SettingChangedHandle Settings::register_setting_changed(const std::string& path, const std::function<SettingChangedCallback>& callback)
{
    return m_dispatchers[path].append(callback);
}

Settings::SettingChangedHandle Settings::register_setting_changed(const std::string& path, const std::function<void()>& callback)
{
    return m_dispatchers[path].append([callback](const std::string&, const Value&, ChangeType) { callback(); });
}

Settings::SettingChangedHandle Settings::register_setting_changed(const std::string& path, const std::function<void(const Value&)>& callback)
{
    return m_dispatchers[path].append([callback](const std::string&, const Value& val, ChangeType) { callback(val); });
}

Settings::SettingChangedHandle Settings::register_setting_changed(const std::string& path,
                                                                  const std::function<void(const std::string&, const Value&)>& callback)
{
    return m_dispatchers[path].append([callback](const std::string& path, const Value& val, ChangeType) { callback(path, val); });
}

void Settings::unregister_setting_changed(const std::string& path, SettingChangedHandle handle)
{
    auto iter = m_dispatchers.find(path);
    if (iter == m_dispatchers.end())
        return;
    iter->second.remove(handle);
}

Settings::ValueHolder Settings::get_raw(const std::string& path) const
{
    return get_impl(path);
}

void Settings::reset(const std::string& path)
{
    if (!is_valid_path(path))
        return;
    bool should_serialize = false;
    const auto persistent = !path_starts_with(path, session_prefix);
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (path_starts_with(it->first, path))
        {
            const auto cur_path = it->first;
            auto default_val = m_defaults.find(cur_path);
            if (persistent)
            {
                if (default_val != m_defaults.end())
                    set_value_at_path(cur_path, default_val->second);
                else
                    remove_value_at_path(cur_path);
                should_serialize = true;
            }
            it = m_values.erase(it);
            notify_change(cur_path, default_val != m_defaults.end() ? default_val->second : ValueHolder(), ChangeType::RESET);
        }
        else
        {
            ++it;
        }
    }
    if (should_serialize)
        serialize();
}

void Settings::remove(const std::string& path)
{
    if (!is_valid_path(path))
        return;
    std::unordered_set<std::string> removed;
    for (auto it = m_values.begin(); it != m_values.end();)
    {
        if (path_starts_with(it->first, path))
        {
            removed.insert(it->first);
            it = m_values.erase(it);
        }
        else
        {
            ++it;
        }
    }
    for (auto it = m_defaults.begin(); it != m_defaults.end();)
    {
        if (path_starts_with(it->first, path))
        {
            removed.insert(it->first);
            it = m_defaults.erase(it);
        }
        else
        {
            ++it;
        }
    }

    const auto should_serialize = !path_starts_with(path, session_prefix) && !removed.empty();
    for (const auto& entry : removed)
    {
        notify_change(entry, ValueHolder(), ChangeType::REMOVED);
        if (should_serialize)
            remove_value_at_path(entry);
    }

    if (should_serialize)
        serialize();
}

bool Settings::has(const std::string& path) const
{
    if (!is_valid_path(path))
        return false;

    for (const auto& dict : { m_defaults, m_values })
    {
        for (const auto& entry : dict)
        {
            if (path_starts_with(entry.first, path))
                return true;
        }
    }

    return false;
}

char Settings::get_separator()
{
    return '.';
}

void Settings::notify_change(const std::string& path, const ValueHolder& value, ChangeType event_type) const
{
    auto current_path = path;
    while (true)
    {
        auto iter = m_dispatchers.find(current_path);
        if (iter != m_dispatchers.end())
            iter->second(path, value, event_type);
        const auto sep_pos = current_path.rfind(get_separator());
        if (sep_pos == std::string::npos)
            break;
        current_path = current_path.substr(0, sep_pos);
    };
}

void Settings::set(const std::string& path, const ValueHolder& value, const std::type_info& type)
{
    set_impl(path, value, m_values, type);
}

void Settings::set_default(const std::string& path, const ValueHolder& value, const std::type_info& type)
{
    set_impl(path, value, m_defaults, type);
}

void Settings::set_impl(const std::string& path, const ValueHolder& value, std::unordered_map<std::string, ValueHolder>& collection,
                        const std::type_info& type)
{
    if (value.isObject())
    {
        TF_RUNTIME_ERROR("Failed to set setting at path '%s': json object values are not supported.", path.c_str());
        return;
    }
    if (value.isNull())
    {
        TF_RUNTIME_ERROR("Failed to set setting at path '%s': json value is null.", path.c_str());
        return;
    }
    auto iter = collection.find(path);
    if (iter != collection.end())
    {
        if (iter->second == value)
            return;
        iter->second = value;
    }
    else
    {
        if (!is_valid_path(path))
            return;
        for (const auto& e : collection)
        {
            if (path_starts_with(e.first, path) || path_starts_with(path, e.first))
                return;
        }
        collection[path] = value;
    }

    // Notify and serialize only if the actual value is really changes
    // if edited collection is not default or if there are no value
    if (&collection == &m_values || m_values.find(path) == m_values.end())
    {
        notify_change(path, value, ChangeType::UPDATED);
        if (!path_starts_with(path, session_prefix))
        {
            set_value_at_path(path, value);
            serialize();
        }
    }
}

Settings::ValueHolder Settings::get_impl(const std::string& path) const
{
    const auto result = get_impl(path, m_values);
    if (!result.empty())
        return result;
    return get_default_impl(path);
}

Settings::ValueHolder Settings::get_default_impl(const std::string& path) const
{
    return get_impl(path, m_defaults);
}

Settings::ValueHolder Settings::get_impl(const std::string& path, const std::unordered_map<std::string, ValueHolder>& collection) const
{
    auto iter = collection.find(path);
    if (iter != collection.end())
        return iter->second;

    return ValueHolder();
}

bool Settings::is_valid_path(const std::string& path) const
{
    // empty first token
    if (path.empty())
        return false;

    auto pos = path.find(get_separator(), 0);
    auto prev_pos = std::string::npos;

    // check for empty substrings
    while (pos != std::string::npos)
    {
        if (pos == prev_pos + 1)
            return false;
        prev_pos = pos;
        pos = path.find(get_separator(), pos + 1);
    }

    // empty last token
    if (prev_pos == path.length() - 1)
        return false;
    return true;
}

void Settings::serialize() const
{
    if (m_settings_file.empty())
        return;

    auto file_stream = std::ofstream(m_settings_file);
    if (!file_stream.is_open())
    {
        TF_RUNTIME_ERROR("Failed to open application settings file.");
        return;
    }

    Json::StreamWriterBuilder writer_builder;
    auto writer = std::unique_ptr<Json::StreamWriter>(writer_builder.newStreamWriter());
    writer->write(m_json_root, &file_stream);
}

void Settings::deserialize()
{
    std::function<void(const Json::Value& node, const std::string& path)> traverse = [&traverse, this](const Json::Value& node,
                                                                                                       const std::string& path) {
        if (!node.isObject())
        {
            m_values[path] = node;
            return;
        }

        const auto path_prefix = path + (path.empty() ? "" : ".");
        for (const auto& name : node.getMemberNames())
        {
            const auto new_path = path_prefix + name;
            traverse(node[name], new_path);
        }
    };

    traverse(m_json_root, "");
}

void Settings::remove_value_at_path(const std::string& path)
{
    size_t pos = 0;
    size_t prev_pos = 0;
    Json::Value* cur_val = &m_json_root;
    std::vector<Json::Value*> vals = { &m_json_root };
    std::vector<std::string> tokens = {};

    while ((pos = path.find(get_separator(), prev_pos)) != std::string::npos)
    {
        const auto token = path.substr(prev_pos, pos - prev_pos);
        tokens.push_back(token);
        cur_val = &(*cur_val)[token];
        vals.push_back(cur_val);
        prev_pos = pos + 1;
    }

    tokens.push_back(path.substr(prev_pos));

    for (int i = vals.size() - 1; i >= 0; i--)
    {
        auto cur_val = vals[i];
        cur_val->removeMember(tokens[i]);
        if (!cur_val->empty())
            return;
    }
}

void Settings::set_value_at_path(const std::string& path, const ValueHolder& val)
{
    size_t pos = 0;
    size_t prev_pos = 0;
    Json::Value* cur_val = &m_json_root;
    while ((pos = path.find(get_separator(), prev_pos)) != std::string::npos)
    {
        const auto token = path.substr(prev_pos, pos - prev_pos);
        cur_val = &(*cur_val)[token];
        prev_pos = pos + 1;
    }
    (*cur_val)[path.substr(prev_pos)] = std::move(val);
}

std::unordered_map<std::type_index, Settings::TypeHelpers> Settings::s_type_helpers;

OPENDCC_NAMESPACE_CLOSE

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest/doctest.h>
OPENDCC_NAMESPACE_USING

DOCTEST_TEST_SUITE("SettingsTests")
{
    DOCTEST_TEST_CASE("type_resolving")
    {
        using namespace details::settings;
        static_assert(std::is_same<underlying_type<bool>::type, bool>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<uint8_t>::type, uint8_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<uint16_t>::type, uint16_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<uint32_t>::type, uint32_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<uint64_t>::type, uint64_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<int8_t>::type, int8_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<int16_t>::type, int16_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<int32_t>::type, int32_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<int64_t>::type, int64_t>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<float>::type, float>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<double>::type, double>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<long double>::type, double>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<std::string>::type, std::string>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<const char*>::type, std::string>::value, "Invalid type casting");
        static_assert(std::is_same<underlying_type<char[32]>::type, std::string>::value, "Invalid type casting");

        static_assert(std::is_same<underlying_type<Settings>::type, Settings>::value, "Invalid type casting");
    }
    DOCTEST_TEST_CASE("get/set_type_correctness_tests")
    {
        auto settings = Settings();
        settings.set("bool", false);
        settings.set("int", 5);
        settings.set("double", 3.5);
        settings.set("string", "asdf");
        settings.set("vector<bool>", std::vector<bool> { true, false, true });
        settings.set("vector<int>", std::vector<int> { 4, 2 });
        settings.set("vector<double>", std::vector<double> { 3.5, 8.5 });
        settings.set("vector<string>", std::vector<std::string> { "zxcv", "cvbn" });
        DOCTEST_SUBCASE("valid_usage")
        {
            DOCTEST_CHECK_EQ(settings.get<bool>("bool"), false);
            DOCTEST_CHECK_EQ(settings.get<int>("int"), 5);
            DOCTEST_CHECK_EQ(settings.get<double>("int"), 5.0);
            DOCTEST_CHECK_EQ(settings.get<double>("double"), 3.5);
            DOCTEST_CHECK_EQ(settings.get<std::string>("string"), "asdf");
            DOCTEST_CHECK_EQ(settings.get<std::vector<bool>>("vector<bool>"), std::vector<bool> { true, false, true });
            DOCTEST_CHECK_EQ(settings.get<std::vector<int>>("vector<int>"), std::vector<int> { 4, 2 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<float>>("vector<int>"), std::vector<float> { 4.0f, 2.0f });
            DOCTEST_CHECK_EQ(settings.get<std::vector<float>>("vector<double>"), std::vector<float> { 3.5, 8.5 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<double>>("vector<double>"), std::vector<double> { 3.5, 8.5 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<std::string>>("vector<string>"), std::vector<std::string> { "zxcv", "cvbn" });
        }
        DOCTEST_SUBCASE("get_by_invalid_type")
        {
            for (const auto& key : { "bool", "int", "double", "string", "vector<bool>", "vector<int>", "vector<double>", "vector<string>" })
            {
                if (key != std::string("bool"))
                    DOCTEST_CHECK_EQ(settings.get<bool>(key), false);
                if (key != std::string("int"))
                    DOCTEST_CHECK_EQ(settings.get<int>(key), 0);
                if (key != std::string("double") && key != std::string("int"))
                    DOCTEST_CHECK_EQ(settings.get<double>(key), 0.0);
                if (key != std::string("string"))
                    DOCTEST_CHECK_EQ(settings.get<std::string>(key), std::string());
                if (key != std::string("vector<bool>"))
                    DOCTEST_CHECK_EQ(settings.get<std::vector<bool>>(key), std::vector<bool>());
                if (key != std::string("vector<int>"))
                    DOCTEST_CHECK_EQ(settings.get<std::vector<int32_t>>(key), std::vector<int32_t>());
                if (key != std::string("vector<double>") && key != std::string("vector<int>"))
                {
                    DOCTEST_CHECK_EQ(settings.get<std::vector<float>>(key), std::vector<float>());
                    DOCTEST_CHECK_EQ(settings.get<std::vector<double>>(key), std::vector<double>());
                }
                if (key != std::string("vector<string>"))
                    DOCTEST_CHECK_EQ(settings.get<std::vector<std::string>>(key), std::vector<std::string>());
            }
        }
    };
    DOCTEST_TEST_CASE("get_default/set_default_type_correctness_tests")
    {
        auto settings = Settings();
        settings.set_default("bool", false);
        settings.set_default("int", 5);
        settings.set_default("double", 3.5);
        settings.set_default("string", std::string("asdf"));
        settings.set_default("vector<bool>", std::vector<bool> { true, false, true });
        settings.set_default("vector<int>", std::vector<int> { 4, 2 });
        settings.set_default("vector<double>", std::vector<double> { 3.5, 8.5 });
        settings.set_default("vector<string>", std::vector<std::string> { "zxcv", "cvbn" });

        settings.set("bool", true);
        settings.set("int", 42);
        settings.set("double", 4.5);
        settings.set("string", std::string("zxcv"));
        settings.set("vector<bool>", std::vector<bool> { false, true, false });
        settings.set("vector<int>", std::vector<int> { 8, 10 });
        settings.set("vector<double>", std::vector<double> { 5.5, 7.5 });
        settings.set("vector<string>", std::vector<std::string> { "ujmi", "tgbv" });
        DOCTEST_SUBCASE("valid_usage")
        {
            DOCTEST_CHECK_EQ(settings.get_default<bool>("bool", true), false);
            DOCTEST_CHECK_EQ(settings.get_default<int>("int"), 5);
            DOCTEST_CHECK_EQ(settings.get_default<double>("double"), 3.5);
            DOCTEST_CHECK_EQ(settings.get_default<std::string>("string"), std::string("asdf"));
            DOCTEST_CHECK_EQ(settings.get_default<std::vector<bool>>("vector<bool>"), std::vector<bool> { true, false, true });
            DOCTEST_CHECK_EQ(settings.get_default<std::vector<int>>("vector<int>"), std::vector<int> { 4, 2 });
            DOCTEST_CHECK_EQ(settings.get_default<std::vector<float>>("vector<double>"), std::vector<float> { 3.5, 8.5 });
            DOCTEST_CHECK_EQ(settings.get_default<std::vector<double>>("vector<double>"), std::vector<double> { 3.5, 8.5 });
            DOCTEST_CHECK_EQ(settings.get_default<std::vector<std::string>>("vector<string>"), std::vector<std::string> { "zxcv", "cvbn" });

            DOCTEST_CHECK_EQ(settings.get<bool>("bool"), true);
            DOCTEST_CHECK_EQ(settings.get<int>("int"), 42);
            DOCTEST_CHECK_EQ(settings.get<double>("double"), 4.5);
            DOCTEST_CHECK_EQ(settings.get<std::string>("string"), std::string("zxcv"));
            DOCTEST_CHECK_EQ(settings.get<std::vector<bool>>("vector<bool>"), std::vector<bool> { false, true, false });
            DOCTEST_CHECK_EQ(settings.get<std::vector<int>>("vector<int>"), std::vector<int> { 8, 10 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<float>>("vector<double>"), std::vector<float> { 5.5, 7.5 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<double>>("vector<double>"), std::vector<double> { 5.5, 7.5 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<std::string>>("vector<string>"), std::vector<std::string> { "ujmi", "tgbv" });
        }
        DOCTEST_SUBCASE("get_by_invalid_type")
        {
            for (const auto& key : { "bool", "int", "double", "string", "vector<bool>", "vector<int>", "vector<double>", "vector<string>" })
            {
                if (key != std::string("bool"))
                    DOCTEST_CHECK_EQ(settings.get_default<bool>(key), false);
                if (key != std::string("int"))
                    DOCTEST_CHECK_EQ(settings.get_default<int>(key), 0);
                if (key != std::string("double") && key != std::string("int"))
                    DOCTEST_CHECK_EQ(settings.get_default<double>(key), 0.0);
                if (key != std::string("string"))
                    DOCTEST_CHECK_EQ(settings.get_default<std::string>(key), std::string());
                if (key != std::string("vector<bool>"))
                    DOCTEST_CHECK_EQ(settings.get_default<std::vector<bool>>(key), std::vector<bool>());
                if (key != std::string("vector<int>"))
                    DOCTEST_CHECK_EQ(settings.get_default<std::vector<int>>(key), std::vector<int32_t>());
                if (key != std::string("vector<double>") && key != std::string("vector<int>"))
                {
                    DOCTEST_CHECK_EQ(settings.get_default<std::vector<float>>(key), std::vector<float>());
                    DOCTEST_CHECK_EQ(settings.get_default<std::vector<double>>(key), std::vector<double>());
                }
                if (key != std::string("vector<string>"))
                    DOCTEST_CHECK_EQ(settings.get_default<std::vector<std::string>>(key), std::vector<std::string>());
            }
        }
    }

    struct TestClass
    {
        int int_val = 0;
        double double_val = 0.0;
        TestClass() = default;
        TestClass(int i, double d)
            : int_val(i)
            , double_val(d)
        {
        }
        bool operator==(const TestClass& other) const { return int_val == other.int_val && double_val == other.double_val; }
    };
    struct TestClassAsObject : TestClass
    {
        TestClassAsObject() = default;
        TestClassAsObject(int i, double d)
            : TestClass(i, d)
        {
        }
    };

    DOCTEST_TEST_CASE("register_custom_type")
    {
        Settings::register_type<TestClassAsObject>(
            [](const nonstd::any& val) {
                auto my_class = nonstd::any_cast<TestClassAsObject>(val);
                Json::Value result(Json::ValueType::objectValue);
                result["int_val"] = my_class.int_val;
                result["double_val"] = my_class.double_val;
                return result;
            },
            [](const Json::Value& val) {
                if (!val.isObject())
                    return nonstd::any();
                if (val.isMember("int_val") && val.isMember("double_val") && val.size() == 2)
                {
                    const auto int_val = val["int_val"];
                    const auto double_val = val["double_val"];
                    if (int_val.isInt() && double_val.isDouble())
                    {
                        TestClassAsObject result;
                        result.int_val = int_val.asInt();
                        result.double_val = double_val.asDouble();
                        return nonstd::make_any<TestClassAsObject>(result);
                    }
                }
                return nonstd::any();
            });
        Settings::register_type<TestClass>(
            [](const nonstd::any& val) {
                auto my_class = nonstd::any_cast<TestClass>(val);
                Json::Value result(Json::ValueType::arrayValue);
                result[0] = my_class.int_val;
                result[1] = my_class.double_val;
                return result;
            },
            [](const Json::Value& val) {
                if (!val.isArray())
                    return nonstd::any();
                if (val.size() == 2)
                {
                    const auto int_val = val[0];
                    const auto double_val = val[1];
                    if (int_val.isInt() && double_val.isDouble())
                    {
                        TestClass result;
                        result.int_val = int_val.asInt();
                        result.double_val = double_val.asDouble();
                        return nonstd::make_any<TestClass>(result);
                    }
                }
                return nonstd::any();
            });
        Settings settings;
        settings.set("custom", TestClassAsObject { 4, 3.2 });
        settings.set("custom", TestClass { 4, 3.2 });

        DOCTEST_CHECK_EQ(settings.get<TestClassAsObject>("custom"), TestClass());
        DOCTEST_CHECK_EQ(settings.get<TestClass>("custom"), TestClass { 4, 3.2 });
    }
    DOCTEST_TEST_CASE("set_tests")
    {
        DOCTEST_SUBCASE("valid_usage")
        {
            auto settings = Settings();
            settings.set("path.to.settings", 40);
            DOCTEST_CHECK_EQ(settings.get<int8_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<int16_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<int32_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<int64_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<uint8_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<uint16_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<uint32_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<uint64_t>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<float>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<double>("path.to.settings"), 40);
            DOCTEST_CHECK_EQ(settings.get<long double>("path.to.settings"), 40);

            settings.set("path.to.settings", "asdf");
            DOCTEST_CHECK_EQ(settings.get<int>("path.to.settings"), 0);
            DOCTEST_CHECK_EQ(settings.get<std::string>("path.to.settings"), "asdf");

            settings.set("path.to", 42);
            DOCTEST_CHECK_EQ(settings.get<int>("path.to"), 0);
        }
        DOCTEST_SUBCASE("invalid_paths")
        {
            auto settings = Settings();
            settings.set(".path.to.settings", 41);
            settings.set("\0path.to.settings", 42);
            settings.set("", 43);
            settings.set("path.to.settings.", 44);
            settings.set("path..settings", 45);

            DOCTEST_CHECK_EQ(settings.get<int>(".path.to.settings"), 0);
            DOCTEST_CHECK_EQ(settings.get<int>("\0path.to.settings"), 0);
            DOCTEST_CHECK_EQ(settings.get<int>(""), 0);
            DOCTEST_CHECK_EQ(settings.get<int>("path.to.settings."), 0);
            DOCTEST_CHECK_EQ(settings.get<int>("path..settings"), 0);

            settings.set("valid.path", 42);
            settings.set("valid.path.to.invalid", 60);
            DOCTEST_CHECK_EQ(settings.get<int>("valid.path"), 42);
            DOCTEST_CHECK_EQ(settings.get<int>("valid.path.to.invalid"), 0);
        }
    }

    DOCTEST_TEST_CASE("get_default_value_resolution")
    {
        auto settings = Settings();
        settings.set_default("value", 42);
        DOCTEST_CHECK_EQ(settings.get_default<int>("value"), 42);
        DOCTEST_CHECK_EQ(settings.get<int>("value"), 42);

        settings.set("value", 84);
        DOCTEST_CHECK_EQ(settings.get_default<int>("value"), 42);
        DOCTEST_CHECK_EQ(settings.get<int>("value"), 84);

        DOCTEST_SUBCASE("after_reset")
        {
            auto _settings = settings;
            _settings.reset("value");
            DOCTEST_CHECK_EQ(_settings.get_default<int>("value"), 42);
            DOCTEST_CHECK_EQ(_settings.get<int>("value"), 42);
        }
        DOCTEST_SUBCASE("after_remove")
        {
            auto _settings = settings;
            _settings.remove("value");
            DOCTEST_CHECK_EQ(_settings.get_default<int>("value"), 0);
            DOCTEST_CHECK_EQ(_settings.get<int>("value"), 0);
        }
    }

    DOCTEST_TEST_CASE("has_tests")
    {
        DOCTEST_SUBCASE("set_as_normal")
        {
            auto settings = Settings();
            settings.set<int>("path.to.value", 42);

            DOCTEST_CHECK(settings.has("path.to.value"));
            DOCTEST_CHECK(settings.has("path.to"));
            DOCTEST_CHECK(settings.has("path"));
        }
        DOCTEST_SUBCASE("set_as_default")
        {
            auto settings = Settings();
            settings.set_default<int>("path.to.value", 42);

            DOCTEST_CHECK(settings.has("path.to.value"));
            DOCTEST_CHECK(settings.has("path.to"));
            DOCTEST_CHECK(settings.has("path"));
        }
    }

    DOCTEST_TEST_CASE("remove_tests")
    {
        auto fixture_settings = Settings();
        fixture_settings.set<int>("path.to.value1", 42);
        fixture_settings.set<int>("path.to.value2", 43);

        DOCTEST_SUBCASE("remove_by_one")
        {
            auto settings = fixture_settings;
            settings.remove("path.to.value1");
            DOCTEST_CHECK_FALSE(settings.has("path.to.value1"));
            settings.remove("path.to.value2");
            DOCTEST_CHECK_FALSE(settings.has("path.to.value2"));
        }
        DOCTEST_SUBCASE("remove_subdir")
        {
            auto settings = fixture_settings;
            settings.remove("path.to");
            DOCTEST_CHECK_FALSE(settings.has("path.to.value1"));
            DOCTEST_CHECK_FALSE(settings.has("path.to.value2"));
            DOCTEST_CHECK_FALSE(settings.has("path.to"));

            settings = fixture_settings;
            settings.remove("path");
            DOCTEST_CHECK_FALSE(settings.has("path.to.value1"));
            DOCTEST_CHECK_FALSE(settings.has("path.to.value2"));
            DOCTEST_CHECK_FALSE(settings.has("path.to"));
            DOCTEST_CHECK_FALSE(settings.has("path"));
        }
    }

    DOCTEST_TEST_CASE("reset_tests")
    {
        auto fixture_settings = Settings();
        fixture_settings.set_default("path.to.value1", 42);
        fixture_settings.set_default("path.to.value2", 43);

        fixture_settings.set("path.to.value1", 5);
        fixture_settings.set("path.to.value2", 6);

        DOCTEST_SUBCASE("reset_by_one")
        {
            auto settings = fixture_settings;
            settings.reset("path.to.value1");
            DOCTEST_CHECK_EQ(settings.get("path.to.value1", 0), 42);
            settings.reset("path.to.value2");
            DOCTEST_CHECK_EQ(settings.get("path.to.value2", 0), 43);
        }
        DOCTEST_SUBCASE("reset_subdir")
        {
            auto settings = fixture_settings;
            settings.reset("path.to");
            DOCTEST_CHECK_EQ(settings.get("path.to.value1", 0), 42);
            DOCTEST_CHECK_EQ(settings.get("path.to.value2", 0), 43);

            settings = fixture_settings;
            settings.reset("path");
            DOCTEST_CHECK_EQ(settings.get("path.to.value1", 0), 42);
            DOCTEST_CHECK_EQ(settings.get("path.to.value2", 0), 43);
        }
    }

    DOCTEST_TEST_CASE("notification_tests")
    {
        struct Notifier
        {
            Settings::SettingChangedHandle handle;
            std::string path;
            Settings::Value value;
            Settings::ChangeType change;
            uint32_t call_count = 0;

            Notifier(Settings& settings, const std::string& path)
            {
                handle = settings.register_setting_changed(
                    path, [this](const std::string& path, const Settings::Value& value, Settings::ChangeType change) {
                        ++call_count;
                        this->path = path;
                        this->value = value;
                        this->change = change;
                    });
            }
        };

        DOCTEST_SUBCASE("registration/unregistration")
        {
            auto settings = Settings();
            Notifier notifier(settings, "value");
            settings.set("value", 42);
            DOCTEST_CHECK_EQ(notifier.call_count, 1);

            settings.unregister_setting_changed("value", notifier.handle);
            settings.set("value", 5);
            DOCTEST_CHECK_EQ(notifier.call_count, 1);
        }
        // It seems we need some more complex system that raw eventpp to implement this
        // DOCTEST_SUBCASE("deregistration_on_handle_destroyed")
        //{
        //    auto settings = Settings2();
        //    settings.set("value", 42);
        //    uint32_t call_count = 0;
        //    {
        //        auto handle = settings.register_setting_changed("value", [&call_count]
        //        (const std::string& p, const Settings2::ValueHolder& v, Settings2::ChangeType change)
        //            {
        //                call_count++;
        //            });
        //        DOCTEST_CHECK_EQ(call_count, 0);
        //    }
        //    settings.set("value", 43);
        //    DOCTEST_CHECK_EQ(call_count, 0);
        //}
        DOCTEST_SUBCASE("valid_usage")
        {
            auto settings = Settings();
            Notifier notifier(settings, "value");

            settings.set_default("value", 42);
            DOCTEST_CHECK_EQ(notifier.path, "value");
            DOCTEST_CHECK_EQ(notifier.value.get<int>(), 42);
            DOCTEST_CHECK_EQ(notifier.change, Settings::ChangeType::UPDATED);

            settings.set("value", 60);
            DOCTEST_CHECK_EQ(notifier.path, "value");
            DOCTEST_CHECK_EQ(notifier.value.get<int>(), 60);
            DOCTEST_CHECK_EQ(notifier.change, Settings::ChangeType::UPDATED);

            settings.reset("value");
            DOCTEST_CHECK_EQ(notifier.path, "value");
            DOCTEST_CHECK_EQ(notifier.value.get<int>(), 42);
            DOCTEST_CHECK_EQ(notifier.change, Settings::ChangeType::RESET);

            settings.remove("value");
            DOCTEST_CHECK_EQ(notifier.path, "value");
            DOCTEST_CHECK_FALSE(notifier.value);
            DOCTEST_CHECK_EQ(notifier.change, Settings::ChangeType::REMOVED);

            DOCTEST_CHECK_EQ(notifier.call_count, 4);
        }
        DOCTEST_SUBCASE("tree_change")
        {
            auto settings = Settings();
            Notifier notifier(settings, "path");
            Notifier notifier_val(settings, "path.val1");

            settings.set("path.val1", 42);
            DOCTEST_CHECK_EQ(notifier.path, "path.val1");
            DOCTEST_CHECK_EQ(notifier.value.get<int>(), 42);

            DOCTEST_CHECK_EQ(notifier.call_count, 1);
            DOCTEST_CHECK_EQ(notifier_val.path, "path.val1");
            DOCTEST_CHECK_EQ(notifier_val.value.get<int>(), 42);
            DOCTEST_CHECK_EQ(notifier_val.call_count, 1);

            settings.set("path.val2", 45);
            DOCTEST_CHECK_EQ(notifier.path, "path.val2");
            DOCTEST_CHECK_EQ(notifier.value.get<int>(), 45);
            DOCTEST_CHECK_EQ(notifier.call_count, 2);
            DOCTEST_CHECK_EQ(notifier_val.call_count, 1);

            settings.reset("path");
            DOCTEST_CHECK_EQ(notifier.call_count, 4);
            DOCTEST_CHECK_EQ(notifier_val.call_count, 2);

            settings.set("path.val1", 42);
            settings.set("path.val2", 45);
            settings.remove("path");
            DOCTEST_CHECK_EQ(notifier.call_count, 8);
            DOCTEST_CHECK_EQ(notifier_val.call_count, 4);
        }
        DOCTEST_SUBCASE("invalid_set")
        {
            auto settings = Settings();
            settings.set("path.to.value", 42);
            Notifier notifier(settings, "path");

            settings.set("path.to", 123);
            DOCTEST_CHECK_EQ(notifier.call_count, 0);
        }
    }
    DOCTEST_TEST_CASE("serialization")
    {
        char tmp_filename[1024] = {};
        tmpnam(tmp_filename);
        std::string test_file(tmp_filename);

        auto init_settings = [&test_file] {
            Settings settings(test_file);
            settings.set("bool", true);
            settings.set("int", 54);
            settings.set("float", 23.54f);
            settings.set("string", "zxcv");
            settings.set("bool_arr", std::vector<bool> { true, false, true });
            settings.set("int_arr", std::vector<int> { 3, 6, 9 });
            settings.set("float_arr", std::vector<float> { 3.2, 6, 1, 9.3 });
            settings.set("string_arr", std::vector<std::string> { "asdf", "vnbm" });
            settings.set("complex.path.1", std::vector<int> { 1, 2, 3 });
            settings.set("complex.path.2", 1.5);
            settings.set("complex.path.3.4", 8.1);
            return settings;
        };

        DOCTEST_SUBCASE("serialization")
        {
            auto read_json = [&test_file] {
                Json::Value root;
                std::ifstream file(test_file);
                Json::CharReaderBuilder builder;
                std::string errs;
                DOCTEST_CHECK(parseFromStream(builder, file, &root, &errs));
                if (!errs.empty())
                {
                    OPENDCC_ERROR(errs.c_str());
                }
                return root;
            };

            DOCTEST_SUBCASE("set")
            {
                init_settings();
                auto root = read_json();
                DOCTEST_CHECK_EQ(root["bool"].asBool(), true);
                DOCTEST_CHECK_EQ(root["int"].asInt(), 54);
                DOCTEST_CHECK_EQ(root["float"].asFloat(), 23.54f);
                DOCTEST_CHECK_EQ(root["string"].asString(), "zxcv");
                DOCTEST_CHECK_EQ(root["bool_arr"][0].asBool(), true);
                DOCTEST_CHECK_EQ(root["bool_arr"][1].asBool(), false);
                DOCTEST_CHECK_EQ(root["bool_arr"][2].asBool(), true);
                DOCTEST_CHECK_EQ(root["int_arr"][0].asInt(), 3);
                DOCTEST_CHECK_EQ(root["int_arr"][1].asInt(), 6);
                DOCTEST_CHECK_EQ(root["int_arr"][2].asInt(), 9);
                DOCTEST_CHECK_EQ(root["float_arr"][0].asFloat(), 3.2f);
                DOCTEST_CHECK_EQ(root["float_arr"][1].asFloat(), 6);
                DOCTEST_CHECK_EQ(root["float_arr"][2].asFloat(), 1);
                DOCTEST_CHECK_EQ(root["float_arr"][3].asFloat(), 9.3f);
                DOCTEST_CHECK_EQ(root["string_arr"][0].asString(), "asdf");
                DOCTEST_CHECK_EQ(root["string_arr"][1].asString(), "vnbm");
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"].size(), 3);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"][0].asInt(), 1);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"][1].asInt(), 2);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"][2].asInt(), 3);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["2"].asDouble(), 1.5);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["3"]["4"].asDouble(), 8.1);
            }
            DOCTEST_SUBCASE("reset")
            {
                auto settings = init_settings();
                settings.set_default("float", 42.5f);
                settings.set_default("string_arr", std::vector<std::string> { "ikm", "tgb" });
                settings.set_default("complex.path.1", std::vector<int> { 6, 7 });
                settings.set_default("complex.path.3.4", 3.4);
                settings.reset("float");
                settings.reset("string_arr");
                settings.reset("complex.path");
                auto root = read_json();
                DOCTEST_CHECK_EQ(root["bool"].asBool(), true);
                DOCTEST_CHECK_EQ(root["int"].asInt(), 54);
                DOCTEST_CHECK_EQ(root["float"].asFloat(), 42.5f);
                DOCTEST_CHECK_EQ(root["string"].asString(), "zxcv");
                DOCTEST_CHECK_EQ(root["bool_arr"][0].asBool(), true);
                DOCTEST_CHECK_EQ(root["bool_arr"][1].asBool(), false);
                DOCTEST_CHECK_EQ(root["bool_arr"][2].asBool(), true);
                DOCTEST_CHECK_EQ(root["int_arr"][0].asInt(), 3);
                DOCTEST_CHECK_EQ(root["int_arr"][1].asInt(), 6);
                DOCTEST_CHECK_EQ(root["int_arr"][2].asInt(), 9);
                DOCTEST_CHECK_EQ(root["float_arr"][0].asFloat(), 3.2f);
                DOCTEST_CHECK_EQ(root["float_arr"][1].asFloat(), 6);
                DOCTEST_CHECK_EQ(root["float_arr"][2].asFloat(), 1);
                DOCTEST_CHECK_EQ(root["float_arr"][3].asFloat(), 9.3f);
                DOCTEST_CHECK_EQ(root["string_arr"][0].asString(), "ikm");
                DOCTEST_CHECK_EQ(root["string_arr"][1].asString(), "tgb");
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"].size(), 2);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"][0].asInt(), 6);
                DOCTEST_CHECK_EQ(root["complex"]["path"]["1"][1].asInt(), 7);
                DOCTEST_CHECK(root["complex"]["path"]["2"].isNull());
                DOCTEST_CHECK_EQ(root["complex"]["path"]["3"]["4"].asDouble(), 3.4);
            }
            DOCTEST_SUBCASE("remove")
            {
                auto settings = init_settings();
                settings.remove("float");
                settings.remove("string_arr");
                settings.remove("complex.path");
                auto root = read_json();
                DOCTEST_CHECK_EQ(root["bool"].asBool(), true);
                DOCTEST_CHECK_EQ(root["int"].asInt(), 54);
                DOCTEST_CHECK(root["float"].isNull());
                DOCTEST_CHECK_EQ(root["string"].asString(), "zxcv");
                DOCTEST_CHECK_EQ(root["bool_arr"][0].asBool(), true);
                DOCTEST_CHECK_EQ(root["bool_arr"][1].asBool(), false);
                DOCTEST_CHECK_EQ(root["bool_arr"][2].asBool(), true);
                DOCTEST_CHECK_EQ(root["int_arr"][0].asInt(), 3);
                DOCTEST_CHECK_EQ(root["int_arr"][1].asInt(), 6);
                DOCTEST_CHECK_EQ(root["int_arr"][2].asInt(), 9);
                DOCTEST_CHECK_EQ(root["float_arr"][0].asFloat(), 3.2f);
                DOCTEST_CHECK_EQ(root["float_arr"][1].asFloat(), 6);
                DOCTEST_CHECK_EQ(root["float_arr"][2].asFloat(), 1);
                DOCTEST_CHECK_EQ(root["float_arr"][3].asFloat(), 9.3f);
                DOCTEST_CHECK(root["string_arr"].isNull());
                DOCTEST_CHECK(root["complex"]["path"].isNull());
            }
        }
        DOCTEST_SUBCASE("deserialization")
        {
            {
                init_settings();
            }
            Settings settings(test_file);

            DOCTEST_CHECK_EQ(settings.get<bool>("bool"), true);
            DOCTEST_CHECK_EQ(settings.get<int>("int"), 54);
            DOCTEST_CHECK_EQ(settings.get<float>("float"), 23.54f);
            DOCTEST_CHECK_EQ(settings.get<std::string>("string"), "zxcv");
            DOCTEST_CHECK_EQ(settings.get<std::vector<bool>>("bool_arr"), std::vector<bool> { true, false, true });
            DOCTEST_CHECK_EQ(settings.get<std::vector<int>>("int_arr"), std::vector<int> { 3, 6, 9 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<float>>("float_arr"), std::vector<float> { 3.2, 6, 1, 9.3 });
            DOCTEST_CHECK_EQ(settings.get<std::vector<std::string>>("string_arr"), std::vector<std::string> { "asdf", "vnbm" });
            DOCTEST_CHECK_EQ(settings.get<std::vector<int>>("complex.path.1"), std::vector<int> { 1, 2, 3 });
            DOCTEST_CHECK_EQ(settings.get<double>("complex.path.2"), 1.5);
            DOCTEST_CHECK_EQ(settings.get<double>("complex.path.3.4"), 8.1);
        }
        std::remove(tmp_filename);
    }
}
