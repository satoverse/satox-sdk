from typing import Dict, List, Optional, Any
from ..core.satox_error import SatoxError
import os
import json
import logging
from datetime import datetime
from cryptography.fernet import Fernet
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC
import base64

class KeyManager:
    """Handles key management and storage."""
    
    def __init__(self, storage_dir: str):
        """Initialize the key manager.
        
        Args:
            storage_dir: Directory to store keys
        """
        self.storage_dir = storage_dir
        self.keys: Dict[str, Dict[str, Any]] = {}  # key_id -> key_info
        self.logger = logging.getLogger(__name__)
        
        # Create storage directory if it doesn't exist
        os.makedirs(storage_dir, exist_ok=True)
        
    def generate_key(self, key_id: str, key_type: str, metadata: Optional[Dict[str, Any]] = None) -> bytes:
        """Generate a new key.
        
        Args:
            key_id: Unique identifier for the key
            key_type: Type of key (e.g., 'symmetric', 'asymmetric')
            metadata: Additional key metadata
            
        Returns:
            Generated key
            
        Raises:
            SatoxError: If key generation fails
        """
        if key_id in self.keys:
            raise SatoxError(f"Key {key_id} already exists")
            
        try:
            # Generate key based on type
            if key_type == 'symmetric':
                key = Fernet.generate_key()
            else:
                raise SatoxError(f"Unsupported key type: {key_type}")
                
            # Store key info
            self.keys[key_id] = {
                'id': key_id,
                'type': key_type,
                'created_at': datetime.now().isoformat(),
                'metadata': metadata or {}
            }
            
            # Save key
            self._save_key(key_id, key)
            
            return key
            
        except Exception as e:
            raise SatoxError(f"Failed to generate key: {str(e)}")
            
    def import_key(self, key_id: str, key_data: bytes, key_type: str, metadata: Optional[Dict[str, Any]] = None) -> None:
        """Import an existing key.
        
        Args:
            key_id: Unique identifier for the key
            key_data: Key data
            key_type: Type of key
            metadata: Additional key metadata
            
        Raises:
            SatoxError: If key import fails
        """
        if key_id in self.keys:
            raise SatoxError(f"Key {key_id} already exists")
            
        try:
            # Store key info
            self.keys[key_id] = {
                'id': key_id,
                'type': key_type,
                'created_at': datetime.now().isoformat(),
                'metadata': metadata or {}
            }
            
            # Save key
            self._save_key(key_id, key_data)
            
        except Exception as e:
            raise SatoxError(f"Failed to import key: {str(e)}")
            
    def get_key(self, key_id: str) -> bytes:
        """Get a key.
        
        Args:
            key_id: Key identifier
            
        Returns:
            Key data
            
        Raises:
            SatoxError: If key does not exist
        """
        if key_id not in self.keys:
            raise SatoxError(f"Key {key_id} does not exist")
            
        try:
            return self._load_key(key_id)
            
        except Exception as e:
            raise SatoxError(f"Failed to get key: {str(e)}")
            
    def delete_key(self, key_id: str) -> None:
        """Delete a key.
        
        Args:
            key_id: Key identifier
            
        Raises:
            SatoxError: If key does not exist
        """
        if key_id not in self.keys:
            raise SatoxError(f"Key {key_id} does not exist")
            
        try:
            # Delete key file
            key_path = os.path.join(self.storage_dir, f"{key_id}.key")
            if os.path.exists(key_path):
                os.remove(key_path)
                
            # Remove key info
            del self.keys[key_id]
            
        except Exception as e:
            raise SatoxError(f"Failed to delete key: {str(e)}")
            
    def list_keys(self) -> List[Dict[str, Any]]:
        """Get list of all keys.
        
        Returns:
            List of key information
        """
        return [key.copy() for key in self.keys.values()]
        
    def get_key_info(self, key_id: str) -> Dict[str, Any]:
        """Get information about a key.
        
        Args:
            key_id: Key identifier
            
        Returns:
            Key information
            
        Raises:
            SatoxError: If key does not exist
        """
        if key_id not in self.keys:
            raise SatoxError(f"Key {key_id} does not exist")
            
        return self.keys[key_id].copy()
        
    def update_key_metadata(self, key_id: str, metadata: Dict[str, Any]) -> None:
        """Update key metadata.
        
        Args:
            key_id: Key identifier
            metadata: New metadata
            
        Raises:
            SatoxError: If key does not exist
        """
        if key_id not in self.keys:
            raise SatoxError(f"Key {key_id} does not exist")
            
        self.keys[key_id]['metadata'].update(metadata)
        
    def _save_key(self, key_id: str, key_data: bytes) -> None:
        """Save key to storage.
        
        Args:
            key_id: Key identifier
            key_data: Key data
        """
        key_path = os.path.join(self.storage_dir, f"{key_id}.key")
        
        with open(key_path, 'wb') as f:
            f.write(key_data)
            
    def _load_key(self, key_id: str) -> bytes:
        """Load key from storage.
        
        Args:
            key_id: Key identifier
            
        Returns:
            Key data
        """
        key_path = os.path.join(self.storage_dir, f"{key_id}.key")
        
        with open(key_path, 'rb') as f:
            return f.read()
            
    def backup_keys(self, backup_dir: str) -> None:
        """Backup all keys.
        
        Args:
            backup_dir: Directory to store backup
            
        Raises:
            SatoxError: If backup fails
        """
        try:
            # Create backup directory
            os.makedirs(backup_dir, exist_ok=True)
            
            # Backup keys
            for key_id in self.keys:
                key_data = self._load_key(key_id)
                backup_path = os.path.join(backup_dir, f"{key_id}.key")
                
                with open(backup_path, 'wb') as f:
                    f.write(key_data)
                    
            # Backup key info
            info_path = os.path.join(backup_dir, 'keys.json')
            with open(info_path, 'w') as f:
                json.dump(self.keys, f, indent=2)
                
        except Exception as e:
            raise SatoxError(f"Failed to backup keys: {str(e)}")
            
    def restore_keys(self, backup_dir: str) -> None:
        """Restore keys from backup.
        
        Args:
            backup_dir: Directory containing backup
            
        Raises:
            SatoxError: If restore fails
        """
        try:
            # Load key info
            info_path = os.path.join(backup_dir, 'keys.json')
            with open(info_path, 'r') as f:
                self.keys = json.load(f)
                
            # Restore keys
            for key_id in self.keys:
                backup_path = os.path.join(backup_dir, f"{key_id}.key")
                
                with open(backup_path, 'rb') as f:
                    key_data = f.read()
                    
                self._save_key(key_id, key_data)
                
        except Exception as e:
            raise SatoxError(f"Failed to restore keys: {str(e)}") 