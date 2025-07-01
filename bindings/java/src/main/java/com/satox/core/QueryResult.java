package com.satox.core;

import java.util.List;
import java.util.Map;

public class QueryResult {
    private boolean success;
    private List<Map<String, Object>> rows;
    private String error;
    private int affectedRows;
    private Integer lastInsertId;

    public QueryResult() {
        this.rows = new java.util.ArrayList<>();
    }

    public QueryResult(boolean success, List<Map<String, Object>> rows, String error) {
        this.success = success;
        this.rows = rows != null ? rows : new java.util.ArrayList<>();
        this.error = error;
    }

    public boolean isSuccess() { return success; }
    public void setSuccess(boolean success) { this.success = success; }

    public List<Map<String, Object>> getRows() { return rows; }
    public void setRows(List<Map<String, Object>> rows) { this.rows = rows; }

    public String getError() { return error; }
    public void setError(String error) { this.error = error; }

    public int getAffectedRows() { return affectedRows; }
    public void setAffectedRows(int affectedRows) { this.affectedRows = affectedRows; }

    public Integer getLastInsertId() { return lastInsertId; }
    public void setLastInsertId(Integer lastInsertId) { this.lastInsertId = lastInsertId; }
} 