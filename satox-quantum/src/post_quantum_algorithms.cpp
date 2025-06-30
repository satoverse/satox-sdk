#include "satox/quantum/post_quantum_algorithms.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <sodium.h>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <cstring>

namespace satox {
namespace quantum {

namespace {
    // Base64 encoding table
    const char* const base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    // NTRU parameters
    const size_t N = 509;  // Ring dimension
    const size_t p = 3;    // Small modulus
    const size_t q = 2048; // Large modulus

    // BIKE parameters
    const size_t BIKE_R = 12323;  // Code length
    const size_t BIKE_W = 142;    // Hamming weight
    const size_t BIKE_T = 134;    // Error correction capability

    // HQC parameters
    const size_t HQC_N = 17669;  // Code length
    const size_t HQC_K = 16384;  // Code dimension
    const size_t HQC_W = 66;     // Hamming weight
    const size_t HQC_DELTA = 75; // Error correction parameter

    // SABER parameters
    const size_t SABER_L = 3;    // Number of polynomials
    const size_t SABER_N = 256;  // Polynomial degree
    const int SABER_P = 2;       // Modulus

    // McEliece parameters
    const size_t MCELIECE_N = 3488;  // Code length
    const size_t MCELIECE_K = 2720;  // Code dimension
    const size_t MCELIECE_T = 64;    // Error correction capability

    // ThreeBears parameters
    const size_t THREE_BEARS_N = 512;  // Lattice dimension
    const size_t THREE_BEARS_Q = 7681; // Modulus
    const size_t THREE_BEARS_K = 2;    // Number of bears

    // FrodoKEM parameters
    const size_t FRODO_N = 640;  // Lattice dimension
    const size_t FRODO_Q = 32768; // Modulus
    const size_t FRODO_NBAR = 8;  // Number of columns in B
    const size_t FRODO_MU = 2;    // Number of bits per coefficient
    const size_t FRODO_B = 2;     // Number of bits per coefficient in B

    // Base64 encoding/decoding functions
    std::string base64Encode(const unsigned char* data, size_t length) {
        std::string result;
        result.reserve(((length + 2) / 3) * 4);

        for (size_t i = 0; i < length; i += 3) {
            unsigned char octet_a = i < length ? data[i] : 0;
            unsigned char octet_b = i + 1 < length ? data[i + 1] : 0;
            unsigned char octet_c = i + 2 < length ? data[i + 2] : 0;

            unsigned char triple = (octet_a << 16) + (octet_b << 8) + octet_c;

            result.push_back(base64_chars[(triple >> 18) & 0x3F]);
            result.push_back(base64_chars[(triple >> 12) & 0x3F]);
            result.push_back(base64_chars[(triple >> 6) & 0x3F]);
            result.push_back(base64_chars[triple & 0x3F]);
        }

        // Add padding
        switch (length % 3) {
            case 1:
                result[result.size() - 2] = '=';
                result[result.size() - 1] = '=';
                break;
            case 2:
                result[result.size() - 1] = '=';
                break;
        }

        return result;
    }

    std::string base64EncodeInts(const int* data, size_t length) {
        std::vector<unsigned char> bytes(length * sizeof(int));
        for (size_t i = 0; i < length; ++i) {
            bytes[i * sizeof(int)] = (data[i] >> 24) & 0xFF;
            bytes[i * sizeof(int) + 1] = (data[i] >> 16) & 0xFF;
            bytes[i * sizeof(int) + 2] = (data[i] >> 8) & 0xFF;
            bytes[i * sizeof(int) + 3] = data[i] & 0xFF;
        }
        return base64Encode(bytes.data(), bytes.size());
    }

    std::vector<unsigned char> base64Decode(const std::string& encoded) {
        std::vector<unsigned char> result;
        result.reserve((encoded.size() / 4) * 3);

        // Create reverse lookup table
        std::array<int, 256> lookup;
        std::fill(lookup.begin(), lookup.end(), -1);
        for (int i = 0; i < 64; ++i) {
            lookup[base64_chars[i]] = i;
        }

        for (size_t i = 0; i < encoded.size(); i += 4) {
            if (i + 3 >= encoded.size()) break;

            int sextet_a = lookup[encoded[i]];
            int sextet_b = lookup[encoded[i + 1]];
            int sextet_c = lookup[encoded[i + 2]];
            int sextet_d = lookup[encoded[i + 3]];

            if (sextet_a == -1 || sextet_b == -1) break;

            unsigned char triple = (sextet_a << 18) + (sextet_b << 12);
            if (sextet_c != -1) {
                triple += (sextet_c << 6);
                if (sextet_d != -1) {
                    triple += sextet_d;
                }
            }

            result.push_back((triple >> 16) & 0xFF);
            if (sextet_c != -1) {
                result.push_back((triple >> 8) & 0xFF);
                if (sextet_d != -1) {
                    result.push_back(triple & 0xFF);
                }
            }
        }

        return result;
    }

    // Serialization functions for BIKE
    std::string serializeBIKEPublicKey(const std::vector<int>& h) {
        return base64EncodeInts(h.data(), h.size());
    }

    std::string serializeBIKEPrivateKey(const std::vector<int>& h0, const std::vector<int>& h1) {
        return base64EncodeInts(h0.data(), h0.size()) + ":" + base64EncodeInts(h1.data(), h1.size());
    }

    std::string serializeBIKECiphertext(const std::vector<int>& c) {
        return base64EncodeInts(c.data(), c.size());
    }

    std::vector<int> deserializeBIKEPublicKey(const std::string& str) {
        std::vector<unsigned char> bytes = base64Decode(str);
        std::vector<int> result(bytes.size() / sizeof(int));
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = (bytes[i * sizeof(int)] << 24) |
                       (bytes[i * sizeof(int) + 1] << 16) |
                       (bytes[i * sizeof(int) + 2] << 8) |
                       bytes[i * sizeof(int) + 3];
        }
        return result;
    }

