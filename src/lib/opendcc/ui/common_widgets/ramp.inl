#include <algorithm>

#include <QColor>
#include <float.h>
#include "opendcc/ui/common_widgets/ramp.h"

#ifndef assert
#define assert(...)
#endif

OPENDCC_NAMESPACE_OPEN

template class Ramp<double>;
template class Ramp<float>;

namespace
{
    template <class T>
    static constexpr auto is_vec_helper() -> decltype((bool)T()[0])
    {
        return true;
    }
    template <class T>
    static constexpr auto is_vec_helper() -> typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
    {
        return false;
    }

    template <class T>
    static constexpr bool is_vec_v = is_vec_helper<T>();

    template <class T>
    static constexpr auto get_element_count() -> typename std::enable_if<is_vec_v<T>, size_t>::type
    {
        return sizeof(T) / sizeof(decltype(T()[0]));
    }

    template <class T>
    static constexpr auto get_element_count() -> typename std::enable_if<!is_vec_v<T>, size_t>::type
    {
        return 1;
    }

    //! Returns a component of the given value
    template <class T>
    static auto comp(const T& val, const int i) -> typename std::enable_if<is_vec_v<T>, double>::type
    {
        return static_cast<double>(val[i]);
    }
    template <class T>
    static auto comp(const T& val, const int i) -> typename std::enable_if<!is_vec_v<T>, double>::type
    {
        return val;
    }

    template <class T>
    static auto set_val(T& vec, double new_val, int i) -> typename std::enable_if<is_vec_v<T>>::type
    {
        vec[i] = new_val;
    }
    template <class T>
    static auto set_val(T& old_val, double new_val, int i) -> typename std::enable_if<!is_vec_v<T>>::type
    {
        old_val = new_val;
    }
};

template <class T>
void Ramp<T>::clamp_curve_segment(const T& delta, T& d1, T& d2)
{
    static constexpr size_t N = get_element_count<T>();
    for (size_t i = 0; i < N; ++i)
    {
        if (comp(delta, i) == 0.0)
        {
            set_val(d1, 0.0, i);
            set_val(d2, 0.0, i);
        }
        else
        {
            set_val(d1, clamp(comp(d1, i) / comp(delta, i), 0.0, 3.0) * comp(delta, i), i);
            set_val(d2, clamp(comp(d2, i) / comp(delta, i), 0.0, 3.0) * comp(delta, i), i);
        }
    }
}

template <class T>
bool Ramp<T>::cv_less_than(const CV& cv1, const CV& cv2)
{
    return cv1.position < cv2.position;
}

template <class T>
bool Ramp<T>::interp_type_valid(InterpType interp)
{
    return interp == kNone || interp == kLinear || interp == kSmooth || interp == kSpline || interp == kMonotoneSpline;
}

template <class T>
typename Ramp<T>::CV Ramp<T>::lower_bound_cv(const double param) const
{
    assert(m_prepared);
    const CV* cvDataBegin = &m_cv_data[0];
    int numPoints = m_cv_data.size();
    int index = std::upper_bound(cvDataBegin, cvDataBegin + numPoints, CV(param, T(), kLinear), cv_less_than) - cvDataBegin;

    index = std::max(1, std::min(index, numPoints - 1));
    if (index - 1 > 0)
        return m_cv_data[index - 1];

    return m_cv_data[index];
}

template <class T>
double Ramp<T>::channel_value(const double param, int channel) const
{
    assert(m_prepared);

    const int numPoints = m_cv_data.size();
    const CV* cvDataBegin = &m_cv_data[0];
    int index = std::upper_bound(cvDataBegin, cvDataBegin + numPoints, CV(param, T(), kLinear), cv_less_than) - cvDataBegin;
    index = std::max(1, std::min(index, numPoints - 1));

    const float t0 = m_cv_data[index - 1].position;
    const double k0 = comp(m_cv_data[index - 1].value, channel);
    const InterpType interp = m_cv_data[index - 1].interp_type;
    const float t1 = m_cv_data[index].position;
    const double k1 = comp(m_cv_data[index].value, channel);

    switch (interp)
    {
    case kNone:
        return k0;
        break;

    case kLinear:
    {
        double u = (param - t0) / (t1 - t0);
        return k0 + u * (k1 - k0);
    }
    break;

    case kSmooth:
    {
        double u = (param - t0) / (t1 - t0);
        return k0 * (u - 1) * (u - 1) * (2 * u + 1) + k1 * u * u * (3 - 2 * u);
    }
    break;

    case kSpline:
    case kMonotoneSpline:
    {
        double x = param - m_cv_data[index - 1].position; // xstart
        double h = m_cv_data[index].position - m_cv_data[index - 1].position; // xend-xstart
        double y = comp(m_cv_data[index - 1].value, channel); // f(xtart)
        double delta = comp(m_cv_data[index].value, channel) - comp(m_cv_data[index - 1].value, channel); // f(xend)-f(xtart)
        double d1 = comp(m_cv_data[index - 1].deriv_val, channel); // f'(xtart)
        double d2 = comp(m_cv_data[index].deriv_val, channel); // f'(xend)

        return (x * (delta * (3 * h - 2 * x) * x + h * (-h + x) * (-(d1 * h) + (d1 + d2) * x))) / (h * h * h) + y;
    }
    break;

    default:
        assert(false);
        return 0;
        break;
    }
}

