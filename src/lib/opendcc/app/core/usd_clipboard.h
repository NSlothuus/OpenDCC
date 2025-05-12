/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "pxr/base/vt/value.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/attribute.h"
#include <pxr/usd/usdUtils/stageCache.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API UsdClipboard
{
public:
    UsdClipboard();
    ~UsdClipboard();

    PXR_NS::UsdStageWeakPtr get_clipboard();
    void clear_clipboard();
    void set_clipboard(const PXR_NS::UsdStageWeakPtr& clipboardData);
    void set_clipboard_path(const std::string& clipboard_path);
    void set_clipboard_file_format(const std::string& clipboard_path);
    void save_clipboard_data(const PXR_NS::UsdStageWeakPtr& stage);
    void set_clipboard_attribute(const PXR_NS::UsdAttribute& attribute);
    void set_clipboard_stage(const PXR_NS::UsdStageWeakPtr& stage);

    PXR_NS::UsdAttribute get_clipboard_attribute();
    PXR_NS::UsdStageWeakPtr get_clipboard_stage();
    PXR_NS::UsdStageWeakPtr get_new_clipboard_stage(const std::string& data_type);
    PXR_NS::UsdAttribute get_new_clipboard_attribute(const PXR_NS::SdfValueTypeName& type_name);

    UsdClipboard(UsdClipboard&&) = delete;
    UsdClipboard& operator=(UsdClipboard&&) = delete;

private:
    std::string m_path_to_clipboard;
    std::string m_clipboard_file_format;
    PXR_NS::UsdStageCache::Id m_clipboardStageCacheId;
    PXR_NS::UsdStageCache::Id m_tmpClipboardStageCacheId;
};

OPENDCC_NAMESPACE_CLOSE
