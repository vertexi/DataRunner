#include <math.h>
#include <limits>
#include <stdio.h>

int MetricFormatter_orig(double value, char *buff, int size, void *data)
{
    const char *unit = (const char *)data;
    static double v[] = {1000000000, 1000000, 1000, 1, 0.001, 0.000001, 0.000000001};
    static const char *p[] = {"G", "M", "k", "", "m", "u", "n"};

    for (int i = 0; i < 7; ++i)
    {
        if (fabs(value) >= v[i])
        {
            return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
        }
    }
    if (fabs(value) < std::numeric_limits<double>::epsilon()) // value == 0
    {
        return snprintf(buff, size, "0 %s", unit);
    }
    return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
}
