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

```bash
g++ avpclean.cpp -o avpclean.exe -I <path_to_boost>
./avpclean.exe
