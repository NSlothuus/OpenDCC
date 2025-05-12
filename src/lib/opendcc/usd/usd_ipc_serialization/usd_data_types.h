/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

// Array types.
xx(Bool, 1, bool, true) xx(UChar, 2, uint8_t, true) xx(Int, 3, int, true) xx(UInt, 4, unsigned int, true) xx(Int64, 5, int64_t, true)
    xx(UInt64, 6, uint64_t, true)

        xx(Half, 7, PXR_NS::GfHalf, true) xx(Float, 8, float, true) xx(Double, 9, double, true)

            xx(String, 10, std::string, true)

                xx(Token, 11, PXR_NS::TfToken, true)

                    xx(AssetPath, 12, PXR_NS::SdfAssetPath, true)

                        xx(Quatd, 16, PXR_NS::GfQuatd, true) xx(Quatf, 17, PXR_NS::GfQuatf, true) xx(Quath, 18, PXR_NS::GfQuath, true)

                            xx(Vec2d, 19, PXR_NS::GfVec2d, true) xx(Vec2f, 20, PXR_NS::GfVec2f, true) xx(Vec2h, 21, PXR_NS::GfVec2h, true)
                                xx(Vec2i, 22, PXR_NS::GfVec2i, true)

                                    xx(Vec3d, 23, PXR_NS::GfVec3d, true) xx(Vec3f, 24, PXR_NS::GfVec3f, true) xx(Vec3h, 25, PXR_NS::GfVec3h, true)
                                        xx(Vec3i, 26, PXR_NS::GfVec3i, true)

                                            xx(Vec4d, 27, PXR_NS::GfVec4d, true) xx(Vec4f, 28, PXR_NS::GfVec4f, true)
                                                xx(Vec4h, 29, PXR_NS::GfVec4h, true) xx(Vec4i, 30, PXR_NS::GfVec4i, true)

                                                    xx(Matrix2d, 13, PXR_NS::GfMatrix2d, true) xx(Matrix3d, 14, PXR_NS::GfMatrix3d, true)
                                                        xx(Matrix4d, 15, PXR_NS::GfMatrix4d, true)

    // Non-array types.
    xx(Dictionary, 31, PXR_NS::VtDictionary, false)

        xx(TokenListOp, 32, PXR_NS::SdfTokenListOp, false) xx(StringListOp, 33, PXR_NS::SdfStringListOp, false)
            xx(PathListOp, 34, PXR_NS::SdfPathListOp, false) xx(ReferenceListOp, 35, PXR_NS::SdfReferenceListOp, false)
                xx(IntListOp, 36, PXR_NS::SdfIntListOp, false) xx(Int64ListOp, 37, PXR_NS::SdfInt64ListOp, false)
                    xx(UIntListOp, 38, PXR_NS::SdfUIntListOp, false) xx(UInt64ListOp, 39, PXR_NS::SdfUInt64ListOp, false)

                        xx(PathVector, 40, PXR_NS::SdfPathVector, false) xx(TokenVector, 41, std::vector<PXR_NS::TfToken>, false)

                            xx(Specifier, 42, PXR_NS::SdfSpecifier, false) xx(Permission, 43, PXR_NS::SdfPermission, false)
                                xx(Variability, 44, PXR_NS::SdfVariability, false)

                                    xx(VariantSelectionMap, 45, PXR_NS::SdfVariantSelectionMap, false) xx(TimeSamples, 46, TimeSamples, false)
                                        xx(Payload, 47, PXR_NS::SdfPayload, false)

                                            xx(DoubleVector, 48, std::vector<double>, false)
                                                xx(LayerOffsetVector, 49, std::vector<PXR_NS::SdfLayerOffset>, false)
                                                    xx(StringVector, 50, std::vector<std::string>, false)

                                                        xx(ValueBlock, 51, PXR_NS::SdfValueBlock, false) xx(Value, 52, PXR_NS::VtValue, false)

                                                            xx(UnregisteredValue, 53, PXR_NS::SdfUnregisteredValue, false)
                                                                xx(UnregisteredValueListOp, 54, PXR_NS::SdfUnregisteredValueListOp, false)
                                                                    xx(PayloadListOp, 55, PXR_NS::SdfPayloadListOp, false)
                                                                        xx(TimeCode, 56, PXR_NS::SdfTimeCode, true)

    // Additional types
    xx(Path, 57, PXR_NS::SdfPath, true) xx(Void, 58, void, false)