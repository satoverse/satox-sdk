#include <emscripten/bind.h>
#include <string>
#include <map>

using namespace emscripten;

class NFT {
public:
    std::string id;
    std::string owner;
    std::string metadata;
};

class NFTManager {
public:
    NFTManager() : initialized(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("NFTManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void createNFT(const std::string& id, const std::string& owner, const std::string& metadata) {
        if (!initialized) throw std::runtime_error("NFTManager not initialized");
        if (nfts.find(id) != nfts.end()) throw std::runtime_error("NFT already exists");
        
        NFT nft;
        nft.id = id;
        nft.owner = owner;
        nft.metadata = metadata;
        nfts[id] = nft;
    }

    NFT getNFT(const std::string& id) {
        if (!initialized) throw std::runtime_error("NFTManager not initialized");
        if (nfts.find(id) == nfts.end()) throw std::runtime_error("NFT not found");
        return nfts[id];
    }

    void transferNFT(const std::string& id, const std::string& newOwner) {
        if (!initialized) throw std::runtime_error("NFTManager not initialized");
        if (nfts.find(id) == nfts.end()) throw std::runtime_error("NFT not found");
        nfts[id].owner = newOwner;
    }

    void deleteNFT(const std::string& id) {
        if (!initialized) throw std::runtime_error("NFTManager not initialized");
        if (nfts.find(id) == nfts.end()) throw std::runtime_error("NFT not found");
        nfts.erase(id);
    }

private:
    bool initialized;
    std::map<std::string, NFT> nfts;
};

EMSCRIPTEN_BINDINGS(NFT) {
    class_<NFT>("NFT")
        .property("id", &NFT::id)
        .property("owner", &NFT::owner)
        .property("metadata", &NFT::metadata)
        ;
}

EMSCRIPTEN_BINDINGS(NFTManager) {
    class_<NFTManager>("NFTManager")
        .constructor<>()
        .function("initialize", &NFTManager::initialize)
        .function("isInitialized", &NFTManager::isInitialized)
        .function("createNFT", &NFTManager::createNFT)
        .function("getNFT", &NFTManager::getNFT)
        .function("transferNFT", &NFTManager::transferNFT)
        .function("deleteNFT", &NFTManager::deleteNFT)
        ;
} 