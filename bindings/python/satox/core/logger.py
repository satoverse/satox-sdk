import logging
import json
import os
import sys
from datetime import datetime
from logging.handlers import RotatingFileHandler
from typing import Dict, Any, Optional, Union
from pathlib import Path

class SatoxLogger:
    """Enhanced logging system for Satox SDK with structured JSON logging and context preservation."""
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize the logger with configuration.
        
        Args:
            config: Optional configuration dictionary with logging settings
        """
        self.config = config or {}
        self.logger = logging.getLogger("satox")
        self.logger.setLevel(logging.DEBUG)
        
        # Set up log directory
        self.log_dir = Path(self.config.get("log_dir", "logs"))
        self.log_dir.mkdir(parents=True, exist_ok=True)
        
        # Configure handlers
        self._setup_handlers()
        
        # Initialize context
        self.context = {}
        
    def _setup_handlers(self):
        """Set up logging handlers with rotation and formatting."""
        # Console handler for development
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging.INFO)
        console_formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        console_handler.setFormatter(console_formatter)
        
        # File handler with rotation
        log_file = self.log_dir / "satox.log"
        file_handler = RotatingFileHandler(
            log_file,
            maxBytes=self.config.get("max_bytes", 10 * 1024 * 1024),  # 10MB
            backupCount=self.config.get("backup_count", 5)
        )
        file_handler.setLevel(logging.DEBUG)
        file_formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        file_handler.setFormatter(file_formatter)
        
        # JSON file handler for structured logging
        json_log_file = self.log_dir / "satox.json"
        json_handler = RotatingFileHandler(
            json_log_file,
            maxBytes=self.config.get("max_bytes", 10 * 1024 * 1024),
            backupCount=self.config.get("backup_count", 5)
        )
        json_handler.setLevel(logging.DEBUG)
        
        # Add handlers to logger
        self.logger.addHandler(console_handler)
        self.logger.addHandler(file_handler)
        self.logger.addHandler(json_handler)
        
    def _format_json_log(self, level: str, message: str, **kwargs) -> str:
        """Format log entry as JSON.
        
        Args:
            level: Log level
            message: Log message
            **kwargs: Additional fields to include in log
            
        Returns:
            JSON formatted log entry
        """
        log_entry = {
            "timestamp": datetime.utcnow().isoformat(),
            "level": level,
            "message": message,
            "context": self.context,
            **kwargs
        }
        return json.dumps(log_entry)
        
    def set_context(self, **kwargs):
        """Set context for subsequent log entries.
        
        Args:
            **kwargs: Context key-value pairs
        """
        self.context.update(kwargs)
        
    def clear_context(self):
        """Clear the current logging context."""
        self.context.clear()
        
    def debug(self, message: str, **kwargs):
        """Log debug message with context.
        
        Args:
            message: Debug message
            **kwargs: Additional fields to include in log
        """
        self.logger.debug(self._format_json_log("DEBUG", message, **kwargs))
        
    def info(self, message: str, **kwargs):
        """Log info message with context.
        
        Args:
            message: Info message
            **kwargs: Additional fields to include in log
        """
        self.logger.info(self._format_json_log("INFO", message, **kwargs))
        
    def warning(self, message: str, **kwargs):
        """Log warning message with context.
        
        Args:
            message: Warning message
            **kwargs: Additional fields to include in log
        """
        self.logger.warning(self._format_json_log("WARNING", message, **kwargs))
        
    def error(self, message: str, **kwargs):
        """Log error message with context.
        
        Args:
            message: Error message
            **kwargs: Additional fields to include in log
        """
        self.logger.error(self._format_json_log("ERROR", message, **kwargs))
        
    def critical(self, message: str, **kwargs):
        """Log critical message with context.
        
        Args:
            message: Critical message
            **kwargs: Additional fields to include in log
        """
        self.logger.critical(self._format_json_log("CRITICAL", message, **kwargs))
        
    def exception(self, message: str, exc_info: Optional[Exception] = None, **kwargs):
        """Log exception with context.
        
        Args:
            message: Exception message
            exc_info: Exception information
            **kwargs: Additional fields to include in log
        """
        self.logger.exception(
            self._format_json_log("EXCEPTION", message, **kwargs),
            exc_info=exc_info
        )
        
    def security(self, message: str, **kwargs):
        """Log security-related message with context.
        
        Args:
            message: Security message
            **kwargs: Additional fields to include in log
        """
        self.logger.warning(
            self._format_json_log("SECURITY", message, **kwargs)
        )
        
    def performance(self, message: str, metrics: Dict[str, Any], **kwargs):
        """Log performance metrics with context.
        
        Args:
            message: Performance message
            metrics: Performance metrics
            **kwargs: Additional fields to include in log
        """
        self.logger.info(
            self._format_json_log("PERFORMANCE", message, metrics=metrics, **kwargs)
        )
        
    def audit(self, message: str, **kwargs):
        """Log audit message with context.
        
        Args:
            message: Audit message
            **kwargs: Additional fields to include in log
        """
        self.logger.info(
            self._format_json_log("AUDIT", message, **kwargs)
        )
        
    def get_log_file(self) -> Path:
        """Get the path to the current log file.
        
        Returns:
            Path to the current log file
        """
        return self.log_dir / "satox.log"
    
    def get_json_log_file(self) -> Path:
        """Get the path to the current JSON log file.
        
        Returns:
            Path to the current JSON log file
        """
        return self.log_dir / "satox.json"
    
    def rotate_logs(self):
        """Rotate log files manually."""
        for handler in self.logger.handlers:
            if isinstance(handler, RotatingFileHandler):
                handler.doRollover()
    
    def set_level(self, level: Union[str, int]):
        """Set the logging level."""
        self.logger.setLevel(level)
    
    def get_level(self) -> int:
        """Get the current logging level."""
        return self.logger.level 