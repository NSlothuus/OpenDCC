/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/usd_ipc_serialization/api.h"
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <pxr/usd/sdf/path.h>

OPENDCC_NAMESPACE_OPEN

enum class UsdEditType : size_t
{
    SET_FIELD,
    SET_FIELD_DICT_VALUE_BY_KEY,
    SET_TIMESAMPLE,
    CREATE_SPEC,
    DELETE_SPEC,
    MOVE_SPEC,
    PUSH_CHILD,
    POP_CHILD,
    CHANGE_BLOCK_CLOSED,
    COUNT
};

class Writer;
class Reader;

class USD_IPC_SERIALIZATION_API UsdEditBase
{
public:
    UsdEditBase() = default;
    virtual ~UsdEditBase() = default;
    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) = 0;
    std::vector<char> write() const;
    static std::unique_ptr<UsdEditBase> read(const std::vector<char>& buffer);

protected:
    virtual void write_data(Writer& packer) const = 0;
    virtual void read_data(Reader& reader) = 0;
};

class USD_IPC_SERIALIZATION_API UsdEditLayerDependent : public UsdEditBase
{
public:
    UsdEditLayerDependent() = default;
    UsdEditLayerDependent(const std::string& layer_id);
    virtual const std::string& get_layer_id() const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

    std::string m_layer_id;
};

class USD_IPC_SERIALIZATION_API UsdEditSetField : public UsdEditLayerDependent
{
public:
    UsdEditSetField() = default;
    UsdEditSetField(const std::string& layer_id, const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::VtValue& value);
    UsdEditSetField(const std::string& layer_id, const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name,
                    const PXR_NS::SdfAbstractDataConstValue& value);
    virtual ~UsdEditSetField() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditSetField& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_path;
    PXR_NS::TfToken m_field_name;
    PXR_NS::VtValue m_value;
};

class USD_IPC_SERIALIZATION_API UsdEditSetFieldDictValueByKey : public UsdEditLayerDependent
{
public:
    UsdEditSetFieldDictValueByKey() = default;
    UsdEditSetFieldDictValueByKey(const std::string& layer_id, const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name,
                                  const PXR_NS::TfToken& key_path, const PXR_NS::VtValue& value);
    UsdEditSetFieldDictValueByKey(const std::string& layer_id, const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name,
                                  const PXR_NS::TfToken& key_path, const PXR_NS::SdfAbstractDataConstValue& value);
    virtual ~UsdEditSetFieldDictValueByKey() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditSetFieldDictValueByKey& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_path;
    PXR_NS::TfToken m_field_name;
    PXR_NS::TfToken m_key_path;
    PXR_NS::VtValue m_value;
};

class USD_IPC_SERIALIZATION_API UsdEditChangeBlockClosed : public UsdEditBase
{
public:
    UsdEditChangeBlockClosed() = default;
    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditChangeBlockClosed& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;
};

class USD_IPC_SERIALIZATION_API UsdEditSetTimesample : public UsdEditLayerDependent
{
public:
    UsdEditSetTimesample() = default;
    UsdEditSetTimesample(const std::string& layer_id, const PXR_NS::SdfPath& path, double time, const PXR_NS::VtValue& value);
    UsdEditSetTimesample(const std::string& layer_id, const PXR_NS::SdfPath& path, double time, const PXR_NS::SdfAbstractDataConstValue& value);
    virtual ~UsdEditSetTimesample() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditSetTimesample& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_path;
    double m_time;
    PXR_NS::VtValue m_value;
};

class USD_IPC_SERIALIZATION_API UsdEditCreateSpec : public UsdEditLayerDependent
{
public:
    UsdEditCreateSpec() = default;
    UsdEditCreateSpec(const std::string& layer_id, const PXR_NS::SdfPath& path, PXR_NS::SdfSpecType specType, bool inert);
    virtual ~UsdEditCreateSpec() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditCreateSpec& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_path;
    PXR_NS::SdfSpecType m_spec_type;
    bool m_inert;
};

class USD_IPC_SERIALIZATION_API UsdEditDeleteSpec : public UsdEditLayerDependent
{
public:
    UsdEditDeleteSpec() = default;
    UsdEditDeleteSpec(const std::string& layer_id, const PXR_NS::SdfPath& path, bool inert);
    virtual ~UsdEditDeleteSpec() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditDeleteSpec& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_path;
    bool m_inert;
};

class USD_IPC_SERIALIZATION_API UsdEditMoveSpec : public UsdEditLayerDependent
{
public:
    UsdEditMoveSpec() = default;
    UsdEditMoveSpec(const std::string& layer_id, const PXR_NS::SdfPath& old_path, const PXR_NS::SdfPath& new_path);
    virtual ~UsdEditMoveSpec() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditMoveSpec& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_old_path;
    PXR_NS::SdfPath m_new_path;
};

class USD_IPC_SERIALIZATION_API UsdEditPushChild : public UsdEditLayerDependent
{
public:
    UsdEditPushChild() = default;
    UsdEditPushChild(const std::string& layer_id, const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name,
                     const PXR_NS::TfToken& value);
    UsdEditPushChild(const std::string& layer_id, const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name,
                     const PXR_NS::SdfPath& value);
    virtual ~UsdEditPushChild() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditPushChild& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_parent_path;
    PXR_NS::TfToken m_field_name;
    PXR_NS::VtValue m_value;
};

class USD_IPC_SERIALIZATION_API UsdEditPopChild : public UsdEditLayerDependent
{
public:
    UsdEditPopChild() = default;
    UsdEditPopChild(const std::string& layer_id, const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name,
                    const PXR_NS::TfToken& old_value);
    UsdEditPopChild(const std::string& layer_id, const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name,
                    const PXR_NS::SdfPath& old_value);
    virtual ~UsdEditPopChild() override = default;

    virtual void apply(PXR_NS::SdfLayerStateDelegateBasePtr layer_state_delegate) override;
    bool operator==(const UsdEditPopChild& other) const;

protected:
    virtual void write_data(Writer& packer) const override;
    virtual void read_data(Reader& reader) override;

private:
    PXR_NS::SdfPath m_parent_path;
    PXR_NS::TfToken m_field_name;
    PXR_NS::VtValue m_old_value;
};

OPENDCC_NAMESPACE_CLOSE
