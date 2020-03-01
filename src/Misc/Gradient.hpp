#pragma once

#include <vector>
#include <math.h>

namespace Gradient
{
    /**
     * @brief Gradient stop
     * 
     * @tparam T Colour data type
     */
    template<class T>
    class GradientStop
    {
        private:
            float t;
            T value;
            template <class Ta>
            friend class Gradient;

        public:
            GradientStop() {}
            GradientStop(float _t, T _val) :t(_t), value(_val) {}
    };

    /**
     * @brief Linear interpolation
     * 
     * @tparam T Colour type
     * @param s Start
     * @param e End
     * @param t 0 -> 1 value to interpolate by
     * @return Interpolated colour 
     */
    template<class T>
    T lerp(T s, T e, float t)
    {
        return s + (e - s) * t;
    }

    /**
     * @brief Gradient colour
     * 
     */
    class GradientColor
    {
        public:
            float r, g, b, a;

            GradientColor(float, float, float, float);
            GradientColor();

            const GradientColor & operator+=(const GradientColor &);
            const GradientColor & operator-=(const GradientColor &);
            const GradientColor & operator*=(const float &);
            const GradientColor operator+(const GradientColor&) const;
            const GradientColor operator-(const GradientColor&) const;
            const GradientColor operator*(const float&) const;
    };

    /**
     * @brief Gradient class
     * 
     * @tparam T Colour type
     */
    template<class T>
    class Gradient
    {
        std::vector<GradientStop<T>> stops;

        public:
            /**
             * @brief Add a colour stop
             * 
             * @param t 
             * @param val 
             */
            void AddColorStop(float t, T val)
            {
                typename std::vector<GradientStop<T>>::iterator it;

                for (it = stops.begin(); it != stops.end(); it++) {
                    if ((*it).t > t)break;
                }

                stops.insert(it, GradientStop<T>(t, val));
            }

            /**
             * @brief Get the colour at a specfic point
             * 
             * @param t 
             * @return T 
             */
            T GetColorAt(float t)
            {
                typename std::vector<GradientStop<T>>::iterator it;
                GradientStop<T> start, stop;
                for (it = stops.begin(); it != stops.end(); it++) {
                    stop = *it;
                    if (stop.t > t)
                        break;
                }

                if (it == stops.begin() || it == stops.end()) return stop.value;
                start = *(--it);
                float frac = (t - start.t) / (stop.t - start.t);
                return lerp(start.value, stop.value, frac);
            }
    };
}