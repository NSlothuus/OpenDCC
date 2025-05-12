/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/usd_ipc_serialization/api.h"
#include <pxr/usd/sdf/layerOffset.h>
#include <pxr/usd/sdf/reference.h>
#include <pxr/usd/sdf/payload.h>
#include <pxr/usd/sdf/listOp.h>
#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/sdf/timeCode.h>
#include <pxr/usd/sdf/types.h>

#include <mutex>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

struct TimeSamples
{
    bool operator==(const TimeSamples&) const { return true; }
};

struct _ListOpHeader
{
    enum _Bits
    {
        IsExplicitBit = 1 << 0,
        HasExplicitItemsBit = 1 << 1,
        HasAddedItemsBit = 1 << 2,
        HasDeletedItemsBit = 1 << 3,
        HasOrderedItemsBit = 1 << 4,
        HasPrependedItemsBit = 1 << 5,
        HasAppendedItemsBit = 1 << 6
    };

    _ListOpHeader()
        : bits(0)
    {
    }

    template <class T>
    explicit _ListOpHeader(PXR_NS::SdfListOp<T> const& op)
        : bits(0)
    {
        bits |= op.IsExplicit() ? IsExplicitBit : 0;
        bits |= op.GetExplicitItems().size() ? HasExplicitItemsBit : 0;
        bits |= op.GetAddedItems().size() ? HasAddedItemsBit : 0;
        bits |= op.GetPrependedItems().size() ? HasPrependedItemsBit : 0;
        bits |= op.GetAppendedItems().size() ? HasAppendedItemsBit : 0;
        bits |= op.GetDeletedItems().size() ? HasDeletedItemsBit : 0;
        bits |= op.GetOrderedItems().size() ? HasOrderedItemsBit : 0;
    }

    bool IsExplicit() const { return bits & IsExplicitBit; }

    bool HasExplicitItems() const { return bits & HasExplicitItemsBit; }
    bool HasAddedItems() const { return bits & HasAddedItemsBit; }
    bool HasPrependedItems() const { return bits & HasPrependedItemsBit; }
    bool HasAppendedItems() const { return bits & HasAppendedItemsBit; }
    bool HasDeletedItems() const { return bits & HasDeletedItemsBit; }
    bool HasOrderedItems() const { return bits & HasOrderedItemsBit; }

    uint8_t bits;
};

template <class T>
struct ValueTypeTraits
{
};

#define xx(_unused1, _unused2, CPPTYPE, SUPPORTS_ARRAY)        \
    template <>                                                \
    struct ValueTypeTraits<CPPTYPE>                            \
    {                                                          \
        static constexpr bool supports_array = SUPPORTS_ARRAY; \
    };

#include "usd_data_types.h"
#undef xx

enum class TypeEnum
{
    Invalid = 0,
#define xx(ENUMNAME, ENUMVALUE, _unused1, _unused2) ENUMNAME = ENUMVALUE,

#include "usd_data_types.h"

#undef xx
    NumTypes
};

class USD_IPC_SERIALIZATION_API Writer
{
public:
    Writer();
    Writer(const std::vector<char>& buffer);
    Writer(const Writer&) = default;
    Writer(Writer&&) = default;
    ~Writer() = default;

    std::vector<char> get_buffer() const { return m_buffer; }

    template <class T>
    void write(const T& val)
    {
        const size_t offset = m_buffer.size();
        const auto new_size = m_buffer.size() + sizeof(T);
        m_buffer.resize(new_size);
        memcpy(m_buffer.data() + offset, &val, sizeof(T));
    }

    template <class TMap>
    void write_map(const TMap& val)
    {
        write(val.size());
        for (auto const& kv : val)
        {
            write(kv.first);
            write(kv.second);
        }
    }

    template <class TArray>
    void write_array(const TArray& val)
    {
        write(static_cast<size_t>(val.size()));
        for (const auto el : val)
            write(el);
    }

    void write(const std::string& val);

    void write(const PXR_NS::SdfPath& val);

    void write(const PXR_NS::TfToken& val);

    void write(const PXR_NS::VtDictionary& val);

