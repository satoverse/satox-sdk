from typing import Dict, Any, Optional
import os
import json
import base64
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
from ..core.error_handling import SatoxError

class WalletEncryption:
    def __init__(self, salt: Optional[bytes] = None):
        """
        Initialize the WalletEncryption with an optional salt.
        
        Args:
            salt: Optional salt for key derivation
        """
        self._salt = salt or os.urandom(16)
        self._iterations = 100000

    def _derive_key(self, password: str) -> bytes:
        """
        Derive an encryption key from a password.
        
        Args:
            password: Password to derive key from
            
        Returns:
            bytes: Derived encryption key
        """
        kdf = PBKDF2HMAC(
            algorithm=hashes.SHA256(),
            length=32,
            salt=self._salt,
            iterations=self._iterations
        )
        return base64.urlsafe_b64encode(kdf.derive(password.encode()))

    def encrypt_wallet(self, wallet_data: Dict[str, Any], password: str) -> Dict[str, Any]:
        """
        Encrypt wallet data.
        
        Args:
            wallet_data: Wallet data to encrypt
            password: Password for encryption
            
        Returns:
            Dict[str, Any]: Encrypted wallet data
            
        Raises:
            SatoxError: If encryption fails
        """
        try:
            # Convert wallet data to JSON string
            wallet_json = json.dumps(wallet_data)
            
            # Derive encryption key
            key = self._derive_key(password)
            f = Fernet(key)
            
            # Encrypt data
            encrypted_data = f.encrypt(wallet_json.encode())
            
            return {
                'encrypted_data': base64.b64encode(encrypted_data).decode(),
                'salt': base64.b64encode(self._salt).decode(),
                'iterations': self._iterations
            }
        except Exception as e:
            raise SatoxError(f"Failed to encrypt wallet: {str(e)}")

    def decrypt_wallet(self, encrypted_wallet: Dict[str, Any], password: str) -> Dict[str, Any]:
        """
        Decrypt wallet data.
        
        Args:
            encrypted_wallet: Encrypted wallet data
            password: Password for decryption
            
        Returns:
            Dict[str, Any]: Decrypted wallet data
            
        Raises:
            SatoxError: If decryption fails
        """
        try:
            # Get encryption parameters
            encrypted_data = base64.b64decode(encrypted_wallet['encrypted_data'])
            salt = base64.b64decode(encrypted_wallet['salt'])
            iterations = encrypted_wallet['iterations']
            
            # Derive encryption key
            kdf = PBKDF2HMAC(
                algorithm=hashes.SHA256(),
                length=32,
                salt=salt,
                iterations=iterations
            )
            key = base64.urlsafe_b64encode(kdf.derive(password.encode()))
            
            # Decrypt data
            f = Fernet(key)
            decrypted_data = f.decrypt(encrypted_data)
            
            # Parse JSON
            return json.loads(decrypted_data.decode())
        except Exception as e:
            raise SatoxError(f"Failed to decrypt wallet: {str(e)}")

    def change_password(self, encrypted_wallet: Dict[str, Any], old_password: str, new_password: str) -> Dict[str, Any]:
        """
        Change the encryption password.
        
        Args:
            encrypted_wallet: Encrypted wallet data
            old_password: Current password
            new_password: New password
            
        Returns:
            Dict[str, Any]: Re-encrypted wallet data
            
        Raises:
            SatoxError: If password change fails
        """
        try:
            # Decrypt with old password
            wallet_data = self.decrypt_wallet(encrypted_wallet, old_password)
            
            # Encrypt with new password
            return self.encrypt_wallet(wallet_data, new_password)
        except Exception as e:
            raise SatoxError(f"Failed to change password: {str(e)}")

    def verify_password(self, encrypted_wallet: Dict[str, Any], password: str) -> bool:
        """
        Verify if a password is correct.
        
        Args:
            encrypted_wallet: Encrypted wallet data
            password: Password to verify
            
        Returns:
            bool: True if password is correct
            
        Raises:
            SatoxError: If verification fails
        """
        try:
            self.decrypt_wallet(encrypted_wallet, password)
            return True
        except Exception:
            return False

    def get_encryption_info(self, encrypted_wallet: Dict[str, Any]) -> Dict[str, Any]:
        """
        Get information about the encryption.
        
        Args:
            encrypted_wallet: Encrypted wallet data
            
        Returns:
            Dict[str, Any]: Encryption information
        """
        return {
            'salt': encrypted_wallet['salt'],
            'iterations': encrypted_wallet['iterations'],
            'encrypted_data_length': len(encrypted_wallet['encrypted_data'])
        }

    def reencrypt_wallet(self, encrypted_wallet: Dict[str, Any], password: str, new_iterations: Optional[int] = None) -> Dict[str, Any]:
        """
        Re-encrypt wallet data with new parameters.
        
        Args:
            encrypted_wallet: Encrypted wallet data
            password: Current password
            new_iterations: Optional new number of iterations
            
        Returns:
            Dict[str, Any]: Re-encrypted wallet data
            
        Raises:
            SatoxError: If re-encryption fails
        """
        try:
            # Decrypt wallet
            wallet_data = self.decrypt_wallet(encrypted_wallet, password)
            
            # Update iterations if specified
            if new_iterations is not None:
                self._iterations = new_iterations
            
            # Generate new salt
            self._salt = os.urandom(16)
            
            # Re-encrypt wallet
            return self.encrypt_wallet(wallet_data, password)
        except Exception as e:
            raise SatoxError(f"Failed to re-encrypt wallet: {str(e)}") 