"""
Satox SDK IPFS Module
"""

from typing import Dict, List, Optional, Any, Union, BinaryIO
from datetime import datetime
import json
import hashlib
import os
import mimetypes
from pathlib import Path
import aiohttp
import aiofiles
import asyncio

class IPFSContent:
    """Represents IPFS content"""
    
    def __init__(self,
                 cid: str,
                 name: str,
                 size: int,
                 content_type: str,
                 created_at: Optional[datetime] = None,
                 metadata: Optional[Dict[str, Any]] = None):
        self.cid = cid
        self.name = name
        self.size = size
        self.content_type = content_type
        self.created_at = created_at or datetime.utcnow()
        self.metadata = metadata or {}
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert content to dictionary"""
        return {
            'cid': self.cid,
            'name': self.name,
            'size': self.size,
            'content_type': self.content_type,
            'created_at': self.created_at.isoformat(),
            'metadata': self.metadata
        }
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'IPFSContent':
        """Create content from dictionary"""
        return cls(
            cid=data['cid'],
            name=data['name'],
            size=data['size'],
            content_type=data['content_type'],
            created_at=datetime.fromisoformat(data['created_at']),
            metadata=data.get('metadata')
        )

class IPFSManager:
    """Manages IPFS operations"""
    
    def __init__(self, api_url: str = "http://localhost:5001"):
        self._api_url = api_url
        self._session: Optional[aiohttp.ClientSession] = None
        self._last_error = ""
    
    async def initialize(self) -> bool:
        """
        Initialize IPFS manager
        
        Returns:
            bool: True if initialization successful, False otherwise
        """
        try:
            self._session = aiohttp.ClientSession()
            return True
        except Exception as e:
            self._last_error = f"Failed to initialize IPFS manager: {str(e)}"
            return False
    
    async def shutdown(self) -> None:
        """Shutdown IPFS manager"""
        if self._session:
            await self._session.close()
            self._session = None
    
    async def add_file(self,
                      file_path: Union[str, Path],
                      metadata: Optional[Dict[str, Any]] = None) -> Optional[IPFSContent]:
        """
        Add file to IPFS
        
        Args:
            file_path: Path to file
            metadata: Additional metadata
            
        Returns:
            Optional[IPFSContent]: IPFS content if successful, None otherwise
        """
        try:
            if not self._session:
                self._last_error = "IPFS manager not initialized"
                return None
            
            file_path = Path(file_path)
            if not file_path.exists():
                self._last_error = f"File not found: {file_path}"
                return None
            
            # Get file info
            file_size = file_path.stat().st_size
            content_type = mimetypes.guess_type(str(file_path))[0] or 'application/octet-stream'
            
            # Prepare form data
            data = aiohttp.FormData()
            data.add_field('file',
                         open(file_path, 'rb'),
                         filename=file_path.name,
                         content_type=content_type)
            
            if metadata:
                data.add_field('metadata', json.dumps(metadata))
            
            # Add to IPFS
            async with self._session.post(f"{self._api_url}/api/v0/add", data=data) as response:
                if response.status != 200:
                    self._last_error = f"Failed to add file: {await response.text()}"
                    return None
                
                result = await response.json()
                
                return IPFSContent(
                    cid=result['Hash'],
                    name=file_path.name,
                    size=file_size,
                    content_type=content_type,
                    metadata=metadata
                )
            
        except Exception as e:
            self._last_error = f"Failed to add file: {str(e)}"
            return None
    
    async def get_file(self,
                      cid: str,
                      output_path: Union[str, Path]) -> bool:
        """
        Get file from IPFS
        
        Args:
            cid: Content ID
            output_path: Path to save file
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._session:
                self._last_error = "IPFS manager not initialized"
                return False
            
            output_path = Path(output_path)
            output_path.parent.mkdir(parents=True, exist_ok=True)
            
            # Get file from IPFS
            async with self._session.get(f"{self._api_url}/api/v0/cat?arg={cid}") as response:
                if response.status != 200:
                    self._last_error = f"Failed to get file: {await response.text()}"
                    return False
                
                # Save file
                async with aiofiles.open(output_path, 'wb') as f:
                    await f.write(await response.read())
                
                return True
            
        except Exception as e:
            self._last_error = f"Failed to get file: {str(e)}"
            return False
    
    async def pin_content(self, cid: str) -> bool:
        """
        Pin content in IPFS
        
        Args:
            cid: Content ID
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._session:
                self._last_error = "IPFS manager not initialized"
                return False
            
            # Pin content
            async with self._session.post(f"{self._api_url}/api/v0/pin/add?arg={cid}") as response:
                if response.status != 200:
                    self._last_error = f"Failed to pin content: {await response.text()}"
                    return False
                
                return True
            
        except Exception as e:
            self._last_error = f"Failed to pin content: {str(e)}"
            return False
    
    async def unpin_content(self, cid: str) -> bool:
        """
        Unpin content from IPFS
        
        Args:
            cid: Content ID
            
        Returns:
            bool: True if successful, False otherwise
        """
        try:
            if not self._session:
                self._last_error = "IPFS manager not initialized"
                return False
            
            # Unpin content
            async with self._session.post(f"{self._api_url}/api/v0/pin/rm?arg={cid}") as response:
                if response.status != 200:
                    self._last_error = f"Failed to unpin content: {await response.text()}"
                    return False
                
                return True
            
        except Exception as e:
            self._last_error = f"Failed to unpin content: {str(e)}"
            return False
    
    def get_last_error(self) -> str:
        """
        Get the last error message
        
        Returns:
            str: Last error message
        """
        return self._last_error
    
    def clear_last_error(self) -> None:
        """Clear the last error message"""
        self._last_error = ""

# Export the manager instance
ipfs_manager = IPFSManager()