    void write(const PXR_NS::SdfAssetPath& val);

    void write(const PXR_NS::SdfTimeCode& val);

    void write(const PXR_NS::SdfUnregisteredValue& val);

    void write(const PXR_NS::SdfVariantSelectionMap& val);

    void write(const PXR_NS::SdfLayerOffset& val);

    void write(const PXR_NS::SdfReference& val);

    void write(const PXR_NS::SdfPayload& val);

    template <class T>
    void write(const PXR_NS::SdfListOp<T>& listOp)
    {
        _ListOpHeader h(listOp);
        write(h);
        if (h.HasExplicitItems())
        {
            write(listOp.GetExplicitItems());
        }
        if (h.HasAddedItems())
        {
            write(listOp.GetAddedItems());
        }
        if (h.HasPrependedItems())
        {
            write(listOp.GetPrependedItems());
        }
        if (h.HasAppendedItems())
        {
            write(listOp.GetAppendedItems());
        }
        if (h.HasDeletedItems())
        {
            write(listOp.GetDeletedItems());
        }
        if (h.HasOrderedItems())
        {
            write(listOp.GetOrderedItems());
        }
    }

    template <class T>
    void write(const std::vector<T>& val)
    {
        write_array(val);
    }

    void write(const PXR_NS::VtValue& val);

private:
    template <class T>
    static void register_type(TypeEnum enum_value);

    std::vector<char> m_buffer;
    static std::unordered_map<std::type_index, std::function<void(const PXR_NS::VtValue&, Writer&)>> s_pack_value_functions;
};

class USD_IPC_SERIALIZATION_API Reader
{
public:
    Reader(const std::vector<char>& buffer, size_t offset = 0);

    size_t tell() const { return m_offset; }

    template <class T>
    T read()
    {
        return read(static_cast<T*>(nullptr));
    }

    template <class TMap>
    TMap read_map()
    {
        TMap result;
        const auto size = read<size_t>();
        for (size_t i = 0; i < size; i++)
        {
            auto key = read<typename TMap::key_type>();
            auto value = read<typename TMap::value_type::second_type>();
            result[key] = value;
        }
        return result;
    }

    template <class TArray>
    TArray read_array()
    {
        const auto size = read<size_t>();
        TArray result;
        result.reserve(size);

        for (size_t i = 0; i < size; i++)
            result.push_back(read<typename TArray::value_type>());

        return result;
    }

private:
    template <class T>
    T read(T*)
    {
        const auto result = *(reinterpret_cast<const T*>(m_buffer.data() + m_offset));
        m_offset += sizeof(T);
        return result;
    }

    std::string read(std::string*);

    PXR_NS::SdfPath read(PXR_NS::SdfPath*);

    PXR_NS::TfToken read(PXR_NS::TfToken*);

    PXR_NS::VtDictionary read(PXR_NS::VtDictionary*);

    PXR_NS::SdfAssetPath read(PXR_NS::SdfAssetPath*);

    PXR_NS::SdfTimeCode read(PXR_NS::SdfTimeCode*);

    PXR_NS::SdfUnregisteredValue read(PXR_NS::SdfUnregisteredValue*);

    PXR_NS::SdfVariantSelectionMap read(PXR_NS::SdfVariantSelectionMap*);

    PXR_NS::SdfLayerOffset read(PXR_NS::SdfLayerOffset*);

    PXR_NS::SdfReference read(PXR_NS::SdfReference*);

    PXR_NS::SdfPayload read(PXR_NS::SdfPayload*);

    template <class T>
    PXR_NS::SdfListOp<T> read(PXR_NS::SdfListOp<T>*)
    {
        PXR_NS::SdfListOp<T> listOp;
        const auto h = read<_ListOpHeader>();
        if (h.IsExplicit())
        {
            listOp.ClearAndMakeExplicit();
        }
        if (h.HasExplicitItems())
            listOp.SetExplicitItems(read<std::vector<T>>());
        if (h.HasAddedItems())
            listOp.SetAddedItems(read<std::vector<T>>());
        if (h.HasPrependedItems())
            listOp.SetPrependedItems(read<std::vector<T>>());
        if (h.HasAppendedItems())
            listOp.SetAppendedItems(read<std::vector<T>>());
        if (h.HasDeletedItems())
            listOp.SetDeletedItems(read<std::vector<T>>());
        if (h.HasOrderedItems())
            listOp.SetOrderedItems(read<std::vector<T>>());
        return listOp;
    }

