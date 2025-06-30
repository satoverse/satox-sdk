/*
 * Copyright (c) 2025 Satoxcoin Core Developers
 * All rights reserved.
 *
 * This file is part of the Satox SDK.
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

#ifndef SATOX_VOTING_MANAGER_HPP
#define SATOX_VOTING_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <functional>
#include <optional>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

#include "satox/common/logger.hpp"
#include "satox/common/error_handler.hpp"
#include "satox/common/thread_safe.hpp"
#include "satox/common/health_check.hpp"
#include "satox/common/event_manager.hpp"
#include "satox/blockchain/blockchain_manager.hpp"
#include "satox/database/database_manager.hpp"
#include "satox/security/security_manager.hpp"

namespace satox {
namespace voting {

/**
 * @brief Voting types supported by the system
 */
enum class VoteType {
    BINARY,         ///< Yes/No voting
    MULTIPLE_CHOICE,///< Multiple choice voting
    RANKED_CHOICE,  ///< Ranked choice voting
    QUADRATIC,      ///< Quadratic voting
    TOKEN_WEIGHTED, ///< Token-weighted voting
    REPUTATION      ///< Reputation-based voting
};

/**
 * @brief Voting status
 */
enum class VoteStatus {
    DRAFT,          ///< Vote is in draft mode
    ACTIVE,         ///< Vote is currently active
    CLOSED,         ///< Vote has been closed
    EXECUTED,       ///< Vote result has been executed
    CANCELLED       ///< Vote was cancelled
};

/**
 * @brief Vote option structure
 */
struct VoteOption {
    std::string id;
    std::string title;
    std::string description;
    std::string metadata;  ///< JSON metadata for additional data
};

/**
 * @brief Vote structure
 */
struct Vote {
    std::string id;
    std::string title;
    std::string description;
    VoteType type;
    VoteStatus status;
    std::vector<VoteOption> options;
    std::chrono::system_clock::time_point start_time;
    std::chrono::system_clock::time_point end_time;
    std::string creator_address;
    std::string metadata;  ///< JSON metadata for additional data
    uint64_t min_quorum;
    uint64_t min_participation;
    bool allow_abstain;
    bool allow_change_vote;
    std::vector<std::string> eligible_voters;  ///< Addresses eligible to vote
    std::map<std::string, std::string> custom_rules;  ///< Custom voting rules
};

/**
 * @brief Vote result structure
 */
struct VoteResult {
    std::string vote_id;
    std::map<std::string, uint64_t> option_votes;  ///< option_id -> vote_count
    uint64_t total_votes;
    uint64_t abstain_votes;
    std::string winning_option;
    double participation_rate;
    bool quorum_met;
    std::chrono::system_clock::time_point calculated_at;
    std::string metadata;  ///< JSON metadata for additional data
};

/**
 * @brief Individual vote cast by a user
 */
struct VoteCast {
    std::string vote_id;
    std::string voter_address;
    std::vector<std::string> selected_options;  ///< For ranked choice, order matters
    std::chrono::system_clock::time_point cast_time;
    std::string signature;  ///< Cryptographic signature
    std::string metadata;   ///< JSON metadata for additional data
};

/**
 * @brief Voting statistics
 */
struct VotingStats {
    uint64_t total_votes_created;
    uint64_t total_votes_active;
    uint64_t total_votes_completed;
    uint64_t total_participants;
    double average_participation_rate;
    std::map<VoteType, uint64_t> votes_by_type;
    std::chrono::system_clock::time_point last_updated;
};

/**
 * @brief Callback function type for vote events
 */
using VoteEventCallback = std::function<void(const std::string& vote_id, const std::string& event_type, const std::string& data)>;

/**
 * @brief Voting Manager - Core voting system for Satox ecosystem
 * 
 * This component provides comprehensive voting functionality for:
 * - DAO governance
 * - Protocol upgrades
 * - Community decisions
 * - Feature proposals
 * - Token holder voting
 * - Reputation-based voting
 * - Quadratic voting
 * - Multi-signature voting
 */
