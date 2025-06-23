## 🔧 `avpinit.cpp`: Shared Memory Initialization for AVP Protocol

This program sets up the **initial shared state** for the Additive Veto Protocol (AVP) by generating and storing two key polynomials in shared memory:
- `a`: A small, public "error" polynomial used in encryption
- `x`: A random, secret polynomial used for vote masking

These polynomials are accessed by all parties to securely encode their individual votes.

---

### 🚀 What It Does

- Generates two polynomials of degree 64:
  - `a` with small integer coefficients in `[-3, 3]`
  - `x` with random coefficients modulo a large prime `q`
- Stores both polynomials in shared memory using Boost Interprocess:
  - `shared_poly_a`
  - `shared_poly_x`
- These memory regions are later read by:
  - `avpvote.cpp` → to encode each party's vote
  - `avptally.cpp` → to decode the final vote sum

---

### 🔗 Role in the Protocol

This file should be run **once at the start** of each protocol instance, before any parties vote.

> Think of this as the **setup phase** for all parties — similar to a key generation or public parameter broadcast step in other cryptographic protocols.

---

### ⚙️ How to Run

1. Ensure Boost is installed and available to your compiler.
2. Compile:
   ```bash
   g++ avpinit.cpp -o avpinit.exe -I <path_to_boost>
3. Run:
   ```bash
   ./avpinit.exe

## 🧹 `avpclean.cpp`: Cleanup Utility for Shared Memory

This utility removes all shared memory segments created by the Additive Veto Protocol (AVP).  
It ensures the system is reset and ready for a new protocol round.

---

### 🔧 What It Does

- Deletes all known shared memory regions used by:
  - Shared polynomials: `shared_poly_a`, `shared_poly_x`
  - Synchronization: `SharedSync`
  - Vote results: `PartyVote_0` to `PartyVote_9`
  - Shares: `Share_i_j` for all `i, j ∈ [0, 9]`

> This ensures no stale data is left from previous runs.

---

### 🧪 When to Run

Run this:
- **Before starting a new voting round**
- **After a failed/aborted protocol run**
- Anytime you want to ensure a clean state

---

### ⚙️ How to Compile and Run
1. Compile:
     ```bash
     g++ avpclean.cpp -o avpclean.exe -I <path_to_boost>
2. Run:
   ```bash
   ./avpclean.exe

## 🗳 `avpvote.cpp`: Vote Encoding and Sharing in AVP Protocol

This program handles the **secure, privacy-preserving encoding of a vote** by a single party in the Additive Veto Protocol (AVP).

---

### 🔧 Responsibilities

- Generates **additive shares** of a vote-masked polynomial, ensuring total sum is zero across all parties.
- Reads shared polynomials (`a`) and uses it to encode a party’s vote.
- Writes the encoded vote (`V_i`) to shared memory for tallying later.

---

### 🔗 Interactions

- Reads:
  - Shared polynomial `a` (`shared_poly_a`)
- Writes:
  - Shares: `Share_i_j` (from party `i` to `j`)
  - Encoded vote: `PartyVote_i`
- Uses:
  - `shared_sync.hpp` to synchronize all parties (barrier after share exchange)

---

### ⚙️ Vote Encoding Logic

- If vote = `0`:  
  Encodes `V_i = a * x_i + e_i`, where `e_i` is small noise  
  → Adds to the valid aggregated signal.

- If vote = `1`:  
  Encodes `V_i` as a **random high-noise polynomial**  
  → Destroys the final result ⇒ veto.

---

### 📦 Dependencies

- **Boost Interprocess**: For all shared memory communication.
- **C++ Standard Library**: Vectors, randomness, math, memory operations.
- **Local File**: `shared_sync.hpp` for party synchronization.

---

### 🛠 How to Compile and Use
1. Compile:
   ```bash
    g++ avpvote.cpp -o avpvote.exe -I <path_to_boost>
2. Run:
     ```bash
    ./avpvote.exe <party_id> <vote:0|1> <total_parties>

## 📊 `avptally.cpp`: Vote Tallying in the Additive Veto Protocol (AVP)

This module tallies the final result of the Additive Veto Protocol (AVP) by reading all encoded votes from shared memory and checking if any party vetoed.

---

### 🔧 What It Does

- Reads all `PartyVote_i` segments from shared memory (each representing a party's encoded vote).
- Sums them coefficient-wise using modular arithmetic.
- Decodes the result using a centered modulus (`mod_q_centered`) and the **infinity norm** of the final polynomial.
- Determines the outcome:
  - ✅ **ALL VOTED YES (0)** if norm ≤ `q/4`
  - ❌ **SOMEONE VETOED (1)** otherwise

---

### 🔗 Interactions

- **Reads** from:  
  `PartyVote_0`, `PartyVote_1`, ..., `PartyVote_n−1` — all party vote encodings written by `avpvote.cpp`.

- **No writes**:  
  It only reads and outputs the result.

---

### ⚙️ How to Use
1. Compile: 
    ```bash
    g++ avptally.cpp -o avptally.exe -I <path_to_boost>
2. Run:
   ```bash
   ./avptally.exe

