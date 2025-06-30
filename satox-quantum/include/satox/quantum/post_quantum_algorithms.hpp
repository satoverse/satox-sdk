#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace satox {
namespace quantum {

class PostQuantumAlgorithmsImpl;

class PostQuantumAlgorithms {
public:
    // Singleton pattern
    static PostQuantumAlgorithms& getInstance();

    // Prevent copying and assignment
    PostQuantumAlgorithms(const PostQuantumAlgorithms&) = delete;
    PostQuantumAlgorithms& operator=(const PostQuantumAlgorithms&) = delete;

    // Constructor and destructor
    PostQuantumAlgorithms();
    ~PostQuantumAlgorithms();

    // Initialize the post-quantum algorithms
    bool initialize();

    // Shutdown the post-quantum algorithms
    void shutdown();

    // CRYSTALS-Kyber
    bool generateKyberKeyPair(std::string& publicKey, std::string& privateKey);
    bool kyberEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool kyberDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // NTRU
    bool generateNTRUKeyPair(std::string& publicKey, std::string& privateKey);
    bool ntruEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool ntruDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // SABER
    bool generateSABERKeyPair(std::string& publicKey, std::string& privateKey);
    bool saberEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool saberDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // Classic McEliece
    bool generateMcElieceKeyPair(std::string& publicKey, std::string& privateKey);
    bool mcelieceEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool mcelieceDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // BIKE
    bool generateBIKEKeyPair(std::string& publicKey, std::string& privateKey);
    bool bikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool bikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // HQC
    bool generateHQCKeyPair(std::string& publicKey, std::string& privateKey);
    bool hqcEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool hqcDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // SIKE
    bool generateSIKEKeyPair(std::string& publicKey, std::string& privateKey);
    bool sikeEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool sikeDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // FrodoKEM
    bool generateFrodoKeyPair(std::string& publicKey, std::string& privateKey);
    bool frodoEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool frodoDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // ThreeBears
    bool generateThreeBearsKeyPair(std::string& publicKey, std::string& privateKey);
    bool threeBearsEncrypt(const std::string& message, const std::string& publicKey, std::string& ciphertext);
    bool threeBearsDecrypt(const std::string& ciphertext, const std::string& privateKey, std::string& message);

    // Status and information
    bool isInitialized() const;
    std::string getAlgorithm() const;
    std::string getVersion() const;
    std::vector<std::string> getAvailableAlgorithms() const;

private:
    std::unique_ptr<PostQuantumAlgorithmsImpl> impl_;
    std::mutex mutex_;
    bool initialized_;
    std::string algorithm_;
    std::string version_;

    // Internal helper functions
    bool initializeAlgorithms();
    void cleanupAlgorithms();
    bool validateKeyPair(const std::string& publicKey, const std::string& privateKey);
    bool validateMessage(const std::string& message);
    bool validateCiphertext(const std::string& ciphertext);
};

} // namespace quantum
} // namespace satox 