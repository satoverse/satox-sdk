package com.satox.quantum;

public class SatoxException extends Exception {
    public SatoxException(String message) {
        super(message);
    }

    public SatoxException(String message, Throwable cause) {
        super(message, cause);
    }
}