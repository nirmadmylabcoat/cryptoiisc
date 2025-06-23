#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;
using namespace boost::interprocess;

const int poly_degree = 64;
//const int q = 257;
const int q = 315521;

int mod_q_centered(int x) {
    x %= q;
    if (x > q / 2) x -= q;
    if (x < -q / 2) x += q;
    return x;
}

string decode_vote(const vector<int>& total) {
    int inf_norm = 0;
    for (int coeff : total)
        inf_norm = max(inf_norm, abs(mod_q_centered(coeff)));

    if (inf_norm <= q / 4)
    {
        cout << inf_norm << " vs. " << q/4 << endl;
        return "ALL VOTED YES (0)";
    }
    else
    {
        cout << inf_norm << " vs. " << q/4 << endl;
        return "SOMEONE VETOED (1)";
    }
}

int main() {
    int n;
    cout << "Enter number of parties: ";
    cin >> n;

    vector<int> total(poly_degree, 0);

    for (int i = 0; i < n; ++i) {
        string shm_name = "PartyVote_" + to_string(i);
        try {
            shared_memory_object shm(open_only, shm_name.c_str(), read_only);
            mapped_region region(shm, read_only);
            int* data = static_cast<int*>(region.get_address());

            for (int j = 0; j < poly_degree; ++j)
                total[j] = (total[j] + data[j]) % q;
        } catch (...) {
            cout << "Error: Could not read party " << i << "'s vote.\n";
            return 1;
        }
    }

    cout << "\nDecoded (centered mod-q) Polynomial Sum:\n";
    for (int j = 0; j < poly_degree; ++j)
        cout << mod_q_centered(total[j]) << " ";
    cout << endl;

    cout << "\nResult: " << decode_vote(total) << endl;
    return 0;
}
