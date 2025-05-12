// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "serialization.h"
#include <array>
OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

std::unordered_map<std::type_index, std::function<void(const VtValue&, Writer&)>> Writer::s_pack_value_functions;
std::array<std::function<void(Reader&, VtValue&)>, static_cast<size_t>(TypeEnum::NumTypes)> Reader::s_unpack_value_functions;

Writer::Writer()
{
    static std::once_flag once;
    std::call_once(once, [] {
#define xx(ENUMTYPE, ENUMVALUE, CPPTYPE, _unused2) register_type<CPPTYPE>(TypeEnum::ENUMTYPE);

#include "usd_data_types.h"

#undef xx
    });
}

Writer::Writer(const std::vector<char>& buffer)
    : Writer()
{
    m_buffer = buffer;
}

void Writer::write(const VtValue& val)
{
    const std::type_index type_index = val.IsArrayValued() ? val.GetElementTypeid() : val.GetTypeid();
    auto packer_iter = s_pack_value_functions.find(type_index);
    if (TF_VERIFY(packer_iter != s_pack_value_functions.end(), "Failed to serialize vtvalue of type \"%s\".", type_index.name()))
        packer_iter->second(val, *this);
}

void Writer::write(const SdfTimeCode& val)
{
    write(val.GetValue());
}

void Writer::write(const SdfAssetPath& val)
{
    write(val.GetAssetPath());
}

void Writer::write(const VtDictionary& val)
{
    write_map(val);
}

void Writer::write(const TfToken& val)
{
    write_array(val.GetString());
}

void Writer::write(const SdfPath& val)
{
    write_array(val.GetString());
}

void Writer::write(const SdfUnregisteredValue& val)
{
    write(val.GetValue());
}

void Writer::write(const SdfVariantSelectionMap& val)
{
    write_map(val);
}

void Writer::write(const SdfLayerOffset& val)
{
    write(val.GetOffset());
    write(val.GetScale());
}

void Writer::write(const SdfReference& val)
{
    write(val.GetAssetPath());
    write(val.GetPrimPath());
    write(val.GetLayerOffset());
    write(val.GetCustomData());
}

void Writer::write(const SdfPayload& val)
{
    write(val.GetAssetPath());
    write(val.GetPrimPath());
    write(val.GetLayerOffset());
}

void Writer::write(const std::string& val)
{
    write_array(val);
}

Reader::Reader(const std::vector<char>& buffer, size_t offset)
    : m_buffer(buffer)
    , m_offset(offset)
{
    std::once_flag once;
    std::call_once(once, [] {
#define xx(ENUMTYPE, ENUMVALUE, CPPTYPE, _unused2) register_type<CPPTYPE>(TypeEnum::ENUMTYPE);
#include "usd_data_types.h"
#undef xx
    });
}

SdfPayload Reader::read(SdfPayload*)
{
    const auto asset_path = read_array<std::string>();
    const auto prim_path = read<SdfPath>();
    const auto layer_offset = read<SdfLayerOffset>();
    return SdfPayload(asset_path, prim_path, layer_offset);
}

SdfReference Reader::read(SdfReference*)
{
    const auto asset_path = read_array<std::string>();
    const auto prim_path = read<SdfPath>();
    const auto layer_offset = read<SdfLayerOffset>();
    const auto custom_data = read_map<VtDictionary>();
    return SdfReference(asset_path, prim_path, layer_offset, custom_data);
}

SdfLayerOffset Reader::read(SdfLayerOffset*)
{
    const auto layer_offset = read<double>();
    const auto scale = read<double>();
    return SdfLayerOffset(layer_offset, scale);
}

SdfVariantSelectionMap Reader::read(SdfVariantSelectionMap*)
{
    return read_map<SdfVariantSelectionMap>();
}

SdfUnregisteredValue Reader::read(SdfUnregisteredValue*)
{
    const auto val = read<VtValue>();
    if (val.IsHolding<std::string>())
        return SdfUnregisteredValue(val.UncheckedGet<std::string>());
    if (val.IsHolding<VtDictionary>())
        return SdfUnregisteredValue(val.UncheckedGet<VtDictionary>());
    if (val.IsHolding<SdfUnregisteredValueListOp>())
        return SdfUnregisteredValue(val.UncheckedGet<SdfUnregisteredValueListOp>());
    TF_CODING_ERROR("SdfUnregisteredValue contains invalid "
                    "type '%s' = '%s'; expected string, VtDictionary or "
                    "SdfUnregisteredValueListOp; returning empty",
                    val.GetTypeName().c_str(), TfStringify(val).c_str());
    return SdfUnregisteredValue();
}

SdfTimeCode Reader::read(SdfTimeCode*)
{
    return SdfTimeCode(read<double>());
}

SdfAssetPath Reader::read(SdfAssetPath*)
{
    return SdfAssetPath(read_array<std::string>());
}

VtDictionary Reader::read(VtDictionary*)
{
    return read_map<VtDictionary>();
}

TfToken Reader::read(TfToken*)
{
    return TfToken(read_array<std::string>());
}

SdfPath Reader::read(SdfPath*)
{
    return SdfPath(read_array<std::string>());
}

VtValue Reader::read(VtValue*)
{
    const auto type = read<TypeEnum>();
    VtValue result;
    s_unpack_value_functions[static_cast<size_t>(type)](*this, result);
    return result;
}

