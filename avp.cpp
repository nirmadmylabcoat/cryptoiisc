#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
using namespace std;

int q = 12289;
int n;
vector<int> a;

void setup(int participants, int qval) {
    n = participants;
    q = qval;
    a.resize(n); //vector is resized
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 10 + 1;  // values from 1 to 10 so it's smaller
    }
}

vector<vector<int>> generate_secret_shares() { //all should add up to 0
    vector<vector<int>> s(n, vector<int>(n));
    for (int i = 0; i < n; ++i) {
        int sum = 0;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                s[i][j] = rand() % q;
                cout << "split for participant " << i << ": " << s[i][j] << endl;
                sum = (sum + s[i][j]) % q;
            }
        }
        s[i][i] = (q - sum) % q;
        cout << "split for participant " << i << ": " << s[i][i] << endl << endl;
    }
    return s;
}

vector<int> compute_x(const vector<vector<int>>& s) {
    vector<int> x(n, 0);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            x[i] = (x[i] + s[j][i]) % q;
        }
    }
    return x;
}

vector<int> encode_vote(const vector<int>& x, const vector<int>& r, const vector<int>& e, const vector<int>& v) {
    vector<int> V(n);
    for (int i = 0; i < n; i++) {
        if (v[i] == 0) {
            int val = a[i] * x[i] + e[i];
            V[i] = ((val % q) + q) % q;
            cout << "participant " << i << ": " << V[i] << " = (" << a[i] << " * " << x[i] << " + " << e[i] << ") % " << q << endl;
        } else {
            V[i] = rand() % q;
            cout << "participant " << i << ": " << V[i] << " = random (voted 1)" << endl;
        }
    }
    return V;
}

bool decode(const vector<int>& V, const vector<int>& x, const vector<int>& v) {
    cout << "\nDecoded messages:\n";

    vector<int> decoded_values(n);
    vector<int> clean_decoded;

    for (int i = 0; i < n; ++i) {
        decoded_values[i] = ((V[i] - a[i] * x[i]) % q + q) % q;
        if (v[i] == 0) {
            clean_decoded.push_back(decoded_values[i]);
        }
    }
    //stat way to check if vote is 0 or 1
    int min_legit = *min_element(clean_decoded.begin(), clean_decoded.end());
    int max_legit = *max_element(clean_decoded.begin(), clean_decoded.end());
    int margin = 25;
    min_legit -= margin;
    max_legit += margin;

    bool flag = false;
    for (int i = 0; i < n; i++) {
        int decoded = decoded_values[i];
        cout << "Participant " << i << ": " << decoded;

        if (decoded < min_legit || decoded > max_legit) {
            cout << " (likely vote = 1)";
            flag = true;
        } else {
            cout << " (likely vote = 0)";
        }
        cout << endl;
    }

    return flag;
}

// bool decode(const vector<int>& V, const vector<int>& x, bool flag) {
//     cout << "\nDecoded messages:\n";
//     for (int i = 0; i < n; i++) {
//         int decoded = ((V[i] - a[i]*x[i]) % q + q) % q;
//         cout << "Participant " << i << ": " << decoded;
//         if (decoded <= 20) cout << " (likely vote = 0)";
//         else {cout << " (likely vote = 1)"; flag = true;}
//         cout << endl;
//     }
//     return flag;
// }

// int compute_adaptive_threshold(const vector<int>& V, const vector<int>& v, const vector<int>& x, const vector<int>& e) {
//     int max_expected = 0;
//     for (int i = 0; i < n; ++i) {
//         if (v[i] == 0) {
//             int expected = ((a[i] * x[i] + e[i]) % q + q) % q;
//             int dist = min(expected, q - expected);  // handle wrap-around
//             if (dist > max_expected) max_expected = dist;
//         }
//     }
//     return max_expected; // add margin to avoid false positives
// }

// int compute_veto(const vector<int>& V, int threshold) {
//     int max_dist = 0;
//     for (int i = 0; i < V.size(); i++) {
//         int dist = min(V[i], q - V[i]);
//         if (dist > max_dist) max_dist = dist;
//     }
//     cout << max_dist << " vs." << threshold << endl;
//     return (max_dist <= threshold) ? 0 : 1;
// }

int main() {
    srand(time(0));

    int qval = 12289;
    cout << "Enter number of participants: ";
    cin >> n;
    cout << endl;
    setup(n, qval);

    vector<int> v(n);
    cout << "Enter votes for each participant (0 = allow, 1 = veto):\n";
    for (int i = 0; i < n; i++) {
        cout << "Vote for participant " << i + 1 << ": ";
        cin >> v[i];
    }

    auto s = generate_secret_shares();
    auto x = compute_x(s);

    vector<int> r(n), e(n);
    for (int i = 0; i < n; i++) {
        r[i] = (rand() % 1000 + (q * 3 / 4)) % qval;
        e[i] = rand() % 10;
    }

    auto V = encode_vote(x, r, e, v);
    bool flags = false;
    flags = decode(V, x, v);

    // int adaptive_threshold = compute_adaptive_threshold(V, v, x, e);
    // int result = compute_veto(V, adaptive_threshold);


    cout << "\nEncoded Votes: ";
    for (int i = 0; i < n; i++) {
        cout << V[i] << " ";
    }

    cout << "\n\nVeto Result: ";
    if (flags == 1) cout << "Someone vetoed (vote = 1)";
    else cout << "All allowed (vote = 0)";
    cout << endl;

    return 0;
}
