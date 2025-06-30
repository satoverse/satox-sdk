import { satox_native } from './native/satox_native';
import { ThreadSafeManager } from './thread-safe';

export interface KeyPair {
  publicKey: string;
  privateKey: string;
  address: string;
}

export interface Signature {
  signature: string;
  publicKey: string;
  message: string;
}

export interface Transaction {
  hash: string;
  version: number;
  lockTime: number;
}

export interface SignedTransaction {
  transaction: Transaction;
  signature: string;
  publicKey: string;
}

export interface EncryptedData {
  encryptedData: string;
  iv: string;
  publicKey: string;
}

export interface TokenValidation {
  valid: boolean;
  payload: string;
  expiration: number;
}

export interface SecurityStats {
  totalSignatures: number;
  totalVerifications: number;
  totalEncryptions: number;
  totalDecryptions: number;
  totalTokensGenerated: number;
  totalTokensValidated: number;
  averageSignatureTime: number;
  averageVerificationTime: number;
}

export class SecurityManager extends ThreadSafeManager {
  private manager: any;
  private stats: SecurityStats = {
    totalSignatures: 0,
    totalVerifications: 0,
    totalEncryptions: 0,
    totalDecryptions: 0,
    totalTokensGenerated: 0,
    totalTokensValidated: 0,
    averageSignatureTime: 0,
    averageVerificationTime: 0
  };

  constructor() {
    super();
    this.manager = new satox_native.SecurityManager();
  }

  /**
   * Initialize the security manager
   */
  async initialize(): Promise<boolean> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        const result = this.manager.initialize();
        this.setInitialized(result);
        this.setError(null);
        return result;
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to initialize Security Manager:', error);
        return false;
      }
    });
  }

  /**
   * Shutdown the security manager
   */
  async shutdown(): Promise<void> {
    return this.withLock(async () => {
      try {
        this.ensureNotDisposed();
        this.manager.shutdown();
        this.setInitialized(false);
        this.setError(null);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        console.error('Failed to shutdown Security Manager:', error);
        throw error;
      }
    });
  }

  /**
   * Generate a new key pair
   */
  async generateKeyPair(): Promise<KeyPair> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.generateKeyPair();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Get public key from private key
   */
  async getPublicKey(privateKey: string): Promise<string> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getPublicKey(privateKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Get private key from address
   */
  async getPrivateKey(address: string): Promise<string> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.getPrivateKey(address);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Sign a message with a private key
   */
  async signMessage(message: string, privateKey: string): Promise<Signature> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.signMessage(message, privateKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Verify a signature
   */
  async verifySignature(message: string, signature: string, publicKey: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.verifySignature(message, signature, publicKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Sign a transaction
   */
  async signTransaction(transaction: Transaction, privateKey: string): Promise<SignedTransaction> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.signTransaction(transaction, privateKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Verify a transaction signature
   */
  async verifyTransaction(transaction: Transaction, signature: string, publicKey: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.verifyTransaction(transaction, signature, publicKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Hash data using specified algorithm
   */
  async hashData(data: string, algorithm: string = 'sha256'): Promise<string> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.hashData(data, algorithm);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Validate a Satoxcoin address
   */
  async validateAddress(address: string): Promise<boolean> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateAddress(address);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return false;
      }
    });
  }

  /**
   * Generate a new address
   */
  async generateAddress(): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.generateAddress();
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Encrypt data with a public key
   */
  async encryptData(data: string, publicKey: string): Promise<EncryptedData> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.encryptData(data, publicKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Decrypt data with a private key
   */
  async decryptData(encryptedData: string, iv: string, privateKey: string): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.decryptData(encryptedData, iv, privateKey);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Generate a JWT token
   */
  async generateToken(payload: Record<string, any>, secret: string = 'default_secret', expiration: number = 3600): Promise<string> {
    return this.withLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.generateToken(payload, secret, expiration);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return '';
      }
    });
  }

  /**
   * Validate a JWT token
   */
  async validateToken(token: string, secret: string): Promise<TokenValidation> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return this.manager.validateToken(token, secret);
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        throw error;
      }
    });
  }

  /**
   * Get security statistics
   */
  async getSecurityStats(): Promise<SecurityStats> {
    return this.withReadLock(async () => {
      try {
        this.ensureInitialized();
        return { ...this.stats };
      } catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error';
        this.setError(errorMessage);
        return { ...this.stats };
      }
    });
  }

  /**
   * Reset security statistics
   */
  async resetStats(): Promise<void> {
    return this.withLock(async () => {
      this.ensureInitialized();
      this.stats = {
        totalSignatures: 0,
        totalVerifications: 0,
        totalEncryptions: 0,
        totalDecryptions: 0,
        totalTokensGenerated: 0,
        totalTokensValidated: 0,
        averageSignatureTime: 0,
        averageVerificationTime: 0
      };
    });
  }

  /**
   * Enable or disable statistics collection
   */
  async enableStats(enable: boolean): Promise<boolean> {
    return this.withLock(async () => {
      this.ensureInitialized();
      // Placeholder implementation
      return true;
    });
  }

  /**
   * Get the last error message
   */
  override getLastError(): string {
    return super.getLastError() || '';
  }

  /**
   * Clear the last error message
   */
  override async clearLastError(): Promise<void> {
    return this.withLock(async () => {
      super.clearLastError();
    });
  }
} 