    template <class T>
    std::vector<T> read(std::vector<T>*)
    {
        return read_array<std::vector<T>>();
    }

    PXR_NS::VtValue read(PXR_NS::VtValue*);

    template <class T>
    static void register_type(TypeEnum enum_value);

    static std::array<std::function<void(Reader&, PXR_NS::VtValue&)>, static_cast<size_t>(TypeEnum::NumTypes)> s_unpack_value_functions;
    std::vector<char> m_buffer;
    size_t m_offset;
};

template <class T>
struct ScalarValueHolder
{
    static void pack(const T& val, Writer& packer) { packer.write(val); }
    static void unpack(Reader& reader, T& val) { val = reader.read<T>(); }
};

template <>
struct ScalarValueHolder<void>
{
};

template <class T, bool SupportsArray = false>
struct ArrayValueHolder : ScalarValueHolder<T>
{
    static void pack_vtvalue(const PXR_NS::VtValue& val, Writer& packer) { ArrayValueHolder::pack(val.UncheckedGet<T>(), packer); }

    static void unpack_vtvalue(Reader& reader, PXR_NS::VtValue& val)
    {
        T unpacked_val;
        ArrayValueHolder::unpack(reader, unpacked_val);
        val.Swap(unpacked_val);
    }
};

template <>
struct ArrayValueHolder<void, false>
{
    static void pack_vtvalue(const PXR_NS::VtValue& val, Writer& packer) {}

    static void unpack_vtvalue(Reader& reader, PXR_NS::VtValue& val) { val = PXR_NS::VtValue(); }
};

template <class T>
struct ArrayValueHolder<T, true> : ScalarValueHolder<T>
{
    static void pack_vtvalue(const PXR_NS::VtValue& val, Writer& packer)
    {
        if (val.IsArrayValued())
        {
            pack_array(val.UncheckedGet<PXR_NS::VtArray<T>>(), packer);
        }
        else
        {
            packer.write(false);
            ArrayValueHolder::pack(val.UncheckedGet<T>(), packer);
        }
    }

    static void unpack_vtvalue(Reader& reader, PXR_NS::VtValue& val)
    {
        const auto is_array = reader.read<bool>();
        if (is_array)
        {
            PXR_NS::VtArray<T> unpacked_array;
            unpack_array(reader, unpacked_array);
            val.Swap(unpacked_array);
        }
        else
        {
            T unpacked_val;
            ArrayValueHolder::unpack(reader, unpacked_val);
            val.Swap(unpacked_val);
        }
    }

    static void pack_array(const PXR_NS::VtArray<T>& val, Writer& packer)
    {
        packer.write(true);
        packer.write_array(val);
    }

    static void unpack_array(Reader& reader, PXR_NS::VtArray<T>& val) { val = reader.read_array<PXR_NS::VtArray<T>>(); }
};

template <class T>
void Writer::register_type(TypeEnum enum_value)
{
    using ArrayValueHolder = ArrayValueHolder<T, ValueTypeTraits<T>::supports_array>;
    s_pack_value_functions[std::type_index(typeid(T))] = [enum_value](const PXR_NS::VtValue& val, Writer& packer) {
        packer.write(enum_value);
        ArrayValueHolder::pack_vtvalue(val, packer);
    };
}

template <class T>
void Reader::register_type(TypeEnum enum_value)
{
    using ArrayValueHolder = ArrayValueHolder<T, ValueTypeTraits<T>::supports_array>;
    s_unpack_value_functions[static_cast<size_t>(enum_value)] = [](Reader& reader, PXR_NS::VtValue& val) {
        ArrayValueHolder::unpack_vtvalue(reader, val);
    };
}
OPENDCC_NAMESPACE_CLOSE
