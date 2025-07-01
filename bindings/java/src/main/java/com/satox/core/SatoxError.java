package com.satox.core;

public class SatoxError extends Exception {
    private String code;

    public SatoxError(String message) {
        super(message);
        this.code = "UNKNOWN_ERROR";
    }

    public SatoxError(String message, String code) {
        super(message);
        this.code = code;
    }

    public String getCode() { return code; }
    public void setCode(String code) { this.code = code; }
} 