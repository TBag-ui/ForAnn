#include <iostream>
#include <vector>

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

struct Results
{
    float sum = 0.f;
    float product = 0.f;
    std::vector<float> differences;
};

Results computations(const std::vector<float>& values)
{
    const size_t num_elements = values.size();
    Results stats;
    stats.differences.resize(num_elements-1);
    stats.sum = 0.f;
    stats.product = 0.f;

    // most computationally efficient to be done in one loop, requires less paging over memory.
    for(size_t i=0; i<num_elements; i++)
    {
        const float& iteration_value = values[i];
        if(i > 0){
            const float& last_iteration_value = values[i-1];
            stats.differences[i-1] = iteration_value-last_iteration_value;
        }
        stats.sum += iteration_value;
        stats.product *= iteration_value;
    }
    return stats;

}

int main()
{
    const std::vector<float> values = {0,1,2,3,4,5};
    std::cout << "values:" << std::endl
              << values << std::endl;

    Results comp_out = computations(values);
    std::cout << "differences:" << std::endl
              << comp_out.differences << std::endl;
    std::cout << "Product: " << comp_out.product << std::endl;
    std::cout << "Sum: " << comp_out.sum << std::endl;

    return 0;
}