std::string Reader::read(std::string*)
{
    return read_array<std::string>();
}
OPENDCC_NAMESPACE_CLOSE

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
// Note: this define should be used once per shared lib
#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include <doctest/doctest.h>

OPENDCC_NAMESPACE_USING

DOCTEST_TEST_SUITE("serialization")
{
    DOCTEST_TEST_CASE("serialize_multiple_basic_types")
    {
        Writer packer;
        packer.write(true);
        packer.write(3);
        packer.write(-87);
        packer.write((size_t)264425);
        packer.write(76.15f);
        packer.write(567.134);
        packer.write('d');

        const auto actual_size = packer.get_buffer().size();
        const auto expected_size = sizeof(bool) + sizeof(int) + sizeof(int) + sizeof(size_t) + sizeof(float) + sizeof(double) + sizeof(char);
        DOCTEST_CHECK(actual_size == expected_size);

        Reader reader(packer.get_buffer());
        DOCTEST_CHECK(reader.read<bool>() == true);
        DOCTEST_CHECK(reader.read<int>() == 3);
        DOCTEST_CHECK(reader.read<int>() == -87);
        DOCTEST_CHECK(reader.read<size_t>() == 264425);
        DOCTEST_CHECK(reader.read<float>() == 76.15f);
        DOCTEST_CHECK(reader.read<double>() == 567.134);
        DOCTEST_CHECK(reader.read<char>() == 'd');
        DOCTEST_CHECK(reader.tell() == expected_size);
    }

    DOCTEST_TEST_CASE("serialize_strings")
    {
        Writer packer;
        packer.write(std::string("/asdfj/fasd45/13hn,;.")); // len 21
        packer.write(SdfPath("/abc/def/ghjk231")); // len 16
        packer.write(TfToken("my_token")); // len 8

        const std::vector<std::string> array_of_strings = {
            "asdfzxcvb", "ncbntqrgqrg", "r4yq38467tq" // len 9, 11, 11
        };
        const std::vector<SdfPath> array_of_paths = {
            SdfPath("/asdf/vgfaf"), SdfPath("/afawef/adfvgar"), SdfPath("/gjuq3/t213opa")
            // len 11, 15, 14
        };
        packer.write_array(array_of_strings);
        packer.write_array(array_of_paths);
        const auto actual_size = packer.get_buffer().size();
        const auto expected_size = sizeof(char) * (21 + 16 + 8 + 9 + 11 + 11 + 11 + 15 + 14) + sizeof(size_t) * 11;
        DOCTEST_CHECK(actual_size == expected_size);

        Reader reader(packer.get_buffer());
        DOCTEST_CHECK(reader.read<std::string>() == "/asdfj/fasd45/13hn,;.");
        DOCTEST_CHECK(reader.read<SdfPath>() == SdfPath("/abc/def/ghjk231"));
        DOCTEST_CHECK(reader.read<TfToken>() == TfToken("my_token"));
        DOCTEST_CHECK(reader.read_array<std::vector<std::string>>() == array_of_strings);
        DOCTEST_CHECK(reader.read_array<std::vector<SdfPath>>() == array_of_paths);
        DOCTEST_CHECK(reader.tell() == expected_size);
    }

    DOCTEST_TEST_CASE("serialize_basic_types_arrays")
    {
        Writer packer;
        packer.write_array(VtIntArray({ 3, 7, -2 }));
        packer.write_array(VtFloatArray({ 76.15, 32.5, -13.43 }));
        packer.write_array(std::vector<double>({ 31, 43.23 }));

        const auto actual_size = packer.get_buffer().size();
        const auto expected_size = sizeof(int) * 3 + sizeof(float) * 3 + sizeof(double) * 2 + sizeof(size_t) * 3;
        DOCTEST_CHECK(actual_size == expected_size);

        Reader reader(packer.get_buffer());
        DOCTEST_CHECK(reader.read_array<VtIntArray>() == VtIntArray({ 3, 7, -2 }));
        DOCTEST_CHECK(reader.read_array<VtFloatArray>() == VtFloatArray({ 76.15, 32.5, -13.43 }));
        DOCTEST_CHECK(reader.read_array<std::vector<double>>() == std::vector<double>({ 31, 43.23 }));
        DOCTEST_CHECK(reader.tell() == expected_size);
    }

    DOCTEST_TEST_CASE("serialize_vtvalue")
    {
        Writer packer;
        packer.write(VtValue(true));
        packer.write(VtValue(5));
        packer.write(VtValue(VtFloatArray({ 76.15, 32.5, -13.43 })));
        packer.write(VtValue(SdfPath("/asdf/xcvb")));
        packer.write(VtValue(GfVec3f(5, 8, 9)));
        packer.write(VtValue(std::string("deus_vult")));
        packer.write(VtValue(VtValue(std::string("tyuio"))));
        packer.write(VtValue());

        Reader reader(packer.get_buffer());
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(true));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(5));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(VtFloatArray({ 76.15, 32.5, -13.43 })));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(SdfPath("/asdf/xcvb")));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(GfVec3f(5, 8, 9)));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(std::string("deus_vult")));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue(VtValue(std::string("tyuio"))));
        DOCTEST_CHECK(reader.read<VtValue>() == VtValue());
    }
}
