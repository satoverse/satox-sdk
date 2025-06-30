/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <queue>
#include <functional>
#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace satox {

class ThreadPool {
public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        if(this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
    }

    template<class F>
    std::future<void> enqueue(F&& f) {
        std::packaged_task<void()> task(std::forward<F>(f));
        std::future<void> res = task.get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if(stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

class IPFSClient {
public:
    virtual std::string add(const std::string& data) = 0;
    virtual std::string get(const std::string& hash) = 0;
    virtual ~IPFSClient() = default;
};

class NFT {
public:
    std::string id;
    std::string name;
    std::string description;
    std::string owner;
    std::string collection;
    std::unordered_map<std::string, std::string> metadata;
};

class NFTEventListener {
public:
    virtual void onNFTEvent(const NFT& nft, const std::string& event) = 0;
    virtual ~NFTEventListener() = default;
};

class NFTManager {
public:
    static NFTManager& getInstance() {
        static NFTManager instance;
        return instance;
    }

    bool initialize() {
        return impl_->initialize();
    }

    void shutdown() {
        impl_->shutdown();
    }

    std::string createNFT(const NFT& nft) {
        return impl_->createNFT(nft);
    }

    void createNFTs(const std::vector<NFT>& nfts) {
        impl_->createNFTs(nfts);
    }

    NFT getNFT(const std::string& nftId) {
        return impl_->getNFT(nftId);
    }

    std::vector<NFT> getNFTsByOwner(const std::string& owner) {
        return impl_->getNFTsByOwner(owner);
    }

    bool transferNFT(const std::string& nftId, const std::string& from, const std::string& to) {
        return impl_->transferNFT(nftId, from, to);
    }

    void addEventListener(NFTEventListener* listener) {
        impl_->addEventListener(listener);
    }

    void removeEventListener(NFTEventListener* listener) {
        impl_->removeEventListener(listener);
    }

private:
    class NFTManagerImpl {
    public:
        NFTManagerImpl() 
            : threadPool_(4)
            , max_cache_size_(1000)
            , initialized_(false)
            , should_stop_(false) {
            startCleanupThread();
        }

        bool initialize() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (initialized_) return true;

            // Initialize IPFS pool
            for (int i = 0; i < 4; ++i) {
                ipfsPool_.clients.push_back(createIPFSClient());
            }

            initialized_ = true;
            return true;
        }

        void shutdown() {
            std::lock_guard<std::mutex> lock(mutex_);
            should_stop_ = true;
            cleanup_cv_.notify_one();
            if (cleanup_thread_.joinable()) {
                cleanup_thread_.join();
            }
        }

        std::string createNFT(const NFT& nft) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!initialized_) {
                throw std::runtime_error("NFTManager not initialized");
            }

            if (nfts_.find(nft.id) != nfts_.end()) {
                throw std::runtime_error("NFT already exists");
            }

            // Store metadata in IPFS
            std::string metadataJson = serializeMetadata(nft.metadata);
            std::string metadataCid = storeInIPFS(metadataJson);

            // Create NFT on blockchain
            NFT nftCopy = nft;
            nftCopy.metadata["ipfs_cid"] = metadataCid;
            nfts_[nft.id] = nftCopy;

            // Update indices
            ownerNFTs_[nft.owner].push_back(nft.id);
            if (!nft.collection.empty()) {
                collectionNFTs_[nft.collection].push_back(nft.id);
            }

            // Notify listeners asynchronously
            notifyListenersAsync(nftCopy, "created");

            return nft.id;
        }

        void createNFTs(const std::vector<NFT>& nfts) {
            std::vector<std::future<void>> futures;
            for (const auto& nft : nfts) {
                futures.push_back(threadPool_.enqueue([this, nft]() {
                    createNFT(nft);
                }));
            }

            for (auto& future : futures) {
                future.get();
            }
        }

        NFT getNFT(const std::string& nftId) {
            {
                std::shared_lock<std::shared_mutex> lock(mutex_);
                auto cacheIt = cache_.find(nftId);
                if (cacheIt != cache_.end()) {
                    cacheIt->second.lastAccessed = std::chrono::system_clock::now();
                    return cacheIt->second.nft;
                }
            }

            std::unique_lock<std::shared_mutex> lock(mutex_);
            auto it = nfts_.find(nftId);
            if (it == nfts_.end()) {
                throw std::runtime_error("NFT not found");
            }

            // Update cache
            CacheEntry entry{it->second, std::chrono::system_clock::now()};
            cache_[nftId] = entry;

            // Implement cache size limit
            if (cache_.size() > max_cache_size_) {
                cleanupCache();
            }

            return it->second;
        }

        std::vector<NFT> getNFTsByOwner(const std::string& owner) {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            auto it = ownerNFTs_.find(owner);
            if (it == ownerNFTs_.end()) {
                return {};
            }

            std::vector<NFT> result;
            result.reserve(it->second.size());
            for (const auto& nftId : it->second) {
                result.push_back(nfts_[nftId]);
            }
            return result;
        }

        bool transferNFT(const std::string& nftId, const std::string& from, const std::string& to) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            auto it = nfts_.find(nftId);
            if (it == nfts_.end()) {
                return false;
            }

            if (it->second.owner != from) {
                return false;
            }

            // Update NFT
            it->second.owner = to;

            // Update indices
            auto& fromNFTs = ownerNFTs_[from];
            fromNFTs.erase(std::remove(fromNFTs.begin(), fromNFTs.end(), nftId), fromNFTs.end());
            ownerNFTs_[to].push_back(nftId);

            // Update cache
            auto cacheIt = cache_.find(nftId);
            if (cacheIt != cache_.end()) {
                cacheIt->second.nft = it->second;
                cacheIt->second.lastAccessed = std::chrono::system_clock::now();
            }

            // Notify listeners asynchronously
            notifyListenersAsync(it->second, "transferred");

            return true;
        }

