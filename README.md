# 🛡️ Additive Veto Protocol (AVP)

The Additive Veto Protocol is a simple, privacy-preserving voting scheme where:

- Each party contributes an encrypted "yes" or "veto" vote.
- If **all** parties vote "yes", the result reveals **yes**.
- If **even one** party vetoes (votes 1), the final output is scrambled and shows **vetoed**.

It works by encoding votes as polynomials in a shared modular ring, adding controlled noise, and then securely aggregating all votes.

This repository contains the core implementation in C++ with Boost shared memory, along with Python scripts for automated testing and analysis.

---

## 🔧 `avpinit.cpp`: Shared Memory Initialization for AVP Protocol

This program sets up the **initial shared state** for the Additive Veto Protocol (AVP) by generating and storing two key polynomials in shared memory:
- `a`: A small, public "error" polynomial used in encryption
- `x`: A random, secret polynomial used for vote masking

These polynomials are accessed by all parties to securely encode their individual votes.

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

### 🔗 Role in the Protocol

This file should be run **once at the start** of each protocol instance, before any parties vote.

> Think of this as the **setup phase** for all parties — similar to a key generation or public parameter broadcast step in other cryptographic protocols.

### ⚙️ How to Run

1. Ensure Boost is installed and available to your compiler.
2. Compile:
   ```bash
   g++ avpinit.cpp -o avpinit.exe -I <path_to_boost>
3. Run:
   ```bash
   ./avpinit.exe

--- 

## 🧹 `avpclean.cpp`: Cleanup Utility for Shared Memory

This utility removes all shared memory segments created by the Additive Veto Protocol (AVP).  
It ensures the system is reset and ready for a new protocol round.

### 🔧 What It Does

- Deletes all known shared memory regions used by:
  - Shared polynomials: `shared_poly_a`, `shared_poly_x`
  - Synchronization: `SharedSync`
  - Vote results: `PartyVote_0` to `PartyVote_9`
  - Shares: `Share_i_j` for all `i, j ∈ [0, 9]`

> This ensures no stale data is left from previous runs.

### 🧪 When to Run

Run this:
- **Before starting a new voting round**
- **After a failed/aborted protocol run**
- Anytime you want to ensure a clean state

### ⚙️ How to Compile and Run
1. Compile:
     ```bash
     g++ avpclean.cpp -o avpclean.exe -I <path_to_boost>
2. Run:
   ```bash
   ./avpclean.exe

--- 

## 🗳 `avpvote.cpp`: Vote Encoding and Sharing in AVP Protocol

This program handles the **secure, privacy-preserving encoding of a vote** by a single party in the Additive Veto Protocol (AVP).

### 🔧 Responsibilities

- Generates **additive shares** of a vote-masked polynomial, ensuring total sum is zero across all parties.
- Reads shared polynomials (`a`) and uses it to encode a party’s vote.
- Writes the encoded vote (`V_i`) to shared memory for tallying later.

### 🔗 Interactions

- Reads:
  - Shared polynomial `a` (`shared_poly_a`)
- Writes:
  - Shares: `Share_i_j` (from party `i` to `j`)
  - Encoded vote: `PartyVote_i`
- Uses:
  - `shared_sync.hpp` to synchronize all parties (barrier after share exchange)

### ⚙️ Vote Encoding Logic

- If vote = `0`:  
  Encodes `V_i = a * x_i + e_i`, where `e_i` is small noise  
  → Adds to the valid aggregated signal.

- If vote = `1`:  
  Encodes `V_i` as a **random high-noise polynomial**  
  → Destroys the final result ⇒ veto.

### 📦 Dependencies

- **Boost Interprocess**: For all shared memory communication.
- **C++ Standard Library**: Vectors, randomness, math, memory operations.
- **Local File**: `shared_sync.hpp` for party synchronization.

### 🛠 How to Compile and Use
1. Compile:
   ```bash
    g++ avpvote.cpp -o avpvote.exe -I <path_to_boost>
2. Run:
     ```bash
    ./avpvote.exe <party_id> <vote:0|1> <total_parties>

--- 

## 📊 `avptally.cpp`: Vote Tallying in the Additive Veto Protocol (AVP)

This module tallies the final result of the Additive Veto Protocol (AVP) by reading all encoded votes from shared memory and checking if any party vetoed.

### 🔧 What It Does

