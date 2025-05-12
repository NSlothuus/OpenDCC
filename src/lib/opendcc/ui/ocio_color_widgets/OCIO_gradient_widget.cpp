// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/ocio_color_widgets/OCIO_gradient_widget.h"
#include "opendcc/ui/ocio_color_widgets/OCIO_color_widget.h"

OPENDCC_NAMESPACE_OPEN

OCIOGradientEditor::OCIOGradientEditor()
    : OCIOGradientEditor(std::make_shared<Ramp<PXR_NS::GfVec3f>>())
{
}

OCIOGradientEditor::OCIOGradientEditor(std::shared_ptr<RampV3f> color_ramp)
    : GradientEditor(color_ramp, new OCIOColorPickDialog())
{
}

OPENDCC_NAMESPACE_CLOSE
