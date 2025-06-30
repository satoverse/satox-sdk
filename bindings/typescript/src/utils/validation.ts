/**
 * @file validation.ts
 * @brief Validation utility functions for the Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { VALIDATION_PATTERNS, ERROR_CODES } from '../constants';
import { SatoxError } from '../types';

/**
 * Validate Satox address format
 */
export function validateAddress(address: string): boolean {
  if (!address || typeof address !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.ADDRESS.test(address);
}

/**
 * Validate private key format
 */
export function validatePrivateKey(privateKey: string): boolean {
  if (!privateKey || typeof privateKey !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.PRIVATE_KEY.test(privateKey);
}

/**
 * Validate public key format
 */
export function validatePublicKey(publicKey: string): boolean {
  if (!publicKey || typeof publicKey !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.PUBLIC_KEY.test(publicKey);
}

/**
 * Validate transaction ID format
 */
export function validateTransactionId(txId: string): boolean {
  if (!txId || typeof txId !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.TRANSACTION_ID.test(txId);
}

/**
 * Validate block hash format
 */
export function validateBlockHash(hash: string): boolean {
  if (!hash || typeof hash !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.BLOCK_HASH.test(hash);
}

/**
 * Validate IPFS hash format
 */
export function validateIPFSHash(hash: string): boolean {
  if (!hash || typeof hash !== 'string') {
    return false;
  }
  return VALIDATION_PATTERNS.IPFS_HASH.test(hash);
}

/**
 * Validate amount (must be positive number)
 */
export function validateAmount(amount: number): boolean {
  return typeof amount === 'number' && amount > 0 && isFinite(amount);
}

/**
 * Validate fee (must be non-negative number)
 */
export function validateFee(fee: number): boolean {
  return typeof fee === 'number' && fee >= 0 && isFinite(fee);
}

/**
 * Validate wallet ID format
 */
export function validateWalletId(walletId: string): boolean {
  if (!walletId || typeof walletId !== 'string') {
    return false;
  }
  // UUID v4 format
  const uuidPattern = /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
  return uuidPattern.test(walletId);
}

/**
 * Validate asset ID format
 */
export function validateAssetId(assetId: string): boolean {
  if (!assetId || typeof assetId !== 'string') {
    return false;
  }
  // UUID v4 format
  const uuidPattern = /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
  return uuidPattern.test(assetId);
}

/**
 * Validate NFT ID format
 */
export function validateNFTId(nftId: string): boolean {
  if (!nftId || typeof nftId !== 'string') {
    return false;
  }
  // UUID v4 format
  const uuidPattern = /^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$/i;
  return uuidPattern.test(nftId);
}

/**
 * Validate key ID format
 */
export function validateKeyId(keyId: string): boolean {
  if (!keyId || typeof keyId !== 'string') {
    return false;
  }
  // Alphanumeric with underscores and hyphens, 3-50 characters
  const keyPattern = /^[a-zA-Z0-9_-]{3,50}$/;
  return keyPattern.test(keyId);
}

/**
 * Validate asset metadata
 */
export function validateAssetMetadata(metadata: any): boolean {
  if (!metadata || typeof metadata !== 'object') {
    return false;
  }
  
  // Required fields
  if (!metadata.name || typeof metadata.name !== 'string' || metadata.name.length === 0) {
    return false;
  }
  
  if (!metadata.symbol || typeof metadata.symbol !== 'string' || metadata.symbol.length === 0) {
    return false;
  }
  
  if (!validateAmount(metadata.total_supply)) {
    return false;
  }
  
  if (typeof metadata.decimals !== 'number' || metadata.decimals < 0 || metadata.decimals > 18) {
    return false;
  }
  
  return true;
}

/**
 * Validate NFT metadata
 */
export function validateNFTMetadata(metadata: any): boolean {
  if (!metadata || typeof metadata !== 'object') {
    return false;
  }
  
  // Required fields
  if (!metadata.name || typeof metadata.name !== 'string' || metadata.name.length === 0) {
    return false;
  }
  
  // Optional fields validation
  if (metadata.description && typeof metadata.description !== 'string') {
    return false;
  }
  
  if (metadata.image && typeof metadata.image !== 'string') {
    return false;
  }
  
  if (metadata.external_url && typeof metadata.external_url !== 'string') {
    return false;
  }
  
  if (metadata.attributes && !Array.isArray(metadata.attributes)) {
    return false;
  }
  
  if (metadata.properties && typeof metadata.properties !== 'object') {
    return false;
  }
  
  return true;
}

/**
 * Validate network type
 */
export function validateNetwork(network: string): boolean {
  const validNetworks = ['mainnet', 'testnet', 'regtest'];
  return validNetworks.includes(network);
}

/**
 * Validate port number
 */
export function validatePort(port: number): boolean {
  return typeof port === 'number' && port >= 1 && port <= 65535 && Number.isInteger(port);
}

/**
 * Validate host address
 */
export function validateHost(host: string): boolean {
  if (!host || typeof host !== 'string') {
    return false;
  }
  
  // IPv4 pattern
  const ipv4Pattern = /^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  
  // IPv6 pattern (simplified)
  const ipv6Pattern = /^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$/;
  
  // Hostname pattern
  const hostnamePattern = /^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$/;
  
  return ipv4Pattern.test(host) || ipv6Pattern.test(host) || hostnamePattern.test(host) || host === 'localhost';
}

/**
 * Validate file path
 */
export function validateFilePath(path: string): boolean {
  if (!path || typeof path !== 'string') {
    return false;
  }
  
  // Basic path validation - no null bytes, no directory traversal
  if (path.includes('\0') || path.includes('..')) {
    return false;
  }
  
  return true;
}

/**
 * Validate URL
 */
export function validateURL(url: string): boolean {
  if (!url || typeof url !== 'string') {
    return false;
  }
  
  try {
    new URL(url);
    return true;
  } catch {
    return false;
  }
}

/**
 * Validate email address
 */
export function validateEmail(email: string): boolean {
  if (!email || typeof email !== 'string') {
    return false;
  }
  
  const emailPattern = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
  return emailPattern.test(email);
}

/**
 * Validate JSON string
 */
export function validateJSON(jsonString: string): boolean {
  if (!jsonString || typeof jsonString !== 'string') {
    return false;
  }
  
  try {
    JSON.parse(jsonString);
    return true;
  } catch {
    return false;
  }
}

/**
 * Validate object properties
 */
export function validateObjectProperties(obj: any, requiredProps: string[], optionalProps: string[] = []): boolean {
  if (!obj || typeof obj !== 'object') {
    return false;
  }
  
  // Check required properties
  for (const prop of requiredProps) {
    if (!(prop in obj)) {
      return false;
    }
  }
  
  // Check that all properties are either required or optional
  const allValidProps = [...requiredProps, ...optionalProps];
  for (const prop in obj) {
    if (!allValidProps.includes(prop)) {
      return false;
    }
  }
  
  return true;
}

/**
 * Validate array elements
 */
export function validateArrayElements<T>(array: T[], validator: (item: T) => boolean): boolean {
  if (!Array.isArray(array)) {
    return false;
  }
  
  return array.every(validator);
}

/**
 * Validate string length
 */
export function validateStringLength(str: string, minLength: number, maxLength: number): boolean {
  if (typeof str !== 'string') {
    return false;
  }
  
  const length = str.length;
  return length >= minLength && length <= maxLength;
}

/**
 * Validate number range
 */
export function validateNumberRange(num: number, min: number, max: number): boolean {
  if (typeof num !== 'number' || !isFinite(num)) {
    return false;
  }
  
  return num >= min && num <= max;
}

/**
 * Validate integer
 */
export function validateInteger(num: number): boolean {
  return typeof num === 'number' && Number.isInteger(num) && isFinite(num);
}

/**
 * Validate positive integer
 */
export function validatePositiveInteger(num: number): boolean {
  return validateInteger(num) && num > 0;
}

/**
 * Validate non-negative integer
 */
export function validateNonNegativeInteger(num: number): boolean {
  return validateInteger(num) && num >= 0;
}

/**
 * Create validation error
 */
export function createValidationError(message: string, code: string = ERROR_CODES.INVALID_CONFIG): SatoxError {
  return new SatoxError(message, code);
}

/**
 * Validate with custom error message
 */
export function validateWithError<T>(
  value: T,
  validator: (value: T) => boolean,
  errorMessage: string,
  errorCode?: string
): void {
  if (!validator(value)) {
    throw createValidationError(errorMessage, errorCode);
  }
} 