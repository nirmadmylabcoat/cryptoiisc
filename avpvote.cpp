#include <boost/interprocess/shared_memory_object.hpp>  // For creating named shared memory segments
#include <boost/interprocess/mapped_region.hpp>         // To map and access shared memory
#include "shared_sync.hpp"                              // Synchronization utility (barrier-style)
#include <vector>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cmath>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;
const int q = 315521;
typedef vector<int> Poly;

// Returns a random number in [0, mod)
int get_rand(int mod = q) {
    return rand() % mod;
}

// Generates a small centered noise in the range [-3, 3]
int small_noise() {
    return (rand() % 7) - 3;
}

// Generates a random polynomial with either small or full-range coefficients
Poly rand_poly(bool large = false) {
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        p[i] = large ? get_rand() : small_noise();
    return p;
}

// Generates a high-noise polynomial for vote = 1 encoding
Poly rand_high_noise_poly() {
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i) {
        int sign = (rand() % 2 == 0) ? 1 : -1;
        int mag = (q / 3) + rand() % (q / 4);
        p[i] = (sign * mag + q) % q;
    }
    return p;
}

// Computes the max zeta bound for noise based on q and n (used to keep errors within threshold)
double compute_max_zeta(int q, int n) {
    double A = n * n;
    double B = n;
    double C = -q / 8.0;
    double disc = B * B - 4 * A * C;

    if (disc < 0) return 0.0;

    double root1 = (-B + sqrt(disc)) / (2 * A);
    double root2 = (-B - sqrt(disc)) / (2 * A);
    return std::max(root1, root2);
}

Poly rand_error_poly(int q, int n) {
    double zeta = compute_max_zeta(q, n);
    int bound = static_cast<int>(floor(zeta));
    // Enforce a minimum error range for unpredictability
    if (bound <= 2) {
        cerr << "⚠️  zeta too small (" << bound << "). Error polynomial would be too predictable.\n";
        cerr << "❌  Consider increasing q or reducing n.\n";
        exit(1);
    }
    Poly p(poly_degree);
    for (int i = 0; i < poly_degree; ++i) {
        int e = (rand() % (2 * bound + 1)) - bound;
        p[i] = e;  // signed noise in [-bound, +bound]
    }
    return p;
}

Poly poly_add(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] + b[i]) % q;
    return res;
}

Poly poly_sub(const Poly &a, const Poly &b) {
    Poly res(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        res[i] = (a[i] - b[i] + q) % q;
    return res;
}

Poly poly_mul(const Poly &a, const Poly &b) {
    vector<int> res(2 * poly_degree - 1, 0);
    for (int i = 0; i < poly_degree; ++i)
        for (int j = 0; j < poly_degree; ++j)
            res[i + j] = (res[i + j] + a[i] * b[j]) % q;

    for (int i = poly_degree; i < 2 * poly_degree - 1; ++i)
        res[i - poly_degree] = (res[i - poly_degree] - res[i] + q) % q;

    Poly final(poly_degree);
    for (int i = 0; i < poly_degree; ++i)
        final[i] = res[i];
    return final;
}

// Writes a polynomial to shared memory named "Share_from_to"
void write_share(int from, int to, const Poly &value) {
    string name = "Share_" + to_string(from) + "_" + to_string(to);
    shared_memory_object::remove(name.c_str());
    shared_memory_object shm(create_only, name.c_str(), read_write);
    shm.truncate(sizeof(int) * poly_degree);
    mapped_region region(shm, read_write);
    memcpy(region.get_address(), value.data(), sizeof(int) * poly_degree);
}

// Reads a polynomial share from shared memory
Poly read_share(int from, int to) {
    string name = "Share_" + to_string(from) + "_" + to_string(to);
    shared_memory_object shm(open_only, name.c_str(), read_only);
    mapped_region region(shm, read_only);
    Poly val(poly_degree);
    memcpy(val.data(), region.get_address(), sizeof(int) * poly_degree);
    return val;
}

int main(int argc, char *argv[]) {
    if (argc != 4) { // Usage: avpvote <id> <vote:0|1> <n>
        return 1;
    }
    int id = atoi(argv[1]);
    int vote = atoi(argv[2]);
    int n = atoi(argv[3]);
    srand(time(0) + id);  // Ensure different seed for each party

    // 1. Generate n polynomial additive shares such that all shares sum to zero
    vector<Poly> shares(n, Poly(poly_degree, 0));
    Poly sum(poly_degree, 0);
    for (int i = 0; i < n; ++i) {
        if (i == id) continue;
        shares[i] = rand_poly();
        sum = poly_add(sum, shares[i]);
    }
    // Set own share to balance the sum to 0
    for (int i = 0; i < poly_degree; ++i)
        shares[id][i] = (-sum[i] + q) % q;

    // 2. Send shares to shared memory
    for (int i = 0; i < n; ++i)
        write_share(id, i, shares[i]);
    cout << "Party " << id << " finished writing shares. Waiting for others...\n";
    SyncBlock* sync = get_sync_block();
    sync->arrive_and_wait(n); // Wait for all parties to write

    // 3. Read shares sent to me and compute x_i (a polynomial now)
    Poly x_i(poly_degree, 0);
    for (int j = 0; j < n; ++j)
        x_i = poly_add(x_i, read_share(j, id));

    // 4. Read public polynomial 'a'
    Poly a(poly_degree);
    shared_memory_object shm_a(open_only, "shared_poly_a", read_only);
    mapped_region reg_a(shm_a, read_only);
    memcpy(a.data(), reg_a.get_address(), sizeof(int) * poly_degree);

    // 5. Encode vote
    Poly e = rand_error_poly(q, n); // auto-scaled safe noise based on zeta
    Poly V;

    if (vote == 0)
        V = poly_add(poly_mul(a, x_i), e); //structured noise
    else
        V = rand_high_noise_poly(); //random noise

    // 6. Write encoded vote V_i to shared memory
    string vote_name = "PartyVote_" + to_string(id);
    shared_memory_object::remove(vote_name.c_str());
    shared_memory_object shm(create_only, vote_name.c_str(), read_write);
    shm.truncate(sizeof(int) * poly_degree);
    mapped_region region(shm, read_write);
    memcpy(region.get_address(), V.data(), sizeof(int) * poly_degree);

    return 0;
}
