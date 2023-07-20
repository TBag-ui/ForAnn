#include <BasicStats.h>
#include <numeric>
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
    BasicStats stats(values);

    EXPECT_TRUE(checkProductFunction(stats.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(stats.getDifferences(), values));
}

// Legacy systems sometimes use an assigned float as a no data value,
// for example 0x7f7fffff (maximum value of the float)
TEST(BasicStats, NDVSeries)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    BasicStats stats;
    std::vector<float> ndvs = {0};
    bool initialized_correctly = stats.reInitialize(values, ndvs);

    // product will not be the same anymore because stl will multiply by 0
    EXPECT_FALSE(checkProductFunction(stats.getProduct(), values));
    // should be the same as stl if we tell stl to ignore first value
    EXPECT_TRUE(stats.getProduct() ==  std::accumulate(values.begin()+1, values.end(),
                                                       1.f, std::multiplies<float>()));

    // sums should remain the same
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));

    // should no longer be "good"
    EXPECT_FALSE(stats.isGood());
    EXPECT_FALSE(initialized_correctly);
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

    BasicStats stats_inf(values_contains_inf);
    EXPECT_FALSE(stats_inf.isGood());

    BasicStats stats_nan(values_contains_inf);
    EXPECT_FALSE(stats_nan.isGood());
}

/* Reinitializing objects can be a bit of a code smell, it's safer to instantiate
 * a new object for simple classes like this. However, I've run into bugs deriving
 * from this scenario several times when a well meaning colleague has extended
 * functionality or added their own reinitialize to an object so I'm adding it
 * myself with a test to reduce the chance of mistakes
 */
TEST(BasicStats, Reinitialize)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    BasicStats stats(values);

    EXPECT_TRUE(checkProductFunction(stats.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(stats.getDifferences(), values));

    stats.reInitialize(values);
    EXPECT_TRUE(checkProductFunction(stats.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(stats.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(stats.getDifferences(), values));
}

TEST(BasicStats, Copy)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    BasicStats copy;
    {
        BasicStats stats(values);
        copy = stats;
    }
    EXPECT_TRUE(checkProductFunction(copy.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(copy.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(copy.getDifferences(), values));
}

TEST(BasicStats, CopyConstructor)
{
    const std::vector<float> values = {0,1,2,3,4,5};
    BasicStats stats(values);
    BasicStats copy(stats);
    EXPECT_TRUE(checkProductFunction(copy.getProduct(), values));
    EXPECT_TRUE(checkSumFunction(copy.getSum(), values));
    EXPECT_TRUE(checkDifferencesFunction(copy.getDifferences(), values));
}
