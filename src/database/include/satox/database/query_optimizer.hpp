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

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

namespace satox {
namespace database {

class QueryOptimizer {
public:
    struct QueryPlan {
        std::string query;
        std::vector<std::string> usedIndexes;
        size_t estimatedRows;
        double estimatedCost;
    };

    struct IndexInfo {
        std::string name;
        std::string table;
        std::vector<std::string> columns;
        bool isUnique;
        size_t size;
        double selectivity;
    };

    struct TableStats {
        size_t rowCount;
        size_t pageCount;
        double avgRowSize;
        std::unordered_map<std::string, double> columnSelectivity;
    };

    QueryOptimizer();
    ~QueryOptimizer();

    // Query optimization
    QueryPlan optimizeQuery(const std::string& query, const nlohmann::json& params);
    QueryPlan optimizeSelect(const std::string& table, const nlohmann::json& conditions);
    QueryPlan optimizeJoin(const std::string& table1, const std::string& table2, const nlohmann::json& joinConditions);
    QueryPlan optimizeAggregate(const std::string& table, const std::vector<std::string>& groupBy, const std::vector<std::string>& aggregate);

    // Index management
    void addIndex(const IndexInfo& index);
    void removeIndex(const std::string& indexName);
    std::vector<IndexInfo> getIndexes(const std::string& table) const;
    bool hasIndex(const std::string& table, const std::vector<std::string>& columns) const;

    // Statistics management
    void updateTableStats(const std::string& table, const TableStats& stats);
    TableStats getTableStats(const std::string& table) const;
    void updateColumnStats(const std::string& table, const std::string& column, double selectivity);

    // Cost estimation
    double estimateQueryCost(const QueryPlan& plan) const;
    double estimateJoinCost(const std::string& table1, const std::string& table2, const nlohmann::json& conditions) const;
    double estimateScanCost(const std::string& table, const nlohmann::json& conditions) const;

private:
    // Helper methods
    std::vector<IndexInfo> findApplicableIndexes(const std::string& table, const nlohmann::json& conditions) const;
    double calculateSelectivity(const nlohmann::json& conditions) const;
    std::string buildIndexScanQuery(const std::string& table, const IndexInfo& index, const nlohmann::json& conditions) const;
    std::string buildTableScanQuery(const std::string& table, const nlohmann::json& conditions) const;
    std::string buildJoinQuery(const std::string& table1, const std::string& table2, const nlohmann::json& conditions) const;

    // Member variables
    std::unordered_map<std::string, std::vector<IndexInfo>> tableIndexes_;
    std::unordered_map<std::string, TableStats> tableStats_;
};

} // namespace database
} // namespace satox 