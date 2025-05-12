/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <vector>
#include <QColor>

#ifndef assert
#define assert(...)
#endif

OPENDCC_NAMESPACE_OPEN

template <class T>
T clamp(const T& val, const T& min, const T& max)
{
    return val < min ? min : val > max ? max : val;
}

struct float4
{
    float x, y, z, w;

    float4() {}
    float4(float val)
        : x(val)
        , y(val)
        , z(val)
        , w(val)
    {
    }
    float4(float r, float g, float b, float a)
        : x(r)
        , y(g)
        , z(b)
        , w(a)
    {
    }
    float4(const QColor& val)
    {
        x = val.redF();
        y = val.greenF();
        z = val.blueF();
        w = val.alphaF();
    }

    float4& operator+=(const float4& rval)
    {
        x += rval.x;
        y += rval.y;
        z += rval.z;
        w += rval.w;
        return *this;
    }

    float4& operator-=(const float4& rval)
    {
        x -= rval.x;
        y -= rval.y;
        z -= rval.z;
        w -= rval.w;
        return *this;
    }

    float4& operator+=(const float& rval)
    {
        x += rval;
        y += rval;
        z += rval;
        w += rval;
        return *this;
    }

    float4& operator-=(const float& rval)
    {
        x -= rval;
        y -= rval;
        z -= rval;
        w -= rval;
        return *this;
    }

    float4& operator*=(const float4& rval)
    {
        x *= rval.x;
        y *= rval.y;
        z *= rval.z;
        w *= rval.w;
        return *this;
    }

    float4& operator/=(const float4& rval)
    {
        x /= rval.x;
        y /= rval.y;
        z /= rval.z;
        w /= rval.w;
        return *this;
    }

    float4& operator*=(const float rval)
    {
        x *= rval;
        y *= rval;
        z *= rval;
        w *= rval;
        return *this;
    }

    float4& operator/=(const float rval)
    {
        x /= rval;
        y /= rval;
        z /= rval;
        w /= rval;
        return *this;
    }

    float& operator[](int i)
    {
        assert(i >= 0 && i < 4);
        return *(&x + i);
    }
    const float& operator[](int i) const
    {
        assert(i >= 0 && i < 4);
        return *(&x + i);
    }
};

inline float4 operator-(const float4& left, const float4& right)
{
    return { left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
}
inline float4 operator+(const float4& left, const float4& right)
{
    return { left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
}
inline float4 operator*(const float4& left, const float4& right)
{
    return { left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w };
}
inline float4 operator/(const float4& left, const float4& right)
{
    return { left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w };
}

inline float4 operator-(const float4& left, float right)
{
    return { left.x - right, left.y - right, left.z - right, left.w - right };
}
inline float4 operator+(const float4& left, float right)
{
    return { left.x + right, left.y + right, left.z + right, left.w + right };
}
inline float4 operator*(const float4& left, float right)
{
    return { left.x * right, left.y * right, left.z * right, left.w * right };
}
inline float4 operator/(const float4& left, float right)
{
    return { left.x / right, left.y / right, left.z / right, left.w / right };
}
inline float4 operator*(float right, const float4& left)
{
    return { left.x * right, left.y * right, left.z * right, left.w * right };
}

inline float4 operator-(const float4& left)
{
    return { left.x * -1, left.y * -1, left.z * -1, left.w * -1 };
}

/**
 * @brief The Ramp class provides functionality for evaluating the curve,
 * accessing, adding and removing its control points.
 */
template <class T>
class Ramp
{
public:
    /**
     * @enum The curve interpolation type.
     */
    enum InterpType
    {
        kNone = 0,
        kLinear,
        kSmooth,
        kSpline,
        kMonotoneSpline
    };

    struct CV
    {
        CV(double pos, const T& val, InterpType type, int cv_id = -1)
            : position(pos)
            , value(val)
            , interp_type(type)
            , id(cv_id)
        {
        }

        int id;
        double position;
        T value, deriv_val;
        InterpType interp_type;
    };

private:
    mutable int m_cache_cv;
    std::vector<CV> m_cv_data;

    bool m_prepared;
    int ids = 654;

public:
    Ramp();

    /**
     * @brief Removes all control points from the curve.
     */
    void clear();
    /**
     * @brief Gets the control point at the specified index.
     *
     * @param id The index of the control point.
     *
     * @return The control point at the specified index.
     */
    CV& cv(int id);

    /**
     * @brief Gets the control points.
     *
     * @return The control points.
     */
    std::vector<CV>& cv() { return m_cv_data; }
    /**
     * @brief Gets the control points.
     *
     * @return The control points.
     */
    const std::vector<CV>& cv() const { return m_cv_data; }
    /**
     * @brief Adds a control point to the curve.
     *
     * @param position The position of the control point.
     * @param val The value of the control point.
     * @param type The interpolation type of the control point.
     */
    void add_point(double position, const T& val, InterpType type);
    /**
     * @brief Removes a control point from the curve by id.
     *
     * @param id The index of the control point to remove.
     */
    void remove_point(int id);
    /**
     * @brief Prepares the control points for evaluation.
     *
     * It includes sorting the control points, computing boundaries,
     * and clamping extrema.
     */
    void prepare_points();
    /**
     * @brief Evaluates the curve and returns the value at the specified parameter.
     *
     * @param param The parameter at which to evaluate the curve.
     *
     * @return The value at the specified parameter.
     */
    T value_at(const double param) const;
    /**
     * @brief Evaluates the curve for a sub-component of the interpolation values.
     *
     * This method should be called after ``prepare_points()``.
     *
     * @param param The parameter at which to evaluate the curve.
     * @param channel The channel of the interpolation values to evaluate.
     *
     * @return The specified channel parameter value.
     */
    double channel_value(const double param, int channel) const;
    /**
     * @brief Returns the control point that is less than the specified parameter.
     *
     * If there is no point less than the parameter, it returns the right
     * point or nothing.
     *
     * @param param The parameter to compare against.
     *
     * @return The control point that is less than the specified parameter.
     */
    CV lower_bound_cv(const double param) const;
    /**
     * @brief Checks if the given interpolation type is supported.
     *
     * @param interp The interpolation type to check.
     *
     * @return True if the interpolation type is supported, false otherwise.
     */
    static bool interp_type_valid(InterpType interp);

private:
    //! CV Parameter ordering (cv1._pos < cv2._pos)
    static bool cv_less_than(const CV& cv1, const CV& cv2);
    //! Performs hermite derivative clamping in canonical space
    static void clamp_curve_segment(const T& delta, T& d1, T& d2);
};
OPENDCC_NAMESPACE_CLOSE

#include "ramp.inl"