template <class T>
T Ramp<T>::value_at(const double param) const
{
    assert(m_prepared);

    const int numPoints = m_cv_data.size();
    const CV* cvDataBegin = &m_cv_data[0];
    int index = std::upper_bound(cvDataBegin, cvDataBegin + numPoints, CV(param, T(), kLinear), cv_less_than) - cvDataBegin;
    index = std::max(1, std::min(index, numPoints - 1));

    const float t0 = m_cv_data[index - 1].position;
    const T k0 = m_cv_data[index - 1].value;
    const InterpType interp = m_cv_data[index - 1].interp_type;
    const float t1 = m_cv_data[index].position;
    const T k1 = m_cv_data[index].value;

    switch (interp)
    {
    case kNone:
        return k0;
        break;

    case kLinear:
    {
        double u = (t1 - t0 != 0 ? (param - t0) / (t1 - t0) : 0);
        return k0 + (k1 - k0) * u;
    }
    break;

    case kSmooth:
    {
        double u = (t1 - t0 != 0 ? (param - t0) / (t1 - t0) : 0);
        return k0 * (u - 1) * (u - 1) * (2 * u + 1) + k1 * u * u * (3 - 2 * u);
    }
    break;

    case kSpline:
    case kMonotoneSpline:
    {
        double x = param - m_cv_data[index - 1].position; // xstart
        double h = m_cv_data[index].position - m_cv_data[index - 1].position; // xend-xstart
        T y = m_cv_data[index - 1].value; // f(xstart)
        T delta = m_cv_data[index].value - m_cv_data[index - 1].value; // f(xend)-f(xstart)
        T d1 = m_cv_data[index - 1].deriv_val; // f'(xstart)
        T d2 = m_cv_data[index].deriv_val; // f'(xend)
        return (x * (delta * (3 * h - 2 * x) * x + h * (-h + x) * (-(d1 * h) + (d1 + d2) * x))) / (h * h * h) + y;
    }
    break;

    default:
        assert(false);
        return T();
        break;
    }
}

template <class T>
void Ramp<T>::prepare_points()
{
    m_prepared = true;
    m_cache_cv = 0;
    std::sort(m_cv_data.begin(), m_cv_data.end(), cv_less_than);

    CV& end = *(m_cv_data.end() - 1);
    CV& begin = *(m_cv_data.begin());
    int realCVs = m_cv_data.size() - 2;

    assert(realCVs >= 0);

    if (realCVs > 0)
    {
        begin.value = m_cv_data[1].value;
        begin.deriv_val = T();
        begin.interp_type = kNone;
        int lastIndex = m_cv_data.size() - 1;
        end.value = m_cv_data[lastIndex - 1].value;
        end.deriv_val = T();
        end.interp_type = kNone;
    }
    else
    {
        // begin.position = end.position = 0;
        begin.value = end.value = T();
        begin.interp_type = kNone;
        begin.deriv_val = end.deriv_val = T();
    }

    for (unsigned int i = 1; i < m_cv_data.size() - 1; i++)
    {
        m_cv_data[i].deriv_val = (m_cv_data[i + 1].value - m_cv_data[i - 1].value) / (m_cv_data[i + 1].position - m_cv_data[i - 1].position);
    }

    for (unsigned int i = 0; i < m_cv_data.size() - 1; i++)
    {
        if (m_cv_data[i].interp_type == kMonotoneSpline)
        {
            double h = m_cv_data[i + 1].position - m_cv_data[i].position;

            if (h == 0)
                m_cv_data[i].deriv_val = m_cv_data[i + 1].deriv_val = T();
            else
            {
                T delta = (m_cv_data[i + 1].value - m_cv_data[i].value) / h;

                clamp_curve_segment(delta, m_cv_data[i].deriv_val, m_cv_data[i + 1].deriv_val);
            }
        }
    }
}

template <class T>
void Ramp<T>::add_point(double position, const T& val, InterpType type)
{
    m_prepared = false;
    m_cv_data.push_back(CV(position, val, type, ids++));
}

template <class T>
void Ramp<T>::remove_point(int id)
{
    if (id < 0)
        return;

    m_prepared = false;
    m_cv_data.erase(std::remove_if(m_cv_data.begin(), m_cv_data.end(), [id](const CV& cv) { return id == cv.id; }));
}

template <class T>
typename Ramp<T>::CV& Ramp<T>::cv(int id)
{
    static CV invalid(0, T(), InterpType::kNone, -1);
    if (id < 0)
        return invalid;

    for (auto& cv_val : m_cv_data)
        if (cv_val.id == id)
            return cv_val;

    return invalid;
}

template <class T>
void Ramp<T>::clear()
{
    m_cache_cv = 0;
    m_prepared = false;

    m_cv_data.clear();
    m_cv_data.push_back(CV(-FLT_MAX, T(), kNone));
    m_cv_data.push_back(CV(FLT_MAX, T(), kNone));
}

template <class T>
Ramp<T>::Ramp()
    : m_cache_cv(0)
    , m_prepared(false)
{
    m_cv_data.push_back(CV(-FLT_MAX, T(), kNone));
    m_cv_data.push_back(CV(FLT_MAX, T(), kNone));
}

template class Ramp<float4>;
OPENDCC_NAMESPACE_CLOSE