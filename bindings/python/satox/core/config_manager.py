"""
Config Manager Python Bindings
"""

from typing import Dict, List, Optional, Any, Union
import satox_cpp

class ConfigManager:
    """Python wrapper for the C++ ConfigManager class."""
    
    def __init__(self):
        """Initialize the Config Manager."""
        self._manager = satox_cpp.ConfigManager()
        self._initialized = False
    
    def initialize(self) -> bool:
        """Initialize the config manager.
        
        Returns:
            bool: True if initialization was successful, False otherwise.
        """
        if not self._initialized:
            self._initialized = self._manager.initialize()
        return self._initialized
    
    def shutdown(self) -> bool:
        """Shutdown the config manager.
        
        Returns:
            bool: True if shutdown was successful, False otherwise.
        """
        if self._initialized:
            self._initialized = not self._manager.shutdown()
        return not self._initialized
    
    def load_config(self, config_path: str) -> bool:
        """Load configuration from a file.
        
        Args:
            config_path (str): Path to the configuration file
            
        Returns:
            bool: True if loading was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not config_path:
            raise ValueError("Invalid config path")
        
        return self._manager.load_config(config_path)
    
    def save_config(self, config_path: str) -> bool:
        """Save configuration to a file.
        
        Args:
            config_path (str): Path to save the configuration file
            
        Returns:
            bool: True if saving was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not config_path:
            raise ValueError("Invalid config path")
        
        return self._manager.save_config(config_path)
    
    def get_value(self, key: str, default: Any = None) -> Any:
        """Get a configuration value.
        
        Args:
            key (str): The configuration key
            default (Any, optional): Default value if key is not found
            
        Returns:
            Any: The configuration value, or default if not found
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not key:
            raise ValueError("Invalid key")
        
        return self._manager.get_value(key, default)
    
    def set_value(self, key: str, value: Any) -> bool:
        """Set a configuration value.
        
        Args:
            key (str): The configuration key
            value (Any): The value to set
            
        Returns:
            bool: True if setting was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not key:
            raise ValueError("Invalid key")
        
        return self._manager.set_value(key, value)
    
    def has_key(self, key: str) -> bool:
        """Check if a configuration key exists.
        
        Args:
            key (str): The configuration key to check
            
        Returns:
            bool: True if the key exists, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not key:
            raise ValueError("Invalid key")
        
        return self._manager.has_key(key)
    
    def remove_key(self, key: str) -> bool:
        """Remove a configuration key.
        
        Args:
            key (str): The configuration key to remove
            
        Returns:
            bool: True if removal was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not key:
            raise ValueError("Invalid key")
        
        return self._manager.remove_key(key)
    
    def get_all_keys(self) -> List[str]:
        """Get all configuration keys.
        
        Returns:
            List[str]: List of all configuration keys
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        return self._manager.get_all_keys()
    
    def get_all_values(self) -> Dict[str, Any]:
        """Get all configuration values.
        
        Returns:
            Dict[str, Any]: Dictionary of all configuration values
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        return self._manager.get_all_values()
    
    def clear_config(self) -> bool:
        """Clear all configuration values.
        
        Returns:
            bool: True if clearing was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        return self._manager.clear_config()
    
    def is_initialized(self) -> bool:
        """Check if the config manager is initialized.
        
        Returns:
            bool: True if initialized, False otherwise.
        """
        return self._initialized
    
    def get_version(self) -> str:
        """Get the version of the config manager.
        
        Returns:
            str: The version string
        """
        return self._manager.get_version()
    
    def get_config_format(self) -> str:
        """Get the current configuration format.
        
        Returns:
            str: The configuration format
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        return self._manager.get_config_format()
    
    def set_config_format(self, format: str) -> bool:
        """Set the configuration format.
        
        Args:
            format (str): The configuration format to use
            
        Returns:
            bool: True if setting was successful, False otherwise
            
        Raises:
            RuntimeError: If the manager is not initialized.
            ValueError: If the input is invalid.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        if not format:
            raise ValueError("Invalid format")
        
        return self._manager.set_config_format(format)
    
    def get_supported_formats(self) -> List[str]:
        """Get the list of supported configuration formats.
        
        Returns:
            List[str]: List of supported format names
            
        Raises:
            RuntimeError: If the manager is not initialized.
        """
        if not self._initialized:
            raise RuntimeError("Config Manager not initialized")
        
        return self._manager.get_supported_formats() 