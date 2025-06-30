#include <emscripten/bind.h>
#include <string>

using namespace emscripten;

class CoreManager {
public:
    CoreManager() : initialized(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("CoreManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    std::string getVersion() const { return "1.0.0-wasm"; }

private:
    bool initialized;
};

EMSCRIPTEN_BINDINGS(CoreManager) {
    class_<CoreManager>("CoreManager")
        .constructor<>()
        .function("initialize", &CoreManager::initialize)
        .function("isInitialized", &CoreManager::isInitialized)
        .function("getVersion", &CoreManager::getVersion)
        ;
} 