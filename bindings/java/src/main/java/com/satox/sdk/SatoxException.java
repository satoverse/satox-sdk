package com.satox.sdk;

/**
 * Exception thrown by the Satox SDK
 */
public class SatoxException extends Exception {
    
    /**
     * Constructs a new SatoxException with the specified detail message.
     * 
     * @param message the detail message
     */
    public SatoxException(String message) {
        super(message);
    }
    
    /**
     * Constructs a new SatoxException with the specified detail message and cause.
     * 
     * @param message the detail message
     * @param cause the cause
     */
    public SatoxException(String message, Throwable cause) {
        super(message, cause);
    }
    
    /**
     * Constructs a new SatoxException with the specified cause.
     * 
     * @param cause the cause
     */
    public SatoxException(Throwable cause) {
        super(cause);
    }
} 