class VotingManager : public common::ThreadSafe, public common::HealthCheck {
public:
    /**
     * @brief Constructor
     * @param logger Logger instance
     * @param error_handler Error handler instance
     * @param blockchain_manager Blockchain manager for on-chain voting
     * @param database_manager Database manager for vote storage
     * @param security_manager Security manager for vote verification
     */
    VotingManager(
        std::shared_ptr<common::Logger> logger,
        std::shared_ptr<common::ErrorHandler> error_handler,
        std::shared_ptr<blockchain::BlockchainManager> blockchain_manager,
        std::shared_ptr<database::DatabaseManager> database_manager,
        std::shared_ptr<security::SecurityManager> security_manager
    );

    /**
     * @brief Destructor
     */
    ~VotingManager();

    // ===== VOTE MANAGEMENT =====

    /**
     * @brief Create a new vote
     * @param vote Vote structure
     * @return Vote ID if successful, empty string otherwise
     */
    std::string createVote(const Vote& vote);

    /**
     * @brief Get vote by ID
     * @param vote_id Vote ID
     * @return Vote if found, nullopt otherwise
     */
    std::optional<Vote> getVote(const std::string& vote_id);

    /**
     * @brief Update vote
     * @param vote_id Vote ID
     * @param vote Updated vote structure
     * @return true if successful
     */
    bool updateVote(const std::string& vote_id, const Vote& vote);

    /**
     * @brief Delete vote (only if in DRAFT status)
     * @param vote_id Vote ID
     * @return true if successful
     */
    bool deleteVote(const std::string& vote_id);

    /**
     * @brief Activate a vote
     * @param vote_id Vote ID
     * @return true if successful
     */
    bool activateVote(const std::string& vote_id);

    /**
     * @brief Close a vote
     * @param vote_id Vote ID
     * @return true if successful
     */
    bool closeVote(const std::string& vote_id);

    /**
     * @brief Execute vote result
     * @param vote_id Vote ID
     * @return true if successful
     */
    bool executeVote(const std::string& vote_id);

    // ===== VOTING =====

    /**
     * @brief Cast a vote
     * @param vote_cast Vote cast structure
     * @return true if successful
     */
    bool castVote(const VoteCast& vote_cast);

    /**
     * @brief Get vote result
     * @param vote_id Vote ID
     * @return Vote result if available, nullopt otherwise
     */
    std::optional<VoteResult> getVoteResult(const std::string& vote_id);

    /**
     * @brief Check if user has voted
     * @param vote_id Vote ID
     * @param voter_address Voter address
     * @return true if user has voted
     */
    bool hasVoted(const std::string& vote_id, const std::string& voter_address);

    /**
     * @brief Get user's vote
     * @param vote_id Vote ID
     * @param voter_address Voter address
     * @return Vote cast if found, nullopt otherwise
     */
    std::optional<VoteCast> getUserVote(const std::string& vote_id, const std::string& voter_address);

    // ===== QUERIES =====

    /**
     * @brief Get all votes
     * @param status Filter by status (optional)
     * @param type Filter by type (optional)
     * @param creator Filter by creator (optional)
     * @return Vector of votes
     */
    std::vector<Vote> getVotes(
        std::optional<VoteStatus> status = std::nullopt,
        std::optional<VoteType> type = std::nullopt,
        std::optional<std::string> creator = std::nullopt
    );

    /**
     * @brief Get active votes
     * @return Vector of active votes
     */
    std::vector<Vote> getActiveVotes();

    /**
     * @brief Get votes created by address
     * @param creator_address Creator address
     * @return Vector of votes
     */
    std::vector<Vote> getVotesByCreator(const std::string& creator_address);

    /**
     * @brief Get votes where address is eligible to vote
     * @param voter_address Voter address
     * @return Vector of votes
     */
    std::vector<Vote> getEligibleVotes(const std::string& voter_address);

    /**
     * @brief Get voting statistics
     * @return Voting statistics
     */
    VotingStats getVotingStats();

    // ===== EVENTS =====

    /**
     * @brief Register vote event callback
     * @param callback Event callback function
     */
    void registerVoteEventCallback(VoteEventCallback callback);

    /**
     * @brief Unregister vote event callback
     */
    void unregisterVoteEventCallback();

    // ===== ADVANCED FEATURES =====

    /**
     * @brief Create quadratic vote
     * @param vote Base vote structure
     * @param cost_per_vote Cost per vote in tokens
     * @return Vote ID if successful, empty string otherwise
     */
    std::string createQuadraticVote(const Vote& vote, double cost_per_vote);

