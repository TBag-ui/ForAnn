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
    m_contains_ndvs = false;
    m_contains_nan_infs = false;
}

bool BasicStats::reInitialize(const std::vector<float>& data, const std::vector<float>& no_data_values)
{
    const size_t num_elements = data.size();
    clear();
    m_ndvs = no_data_values;
    m_differences.resize(num_elements-1);

    const size_t num_prev_values = 1;

    // do first values
    {
        for(size_t i=0; i<num_prev_values; i++)
        {
            if(isFloatBad(data[i])){
                m_contains_nan_infs = true;
                continue;
            }
            if(isFloatNoDataValue(data[i])){
                m_contains_ndvs = true;
                continue;
            }
            m_sum = addSum(data[i], m_sum);
            m_product = addProduct(data[i], m_product);
        }
    }

    const int64_t num_elements_64_t = static_cast<int64_t>(num_elements);
    const int64_t num_prev_values_64_t = static_cast<int64_t>(num_prev_values);
        // Generally it's most computationally efficient done in one loop.
        // Requires less paging of heap memory into cache.
    #pragma omp parallel
    {
        float thread_local_sum = 0;
        float thread_local_product = m_product;
        #pragma omp for
        for(int64_t i=num_prev_values_64_t; i<num_elements_64_t; i++)
        {
            // Zero cost abstraction but very helpful for debugging because opening
            // a large vector is very slow. Could be achieved using range based for,
            // but we need to index into differences vector
            const float& iteration_value = data[i];
            if(isFloatBad(iteration_value)){
                m_contains_nan_infs = true;
                continue;
            }
            if(isFloatNoDataValue(iteration_value)){
                // could choose to carry forward the ndv in differences vector or
                // handle some over way with a priori knowledge
                m_contains_ndvs = true;
                continue;
            }
            const float& last_iteration_value = data[i-1];
            addDifference(i-1, last_iteration_value, iteration_value, m_differences);
            thread_local_sum = addSum(iteration_value, thread_local_sum);
            thread_local_product = addProduct(iteration_value, thread_local_product);
            // new functionality would be added here
        }
        #pragma omp critical
        {
            m_sum += thread_local_sum;
            m_product *= thread_local_product;
        }
    }
    return isGood();
}

FORCE_INLINE float BasicStats::addProduct(float data_value, float product_carryforward) const
{
    product_carryforward *= data_value;
    return product_carryforward;
}

FORCE_INLINE float BasicStats::addSum(float data_value, float sum_carryforward) const
{
    sum_carryforward += data_value;
    return sum_carryforward;
}

FORCE_INLINE void BasicStats::addDifference(size_t index, float last_data_value, float data_value, std::vector<float>& vector_carryforward) const
{
    vector_carryforward[index] = data_value-last_data_value;
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
    return !m_contains_ndvs && !m_contains_nan_infs;
}
