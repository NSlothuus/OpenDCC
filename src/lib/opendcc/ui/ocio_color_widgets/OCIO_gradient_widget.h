/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/ocio_color_widgets/api.h"

#include "opendcc/ui/common_widgets/gradient_widget.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class extends the GradientEditor class and provides functionality
 * for editing OCIO gradients.
 */

class OPENDCC_UI_OCIO_COLOR_WIDGETS_API OCIOGradientEditor : public GradientEditor
{
    Q_OBJECT;
    using RampV3f = Ramp<PXR_NS::GfVec3f>;

public:
    /**
     * @brief Initialize an empty gradient.
     *
     */
    OCIOGradientEditor();
    /**
     * @brief Initialize an OCIOGradientEditor object with the specified
     * color ramp.
     *
     * @param color_ramp The color ramp to copy.
     */
    OCIOGradientEditor(std::shared_ptr<RampV3f> color_ramp);
};

OPENDCC_NAMESPACE_CLOSE
