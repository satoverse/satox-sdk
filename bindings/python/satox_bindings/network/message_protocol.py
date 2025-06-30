from typing import Dict, List, Optional, Any
from ..core.satox_error import SatoxError
import json
import hashlib
from datetime import datetime

class MessageProtocol:
    """Handles message formatting and validation for network communication."""
    
    def __init__(self):
        """Initialize the message protocol handler."""
        self.supported_versions = ['1.0']
        self.current_version = '1.0'
        
    def create_message(self, message_type: str, data: Dict[str, Any], version: Optional[str] = None) -> Dict[str, Any]:
        """Create a formatted message.
        
        Args:
            message_type: Type of message
            data: Message data
            version: Protocol version (defaults to current version)
            
        Returns:
            Formatted message dictionary
            
        Raises:
            SatoxError: If version is not supported
        """
        if version and version not in self.supported_versions:
            raise SatoxError(f"Unsupported protocol version: {version}")
            
        version = version or self.current_version
        
        message = {
            'version': version,
            'type': message_type,
            'data': data,
            'timestamp': datetime.now().isoformat()
        }
        
        # Add message hash
        message['hash'] = self._calculate_message_hash(message)
        
        return message
        
    def validate_message(self, message: Dict[str, Any]) -> bool:
        """Validate a message.
        
        Args:
            message: Message to validate
            
        Returns:
            True if message is valid, False otherwise
            
        Raises:
            SatoxError: If message is invalid
        """
        try:
            # Check required fields
            required_fields = ['version', 'type', 'data', 'timestamp', 'hash']
            for field in required_fields:
                if field not in message:
                    raise SatoxError(f"Missing required field: {field}")
                    
            # Check version
            if message['version'] not in self.supported_versions:
                raise SatoxError(f"Unsupported protocol version: {message['version']}")
                
            # Check timestamp
            try:
                datetime.fromisoformat(message['timestamp'])
            except ValueError:
                raise SatoxError("Invalid timestamp format")
                
            # Check hash
            expected_hash = self._calculate_message_hash(message)
            if message['hash'] != expected_hash:
                raise SatoxError("Message hash mismatch")
                
            return True
            
        except Exception as e:
            raise SatoxError(f"Message validation failed: {str(e)}")
            
    def parse_message(self, message_data: str) -> Dict[str, Any]:
        """Parse a message from string data.
        
        Args:
            message_data: Message data as string
            
        Returns:
            Parsed message dictionary
            
        Raises:
            SatoxError: If message parsing fails
        """
        try:
            message = json.loads(message_data)
            self.validate_message(message)
            return message
            
        except json.JSONDecodeError:
            raise SatoxError("Invalid message format")
            
    def _calculate_message_hash(self, message: Dict[str, Any]) -> str:
        """Calculate hash of a message.
        
        Args:
            message: Message to hash
            
        Returns:
            Message hash
        """
        # Create copy without hash
        message_copy = message.copy()
        message_copy.pop('hash', None)
        
        # Convert to string and hash
        message_str = json.dumps(message_copy, sort_keys=True)
        return hashlib.sha256(message_str.encode()).hexdigest()
        
    def get_supported_versions(self) -> List[str]:
        """Get list of supported protocol versions.
        
        Returns:
            List of supported versions
        """
        return list(self.supported_versions)
        
    def get_current_version(self) -> str:
        """Get current protocol version.
        
        Returns:
            Current version
        """
        return self.current_version 