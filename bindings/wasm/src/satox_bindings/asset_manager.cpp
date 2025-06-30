#include <emscripten/bind.h>
#include <string>
#include <map>
#include <vector>

using namespace emscripten;

class Asset {
public:
    Asset(const std::string& id, const std::string& name, double amount)
        : id(id), name(name), amount(amount) {}

    std::string getId() const { return id; }
    std::string getName() const { return name; }
    double getAmount() const { return amount; }
    void setAmount(double newAmount) { amount = newAmount; }

private:
    std::string id;
    std::string name;
    double amount;
};

class AssetManager {
public:
    AssetManager() : initialized(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("AssetManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    std::string createAsset(const std::string& name, double amount) {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        std::string id = "asset_" + std::to_string(assets.size());
        assets[id] = Asset(id, name, amount);
        return id;
    }

    Asset getAsset(const std::string& id) {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        if (assets.find(id) == assets.end()) throw std::runtime_error("Asset not found");
        return assets[id];
    }

    void updateAsset(const std::string& id, double amount) {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        if (assets.find(id) == assets.end()) throw std::runtime_error("Asset not found");
        assets[id].setAmount(amount);
    }

    void deleteAsset(const std::string& id) {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        if (assets.find(id) == assets.end()) throw std::runtime_error("Asset not found");
        assets.erase(id);
    }

    std::vector<std::string> listAssets() const {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        std::vector<std::string> result;
        for (const auto& asset : assets) {
            result.push_back(asset.first);
        }
        return result;
    }

    void transferAsset(const std::string& fromId, const std::string& toId, double amount) {
        if (!initialized) throw std::runtime_error("AssetManager not initialized");
        if (assets.find(fromId) == assets.end()) throw std::runtime_error("Source asset not found");
        if (assets.find(toId) == assets.end()) throw std::runtime_error("Destination asset not found");
        
        Asset& fromAsset = assets[fromId];
        Asset& toAsset = assets[toId];
        
        if (fromAsset.getAmount() < amount) throw std::runtime_error("Insufficient balance");
        
        fromAsset.setAmount(fromAsset.getAmount() - amount);
        toAsset.setAmount(toAsset.getAmount() + amount);
    }

private:
    bool initialized;
    std::map<std::string, Asset> assets;
};

EMSCRIPTEN_BINDINGS(AssetManager) {
    class_<Asset>("Asset")
        .constructor<std::string, std::string, double>()
        .function("getId", &Asset::getId)
        .function("getName", &Asset::getName)
        .function("getAmount", &Asset::getAmount)
        .function("setAmount", &Asset::setAmount)
        ;

    class_<AssetManager>("AssetManager")
        .constructor<>()
        .function("initialize", &AssetManager::initialize)
        .function("isInitialized", &AssetManager::isInitialized)
        .function("createAsset", &AssetManager::createAsset)
        .function("getAsset", &AssetManager::getAsset)
        .function("updateAsset", &AssetManager::updateAsset)
        .function("deleteAsset", &AssetManager::deleteAsset)
        .function("listAssets", &AssetManager::listAssets)
        .function("transferAsset", &AssetManager::transferAsset)
        ;
} 