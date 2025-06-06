#include <iostream>
#include <vector>
#include <random>
#include <cassert>

using namespace std;

static int n=0;        // degree of polynomials (x^n + 1)
static int q=0;        // modulus for coefficients 

typedef vector<int> Poly;

void print_poly(const Poly& p, const string& name) {
    cout << name << "= ";
    for (int i = 0; i < n; i++) {
        cout << p[i];
        if (i > 0) cout << "*x^" << i;
        if (i < n - 1) cout << " + ";
    }
    cout << " mod " << q << endl;
}

int modq(int x) {
    x %= q;
    return (x < 0) ? x + q : x;
}

Poly poly_add(const Poly& a, const Poly& b) {
    Poly result(n);
    for (int i = 0; i < n; i++) {
        result[i] = modq(a[i] + b[i]);
    }
    return result;
}

Poly poly_mul(const Poly& a, const Poly& b) {
    Poly result(2 * n - 1, 0);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            result[i + j] += a[i] * b[j];

    Poly reduced(n, 0);
    for (int i = 0; i < n; i++) {
        reduced[i] = result[i];
    }
    for (int i = n; i < 2 * n - 1; i++) {
        reduced[i - n] = modq(reduced[i - n] - result[i]);  // x^n = -1
    }

    for (int i = 0; i < n; i++)
        reduced[i] = modq(reduced[i]);

    return reduced;
}

Poly sample_small_poly(int bound = 1) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(-bound, bound);
    Poly p(n);
    for (int i = 0; i < n; i++) {
        p[i] = modq(dis(gen));
    }
    return p;
}

// Message sampling with 0-padding
Poly sample_message_bits(int bitlen) {
    assert(bitlen < n);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, 1);

    Poly msg(n, 0);
    for (int i = 0; i < bitlen; i++) {
        msg[i] = dis(gen);
    }
    return msg;
}

// Add this function
Poly string_to_poly(const string& bitstr) {
    assert(bitstr.length() < n && "Bitstring too long for current n");
    Poly p(n, 0);
    for (size_t i = 0; i < bitstr.length(); i++) {
        assert(bitstr[i] == '0' || bitstr[i] == '1');
        p[i] = bitstr[i] - '0';
    }
    return p;
}

pair<Poly, Poly> KeyGen() {
    Poly a = sample_small_poly(q - 1);
    Poly s = sample_small_poly(1);
    Poly e0 = sample_small_poly(1);
    Poly b = poly_add(poly_mul(a, s), e0);
    return {a, b};
}

void setup(int nval, int qval) {
    n = nval;
    q = qval;
}

Poly Commit(const Poly& a, const Poly& b, const Poly& m, Poly& r_out, Poly& e_out) {
    Poly r = sample_small_poly(1);
    Poly e = sample_small_poly(1);

    r_out = r;
    e_out = e;

    Poly c = poly_add(poly_add(poly_mul(a, m), poly_mul(b, r)), e);
    return c;
}

bool Open(const Poly& c, const Poly& a, const Poly& b, const Poly& m, const Poly& r, const Poly& e) {
    Poly c_check = poly_add(poly_add(poly_mul(a, m), poly_mul(b, r)), e);
    return c_check == c;
}