        void addEventListener(NFTEventListener* listener) {
            std::lock_guard<std::mutex> lock(mutex_);
            listeners_.push_back(listener);
        }

        void removeEventListener(NFTEventListener* listener) {
            std::lock_guard<std::mutex> lock(mutex_);
            listeners_.erase(
                std::remove(listeners_.begin(), listeners_.end(), listener),
                listeners_.end()
            );
        }

    private:
        struct CacheEntry {
            NFT nft;
            std::chrono::system_clock::time_point lastAccessed;
        };

        struct IPFSPool {
            std::vector<std::shared_ptr<IPFSClient>> clients;
            std::mutex mutex;
            size_t currentIndex = 0;

            std::shared_ptr<IPFSClient> getClient() {
                std::lock_guard<std::mutex> lock(mutex);
                auto client = clients[currentIndex];
                currentIndex = (currentIndex + 1) % clients.size();
                return client;
            }
        };

        void startCleanupThread() {
            cleanup_thread_ = std::thread([this]() {
                while (!should_stop_) {
                    {
                        std::unique_lock<std::mutex> lock(cleanup_mutex_);
                        cleanup_cv_.wait_for(lock, std::chrono::minutes(5), [this]() {
                            return should_stop_;
                        });
                    }
                    cleanupCache();
                }
            });
        }

        void cleanupCache() {
            std::lock_guard<std::mutex> lock(cleanup_mutex_);
            
            auto now = std::chrono::system_clock::now();
            std::vector<std::string> toRemove;

            for (const auto& entry : cache_) {
                auto age = std::chrono::duration_cast<std::chrono::minutes>(
                    now - entry.second.lastAccessed
                );
                if (age.count() > 30) { // Remove entries older than 30 minutes
                    toRemove.push_back(entry.first);
                }
            }

            for (const auto& key : toRemove) {
                cache_.erase(key);
            }
        }

        void notifyListenersAsync(const NFT& nft, const std::string& event) {
            threadPool_.enqueue([this, nft, event]() {
                for (auto listener : listeners_) {
                    try {
                        listener->onNFTEvent(nft, event);
                    } catch (const std::exception& e) {
                        // Log error but continue with other listeners
                        lastError_ = e.what();
                    }
                }
            });
        }

        std::string storeInIPFS(const std::string& data) {
            auto client = ipfsPool_.getClient();
            return client->add(data);
        }

        std::string serializeMetadata(const std::unordered_map<std::string, std::string>& metadata) {
            std::stringstream ss;
            ss << "{";
            bool first = true;
            for (const auto& pair : metadata) {
                if (!first) ss << ",";
                ss << "\"" << pair.first << "\":\"" << pair.second << "\"";
                first = false;
            }
            ss << "}";
            return ss.str();
        }

        std::shared_ptr<IPFSClient> createIPFSClient() {
            // Implementation depends on the actual IPFS client library
            return nullptr;
        }

        ThreadPool threadPool_;
        IPFSPool ipfsPool_;
        std::unordered_map<std::string, NFT> nfts_;
        std::unordered_map<std::string, CacheEntry> cache_;
        std::unordered_map<std::string, std::vector<std::string>> ownerNFTs_;
        std::unordered_map<std::string, std::vector<std::string>> collectionNFTs_;
        std::vector<NFTEventListener*> listeners_;
        std::shared_mutex mutex_;
        std::mutex cleanup_mutex_;
        std::condition_variable cleanup_cv_;
        std::thread cleanup_thread_;
        bool initialized_;
        bool should_stop_;
        const size_t max_cache_size_;
        std::string lastError_;
    };

    NFTManager() : impl_(std::make_unique<NFTManagerImpl>()) {}
    ~NFTManager() = default;

    std::unique_ptr<NFTManagerImpl> impl_;
};

} // namespace satox 