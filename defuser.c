#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 0;

    uint32_t a = (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)(challenge >> 16);
    uint32_t d = (uint32_t)challenge;

    uint32_t s1 = (b + 0x9a93u) ^ a;
    uint32_t s0 = 3u * (d - a) + 0xcad6u;
    uint32_t answer = (s0 + 2u * s1) ^ 0x88fu;

    values[0] = answer;
    count = 1;
    return write_answer(out, cap, values, count);
}


int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = 3u * ((uint32_t)(challenge >> 36) & 0x7fu);
    for (uint32_t i = 1; i < 6; i++) {
        uint32_t nib = (uint32_t)(challenge >> (i + 13)) & 0xfu;
        values[i] = 5u * values[i - 1] ^ (nib + i);
    }
    return write_answer(out, cap, values, count);
}


int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t k = (uint32_t)challenge & 0x7u;
    uint32_t arg = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    uint32_t result = 0;

    switch (k) {
        case 0: result = 5u * arg + 0xaed0u; break;
        case 1: result = (arg ^ 0x1deeu) + (arg >> 1); break;
        case 2: result = (arg ^ 0xa9cfu) + (arg >> 2); break;
        case 3: result = 5u * arg + 0x2422au; break;
        case 4: result = 5u * arg + 0x1f58bu; break;
        case 5: result = (2u * arg) ^ 0xdeddu; break;
        case 6: result = 5u * arg + 0x50e1u; break;
        case 7: result = (4u * arg) ^ 0xb524u; break;
    }
    values[0] = k;
    values[1] = result;
    return write_answer(out, cap, values, count);
}


static uint32_t p4_walk(uint32_t t, uint32_t lo, uint32_t hi, uint32_t key)
{
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (mid == t)
        return (mid + 0x3bu) ^ key;
    uint32_t node = mid + key;
    if (t > mid)
        return (5u * p4_walk(t, mid + 1, hi, key) ^ node) ^ 0x13u;
    else
        return (5u * p4_walk(t, lo, mid - 1, key) ^ node) ^ 0x18u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t t = (uint32_t)challenge & 0x7fu;
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = p4_walk(t, 0, 0x7fu, key);
    return write_answer(out, cap, values, count);
}


int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    static const uint32_t b[12] = {
        2914, 812, 1668, 2441, 537, 1830, 66, 2175, 302, 2682, 1138, 1350
    };
    static const uint32_t c[12] = {
        56443, 51520, 19574, 6649, 52704, 60966, 11091, 37947, 47300, 22814, 54484, 12471
    };
    static const int nxt[12] = {
        7, 0, 3, 6, 11, 9, 4, 2, 5, -1, 8, 10
    };
    static const int start[8] = {
        1, 0, 7, 2, 3, 6, 4, 11
    };

    uint32_t sel = (uint32_t)challenge & 0x7u;
    uint32_t steps = ((uint32_t)(challenge >> 3)) & 0x3u;
    int cur = start[sel];

    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32)
               ^ c[cur] ^ 0x55cu;
    x = x + b[cur];
    uint32_t answer1 = b[cur];

    for (uint32_t i = 0; i < steps; i++) {
        cur = nxt[cur];
        x = (x ^ c[cur]) + b[cur];
        answer1 = b[cur];
    }

    values[0] = x;
    values[1] = answer1;
    return write_answer(out, cap, values, count);
}


static uint32_t p6_rol(uint32_t x, unsigned n) {
    return (x << n) | (x >> (32 - n));
}
static uint32_t p6_ror(uint32_t x, unsigned n) {
    return (x >> n) | (x << (32 - n));
}
static uint32_t p6_op(uint32_t id, uint32_t x) {
    switch (id) {
        case 0: return (x ^ (x >> 5)) + 0xa7d4u;
        case 1: return p6_ror(x, 7);
        case 2: return p6_rol(x, 2) ^ 0x609u;
        case 3: return (x ^ 0x6872u) + 8u * x;
        case 4: return (x + 0x2e19u) ^ (x >> 1);
        case 5: return ((x << 7) + 0x1a84u) ^ (5u * x);
    }
    return 0;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    static const uint8_t table[8] = {4, 1, 5, 2, 0, 5, 3, 0};

    uint32_t arg = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t ra = p6_op(table[(uint32_t)challenge & 0x7u], arg);
    uint32_t r1 = ra ^ 0xf91fu;
    uint32_t rb = p6_op(table[((uint32_t)(challenge >> 3)) & 0x7u], r1);
    uint32_t answer = rb + 0xdb29u;

    values[0] = answer;
    return write_answer(out, cap, values, count);
}


int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t cl = (uint32_t)challenge;
    uint32_t ch = (uint32_t)(challenge >> 32);
    uint32_t xl = (uint32_t)context;
    uint32_t xh = (uint32_t)(context >> 32);

    uint32_t v0 = xl & 3u;
    uint32_t A = ch ^ cl;
    uint32_t B = xh ^ xl;
    uint32_t r9 = (v0 * 0xd6ddu) ^ (uint32_t)(context >> 16);
    uint32_t mix = (p6_rol(A ^ B, 7) + 0xd6ddu) ^ 0x55b9u;
    uint32_t v1 = mix ^ r9;
    uint32_t v2 = v1 ^ (B + v0 + 0x55b9u + (A >> 7));

    int n = snprintf(out, cap,
                     "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXOPEN %u %u %u",
                     v0, v1, v2);
    if (n < 0 || (size_t)n >= cap) {
        return 1;
    }
    return 0;
}