    /**
     * @brief Create token-weighted vote
     * @param vote Base vote structure
     * @param token_address Token contract address
     * @param min_tokens Minimum tokens required to vote
     * @return Vote ID if successful, empty string otherwise
     */
    std::string createTokenWeightedVote(const Vote& vote, const std::string& token_address, uint64_t min_tokens);

    /**
     * @brief Create reputation-based vote
     * @param vote Base vote structure
     * @param reputation_contract Reputation contract address
     * @param min_reputation Minimum reputation required to vote
     * @return Vote ID if successful, empty string otherwise
     */
    std::string createReputationVote(const Vote& vote, const std::string& reputation_contract, uint64_t min_reputation);

    /**
     * @brief Create multi-signature vote
     * @param vote Base vote structure
     * @param required_signatures Number of required signatures
     * @param signers Vector of signer addresses
     * @return Vote ID if successful, empty string otherwise
     */
    std::string createMultiSigVote(const Vote& vote, uint64_t required_signatures, const std::vector<std::string>& signers);

    // ===== UTILITIES =====

    /**
     * @brief Validate vote structure
     * @param vote Vote to validate
     * @return true if valid
     */
    bool validateVote(const Vote& vote);

    /**
     * @brief Validate vote cast
     * @param vote_cast Vote cast to validate
     * @return true if valid
     */
    bool validateVoteCast(const VoteCast& vote_cast);

    /**
     * @brief Calculate vote weight for address
     * @param vote_id Vote ID
     * @param voter_address Voter address
     * @return Vote weight
     */
    double calculateVoteWeight(const std::string& vote_id, const std::string& voter_address);

    /**
     * @brief Get vote participation rate
     * @param vote_id Vote ID
     * @return Participation rate (0.0 to 1.0)
     */
    double getParticipationRate(const std::string& vote_id);

    // ===== HEALTH CHECK =====

    /**
     * @brief Check component health
     * @return Health status
     */
    common::HealthStatus checkHealth() override;

    /**
     * @brief Get component status
     * @return Status string
     */
    std::string getStatus() override;

private:
    // ===== PRIVATE MEMBERS =====

    std::shared_ptr<common::Logger> logger_;
    std::shared_ptr<common::ErrorHandler> error_handler_;
    std::shared_ptr<blockchain::BlockchainManager> blockchain_manager_;
    std::shared_ptr<database::DatabaseManager> database_manager_;
    std::shared_ptr<security::SecurityManager> security_manager_;
    std::shared_ptr<common::EventManager> event_manager_;

    std::atomic<bool> running_;
    std::thread background_thread_;
    std::condition_variable cv_;
    std::mutex mutex_;

    VoteEventCallback event_callback_;

    // ===== PRIVATE METHODS =====

    /**
     * @brief Initialize component
     */
    void initialize();

    /**
     * @brief Shutdown component
     */
    void shutdown();

    /**
     * @brief Background processing thread
     */
    void backgroundProcessing();

    /**
     * @brief Process vote expiration
     */
    void processVoteExpiration();

    /**
     * @brief Calculate vote result
     * @param vote_id Vote ID
     * @return Vote result
     */
    VoteResult calculateVoteResult(const std::string& vote_id);

    /**
     * @brief Emit vote event
     * @param vote_id Vote ID
     * @param event_type Event type
     * @param data Event data
     */
    void emitVoteEvent(const std::string& vote_id, const std::string& event_type, const std::string& data);

    /**
     * @brief Verify vote signature
     * @param vote_cast Vote cast to verify
     * @return true if signature is valid
     */
    bool verifyVoteSignature(const VoteCast& vote_cast);

    /**
     * @brief Check voter eligibility
     * @param vote_id Vote ID
     * @param voter_address Voter address
     * @return true if eligible
     */
    bool isVoterEligible(const std::string& vote_id, const std::string& voter_address);

    /**
     * @brief Get voter token balance
     * @param token_address Token address
     * @param voter_address Voter address
     * @return Token balance
     */
    uint64_t getVoterTokenBalance(const std::string& token_address, const std::string& voter_address);

    /**
     * @brief Get voter reputation
     * @param reputation_contract Reputation contract address
     * @param voter_address Voter address
     * @return Reputation score
     */
    uint64_t getVoterReputation(const std::string& reputation_contract, const std::string& voter_address);
};

} // namespace voting
} // namespace satox

#endif // SATOX_VOTING_MANAGER_HPP 