"""Config Manager for Satox SDK.
Handles configuration management and settings.
"""

from typing import Optional, Dict, Any
import json
import os
import yaml

class ConfigManager:
    """Manages configuration settings and environment variables."""
    
    def __init__(self):
        """Initialize the config manager."""
        self._initialized = False
        self._last_error = ""
        self._config = {}
        self._env_vars = {}

    def initialize(self) -> bool:
        """Initialize the config manager."""
        try:
            self._initialized = True
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def shutdown(self) -> bool:
        """Shutdown the config manager."""
        try:
            self._initialized = False
            self._config.clear()
            self._env_vars.clear()
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def load_config(self, config_path: str) -> bool:
        """Load configuration from a file."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return False
        try:
            if not os.path.exists(config_path):
                self._last_error = f"Config file not found: {config_path}"
                return False

            with open(config_path, "r") as f:
                if config_path.endswith(".json"):
                    self._config = json.load(f)
                elif config_path.endswith((".yml", ".yaml")):
                    self._config = yaml.safe_load(f)
                else:
                    self._last_error = "Unsupported config file format"
                    return False

            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def save_config(self, config_path: str) -> bool:
        """Save configuration to a file."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return False
        try:
            with open(config_path, "w") as f:
                if config_path.endswith(".json"):
                    json.dump(self._config, f, indent=4)
                elif config_path.endswith((".yml", ".yaml")):
                    yaml.dump(self._config, f)
                else:
                    self._last_error = "Unsupported config file format"
                    return False

            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_config(self, key: str, default: Any = None) -> Any:
        """Get a configuration value."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return default
        return self._config.get(key, default)

    def set_config(self, key: str, value: Any) -> bool:
        """Set a configuration value."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return False
        try:
            self._config[key] = value
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_all_config(self) -> Dict[str, Any]:
        """Get all configuration values."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return {}
        return self._config.copy()

    def load_env_vars(self, prefix: str = "SATOX_") -> bool:
        """Load environment variables with a specific prefix."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return False
        try:
            for key, value in os.environ.items():
                if key.startswith(prefix):
                    config_key = key[len(prefix):].lower()
                    self._config[config_key] = value
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_env_var(self, key: str, default: Any = None) -> Any:
        """Get an environment variable."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return default
        return os.environ.get(key, default)

    def set_env_var(self, key: str, value: str) -> bool:
        """Set an environment variable."""
        if not self._initialized:
            self._last_error = "Config manager not initialized"
            return False
        try:
            os.environ[key] = value
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_last_error(self) -> str:
        """Get the last error message."""
        return self._last_error

    def clear_last_error(self) -> None:
        """Clear the last error message."""
        self._last_error = ""
