#include <boost/interprocess/shared_memory_object.hpp> //Provides shared memory primitives for inter-process communication.
#include <boost/interprocess/mapped_region.hpp> //Allows processes to map and access shared memory regions.
#include <vector> //For representing polynomials as vectors of integers.
#include <cstdlib> //Used for random number generation with rand() and srand().
#include <ctime> 
#include <cstring> //For memcpy() to copy data into shared memory.
#include <iostream> //Used for optional debug output.
using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64; //Degree of the polynomial
const int q = 315521; //large prime modulus used for coefficient arithmetic. Must satisfy cryptographic constraints
typedef vector<int> Poly; //Defines a Poly as a vector of integers

//Returns a random integer in the range [0, mod).
int get_rand(int mod = q) {
    return rand() % mod;
}

//Small, centered noise in the range [-3, 3].
int small_noise() {
    return (rand() % 7) - 3;
}

int main() {
    srand(time(0)); //Seeds the random number generator based on the current time.
    Poly a(poly_degree), x(poly_degree);
    for (int i = 0; i < poly_degree; ++i) {
        a[i] = small_noise(); //small, noise-like public polynomial.
        x[i] = get_rand(); //secret random polynomial.
    }
    //Removes any existing shared memory segments with the same names
    shared_memory_object::remove("shared_poly_a");
    shared_memory_object::remove("shared_poly_x");
    //Creates and maps shared memory for polynomial a
    shared_memory_object shm_a(create_only, "shared_poly_a", read_write);
    shm_a.truncate(sizeof(int) * poly_degree);
    mapped_region reg_a(shm_a, read_write);
    memcpy(reg_a.get_address(), a.data(), sizeof(int) * poly_degree);
    //Creates and maps shared memory for polynomial x
    shared_memory_object shm_x(create_only, "shared_poly_x", read_write);
    shm_x.truncate(sizeof(int) * poly_degree);
    mapped_region reg_x(shm_x, read_write);
    memcpy(reg_x.get_address(), x.data(), sizeof(int) * poly_degree);
    //cout << "Shared polynomials a and x initialized.\n";
    return 0;
}
