#ifndef BASICSTATS_H
#define BASICSTATS_H
#include <vector>
#include <ForceInline.h>
#include <cmath>
#include <array>
#include <tuple>
#include <optional>

// iterates through data vector efficiently and applies lambdas
template<typename... Args>
class BasicStatsLoop
{
    std::tuple<Args...> lambdas;
    const static size_t tuple_size = std::tuple_size_v<std::tuple<Args...> >;
    std::array<float, tuple_size> results;
    std::vector<float> m_ndvs;
    bool m_contains_nan_infs = false;
    bool m_contains_ndvs = false;
    FORCE_INLINE bool isFloatBad(float data_value) const;
    FORCE_INLINE bool isFloatNoDataValue(float data_value) const;
public:
    BasicStatsLoop(const std::vector<float>& data, const std::vector<float>& no_data_values, const std::array<float, tuple_size>& starting_values, Args... args);
    void setNonDataValues(const std::vector<float>& ndvs);
    bool isGood() const;
    template<int i> float getResult() const{
        return std::get<i>(results);
    }
};

// assembles a BasicStatsLoop using lambdas for sum, product, differences
template<int i = 0>
class DoesTheStats
{
private:
    std::vector<float> diffs_array;
    float sum;
    float product;
    bool good;
public:
    DoesTheStats(const std::vector<float>& numbers, const std::vector<float>& ndvs)
    {
        // added methods would go here as lambda expressions following the same outline
        diffs_array.resize(numbers.size()-1);
        constexpr auto plus = [](std::optional<size_t> index, float value, float& total)->void{
            total += value;
        };
        constexpr auto multiply = [](std::optional<size_t> index, float value, float& total)->void{
            total *= value;
        };
        auto diff = [&](std::optional<size_t> index, float value, float& total)->void{
            if(index){
                size_t index_v = index.value();
                if(index_v > 0){
                    diffs_array[index_v-1] = value - numbers[index_v-1];
                }
            }
        };
        auto the_action = BasicStatsLoop(numbers, ndvs, {0.f, 1.f, 0.f}, plus, multiply, diff);
        sum = the_action.getResult<0>();
        product = the_action.getResult<1>();
        good = the_action.isGood();
    }
    float getSum() const{
        return sum;
    }
    float getProduct() const{
        return product;
    }
    const std::vector<float>& getDifferences() const{
        return diffs_array;
    }
    bool isGood() const{
        return good;
    }
};

template <unsigned N, typename Tup> struct faux_unroll_tuple_fns {
    static void call(size_t i, float iteration_value, float* totals, const Tup & tup) {
        std::get<N-1>(tup)(i, iteration_value, totals[N-1]);
        faux_unroll_tuple_fns<N-1, Tup>::call(i, iteration_value, totals, tup);
    }
};

template <typename Tup> struct faux_unroll_tuple_fns<0u, Tup> {
    static void call(size_t i, float iteration_value, float* totals, const Tup&) {}
};


template <unsigned N, typename Tup> struct faux_unroll_tuple_fns_critical_section {
    static void call(float* iteration_values, float* totals, const Tup & tup) {
        std::get<N-1>(tup)({}, iteration_values[N-1], totals[N-1]);
        faux_unroll_tuple_fns_critical_section<N-1, Tup>::call(iteration_values, totals, tup);
    }
};

template <typename Tup> struct faux_unroll_tuple_fns_critical_section<0u, Tup> {
    static void call(float* iteration_values, float* totals, const Tup&) {}
};

template <typename... Args>
FORCE_INLINE bool BasicStatsLoop<Args...>::isFloatBad(float data_value) const
{
    return std::isnan(data_value) || std::isinf(data_value);
}

template <typename... Args>
FORCE_INLINE bool BasicStatsLoop<Args...>::isFloatNoDataValue(float data_value) const
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

template <typename... Args>
bool BasicStatsLoop<Args...>::isGood() const
{
    return !m_contains_ndvs && !m_contains_nan_infs;
}

template <typename... Args>
BasicStatsLoop<Args...>::BasicStatsLoop(const std::vector<float>& data, const std::vector<float>& no_data_values,
                                        const std::array<float, tuple_size>& starting_values, Args... args) : lambdas(args...)
{
    const size_t num_elements = data.size();
    m_ndvs = no_data_values;
    results = starting_values;

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
            faux_unroll_tuple_fns<tuple_size, std::tuple<Args...> >::call(i, data[i], &results[0], lambdas);
        }
    }

    const int64_t num_elements_64_t = static_cast<int64_t>(num_elements);
    const int64_t num_prev_values_64_t = static_cast<int64_t>(num_prev_values);
    // Generally it's most computationally efficient done in one loop.
    // Requires less paging of heap memory into cache.
    #pragma omp parallel
    {
        std::array<float, tuple_size> thread_local_totals = starting_values;
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
            faux_unroll_tuple_fns<tuple_size, std::tuple<Args...> >::call(i, iteration_value, &thread_local_totals[0], lambdas);
            // new functionality would be added here
        }
        #pragma omp critical
        {
            faux_unroll_tuple_fns_critical_section<tuple_size, std::tuple<Args...> >::call(&thread_local_totals[0], &results[0], lambdas);
        }
    }
}

#endif // BASICSTATS_H
