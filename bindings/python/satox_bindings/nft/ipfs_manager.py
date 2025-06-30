from typing import Dict, List, Optional, Any, Union
from ..core.satox_error import SatoxError
import ipfshttpclient
import json
import logging
import os
from datetime import datetime

class IPFSManager:
    """Manages IPFS operations."""
    
    def __init__(self, ipfs_api_url: str = "/ip4/127.0.0.1/tcp/5001"):
        """Initialize the IPFS manager.
        
        Args:
            ipfs_api_url: IPFS API URL
        """
        self.ipfs_client = ipfshttpclient.connect(ipfs_api_url)
        self.logger = logging.getLogger(__name__)
        
    def add_file(self, file_path: str) -> str:
        """Add a file to IPFS.
        
        Args:
            file_path: Path to file
            
        Returns:
            IPFS hash
            
        Raises:
            SatoxError: If file addition fails
        """
        try:
            with open(file_path, 'rb') as f:
                return self.ipfs_client.add_bytes(f.read())
                
        except Exception as e:
            raise SatoxError(f"Failed to add file to IPFS: {str(e)}")
            
    def add_data(self, data: bytes) -> str:
        """Add data to IPFS.
        
        Args:
            data: Data to add
            
        Returns:
            IPFS hash
            
        Raises:
            SatoxError: If data addition fails
        """
        try:
            return self.ipfs_client.add_bytes(data)
            
        except Exception as e:
            raise SatoxError(f"Failed to add data to IPFS: {str(e)}")
            
    def get_file(self, ipfs_hash: str, output_path: str) -> None:
        """Get a file from IPFS.
        
        Args:
            ipfs_hash: IPFS hash
            output_path: Path to save file
            
        Raises:
            SatoxError: If file retrieval fails
        """
        try:
            data = self.ipfs_client.cat(ipfs_hash)
            
            with open(output_path, 'wb') as f:
                f.write(data)
                
        except Exception as e:
            raise SatoxError(f"Failed to get file from IPFS: {str(e)}")
            
    def get_data(self, ipfs_hash: str) -> bytes:
        """Get data from IPFS.
        
        Args:
            ipfs_hash: IPFS hash
            
        Returns:
            Data
            
        Raises:
            SatoxError: If data retrieval fails
        """
        try:
            return self.ipfs_client.cat(ipfs_hash)
            
        except Exception as e:
            raise SatoxError(f"Failed to get data from IPFS: {str(e)}")
            
    def pin_hash(self, ipfs_hash: str) -> None:
        """Pin an IPFS hash.
        
        Args:
            ipfs_hash: IPFS hash
            
        Raises:
            SatoxError: If pinning fails
        """
        try:
            self.ipfs_client.pin_add(ipfs_hash)
            
        except Exception as e:
            raise SatoxError(f"Failed to pin IPFS hash: {str(e)}")
            
    def unpin_hash(self, ipfs_hash: str) -> None:
        """Unpin an IPFS hash.
        
        Args:
            ipfs_hash: IPFS hash
            
        Raises:
            SatoxError: If unpinning fails
        """
        try:
            self.ipfs_client.pin_rm(ipfs_hash)
            
        except Exception as e:
            raise SatoxError(f"Failed to unpin IPFS hash: {str(e)}")
            
    def list_pins(self) -> List[str]:
        """List pinned IPFS hashes.
        
        Returns:
            List of IPFS hashes
        """
        try:
            return self.ipfs_client.pin_ls()['Keys'].keys()
            
        except Exception as e:
            self.logger.error(f"Failed to list pins: {str(e)}")
            return []
            
    def verify_hash(self, ipfs_hash: str) -> bool:
        """Verify if an IPFS hash exists.
        
        Args:
            ipfs_hash: IPFS hash
            
        Returns:
            True if hash exists
        """
        try:
            self.ipfs_client.cat(ipfs_hash)
            return True
            
        except Exception as e:
            self.logger.error(f"Hash verification failed: {str(e)}")
            return False
            
    def get_hash_info(self, ipfs_hash: str) -> Dict[str, Any]:
        """Get information about an IPFS hash.
        
        Args:
            ipfs_hash: IPFS hash
            
        Returns:
            Hash information
            
        Raises:
            SatoxError: If info retrieval fails
        """
        try:
            return self.ipfs_client.object_get(ipfs_hash)
            
        except Exception as e:
            raise SatoxError(f"Failed to get hash info: {str(e)}")
            
    def add_directory(self, dir_path: str) -> str:
        """Add a directory to IPFS.
        
        Args:
            dir_path: Path to directory
            
        Returns:
            IPFS hash
            
        Raises:
            SatoxError: If directory addition fails
        """
        try:
            return self.ipfs_client.add_dir(dir_path)
            
        except Exception as e:
            raise SatoxError(f"Failed to add directory to IPFS: {str(e)}")
            
    def get_directory(self, ipfs_hash: str, output_path: str) -> None:
        """Get a directory from IPFS.
        
        Args:
            ipfs_hash: IPFS hash
            output_path: Path to save directory
            
        Raises:
            SatoxError: If directory retrieval fails
        """
        try:
            # Create output directory
            os.makedirs(output_path, exist_ok=True)
            
            # Get directory contents
            dir_info = self.ipfs_client.object_get(ipfs_hash)
            
            # Download each file
            for link in dir_info['Links']:
                file_path = os.path.join(output_path, link['Name'])
                file_data = self.ipfs_client.cat(link['Hash'])
                
                with open(file_path, 'wb') as f:
                    f.write(file_data)
                    
        except Exception as e:
            raise SatoxError(f"Failed to get directory from IPFS: {str(e)}") 