    std::pair<std::vector<int>, std::vector<int>> deserializeBIKEPrivateKey(const std::string& str) {
        size_t pos = str.find(':');
        if (pos == std::string::npos) {
            return {{}, {}};
        }

        std::string h0_str = str.substr(0, pos);
        std::string h1_str = str.substr(pos + 1);

        std::vector<unsigned char> h0_bytes = base64Decode(h0_str);
        std::vector<unsigned char> h1_bytes = base64Decode(h1_str);

        std::vector<int> h0(h0_bytes.size() / sizeof(int));
        std::vector<int> h1(h1_bytes.size() / sizeof(int));

        for (size_t i = 0; i < h0.size(); ++i) {
            h0[i] = (h0_bytes[i * sizeof(int)] << 24) |
                    (h0_bytes[i * sizeof(int) + 1] << 16) |
                    (h0_bytes[i * sizeof(int) + 2] << 8) |
                    h0_bytes[i * sizeof(int) + 3];
        }

        for (size_t i = 0; i < h1.size(); ++i) {
            h1[i] = (h1_bytes[i * sizeof(int)] << 24) |
                    (h1_bytes[i * sizeof(int) + 1] << 16) |
                    (h1_bytes[i * sizeof(int) + 2] << 8) |
                    h1_bytes[i * sizeof(int) + 3];
        }

        return {h0, h1};
    }

    std::vector<int> deserializeBIKECiphertext(const std::string& str) {
        std::vector<unsigned char> bytes = base64Decode(str);
        std::vector<int> result(bytes.size() / sizeof(int));
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = (bytes[i * sizeof(int)] << 24) |
                       (bytes[i * sizeof(int) + 1] << 16) |
                       (bytes[i * sizeof(int) + 2] << 8) |
                       bytes[i * sizeof(int) + 3];
        }
        return result;
    }

    // Serialization functions for HQC
    std::string serializeHQCPublicKey(const std::vector<int>& h) {
        return base64EncodeInts(h.data(), h.size());
    }

    std::string serializeHQCPrivateKey(const std::vector<int>& x, const std::vector<int>& y) {
        return base64EncodeInts(x.data(), x.size()) + ":" + base64EncodeInts(y.data(), y.size());
    }

    std::string serializeHQCCiphertext(const std::vector<int>& c) {
        return base64EncodeInts(c.data(), c.size());
    }

