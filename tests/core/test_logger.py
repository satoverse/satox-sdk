import unittest
import os
import json
import tempfile
from pathlib import Path
from satox.core.logger import SatoxLogger

class TestSatoxLogger(unittest.TestCase):
    def setUp(self):
        """Set up test environment."""
        self.temp_dir = tempfile.mkdtemp()
        self.config = {
            "log_dir": self.temp_dir,
            "max_bytes": 1024,  # 1KB for testing
            "backup_count": 2
        }
        self.logger = SatoxLogger(self.config)

    def tearDown(self):
        """Clean up test environment."""
        for file in os.listdir(self.temp_dir):
            os.remove(os.path.join(self.temp_dir, file))
        os.rmdir(self.temp_dir)

    def test_logger_initialization(self):
        """Test logger initialization."""
        self.assertIsNotNone(self.logger)
        self.assertEqual(self.logger.get_level(), 10)  # DEBUG level
        self.assertTrue(os.path.exists(self.logger.get_log_file()))
        self.assertTrue(os.path.exists(self.logger.get_json_log_file()))

    def test_context_management(self):
        """Test context management."""
        # Test setting context
        self.logger.set_context(user_id="123", action="test")
        self.assertEqual(self.logger.context["user_id"], "123")
        self.assertEqual(self.logger.context["action"], "test")

        # Test clearing context
        self.logger.clear_context()
        self.assertEqual(len(self.logger.context), 0)

    def test_log_levels(self):
        """Test different log levels."""
        # Test debug logging
        self.logger.debug("Debug message", extra_field="debug")
        self._verify_log_entry("DEBUG", "Debug message", extra_field="debug")

        # Test info logging
        self.logger.info("Info message", extra_field="info")
        self._verify_log_entry("INFO", "Info message", extra_field="info")

        # Test warning logging
        self.logger.warning("Warning message", extra_field="warning")
        self._verify_log_entry("WARNING", "Warning message", extra_field="warning")

        # Test error logging
        self.logger.error("Error message", extra_field="error")
        self._verify_log_entry("ERROR", "Error message", extra_field="error")

        # Test critical logging
        self.logger.critical("Critical message", extra_field="critical")
        self._verify_log_entry("CRITICAL", "Critical message", extra_field="critical")

    def test_exception_logging(self):
        """Test exception logging."""
        try:
            raise ValueError("Test exception")
        except ValueError as e:
            self.logger.exception("Exception occurred", exc_info=e)
            self._verify_log_entry("EXCEPTION", "Exception occurred")

    def test_security_logging(self):
        """Test security logging."""
        self.logger.security("Security event", event_type="auth_failure")
        self._verify_log_entry("SECURITY", "Security event", event_type="auth_failure")

    def test_performance_logging(self):
        """Test performance logging."""
        metrics = {
            "response_time": 100,
            "memory_usage": 1024
        }
        self.logger.performance("Performance metrics", metrics=metrics)
        self._verify_log_entry("PERFORMANCE", "Performance metrics", metrics=metrics)

    def test_audit_logging(self):
        """Test audit logging."""
        self.logger.audit("Audit event", action="user_login")
        self._verify_log_entry("AUDIT", "Audit event", action="user_login")

    def test_log_rotation(self):
        """Test log rotation."""
        # Fill log file to trigger rotation
        for i in range(100):
            self.logger.info(f"Test message {i}" * 100)

        # Check if rotation occurred
        log_files = list(Path(self.temp_dir).glob("satox.log*"))
        self.assertGreater(len(log_files), 1)

    def test_log_level_changes(self):
        """Test log level changes."""
        # Test setting level
        self.logger.set_level("INFO")
        self.assertEqual(self.logger.get_level(), 20)  # INFO level

        # Test getting level
        level = self.logger.get_level()
        self.assertEqual(level, 20)

    def test_structured_logging(self):
        """Test structured logging format."""
        self.logger.set_context(user_id="123", action="test")
        self.logger.info("Test message", extra_field="test")
        
        with open(self.logger.get_json_log_file(), 'r') as f:
            log_entry = json.loads(f.readline())
            
        self.assertIn("timestamp", log_entry)
        self.assertEqual(log_entry["level"], "INFO")
        self.assertEqual(log_entry["message"], "Test message")
        self.assertEqual(log_entry["context"]["user_id"], "123")
        self.assertEqual(log_entry["context"]["action"], "test")
        self.assertEqual(log_entry["extra_field"], "test")

    def _verify_log_entry(self, level: str, message: str, **kwargs):
        """Verify log entry in JSON log file.
        
        Args:
            level: Expected log level
            message: Expected message
            **kwargs: Expected additional fields
        """
        with open(self.logger.get_json_log_file(), 'r') as f:
            log_entry = json.loads(f.readline())
            
        self.assertEqual(log_entry["level"], level)
        self.assertEqual(log_entry["message"], message)
        
        for key, value in kwargs.items():
            self.assertEqual(log_entry[key], value)

if __name__ == "__main__":
    unittest.main() 