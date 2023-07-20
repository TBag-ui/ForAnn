#ifndef BASICSTATS_H
#define BASICSTATS_H
#include <vector>
#include <ForceInline.h>

class BasicStats
{
public:
    BasicStats();
    BasicStats(const std::vector<float>& data, const std::vector<float>& no_data_values = {});
    bool reInitialize(const std::vector<float>& data, const std::vector<float>& no_data_values = {});
    void clear();
    void setNonDataValues(const std::vector<float>& ndvs);
    bool isGood() const;
    float getSum() const;
    float getProduct() const;
    const std::vector<float>& getDifferences() const;
private:

    FORCE_INLINE void addProduct(float data_value);
    FORCE_INLINE void addSum(float data_value);
    FORCE_INLINE void addDifference(size_t index, float last_data_value, float data_value);
    FORCE_INLINE bool isFloatBad(float data_value) const;
    FORCE_INLINE bool isFloatNoDataValue(float data_value) const;
    size_t m_num_elements = 0;
    float m_sum = 0.f;
    float m_product = 1.f;
    std::vector<float> m_differences;
    std::vector<float> m_ndvs;
    bool m_is_good = true;
};

#endif // BASICSTATS_H