    std::vector<int> deserializeHQCPublicKey(const std::string& str) {
        std::vector<unsigned char> bytes = base64Decode(str);
        std::vector<int> result(bytes.size() / sizeof(int));
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = (bytes[i * sizeof(int)] << 24) |
                       (bytes[i * sizeof(int) + 1] << 16) |
                       (bytes[i * sizeof(int) + 2] << 8) |
                       bytes[i * sizeof(int) + 3];
        }
        return result;
    }

    std::pair<std::vector<int>, std::vector<int>> deserializeHQCPrivateKey(const std::string& str) {
        size_t pos = str.find(':');
        if (pos == std::string::npos) {
            return {{}, {}};
        }

        std::string x_str = str.substr(0, pos);
        std::string y_str = str.substr(pos + 1);

        std::vector<unsigned char> x_bytes = base64Decode(x_str);
        std::vector<unsigned char> y_bytes = base64Decode(y_str);

        std::vector<int> x(x_bytes.size() / sizeof(int));
        std::vector<int> y(y_bytes.size() / sizeof(int));

        for (size_t i = 0; i < x.size(); ++i) {
            x[i] = (x_bytes[i * sizeof(int)] << 24) |
                   (x_bytes[i * sizeof(int) + 1] << 16) |
                   (x_bytes[i * sizeof(int) + 2] << 8) |
                   x_bytes[i * sizeof(int) + 3];
        }

        for (size_t i = 0; i < y.size(); ++i) {
            y[i] = (y_bytes[i * sizeof(int)] << 24) |
                   (y_bytes[i * sizeof(int) + 1] << 16) |
                   (y_bytes[i * sizeof(int) + 2] << 8) |
                   y_bytes[i * sizeof(int) + 3];
        }

        return {x, y};
    }

    std::vector<int> deserializeHQCCiphertext(const std::string& str) {
        std::vector<unsigned char> bytes = base64Decode(str);
        std::vector<int> result(bytes.size() / sizeof(int));
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = (bytes[i * sizeof(int)] << 24) |
                       (bytes[i * sizeof(int) + 1] << 16) |
                       (bytes[i * sizeof(int) + 2] << 8) |
                       bytes[i * sizeof(int) + 3];
        }
        return result;
    }

    // Serialization functions for SABER
    std::string serializeSABERPublicKey(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b) {
        std::stringstream ss;
        for (size_t i = 0; i < a.size(); ++i) {
            ss << base64EncodeInts(a[i].data(), a[i].size()) << ":";
        }
        for (size_t i = 0; i < b.size(); ++i) {
            ss << base64EncodeInts(b[i].data(), b[i].size());
            if (i < b.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeSABERPrivateKey(const std::vector<std::vector<int>>& s) {
        std::stringstream ss;
        for (size_t i = 0; i < s.size(); ++i) {
            ss << base64EncodeInts(s[i].data(), s[i].size());
            if (i < s.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeSABERCiphertext(const std::vector<std::vector<int>>& u, const std::vector<int>& v) {
        std::stringstream ss;
        for (size_t i = 0; i < u.size(); ++i) {
            ss << base64EncodeInts(u[i].data(), u[i].size()) << ":";
        }
        ss << base64EncodeInts(v.data(), v.size());
        return ss.str();
    }

    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> deserializeSABERPublicKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != SABER_L * 2) {
            return {{}, {}};
        }

        std::vector<std::vector<int>> a(SABER_L);
        std::vector<std::vector<int>> b(SABER_L);

        for (size_t i = 0; i < SABER_L; ++i) {
            std::vector<unsigned char> bytes = base64Decode(parts[i]);
            a[i].resize(bytes.size() / sizeof(int));
            for (size_t j = 0; j < a[i].size(); ++j) {
                a[i][j] = (bytes[j * sizeof(int)] << 24) |
                          (bytes[j * sizeof(int) + 1] << 16) |
                          (bytes[j * sizeof(int) + 2] << 8) |
                          bytes[j * sizeof(int) + 3];
            }
        }

        for (size_t i = 0; i < SABER_L; ++i) {
            std::vector<unsigned char> bytes = base64Decode(parts[i + SABER_L]);
            b[i].resize(bytes.size() / sizeof(int));
            for (size_t j = 0; j < b[i].size(); ++j) {
                b[i][j] = (bytes[j * sizeof(int)] << 24) |
                          (bytes[j * sizeof(int) + 1] << 16) |
                          (bytes[j * sizeof(int) + 2] << 8) |
                          bytes[j * sizeof(int) + 3];
            }
        }

        return {a, b};
    }

    std::vector<std::vector<int>> deserializeSABERPrivateKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != SABER_L) {
            return {};
        }

        std::vector<std::vector<int>> s(SABER_L);
        for (size_t i = 0; i < SABER_L; ++i) {
            std::vector<unsigned char> bytes = base64Decode(parts[i]);
            s[i].resize(bytes.size() / sizeof(int));
            for (size_t j = 0; j < s[i].size(); ++j) {
                s[i][j] = (bytes[j * sizeof(int)] << 24) |
                          (bytes[j * sizeof(int) + 1] << 16) |
                          (bytes[j * sizeof(int) + 2] << 8) |
                          bytes[j * sizeof(int) + 3];
            }
        }

        return s;
    }

    std::pair<std::vector<std::vector<int>>, std::vector<int>> deserializeSABERCiphertext(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != SABER_L + 1) {
            return {{}, {}};
        }

        std::vector<std::vector<int>> u(SABER_L);
        for (size_t i = 0; i < SABER_L; ++i) {
            std::vector<unsigned char> bytes = base64Decode(parts[i]);
            u[i].resize(bytes.size() / sizeof(int));
            for (size_t j = 0; j < u[i].size(); ++j) {
                u[i][j] = (bytes[j * sizeof(int)] << 24) |
                          (bytes[j * sizeof(int) + 1] << 16) |
                          (bytes[j * sizeof(int) + 2] << 8) |
                          bytes[j * sizeof(int) + 3];
            }
        }

        std::vector<unsigned char> v_bytes = base64Decode(parts[SABER_L]);
        std::vector<int> v(v_bytes.size() / sizeof(int));
        for (size_t i = 0; i < v.size(); ++i) {
            v[i] = (v_bytes[i * sizeof(int)] << 24) |
                   (v_bytes[i * sizeof(int) + 1] << 16) |
                   (v_bytes[i * sizeof(int) + 2] << 8) |
                   v_bytes[i * sizeof(int) + 3];
        }

        return {u, v};
    }

    // Serialization functions for McEliece
    std::string serializeMcEliecePublicKey(const std::vector<std::vector<int>>& G) {
        std::stringstream ss;
        for (size_t i = 0; i < G.size(); ++i) {
            ss << base64EncodeInts(G[i].data(), G[i].size());
            if (i < G.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeMcEliecePrivateKey(const std::vector<int>& g, const std::vector<int>& support) {
        return base64EncodeInts(g.data(), g.size()) + ":" + base64EncodeInts(support.data(), support.size());
    }

    std::string serializeMcElieceCiphertext(const std::vector<int>& c) {
        return base64EncodeInts(c.data(), c.size());
    }

    std::vector<std::vector<int>> deserializeMcEliecePublicKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        std::vector<std::vector<int>> G(parts.size());
        for (size_t i = 0; i < parts.size(); ++i) {
            std::vector<unsigned char> bytes = base64Decode(parts[i]);
            G[i].resize(bytes.size() / sizeof(int));
            for (size_t j = 0; j < G[i].size(); ++j) {
                G[i][j] = (bytes[j * sizeof(int)] << 24) |
                          (bytes[j * sizeof(int) + 1] << 16) |
                          (bytes[j * sizeof(int) + 2] << 8) |
                          bytes[j * sizeof(int) + 3];
            }
        }

        return G;
    }

    std::pair<std::vector<int>, std::vector<int>> deserializeMcEliecePrivateKey(const std::string& str) {
        size_t pos = str.find(':');
        if (pos == std::string::npos) {
            return {{}, {}};
        }

        std::string g_str = str.substr(0, pos);
        std::string support_str = str.substr(pos + 1);

        std::vector<unsigned char> g_bytes = base64Decode(g_str);
        std::vector<unsigned char> support_bytes = base64Decode(support_str);

        std::vector<int> g(g_bytes.size() / sizeof(int));
        std::vector<int> support(support_bytes.size() / sizeof(int));

        for (size_t i = 0; i < g.size(); ++i) {
            g[i] = (g_bytes[i * sizeof(int)] << 24) |
                   (g_bytes[i * sizeof(int) + 1] << 16) |
                   (g_bytes[i * sizeof(int) + 2] << 8) |
                   g_bytes[i * sizeof(int) + 3];
        }

        for (size_t i = 0; i < support.size(); ++i) {
            support[i] = (support_bytes[i * sizeof(int)] << 24) |
                        (support_bytes[i * sizeof(int) + 1] << 16) |
                        (support_bytes[i * sizeof(int) + 2] << 8) |
                        support_bytes[i * sizeof(int) + 3];
        }

        return {g, support};
    }

    std::vector<int> deserializeMcElieceCiphertext(const std::string& str) {
        std::vector<unsigned char> bytes = base64Decode(str);
        std::vector<int> result(bytes.size() / sizeof(int));
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = (bytes[i * sizeof(int)] << 24) |
                       (bytes[i * sizeof(int) + 1] << 16) |
                       (bytes[i * sizeof(int) + 2] << 8) |
                       bytes[i * sizeof(int) + 3];
        }
        return result;
    }

    // Serialization functions for ThreeBears
    std::string serializeThreeBearsPublicKey(const std::vector<uint16_t>& A, const std::vector<std::vector<uint16_t>>& b) {
        std::stringstream ss;
        ss << base64Encode(reinterpret_cast<const unsigned char*>(A.data()), A.size() * sizeof(uint16_t)) << ":";
        for (size_t i = 0; i < b.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(b[i].data()), b[i].size() * sizeof(uint16_t));
            if (i < b.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeThreeBearsPrivateKey(const std::vector<std::vector<uint16_t>>& s) {
        std::stringstream ss;
        for (size_t i = 0; i < s.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(s[i].data()), s[i].size() * sizeof(uint16_t));
            if (i < s.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeThreeBearsCiphertext(const std::vector<std::vector<uint16_t>>& c1, const std::vector<std::vector<uint16_t>>& c2) {
        std::stringstream ss;
        for (size_t i = 0; i < c1.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(c1[i].data()), c1[i].size() * sizeof(uint16_t)) << ":";
        }
        for (size_t i = 0; i < c2.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(c2[i].data()), c2[i].size() * sizeof(uint16_t));
            if (i < c2.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::pair<std::vector<uint16_t>, std::vector<std::vector<uint16_t>>> deserializeThreeBearsPublicKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != THREE_BEARS_K + 1) {
            return {{}, {}};
        }

        std::vector<unsigned char> A_bytes = base64Decode(parts[0]);
        std::vector<uint16_t> A(A_bytes.size() / sizeof(uint16_t));
        for (size_t i = 0; i < A.size(); ++i) {
            A[i] = (A_bytes[i * sizeof(uint16_t)] << 8) | A_bytes[i * sizeof(uint16_t) + 1];
        }

        std::vector<std::vector<uint16_t>> b(THREE_BEARS_K);
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            std::vector<unsigned char> b_bytes = base64Decode(parts[i + 1]);
            b[i].resize(b_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < b[i].size(); ++j) {
                b[i][j] = (b_bytes[j * sizeof(uint16_t)] << 8) | b_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return {A, b};
    }

    std::vector<std::vector<uint16_t>> deserializeThreeBearsPrivateKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != THREE_BEARS_K) {
            return {};
        }

        std::vector<std::vector<uint16_t>> s(THREE_BEARS_K);
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            std::vector<unsigned char> s_bytes = base64Decode(parts[i]);
            s[i].resize(s_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < s[i].size(); ++j) {
                s[i][j] = (s_bytes[j * sizeof(uint16_t)] << 8) | s_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return s;
    }

    std::pair<std::vector<std::vector<uint16_t>>, std::vector<std::vector<uint16_t>>> deserializeThreeBearsCiphertext(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != THREE_BEARS_K * 2) {
            return {{}, {}};
        }

        std::vector<std::vector<uint16_t>> c1(THREE_BEARS_K);
        std::vector<std::vector<uint16_t>> c2(THREE_BEARS_K);

        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            std::vector<unsigned char> c1_bytes = base64Decode(parts[i]);
            c1[i].resize(c1_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < c1[i].size(); ++j) {
                c1[i][j] = (c1_bytes[j * sizeof(uint16_t)] << 8) | c1_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            std::vector<unsigned char> c2_bytes = base64Decode(parts[i + THREE_BEARS_K]);
            c2[i].resize(c2_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < c2[i].size(); ++j) {
                c2[i][j] = (c2_bytes[j * sizeof(uint16_t)] << 8) | c2_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return {c1, c2};
    }

    // Serialization functions for FrodoKEM
    std::string serializeFrodoPublicKey(const std::vector<uint16_t>& A, const std::vector<std::vector<uint16_t>>& b) {
        std::stringstream ss;
        ss << base64Encode(reinterpret_cast<const unsigned char*>(A.data()), A.size() * sizeof(uint16_t)) << ":";
        for (size_t i = 0; i < b.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(b[i].data()), b[i].size() * sizeof(uint16_t));
            if (i < b.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeFrodoPrivateKey(const std::vector<std::vector<uint16_t>>& s) {
        std::stringstream ss;
        for (size_t i = 0; i < s.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(s[i].data()), s[i].size() * sizeof(uint16_t));
            if (i < s.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::string serializeFrodoCiphertext(const std::vector<std::vector<uint16_t>>& c1, const std::vector<std::vector<uint16_t>>& c2) {
        std::stringstream ss;
        for (size_t i = 0; i < c1.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(c1[i].data()), c1[i].size() * sizeof(uint16_t)) << ":";
        }
        for (size_t i = 0; i < c2.size(); ++i) {
            ss << base64Encode(reinterpret_cast<const unsigned char*>(c2[i].data()), c2[i].size() * sizeof(uint16_t));
            if (i < c2.size() - 1) ss << ":";
        }
        return ss.str();
    }

    std::pair<std::vector<uint16_t>, std::vector<std::vector<uint16_t>>> deserializeFrodoPublicKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != FRODO_NBAR + 1) {
            return {{}, {}};
        }

        std::vector<unsigned char> A_bytes = base64Decode(parts[0]);
        std::vector<uint16_t> A(A_bytes.size() / sizeof(uint16_t));
        for (size_t i = 0; i < A.size(); ++i) {
            A[i] = (A_bytes[i * sizeof(uint16_t)] << 8) | A_bytes[i * sizeof(uint16_t) + 1];
        }

        std::vector<std::vector<uint16_t>> b(FRODO_NBAR);
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            std::vector<unsigned char> b_bytes = base64Decode(parts[i + 1]);
            b[i].resize(b_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < b[i].size(); ++j) {
                b[i][j] = (b_bytes[j * sizeof(uint16_t)] << 8) | b_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return {A, b};
    }

    std::vector<std::vector<uint16_t>> deserializeFrodoPrivateKey(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != FRODO_NBAR) {
            return {};
        }

        std::vector<std::vector<uint16_t>> s(FRODO_NBAR);
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            std::vector<unsigned char> s_bytes = base64Decode(parts[i]);
            s[i].resize(s_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < s[i].size(); ++j) {
                s[i][j] = (s_bytes[j * sizeof(uint16_t)] << 8) | s_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return s;
    }

    std::pair<std::vector<std::vector<uint16_t>>, std::vector<std::vector<uint16_t>>> deserializeFrodoCiphertext(const std::string& str) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        while (std::getline(ss, part, ':')) {
            parts.push_back(part);
        }

        if (parts.size() != FRODO_NBAR * 2) {
            return {{}, {}};
        }

        std::vector<std::vector<uint16_t>> c1(FRODO_NBAR);
        std::vector<std::vector<uint16_t>> c2(FRODO_NBAR);

        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            std::vector<unsigned char> c1_bytes = base64Decode(parts[i]);
            c1[i].resize(c1_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < c1[i].size(); ++j) {
                c1[i][j] = (c1_bytes[j * sizeof(uint16_t)] << 8) | c1_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            std::vector<unsigned char> c2_bytes = base64Decode(parts[i + FRODO_NBAR]);
            c2[i].resize(c2_bytes.size() / sizeof(uint16_t));
            for (size_t j = 0; j < c2[i].size(); ++j) {
                c2[i][j] = (c2_bytes[j * sizeof(uint16_t)] << 8) | c2_bytes[j * sizeof(uint16_t) + 1];
            }
        }

        return {c1, c2};
    }

    // Helper functions
    std::vector<int> invertPolynomial(const std::vector<int>& poly, int modulus, size_t degree) {
        std::vector<int> r = poly;
        std::vector<int> f(degree, 0);
        f[0] = 1;
        std::vector<int> s(degree, 0);
        std::vector<int> t(degree, 0);
        t[0] = 1;

        while (true) {
            while (!r.empty() && r.back() == 0) {
                r.pop_back();
                f.pop_back();
            }
            if (r.empty()) {
                return std::vector<int>();
            }
            if (r.size() == 1) {
                int inv = 1;
                for (int i = 2; i < modulus; ++i) {
                    if ((i * r[0]) % modulus == 1) {
                        inv = i;
                        break;
                    }
                }
                std::vector<int> result(degree, 0);
                for (size_t i = 0; i < f.size(); ++i) {
                    result[i] = (f[i] * inv) % modulus;
                }
                return result;
            }

            if (r.size() < f.size()) {
                std::vector<int> temp_r = r;
                std::vector<int> temp_f = f;
                std::vector<int> temp_s = s;
                std::vector<int> temp_t = t;
                r = temp_f;
                f = temp_r;
                s = temp_t;
                t = temp_s;
            }

            int lead_r = r.back();
            int lead_f = f.back();
            int q = (lead_r * lead_f) % modulus;
            for (size_t i = 0; i < f.size(); ++i) {
                r[i] = (r[i] - (f[i] * q) % modulus + modulus) % modulus;
            }
            for (size_t i = 0; i < s.size(); ++i) {
                t[i] = (t[i] - (s[i] * q) % modulus + modulus) % modulus;
            }
        }
    }

    std::vector<int> multiplyPolynomials(const std::vector<int>& a, const std::vector<int>& b, int modulus, size_t degree) {
        std::vector<int> result(degree, 0);
        for (size_t i = 0; i < degree; ++i) {
            for (size_t j = 0; j < degree; ++j) {
                if (i + j < degree) {
                    result[i + j] = (result[i + j] + a[i] * b[j]) % modulus;
                }
            }
        }
        return result;
    }

    int evaluatePolynomial(const std::vector<int>& poly, int x, int modulus) {
        int result = 0;
        int x_pow = 1;
        for (int coef : poly) {
            result = (result + coef * x_pow) % modulus;
            x_pow = (x_pow * x) % modulus;
        }
        return result;
    }

    std::string serializePolynomial(const std::vector<int>& poly) {
        std::stringstream ss;
        for (int coef : poly) {
            ss << coef << " ";
        }
        return ss.str();
    }

    std::vector<int> deserializePolynomial(const std::string& str) {
        std::vector<int> result;
        std::stringstream ss(str);
        int coef;
        while (ss >> coef) {
            result.push_back(coef);
        }
        return result;
    }

    // Helper function to convert int vector to unsigned char vector
    std::vector<unsigned char> intVectorToBytes(const std::vector<int>& vec) {
        std::vector<unsigned char> result(vec.size() * sizeof(int));
        for (size_t i = 0; i < vec.size(); ++i) {
            result[i * sizeof(int)] = (vec[i] >> 24) & 0xFF;
            result[i * sizeof(int) + 1] = (vec[i] >> 16) & 0xFF;
            result[i * sizeof(int) + 2] = (vec[i] >> 8) & 0xFF;
            result[i * sizeof(int) + 3] = vec[i] & 0xFF;
        }
        return result;
    }
}

class PostQuantumAlgorithmsImpl {
public:
    PostQuantumAlgorithmsImpl() : initialized_(false), keyUsageCount_(0) {}

    bool initialize() {
        if (initialized_) return true;
        if (sodium_init() < 0) return false;
        initialized_ = true;
        return true;
    }

    void shutdown() {
        if (initialized_) {
            initialized_ = false;
        }
    }

    bool generateKyberKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        unsigned char pk[crypto_kx_PUBLICKEYBYTES];
        unsigned char sk[crypto_kx_SECRETKEYBYTES];

        if (crypto_kx_keypair(pk, sk) != 0) return false;

        publicKey = base64Encode(pk, crypto_kx_PUBLICKEYBYTES);
        privateKey = base64Encode(sk, crypto_kx_SECRETKEYBYTES);

        keyUsageCount_++;
        return true;
    }

    bool kyberEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        std::vector<unsigned char> pk_bytes = base64Decode(publicKey);
        if (pk_bytes.size() != crypto_kx_PUBLICKEYBYTES) return false;

        unsigned char rx[crypto_kx_SESSIONKEYBYTES];
        unsigned char tx[crypto_kx_SESSIONKEYBYTES];
        unsigned char client_pk[crypto_kx_PUBLICKEYBYTES];
        unsigned char client_sk[crypto_kx_SECRETKEYBYTES];

        // Generate temporary client key pair for the session
        if (crypto_kx_keypair(client_pk, client_sk) != 0) return false;

        if (crypto_kx_client_session_keys(rx, tx, client_pk, client_sk, pk_bytes.data()) != 0) return false;

        ciphertext = base64Encode(tx, crypto_kx_SESSIONKEYBYTES);
        return true;
    }

    bool kyberDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        std::vector<unsigned char> sk_bytes = base64Decode(privateKey);
        std::vector<unsigned char> ct_bytes = base64Decode(ciphertext);

        if (sk_bytes.size() != crypto_kx_SECRETKEYBYTES || ct_bytes.size() != crypto_kx_SESSIONKEYBYTES) return false;

        unsigned char rx[crypto_kx_SESSIONKEYBYTES];
        unsigned char tx[crypto_kx_SESSIONKEYBYTES];
        unsigned char server_pk[crypto_kx_PUBLICKEYBYTES];

        // Derive server public key from private key (this is a simplified approach)
        // In a real implementation, you'd store the public key separately
        if (crypto_scalarmult_base(server_pk, sk_bytes.data()) != 0) return false;

        if (crypto_kx_server_session_keys(rx, tx, server_pk, sk_bytes.data(), ct_bytes.data()) != 0) return false;

        message = base64Encode(rx, crypto_kx_SESSIONKEYBYTES);
        return true;
    }

    bool generateNTRUKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        std::vector<int> f(N), g(N), h(N);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-1, 1);

        // Generate random polynomials f and g
        for (size_t i = 0; i < N; ++i) {
            f[i] = dis(gen);
            g[i] = dis(gen);
        }

        // Compute h = g * f^(-1) mod q
        std::vector<int> f_inv = invertPolynomial(f, q, N);
        h = multiplyPolynomials(g, f_inv, q, N);

        publicKey = serializePolynomial(h);
        privateKey = serializePolynomial(f) + ":" + serializePolynomial(g);

        keyUsageCount_++;
        return true;
    }

    bool ntruEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        std::vector<int> h = deserializePolynomial(publicKey);
        if (h.size() != N) return false;

        std::vector<unsigned char> msg_bytes = base64Decode(message);
        std::vector<int> m(N);
        for (size_t i = 0; i < std::min(msg_bytes.size(), N); ++i) {
            m[i] = msg_bytes[i] % p;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-1, 1);

        std::vector<int> r(N);
        for (size_t i = 0; i < N; ++i) {
            r[i] = dis(gen);
        }

        std::vector<int> c = multiplyPolynomials(r, h, q, N);
        for (size_t i = 0; i < N; ++i) {
            c[i] = (c[i] + m[i]) % q;
        }

        ciphertext = serializePolynomial(c);
        return true;
    }

    bool ntruDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        size_t pos = privateKey.find(':');
        if (pos == std::string::npos) return false;

        std::vector<int> f = deserializePolynomial(privateKey.substr(0, pos));
        std::vector<int> g = deserializePolynomial(privateKey.substr(pos + 1));
        std::vector<int> c = deserializePolynomial(ciphertext);

        if (f.size() != N || g.size() != N || c.size() != N) return false;

        std::vector<int> m = multiplyPolynomials(c, f, q, N);
        for (size_t i = 0; i < N; ++i) {
            m[i] = m[i] % p;
        }

        std::vector<unsigned char> msg_bytes(N);
        for (size_t i = 0; i < N; ++i) {
            msg_bytes[i] = static_cast<unsigned char>(m[i]);
        }

        message = base64Encode(msg_bytes.data(), msg_bytes.size());
        return true;
    }

    bool generateBIKEKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        std::vector<int> h0(BIKE_R), h1(BIKE_R);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        // Generate random binary vectors
        for (size_t i = 0; i < BIKE_R; ++i) {
            h0[i] = dis(gen);
            h1[i] = dis(gen);
        }

        publicKey = serializeBIKEPublicKey(h0);
        privateKey = serializeBIKEPrivateKey(h0, h1);

        keyUsageCount_++;
        return true;
    }

    bool bikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        std::vector<int> h = deserializeBIKEPublicKey(publicKey);
        if (h.size() != BIKE_R) return false;

        std::vector<unsigned char> msg_bytes = base64Decode(message);
        std::vector<int> m(BIKE_R);
        for (size_t i = 0; i < std::min(msg_bytes.size(), BIKE_R); ++i) {
            m[i] = msg_bytes[i] % 2;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        std::vector<int> e(BIKE_R);
        for (size_t i = 0; i < BIKE_R; ++i) {
            e[i] = dis(gen);
        }

        std::vector<int> c(BIKE_R);
        for (size_t i = 0; i < BIKE_R; ++i) {
            c[i] = (h[i] * e[i] + m[i]) % 2;
        }

        ciphertext = serializeBIKECiphertext(c);
        return true;
    }

    bool bikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        auto [h0, h1] = deserializeBIKEPrivateKey(privateKey);
        std::vector<int> c = deserializeBIKECiphertext(ciphertext);

        if (h0.size() != BIKE_R || h1.size() != BIKE_R || c.size() != BIKE_R) return false;

        std::vector<int> m(BIKE_R);
        for (size_t i = 0; i < BIKE_R; ++i) {
            m[i] = (c[i] + h0[i] * h1[i]) % 2;
        }

        std::vector<unsigned char> msg_bytes(BIKE_R);
        for (size_t i = 0; i < BIKE_R; ++i) {
            msg_bytes[i] = static_cast<unsigned char>(m[i]);
        }

        message = base64Encode(msg_bytes.data(), msg_bytes.size());
        return true;
    }

    bool generateHQCKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        std::vector<int> h(HQC_N);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        // Generate random binary vector
        for (size_t i = 0; i < HQC_N; ++i) {
            h[i] = dis(gen);
        }

        publicKey = serializeHQCPublicKey(h);
        privateKey = serializeHQCPrivateKey(h, std::vector<int>(HQC_N));

        keyUsageCount_++;
        return true;
    }

    bool hqcEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        std::vector<int> h = deserializeHQCPublicKey(publicKey);
        if (h.size() != HQC_N) return false;

        std::vector<unsigned char> msg_bytes = base64Decode(message);
        std::vector<int> m(HQC_N);
        for (size_t i = 0; i < std::min(msg_bytes.size(), HQC_N); ++i) {
            m[i] = msg_bytes[i] % 2;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        std::vector<int> e(HQC_N);
        for (size_t i = 0; i < HQC_N; ++i) {
            e[i] = dis(gen);
        }

        std::vector<int> c(HQC_N);
        for (size_t i = 0; i < HQC_N; ++i) {
            c[i] = (h[i] * e[i] + m[i]) % 2;
        }

        ciphertext = serializeHQCCiphertext(c);
        return true;
    }

    bool hqcDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        auto [x, y] = deserializeHQCPrivateKey(privateKey);
        std::vector<int> c = deserializeHQCCiphertext(ciphertext);

        if (x.size() != HQC_N || y.size() != HQC_N || c.size() != HQC_N) return false;

        std::vector<int> m(HQC_N);
        for (size_t i = 0; i < HQC_N; ++i) {
            m[i] = (c[i] + x[i] * y[i]) % 2;
        }

        std::vector<unsigned char> msg_bytes(HQC_N);
        for (size_t i = 0; i < HQC_N; ++i) {
            msg_bytes[i] = static_cast<unsigned char>(m[i]);
        }

        message = base64Encode(msg_bytes.data(), msg_bytes.size());
        return true;
    }

    bool generateSIKEKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        // SIKE implementation using OpenSSL
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        if (!ctx) return false;

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        // Export public key
        unsigned char* pub = nullptr;
        size_t pub_len = 0;
        if (EVP_PKEY_get_raw_public_key(pkey, nullptr, &pub_len) <= 0) {
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        pub = new unsigned char[pub_len];
        if (EVP_PKEY_get_raw_public_key(pkey, pub, &pub_len) <= 0) {
            delete[] pub;
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        // Export private key
        unsigned char* priv = nullptr;
        size_t priv_len = 0;
        if (EVP_PKEY_get_raw_private_key(pkey, nullptr, &priv_len) <= 0) {
            delete[] pub;
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        priv = new unsigned char[priv_len];
        if (EVP_PKEY_get_raw_private_key(pkey, priv, &priv_len) <= 0) {
            delete[] pub;
            delete[] priv;
            EVP_PKEY_free(pkey);
            EVP_PKEY_CTX_free(ctx);
            return false;
        }

        publicKey = base64Encode(pub, pub_len);
        privateKey = base64Encode(priv, priv_len);

        delete[] pub;
        delete[] priv;
        EVP_PKEY_free(pkey);
        EVP_PKEY_CTX_free(ctx);

        keyUsageCount_++;
        return true;
    }

    bool sikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        std::vector<unsigned char> pub_bytes = base64Decode(publicKey);
        std::vector<unsigned char> msg_bytes = base64Decode(message);

        EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_EC, nullptr, pub_bytes.data(), pub_bytes.size());
        if (!pkey) return false;

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            return false;
        }

        if (EVP_PKEY_encrypt_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        size_t outlen = 0;
        if (EVP_PKEY_encrypt(ctx, nullptr, &outlen, msg_bytes.data(), msg_bytes.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        std::vector<unsigned char> out(outlen);
        if (EVP_PKEY_encrypt(ctx, out.data(), &outlen, msg_bytes.data(), msg_bytes.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        ciphertext = base64Encode(out.data(), outlen);

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return true;
    }

    bool sikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        std::vector<unsigned char> priv_bytes = base64Decode(privateKey);
        std::vector<unsigned char> ct_bytes = base64Decode(ciphertext);

        EVP_PKEY* pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_EC, nullptr, priv_bytes.data(), priv_bytes.size());
        if (!pkey) return false;

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
        if (!ctx) {
            EVP_PKEY_free(pkey);
            return false;
        }

        if (EVP_PKEY_decrypt_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        size_t outlen = 0;
        if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, ct_bytes.data(), ct_bytes.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        std::vector<unsigned char> out(outlen);
        if (EVP_PKEY_decrypt(ctx, out.data(), &outlen, ct_bytes.data(), ct_bytes.size()) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            EVP_PKEY_free(pkey);
            return false;
        }

        message = base64Encode(out.data(), outlen);

        EVP_PKEY_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return true;
    }

    bool generateFrodoKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        std::vector<uint16_t> A(FRODO_N * FRODO_N);
        std::vector<std::vector<uint16_t>> b(FRODO_NBAR);
        std::vector<std::vector<uint16_t>> s(FRODO_NBAR);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, FRODO_Q - 1);

        // Generate random matrix A
        for (size_t i = 0; i < FRODO_N * FRODO_N; ++i) {
            A[i] = dis(gen);
        }

        // Generate random secret s
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            s[i].resize(FRODO_N);
            for (size_t j = 0; j < FRODO_N; ++j) {
                s[i][j] = dis(gen);
            }
        }

        // Compute b = A * s
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            b[i].resize(FRODO_N);
            for (size_t j = 0; j < FRODO_N; ++j) {
                b[i][j] = 0;
                for (size_t k = 0; k < FRODO_N; ++k) {
                    b[i][j] = (b[i][j] + A[i * FRODO_N + k] * s[k][j]) % FRODO_Q;
                }
            }
        }

        publicKey = serializeFrodoPublicKey(A, b);
        privateKey = serializeFrodoPrivateKey(s);

        keyUsageCount_++;
        return true;
    }

    bool frodoEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        auto [A, b] = deserializeFrodoPublicKey(publicKey);
        if (A.size() != FRODO_N * FRODO_N || b.size() != FRODO_NBAR) return false;

        std::vector<unsigned char> msg_bytes = base64Decode(message);
        std::vector<uint16_t> m(FRODO_NBAR);
        for (size_t i = 0; i < std::min(msg_bytes.size(), FRODO_NBAR); ++i) {
            m[i] = msg_bytes[i] % FRODO_Q;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, FRODO_Q - 1);

        std::vector<std::vector<uint16_t>> r(FRODO_NBAR);
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            r[i].resize(FRODO_N);
            for (size_t j = 0; j < FRODO_N; ++j) {
                r[i][j] = dis(gen);
            }
        }

        std::vector<std::vector<uint16_t>> c1(FRODO_NBAR);
        std::vector<std::vector<uint16_t>> c2(FRODO_NBAR);

        // Compute c1 = r * A
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            c1[i].resize(FRODO_N);
            for (size_t j = 0; j < FRODO_N; ++j) {
                c1[i][j] = 0;
                for (size_t k = 0; k < FRODO_N; ++k) {
                    c1[i][j] = (c1[i][j] + r[i][k] * A[k * FRODO_N + j]) % FRODO_Q;
                }
            }
        }

        // Compute c2 = r * b + m
        for (size_t i = 0; i < FRODO_NBAR; ++i) {
            c2[i].resize(FRODO_N);
            for (size_t j = 0; j < FRODO_N; ++j) {
                c2[i][j] = 0;
                for (size_t k = 0; k < FRODO_N; ++k) {
                    c2[i][j] = (c2[i][j] + r[i][k] * b[k][j]) % FRODO_Q;
                }
                c2[i][j] = (c2[i][j] + m[i]) % FRODO_Q;
            }
        }

        ciphertext = serializeFrodoCiphertext(c1, c2);
        return true;
    }

    bool frodoDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        std::vector<std::vector<uint16_t>> s = deserializeFrodoPrivateKey(privateKey);
        auto [c1, c2] = deserializeFrodoCiphertext(ciphertext);

        if (s.size() != FRODO_NBAR || c1.size() != FRODO_NBAR || c2.size() != FRODO_NBAR) return false;

        std::vector<uint16_t> m(FRODO_N);
        for (size_t i = 0; i < FRODO_N; ++i) {
            m[i] = 0;
            for (size_t j = 0; j < FRODO_NBAR; ++j) {
                for (size_t k = 0; k < FRODO_N; ++k) {
                    m[i] = (m[i] + c1[j][k] * s[j][k]) % FRODO_Q;
                }
            }
            m[i] = (c2[0][i] - m[i]) % FRODO_Q;
        }

        std::vector<unsigned char> msg_bytes(FRODO_N);
        for (size_t i = 0; i < FRODO_N; ++i) {
            msg_bytes[i] = static_cast<unsigned char>(m[i]);
        }

        message = base64Encode(msg_bytes.data(), msg_bytes.size());
        return true;
    }

    bool generateThreeBearsKeyPair(std::string& publicKey, std::string& privateKey) {
        if (!initialized_) return false;
        if (keyUsageCount_ >= MAX_KEY_USAGE) return false;

        std::vector<uint16_t> A(THREE_BEARS_N);
        std::vector<std::vector<uint16_t>> b(THREE_BEARS_K);
        std::vector<std::vector<uint16_t>> s(THREE_BEARS_K);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, THREE_BEARS_Q - 1);

        // Generate random vector A
        for (size_t i = 0; i < THREE_BEARS_N; ++i) {
            A[i] = dis(gen);
        }

        // Generate random secret s
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            s[i].resize(THREE_BEARS_N);
            for (size_t j = 0; j < THREE_BEARS_N; ++j) {
                s[i][j] = dis(gen);
            }
        }

        // Compute b = A * s
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            b[i].resize(THREE_BEARS_N);
            for (size_t j = 0; j < THREE_BEARS_N; ++j) {
                b[i][j] = 0;
                for (size_t k = 0; k < THREE_BEARS_N; ++k) {
                    b[i][j] = (b[i][j] + A[k] * s[i][k]) % THREE_BEARS_Q;
                }
            }
        }

        publicKey = serializeThreeBearsPublicKey(A, b);
        privateKey = serializeThreeBearsPrivateKey(s);

        keyUsageCount_++;
        return true;
    }

    bool threeBearsEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
        if (!initialized_) return false;

        auto [A, b] = deserializeThreeBearsPublicKey(publicKey);
        if (A.size() != THREE_BEARS_N || b.size() != THREE_BEARS_K) return false;

        std::vector<unsigned char> msg_bytes = base64Decode(message);
        std::vector<uint16_t> m(THREE_BEARS_N);
        for (size_t i = 0; i < std::min(msg_bytes.size(), THREE_BEARS_N); ++i) {
            m[i] = msg_bytes[i] % THREE_BEARS_Q;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, THREE_BEARS_Q - 1);

        std::vector<std::vector<uint16_t>> r(THREE_BEARS_K);
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            r[i].resize(THREE_BEARS_N);
            for (size_t j = 0; j < THREE_BEARS_N; ++j) {
                r[i][j] = dis(gen);
            }
        }

        std::vector<std::vector<uint16_t>> c1(THREE_BEARS_K);
        std::vector<std::vector<uint16_t>> c2(THREE_BEARS_K);

        // Compute c1 = r * A
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            c1[i].resize(THREE_BEARS_N);
            for (size_t j = 0; j < THREE_BEARS_N; ++j) {
                c1[i][j] = 0;
                for (size_t k = 0; k < THREE_BEARS_N; ++k) {
                    c1[i][j] = (c1[i][j] + r[i][k] * A[k]) % THREE_BEARS_Q;
                }
            }
        }

        // Compute c2 = r * b + m
        for (size_t i = 0; i < THREE_BEARS_K; ++i) {
            c2[i].resize(THREE_BEARS_N);
            for (size_t j = 0; j < THREE_BEARS_N; ++j) {
                c2[i][j] = 0;
                for (size_t k = 0; k < THREE_BEARS_N; ++k) {
                    c2[i][j] = (c2[i][j] + r[i][k] * b[i][k]) % THREE_BEARS_Q;
                }
                c2[i][j] = (c2[i][j] + m[j]) % THREE_BEARS_Q;
            }
        }

        ciphertext = serializeThreeBearsCiphertext(c1, c2);
        return true;
    }

    bool threeBearsDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
        if (!initialized_) return false;

        std::vector<std::vector<uint16_t>> s = deserializeThreeBearsPrivateKey(privateKey);
        auto [c1, c2] = deserializeThreeBearsCiphertext(ciphertext);

        if (s.size() != THREE_BEARS_K || c1.size() != THREE_BEARS_K || c2.size() != THREE_BEARS_K) return false;

        std::vector<uint16_t> m(THREE_BEARS_N);
        for (size_t i = 0; i < THREE_BEARS_N; ++i) {
            m[i] = 0;
            for (size_t j = 0; j < THREE_BEARS_K; ++j) {
                for (size_t k = 0; k < THREE_BEARS_N; ++k) {
                    m[i] = (m[i] + c1[j][k] * s[j][k]) % THREE_BEARS_Q;
                }
            }
            m[i] = (c2[0][i] - m[i]) % THREE_BEARS_Q;
        }

        std::vector<unsigned char> msg_bytes(THREE_BEARS_N);
        for (size_t i = 0; i < THREE_BEARS_N; ++i) {
            msg_bytes[i] = static_cast<unsigned char>(m[i]);
        }

        message = base64Encode(msg_bytes.data(), msg_bytes.size());
        return true;
    }

