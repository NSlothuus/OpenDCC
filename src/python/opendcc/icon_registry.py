# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0


def init_usd_icons():
    import opendcc.core as dcc_core
    from pxr import Tf

    icon_registry = dcc_core.NodeIconRegistry.instance()
    icon_registry.register_icon("USD", "Mesh", ":/icons/mesh", ":/icons/node_editor/mesh")
    icon_registry.register_icon("USD", "Xform", ":/icons/xform", ":/icons/node_editor/xform")
    icon_registry.register_icon("USD", "Camera", ":/icons/camera", ":/icons/node_editor/camera")
    icon_registry.register_icon(
        "USD", "Material", ":/icons/material", ":/icons/node_editor/material"
    )
    icon_registry.register_icon(
        "USD", "CylinderLight", ":/icons/cylinderlight", ":/icons/node_editor/cylinderlight"
    )
    icon_registry.register_icon(
        "USD", "DiskLight", ":/icons/disklight", ":/icons/node_editor/disklight"
    )
    icon_registry.register_icon(
        "USD", "DistantLight", ":/icons/distantlight", ":/icons/node_editor/distantlight"
    )
    icon_registry.register_icon(
        "USD", "DomeLight", ":/icons/domelight", ":/icons/node_editor/domelight"
    )
    icon_registry.register_icon(
        "USD", "RectLight", ":/icons/rectlight", ":/icons/node_editor/rectlight"
    )
    icon_registry.register_icon(
        "USD", "SphereLight", ":/icons/spherelight", ":/icons/node_editor/spherelight"
    )
    icon_registry.register_icon(
        "USD", "GeometryLight", ":/icons/geometrylight", ":/icons/node_editor/geometrylight"
    )
    icon_registry.register_icon(
        "USD", "LightFilter", ":/icons/lightfilter", ":/icons/node_editor/lightfilter"
    )
    icon_registry.register_icon(
        "USD", "PortalLight", ":/icons/portallight", ":/icons/node_editor/portallight"
    )
    icon_registry.register_icon(
        "USD", "PointInstancer", ":/icons/pointinstancer", ":/icons/node_editor/pointinstancer"
    )
    icon_registry.register_icon("USD", "Scope", ":/icons/scope", ":/icons/node_editor/scope")
    icon_registry.register_icon(
        "USD", "BasisCurves", ":/icons/basiscurves", ":/icons/node_editor/basiscurves"
    )
    icon_registry.register_icon("USD", "Shader", ":/icons/shader", ":/icons/node_editor/shader")
    icon_registry.register_icon("USD", "Cube", ":/icons/cube", ":/icons/node_editor/cube")
    icon_registry.register_icon("USD", "Cone", ":/icons/cone", ":/icons/node_editor/cone")
    icon_registry.register_icon("USD", "Capsule", ":/icons/capsule", ":/icons/node_editor/capsule")
    icon_registry.register_icon(
        "USD", "Cylinder", ":/icons/cylinder", ":/icons/node_editor/cylinder"
    )
    icon_registry.register_icon("USD", "Sphere", ":/icons/sphere", ":/icons/node_editor/sphere")
    icon_registry.register_icon(
        "USD", "Backdrop", ":/icons/backdrop", ":/icons/node_editor/backdrop"
    )
    icon_registry.register_icon(
        "USD", "NurbsCurves", ":/icons/nurbscurves", ":/icons/node_editor/nurbscurves"
    )
    icon_registry.register_icon(
        "USD", "NurbsPatch", ":/icons/nurbspatch", ":/icons/node_editor/nurbspatch"
    )
    icon_registry.register_icon("USD", "Points", ":/icons/points", ":/icons/node_editor/points")
    icon_registry.register_icon(
        "USD", "GeomSubset", ":/icons/geomsubset", ":/icons/node_editor/geomsubset"
    )

    icon_registry.register_icon(
        "USD", "NodeGraph", ":/icons/nodegraph", ":/icons/node_editor/nodegraph"
    )
    icon_registry.register_icon(
        "USD", "RenderSettings", ":/icons/rendersettings", ":/icons/node_editor/rendersettings"
    )