int main() {
    int nval, qval;
    cout << "Enter n and q: ";
    cin >> nval >> qval;

    setup(nval, qval);

    auto [a, b] = KeyGen();

    string bitstring;
    cout << "enter your message" << endl;
    cin >> bitstring; // your custom message (must be < n bits)
    Poly message = string_to_poly(bitstring);


    //Poly message = sample_message_bits(bitlen);
    Poly r, e;

    print_poly(a, "a(x)\n");
    print_poly(b, "b(x)\n");
    print_poly(message, "message(x)\n");

    Poly c = Commit(a, b, message, r, e);
    print_poly(c, "commitment c(x)\n");

    if (Open(c, a, b, message, r, e)) {
        cout << "Commitment successfully opened and verified!" << endl;
    } else {
        cout << "Commitment verification failed!" << endl;
    }

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <random>
// #include <cassert>

// using namespace std;

// const int n = 4;        // degree of polynomials (x^n + 1)
// const int q = 7;        // modulus for coefficients

// // Polynomial type
// typedef vector<int> Poly;

// // Helper to print a polynomial
// void print_poly(const Poly& p, const string& name) {
//     cout << name << "(x) = ";
//     for (int i = 0; i < n; i++) {
//         cout << p[i];
//         if (i > 0) cout << "*x^" << i;
//         if (i < n - 1) cout << " + ";
//     }
//     cout << " mod " << q << endl;
// }

// // Modular reduction helper
// int modq(int x) {
//     x %= q;
//     return (x < 0) ? x + q : x;
// }

// // Polynomial addition
// Poly poly_add(const Poly& a, const Poly& b) {
//     Poly result(n);
//     for (int i = 0; i < n; i++) {
//         result[i] = modq(a[i] + b[i]);
//     }
//     return result;
// }

// // Polynomial multiplication modulo (x^n + 1, q)
// Poly poly_mul(const Poly& a, const Poly& b) {
//     Poly result(2 * n - 1, 0);

//     // Ordinary multiplication
//     for (int i = 0; i < n; i++)
//         for (int j = 0; j < n; j++)
//             result[i + j] += a[i] * b[j];

//     // Reduction modulo x^n + 1
//     Poly reduced(n, 0);
//     for (int i = 0; i < n; i++) {
//         reduced[i] = result[i];
//     }
//     for (int i = n; i < 2 * n - 1; i++) {
//         reduced[i - n] = modq(reduced[i - n] - result[i]);  // x^n = -1
//     }

//     for (int i = 0; i < n; i++)
//         reduced[i] = modq(reduced[i]);

//     return reduced;
// }

// // Generate a small random polynomial
// Poly sample_small_poly(int bound = 1) {
//     random_device rd;
//     mt19937 gen(rd());
//     uniform_int_distribution<> dis(-bound, bound);
//     Poly p(n);
//     for (int i = 0; i < n; i++) {
//         p[i] = modq(dis(gen));
//     }
//     return p;
// }

// // RLWE Key Generation: returns (a, b), discards s, e0
// pair<Poly, Poly> KeyGen() {
//     Poly a = sample_small_poly(q - 1);   // a is uniform mod q
//     Poly s = sample_small_poly(1);       // secret
//     Poly e0 = sample_small_poly(1);      // error
//     Poly b = poly_add(poly_mul(a, s), e0);
//     return {a, b};
// }

// // Commitment: returns c = a⋅m + b⋅r + e
// Poly Commit(const Poly& a, const Poly& b, const Poly& m, Poly& r_out, Poly& e_out) {
//     Poly r = sample_small_poly(1);       // fresh randomness
//     Poly e = sample_small_poly(1);       // fresh error

//     r_out = r;
//     e_out = e;

//     Poly c = poly_add(poly_add(poly_mul(a, m), poly_mul(b, r)), e);
//     return c;
// }

// // Open: check c == a⋅m + b⋅r + e
// bool Open(const Poly& c, const Poly& a, const Poly& b, const Poly& m, const Poly& r, const Poly& e) {
//     Poly c_check = poly_add(poly_add(poly_mul(a, m), poly_mul(b, r)), e);
//     return c_check == c;
// }

// // MAIN
// int main() {
//     auto [a, b] = KeyGen();

//     Poly message = sample_small_poly(1); // message polynomial
//     Poly r, e;

//     print_poly(a, "a");
//     print_poly(b, "b");
//     print_poly(message, "message");

//     Poly c = Commit(a, b, message, r, e);

//     print_poly(c, "commitment c");

//     if (Open(c, a, b, message, r, e)) {
//         cout << "Commitment successfully opened and verified!" << endl;
//     } else {
//         cout << "Commitment verification failed!" << endl;
//     }

//     return 0;
// }
