#include <boost/interprocess/shared_memory_object.hpp>  // For managing named shared memory segments
#include <iostream>  // For optional debug output (currently commented out)
using namespace std;
using namespace boost::interprocess;

int main() {
    // Remove key shared memory segments used during the AVP protocol
    shared_memory_object::remove("SharedSync");
    shared_memory_object::remove("shared_poly_a");
    shared_memory_object::remove("shared_poly_x");

    for (int i = 0; i < 10; ++i) {
        // Remove vote result segment for party i
        shared_memory_object::remove(("PartyVote_" + to_string(i)).c_str());
        // Remove all share segments sent from party i to party j
        for (int j = 0; j < 10; ++j)
            shared_memory_object::remove(("Share_" + to_string(i) + "_" + to_string(j)).c_str());
    }
    // cout << "All shared memory segments cleaned up.\n";
    return 0;
}
