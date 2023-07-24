#include <BasicStats.h>
#include <numeric>
#include <execution>
#include <gtest/gtest.h>

// convenient for printing vectors
template < class T >
std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
{
    const size_t size = v.size();
    for (size_t i=0; i<size; i++)
    {
        os << v[i] << "\t";
    }
    return os;
}

bool checkSumFunction(float sum, const std::vector<float>& values)
{
    float sum_stl = std::accumulate(values.begin(), values.end(), 0.f, std::plus<float>());
    return sum == sum_stl;
}

bool checkProductFunction(float product, const std::vector<float>& values)
{
    float product_stl = std::accumulate(values.begin(), values.end(), 1.f, std::multiplies<float>());
    return product == product_stl;
}

bool checkDifferencesFunction(const std::vector<float>& differences, const std::vector<float>& values)
{
    if(values.empty()){
        return false;
    }
    std::vector<float> differences_stl(values.size());
    std::adjacent_difference(values.begin(), values.end(), differences_stl.begin());

    // zeroth element in stl version is 0. Remove it.
    differences_stl.erase(differences_stl.begin());

    size_t num_differences = differences.size();
    if(num_differences != differences_stl.size()){
        return false;
    }

    for(size_t i=0; i<num_differences; i++){
        if(differences[i] != differences_stl[i]){
            return false;
        }
    }
    return true;
}

TEST(BasicStats, NaiveSeries)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    DoesTheStats stats(values, {});

    EXPECT_TRUE(checkProductFunction(stats.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(stats.getDifferences(), values));
}

// Legacy systems sometimes use an assigned float as a no data value,
// for example 0x7f7fffff (maximum value of the float)
TEST(BasicStats, NDVSeries)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    std::vector<float> ndvs = {0};
    DoesTheStats stats(values, ndvs);

    // product will not be the same anymore because stl will multiply by 0
    EXPECT_FALSE(checkProductFunction(stats.getProduct(), values));
    // should be the same as stl if we tell stl to ignore first value
    EXPECT_TRUE(stats.getProduct() ==  std::accumulate(values.begin()+1, values.end(),
                                                       1.f, std::multiplies<float>()));

    // sums should remain the same
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));

    // should no longer be "good"
    EXPECT_FALSE(stats.isGood());
}

// It's always good to check inf and nan
TEST(BasicStats, ContainsInfNan)
{
    float zero = 0.f;
    const std::vector<float> values_contains_inf = {1.f/zero,1,2,3,4,5};
    EXPECT_TRUE(std::isinf(values_contains_inf[0]));
    float negative = -1.f;
    std::vector<float> values_contains_nan = {std::sqrt(negative),1,2,3,4,5};
    EXPECT_TRUE(std::isnan(values_contains_nan[0]));

    DoesTheStats stats_inf(values_contains_inf, {});
    EXPECT_FALSE(stats_inf.isGood());

    DoesTheStats stats_nan(values_contains_inf, {});
    EXPECT_FALSE(stats_nan.isGood());
}

TEST(BasicStats, Performance)
{
    const size_t big_number = 100000;
    std::vector<float> values(big_number, 1.f); //filled with 1s
    std::chrono::steady_clock::time_point begin_basic_stats = std::chrono::steady_clock::now();
    DoesTheStats stats(values,{});
    std::chrono::steady_clock::time_point end_basic_stats = std::chrono::steady_clock::now();

    // same computations done with parallel algorithms
    std::chrono::steady_clock::time_point begin_stl = std::chrono::steady_clock::now();
    float sum_stl = std::reduce( std::execution::par, values.begin(), values.end(), 0.f, std::plus<float>());
    float product_stl = std::reduce( std::execution::par, values.begin(), values.end(), 1.f, std::multiplies<float>());
    std::vector<float> differences_stl(values.size());
    std::adjacent_difference(std::execution::par, values.begin(), values.end(), differences_stl.begin());
    std::chrono::steady_clock::time_point end_stl = std::chrono::steady_clock::now();
    EXPECT_TRUE(sum_stl == stats.getSum());
    EXPECT_TRUE(product_stl == stats.getProduct());

    auto elapsed_stl = end_stl-begin_stl;
    auto elapsed_basic_stats = end_basic_stats-begin_basic_stats;
    EXPECT_TRUE(elapsed_basic_stats < elapsed_stl);
    const std::vector<float> differences = stats.getDifferences();
    // 0th element in differences_stl is 0
    EXPECT_TRUE(std::equal(differences.begin(), differences.end(), differences_stl.begin()+1, differences_stl.end()));
    std::cout << "Run time factor time/stl time: " << static_cast<double>(elapsed_basic_stats.count())
                 / static_cast<double>(elapsed_stl.count()) << std::endl;

}
