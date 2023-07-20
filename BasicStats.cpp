#include "BasicStats.h"
#include <cmath>
#include <optional>

BasicStats::BasicStats()
{

}

BasicStats::BasicStats(const std::vector<float>& data, const std::vector<float>& no_data_values)
{
    reInitialize(data, no_data_values);
}

void BasicStats::clear()
{
    m_num_elements = 0.f;
    m_sum = 0.f;
    m_product = 1.f;
    // Resize 0 removes allocated memory from vector. std::vector::clear does
    // not do this. Clear would hold the memory, but null terminate the 0th
    // element.
    m_differences.resize(0);
    m_ndvs.resize(0);
    m_is_good = true;
}

bool BasicStats::reInitialize(const std::vector<float>& data, const std::vector<float>& no_data_values)
{
    const size_t num_elements = data.size();
    clear();
    m_ndvs = no_data_values;
    m_differences.resize(num_elements-1);

    // {} to control scope of last_element
    {
        // if more previous elements were need perhaps for moving average, this
        // could become a vector or deque
        std::optional<float> last_element;

        // Generally it's most computationally efficient done in one loop.
        // Requires less paging of heap memory into cache.
        for(size_t i=0; i<num_elements; i++)
        {
            // Zero cost abstraction but very helpful for debugging because opening
            // a large vector is very slow. Could be achieved using range based for,
            // but we need to index into differences vector
            const float& iteration_value = data[i];
            if(isFloatBad(iteration_value)){
                m_is_good = false;
                return false;
            }
            if(isFloatNoDataValue(iteration_value)){
                // could choose to carry forward the ndv in differences vector or
                // handle some over way with a priori knowledge
                last_element = {};
                m_is_good = false;
                continue;
            }
            if(last_element){ // skipped if last value was ndv, or not set because i == 0
                const float& last_iteration_value = last_element.value();
                addDifference(i-1, last_iteration_value, iteration_value);
            }
            addSum(iteration_value);
            addProduct(iteration_value);
            // new functionality would be added here
            last_element = iteration_value;
        }
    }
    return m_is_good;
}

FORCE_INLINE void BasicStats::addProduct(float data_value)
{
    m_product *= data_value;
}

FORCE_INLINE void BasicStats::addSum(float data_value)
{
    m_sum += data_value;
}

FORCE_INLINE void BasicStats::addDifference(size_t index, float last_data_value, float data_value)
{
    m_differences[index] = data_value-last_data_value;
}

void BasicStats::setNonDataValues(const std::vector<float>& ndvs)
{
    m_ndvs = ndvs;
}

FORCE_INLINE bool BasicStats::isFloatNoDataValue(float data_value) const
{
    // m_ndvs is assumed to be small. If m_ndvs were large,
    // checking a hash could be more efficient
    for(float ndv : m_ndvs){
        // floating point comparison should generaly be safe in the case of ndvs,
        if(data_value == ndv){
            return true;
        }
    }
    return false;
}

FORCE_INLINE bool BasicStats::isFloatBad(float data_value) const
{
    return std::isnan(data_value) || std::isinf(data_value);
}

float BasicStats::getSum() const
{
    return m_sum;
}

float BasicStats::getProduct() const
{
    return m_product;
}

const std::vector<float>& BasicStats::getDifferences() const
{
    return m_differences;
}

bool BasicStats::isGood() const
{
    return m_is_good;
}
