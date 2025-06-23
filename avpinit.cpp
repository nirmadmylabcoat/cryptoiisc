#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;       // n = 128
const int q = 315521;            // q = 2^21
typedef vector<int> Poly;

int get_rand(int mod = q) {
    return rand() % mod;
}

int small_noise() {
    return (rand() % 7) - 3; // [-3, ..., 3]
}

int main() {
    srand(time(0));
    Poly a(poly_degree), x(poly_degree);

    for (int i = 0; i < poly_degree; ++i) {
        a[i] = small_noise();
        x[i] = get_rand();
    }

    shared_memory_object::remove("shared_poly_a");
    shared_memory_object::remove("shared_poly_x");

    shared_memory_object shm_a(create_only, "shared_poly_a", read_write);
    shm_a.truncate(sizeof(int) * poly_degree);
    mapped_region reg_a(shm_a, read_write);
    memcpy(reg_a.get_address(), a.data(), sizeof(int) * poly_degree);

    shared_memory_object shm_x(create_only, "shared_poly_x", read_write);
    shm_x.truncate(sizeof(int) * poly_degree);
    mapped_region reg_x(shm_x, read_write);
    memcpy(reg_x.get_address(), x.data(), sizeof(int) * poly_degree);

    //cout << "Shared polynomials a and x initialized.\n";
    return 0;
}
