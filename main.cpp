#include <iostream>
#include <chrono>

// cache = sets * lineSize * associativity
// waySize = sets * lineSize

using namespace std;
using timing = chrono::high_resolution_clock;

typedef chrono::high_resolution_clock::rep duration;
typedef uint32_t uint;

const int readsAmount = 1000 * 1000;
const int loops = 20;

const int maxCacheSize = 1 << 10 << 10;
const int maxAssociativity = 32 + 1;

duration stridestamp(int stride, int associativity) __attribute__ ((optnone)) {
    char *cache = (char*)malloc(maxCacheSize);
    uint next = stride * (associativity - 1);
    *(uint*)cache = next;
    for (; next > 0; next -= stride)
        *(uint*)(cache + next) = next - stride;

    duration s = 0;
    for (int i = 0; i < loops; ++i) {
        auto t0 = timing::now();
        for (int read = 0; read < readsAmount; ++read)
            next = *(uint*)(cache + next);
        auto t1 = timing::now();
        s += (t1 - t0).count();
    }
    free(cache);
    return s / loops;
}

bool diff(duration a, duration b) {
    return (a - b) * 10 > a;
}

/// `input`: two sorted arrays, `output`: minimal shared element if exists
int findMinShared(const vector<int>& a, const vector<int>& b) {
    for (int i = 0, j = 0; i < a.size() && j < b.size();) {
        if (a[i] > b[j]) j++;
        else if (a[i] < b[j]) i++;
        else return a[i];
    }
    return -1;
}

pair<int, int> measureWhatever() {
    const int requiredJumpRepetitions = 3;
    int jumpsInRow = 1;
    int currentJump = -1;
    int firstStrideWithJump = 0;
    vector<int> currentJumps;
    vector<int> previousJumps;
    for (int stride = 256; stride < maxCacheSize / maxAssociativity; stride <<= 1) {
        duration prev = 0;
        for (int associativity = 1; associativity <= maxAssociativity; ++associativity) {
            duration t = stridestamp(stride, associativity);
            if (associativity != 1 && diff(t, prev))
                currentJumps.push_back(associativity - 1);
            prev = t;
        }

        int minJump = findMinShared(previousJumps, currentJumps);
        if (minJump != -1) {
            if (minJump != currentJump) {
                currentJump = minJump;
                firstStrideWithJump = stride / 2;
                jumpsInRow = 1;
            }
            if (++jumpsInRow >= requiredJumpRepetitions) return {firstStrideWithJump, minJump};
        } else jumpsInRow = 1;

        previousJumps = currentJumps;
        currentJumps.clear();
    }
    return {firstStrideWithJump, currentJump};
}

int measureCacheLine(int waySize, int associativity) {
    int previousJump = 0;
    for (int size = 1; size <= 1024; size <<= 1) {
        duration prev = 0;
        int jump = 0;
        for (int a = associativity; a <= 128; a++) {
            duration t = stridestamp(waySize + size, a);
            if (a != associativity && diff(t, prev)) {
                jump = a - 1;
                break;
            }
            prev = t;
        }
        if (size != 1 && jump > previousJump) {
            return size * associativity;
        }
        previousJump = jump;
    }
    return -1;
}

int main(int argc, const char * argv[]) {
    printf("Processing cache...\n");
    auto [waySize, associativity] = measureWhatever();
    printf("Almost there...\n");
    int lineSize = measureCacheLine(waySize, associativity);

    printf("Cache Info:\nSize = %d bytes\nAssociativity = %d\nLine size = %d bytes\n",
           waySize * associativity, associativity, lineSize);

    return 0;
}
