/**
 * Error handling for Satox SDK
 */

export class SatoxError extends Error {
    public errorCode?: string;

    constructor(message: string, errorCode?: string) {
        super(message);
        this.name = 'SatoxError';
        this.errorCode = errorCode;
    }
}

export class InitializationError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'InitializationError';
    }
}

export class ConnectionError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'ConnectionError';
    }
}

export class OperationError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'OperationError';
    }
}

export class ValidationError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'ValidationError';
    }
}

export class TimeoutError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'TimeoutError';
    }
}

export class ResourceError extends SatoxError {
    constructor(message: string, errorCode?: string) {
        super(message, errorCode);
        this.name = 'ResourceError';
    }
} 