- Reads all `PartyVote_i` segments from shared memory (each representing a party's encoded vote).
- Sums them coefficient-wise using modular arithmetic.
- Decodes the result using a centered modulus (`mod_q_centered`) and the **infinity norm** of the final polynomial.
- Determines the outcome:
  - ✅ **ALL VOTED YES (0)** if norm ≤ `q/4`
  - ❌ **SOMEONE VETOED (1)** otherwise

### 🔗 Interactions

- **Reads** from:  
  `PartyVote_0`, `PartyVote_1`, ..., `PartyVote_n−1` — all party vote encodings written by `avpvote.cpp`.

- **No writes**:  
  It only reads and outputs the result.

### ⚙️ How to Use
1. Compile: 
    ```bash
    g++ avptally.cpp -o avptally.exe -I <path_to_boost>
2. Run:
   ```bash
   ./avptally.exe

--- 

## 🔄 `shared_sync.hpp`: Shared Memory Synchronization for AVP

This header provides a minimal synchronization mechanism that allows **multiple parties** to block until all others have reached the same point. It's a **barrier-style utility** implemented using Boost Interprocess primitives.

### 🔧 What It Does

- Defines a shared `SyncBlock` structure containing:
  - A mutex for safe access to shared state
  - A counting semaphore to release all waiting parties
  - An `arrived` counter to track how many parties have reached the sync point
- Provides a function `get_sync_block()` to access a **shared instance** from Boost-managed shared memory.

### 📦 Dependencies

- **Boost Interprocess**:
  - `managed_shared_memory`: Used to create or access a named memory segment.
  - `interprocess_mutex`, `interprocess_semaphore`: Cross-process synchronization.
  - `scoped_lock`: For safe, automatic lock management.

### 🚦 When It's Used

- Inside `avpvote.cpp`:  
  All voting parties must complete writing their polynomial shares **before** moving on to compute the encoded vote.  
  This header ensures they all **wait at the barrier** before proceeding.

### 🛠 How It Works

1. Each party calls `arrive_and_wait(total)` with the total number of parties.
2. They block until all others have arrived.
3. Once the last party arrives, the semaphore releases everyone.
4. They proceed simultaneously to the next phase.

### 🧽 Resetting the Sync

- The optional `reset()` function allows the sync state to be cleared between protocol runs.
- Typically called during cleanup or reruns.

### 📝 Notes

- This is a low-level utility — it **does not require compilation** on its own.
- It’s included via `#include "shared_sync.hpp"` in `avpvote.cpp`.

### ✅ Summary

> `shared_sync.hpp` is the AVP protocol's internal "meeting point" — it ensures all parties are aligned before continuing.

--- 

## 🧪 `find_max_m_powershell.py`: Automated Stress Testing for AVP Protocol

This script automates testing the Additive Veto Protocol (AVP) implementation by incrementally increasing the number of parties `m` and identifying the **maximum value where the protocol remains sound**.

### 🔧 What It Does

- Automatically runs the full AVP pipeline for `m = 1` to `m = 100`:
  - Cleans up shared memory (`avpclean.exe`)
  - Initializes the public parameters (`avpinit.exe`)
  - Launches `m` parallel `avpvote.exe` processes
  - Runs `avptally.exe` to verify output
- Stops when the protocol **breaks** (i.e., some result exceeds the `q/4` threshold).

### 💡 Features

- Ensures a clean slate before each run (`Stop-Process`, `avpclean`).
- Uses PowerShell's `Start-Process` to simulate parallel execution of vote processes.
- Includes a `time.sleep(30)` delay to give all processes time to finish writing their votes.
- Stops as soon as a protocol failure is detected, printing the last successful value of `m`.

### 🧰 Requirements

- Windows (due to use of PowerShell commands)
- Python 3.x
- All compiled executables (`avpclean.exe`, `avpinit.exe`, `avpvote.exe`, `avptally.exe`) present in the working directory

### ▶️ Usage
1. Run:
   ```bash
    python find_max_m_powershell.py

--- 

## 📈 `plot_q_vs_m.py`: Visualizing AVP Protocol Breakdown

This script plots the relationship between the modulus `q` and the maximum number of parties `m_max` the Additive Veto Protocol (AVP) can support before breaking.

### 📊 What It Does

- Displays a **line graph** of `q` (modulus) vs. `m_max` (maximum parties before failure).
- Helps visualize how increasing `q` allows the protocol to tolerate larger party sizes.
- Uses hardcoded experimental values collected from running your AVP implementation.

### 🧰 Dependencies

- Python 3.x
- `matplotlib` (install with `pip install matplotlib`)

### ▶️ How to Run
1. Run:
   ```bash
    python plot_q_vs_m.py
