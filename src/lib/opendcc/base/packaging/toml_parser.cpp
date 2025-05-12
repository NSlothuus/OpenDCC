// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#define CPPTOML_NO_RTTI
#include "opendcc/base/packaging/toml_parser.h"
#include <fstream>
#include "opendcc/base/vendor/cpptoml/cpptoml.h"
#include "opendcc/base/logging/logger.h"
#include <unordered_map>
#include <vector>
#include "pxr/base/vt/array.h"
#include "opendcc/base/vendor/ghc/filesystem.hpp"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN
using namespace cpptoml;

namespace
{
    VtValue parse_val(base& val)
    {
        switch (val.type())
        {
        case base_type::STRING:
            return VtValue(val.as<std::string>()->get());
        case base_type::INT:
            return VtValue(val.as<int64_t>()->get());
        case base_type::FLOAT:
            return VtValue(val.as<double>()->get());
        case base_type::BOOL:
            return VtValue(val.as<bool>()->get());
        case base_type::ARRAY:
        {
            const auto& vals_array = val.as_array()->get();
            VtArray<VtValue> res_array;
            for (const auto& val : vals_array)
            {
                res_array.push_back(parse_val(*val));
            }
            return VtValue(res_array);
        }
        case base_type::TABLE:
        {
            const auto& vals_table = val.as_table();
            VtDictionary res_table;
            for (const auto& val : *vals_table)
            {
                res_table[std::string(val.first)] = parse_val(*val.second);
            }
            return VtValue(res_table);
        }
        case base_type::TABLE_ARRAY:
        {
            const auto& vals_table_arr = val.as_table_array();
            VtArray<VtDictionary> res_table_arr;
            for (const auto& val : *vals_table_arr)
            {
                res_table_arr.push_back(parse_val(*val).Get<VtDictionary>());
            }
            return VtValue(std::move(res_table_arr));
        }
            // rest not implemented yet: date
        default:
            return VtValue();
        }
    }
};

PackageData TOMLParser::parse(const std::string& path)
{
    auto result = PackageData();
    std::shared_ptr<cpptoml::table> table;
    try
    {
        table = parse_file(path);
    }
    catch (cpptoml::parse_exception e)
    {
        OPENDCC_ERROR("Failed to parse package at path '{}': {}", path, e.what());
        return result;
    }

    if (!table)
    {
        return result;
    }

    result.path = ghc::filesystem::path(path).parent_path().generic_string();
    if (auto name_opt = table->get_qualified_as<std::string>("base.name"))
    {
        result.name = *name_opt;
    }
    else
    {
        return result;
    }

    for (const auto& val_entry : *table)
    {
        PackageAttribute attr;
        attr.name = val_entry.first;
        attr.value = parse_val(*val_entry.second);
        result.raw_attributes.push_back(std::move(attr));
    }

    return result;
}

OPENDCC_NAMESPACE_CLOSE

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest/doctest.h>
OPENDCC_NAMESPACE_USING

DOCTEST_TEST_SUITE("PackagingTOMLParserTests")
{
    DOCTEST_TEST_CASE("deserialization")
    {

        char tmp_filename[1024] = {};
        tmpnam(tmp_filename);
        std::string test_file(tmp_filename);
        auto write_toml = [&test_file](const std::string& text) {
            std::ofstream out(test_file);
            out << text;
        };
        PackageData actual;
        auto validate_attr = [&actual](const std::string& name, const VtValue& expected) {
            for (const auto& attr : actual.raw_attributes)
            {
                if (attr.name == name)
                {
                    DOCTEST_CHECK_EQ(attr.value, expected);
                }
            }
        };
        auto parser = TOMLParser();

        DOCTEST_SUBCASE("basic_types")
        {
            const auto text = R"(
bool = true
int = 1
float = 3.1415
str = 'some str'
arr = [1, 2, 3]
mixed_arr = [3.41, 0]

[base]
name = 'asdf'

[table]
val1 = 'cx'
val2 = 3
inline = { a = 'a', b = 'c' }

[[table_arr]]
a = 42

[[table_arr.val]]
"a.b" = 64

[[table_arr]]
c = 'c'
)";
            write_toml(text);

            auto actual = parser.parse(test_file);
            DOCTEST_CHECK_EQ(actual.name, std::string("asdf"));
            validate_attr("name", VtValue("asdf"));
            validate_attr("bool", VtValue(true));
            validate_attr("int", VtValue(1ll));
            validate_attr("float", VtValue(3.1415));
            validate_attr("str", VtValue("some str"));
            validate_attr("arr", VtValue(VtArray<int64_t> { 1, 2, 3 }));
            validate_attr("mixed_arr", VtValue(VtArray<VtValue> { VtValue(3.41), VtValue(0ll) }));
            validate_attr("table", VtValue(VtDictionary { { std::string("val1"), VtValue("cx") },
                                                          { std::string("val2"), VtValue(3ll) },
                                                          { std::string("inline"), VtValue(VtDictionary { { std::string("a"), VtValue("a") },
                                                                                                          { std::string("b"), VtValue("b") } }) } }));
            validate_attr("table_arr", VtValue(VtArray<VtDictionary> {
                                           VtDictionary { { std::string("a"), VtValue(42ll) },
                                                          { std::string("val"), VtValue(VtDictionary { { std::string("a.b"), VtValue(64ll) } }) } },
                                           VtDictionary { { std::string("c"), VtValue("c") } } }));
        }
        std::remove(tmp_filename);
    }
}
