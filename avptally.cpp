#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>
using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;
const int q = 315521;

// Centers a value in range [−q/2, q/2] for correct interpretation of negative votes
int mod_q_centered(int x) {
    x %= q;
    if (x > q / 2) x -= q;
    if (x < -q / 2) x += q;
    return x;
}

// Computes the infinity norm (max absolute coefficient) and returns the vote result
string decode_vote(const vector<int>& total) {
    int inf_norm = 0;
    for (int coeff : total)
        inf_norm = max(inf_norm, abs(mod_q_centered(coeff)));

    // If infinity norm stays below q/4, no one vetoed
    if (inf_norm <= q / 4) {
        cout << inf_norm << " vs. " << q/4 << endl;
        return "ALL VOTED YES (0)";
    } else {
        cout << inf_norm << " vs. " << q/4 << endl;
        return "SOMEONE VETOED (1)";
    }
}

int main() {
    int n;
    cout << "Enter number of parties: ";
    cin >> n;

    vector<int> total(poly_degree, 0);  // Sum of all polynomials from all parties

    for (int i = 0; i < n; ++i) {
        string shm_name = "PartyVote_" + to_string(i);  // Each party's encoded vote
        try {
            shared_memory_object shm(open_only, shm_name.c_str(), read_only);
            mapped_region region(shm, read_only);
            int* data = static_cast<int*>(region.get_address());

            // Add coefficients to running total
            for (int j = 0; j < poly_degree; ++j)
                total[j] = (total[j] + data[j]) % q;
        } catch (...) {
            cout << "Error: Could not read party " << i << "'s vote.\n";
            return 1;
        }
    }

    // Print the centered decoded polynomial sum (optional)
    cout << "\nDecoded (centered mod-q) Polynomial Sum:\n";
    for (int j = 0; j < poly_degree; ++j)
        cout << mod_q_centered(total[j]) << " ";
    cout << endl;

    // Show final result (YES if all voted 0, VETO if someone voted 1)
    cout << "\nResult: " << decode_vote(total) << endl;
    return 0;
}