private:
    bool initialized_;
    size_t keyUsageCount_;
    std::mutex mutex_;
    static constexpr size_t MAX_KEY_USAGE = 10000;
};

PostQuantumAlgorithms& PostQuantumAlgorithms::getInstance() {
    static PostQuantumAlgorithms instance;
    return instance;
}

PostQuantumAlgorithms::PostQuantumAlgorithms() : initialized_(false) {
    impl_ = std::make_unique<PostQuantumAlgorithmsImpl>();
}

PostQuantumAlgorithms::~PostQuantumAlgorithms() {
    shutdown();
}

bool PostQuantumAlgorithms::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }
    initialized_ = impl_->initialize();
    return initialized_;
}

void PostQuantumAlgorithms::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        impl_->shutdown();
        initialized_ = false;
    }
}

bool PostQuantumAlgorithms::generateKyberKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateKyberKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::kyberEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->kyberEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::kyberDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->kyberDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateNTRUKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateNTRUKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::ntruEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->ntruEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::ntruDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->ntruDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateBIKEKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateBIKEKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::bikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->bikeEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::bikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->bikeDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateHQCKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateHQCKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::hqcEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->hqcEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::hqcDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->hqcDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateSIKEKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateSIKEKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::sikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->sikeEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::sikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->sikeDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateFrodoKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateFrodoKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::frodoEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->frodoEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::frodoDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->frodoDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::generateThreeBearsKeyPair(std::string& publicKey, std::string& privateKey) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->generateThreeBearsKeyPair(publicKey, privateKey);
}

bool PostQuantumAlgorithms::threeBearsEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->threeBearsEncrypt(message, publicKey, ciphertext);
}

bool PostQuantumAlgorithms::threeBearsDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message) {
    if (!initialized_) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    return impl_->threeBearsDecrypt(ciphertext, privateKey, message);
}

bool PostQuantumAlgorithms::isInitialized() const {
    return initialized_;
}

std::string PostQuantumAlgorithms::getAlgorithm() const {
    return algorithm_;
}

std::string PostQuantumAlgorithms::getVersion() const {
    return version_;
}

std::vector<std::string> PostQuantumAlgorithms::getAvailableAlgorithms() const {
    return {"CRYSTALS-Kyber", "NTRU", "SABER", "Classic McEliece", "BIKE", "HQC", "SIKE", "FrodoKEM", "ThreeBears"};
}

} // namespace quantum
} // namespace satox