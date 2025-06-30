"""
Tests for the error handler.
"""

import pytest
import asyncio
import time
from datetime import datetime, timedelta
from unittest.mock import Mock, patch

from satox.core.error_handler import ErrorHandler, handle_errors, retry_on_error

@pytest.fixture
def error_handler():
    """Create an error handler instance."""
    return ErrorHandler({
        'max_error_history': 5,
        'max_retries': 3,
        'retry_delay': 0.1,
        'error_threshold': 3,
        'error_window': 1
    })

def test_error_handler_initialization(error_handler):
    """Test error handler initialization."""
    assert error_handler._max_history == 5
    assert error_handler._max_retries == 3
    assert error_handler._retry_delay == 0.1
    assert error_handler._error_threshold == 3
    assert error_handler._error_window == 1
    assert len(error_handler._error_history) == 0
    assert len(error_handler._error_counts) == 0
    assert error_handler._error_metrics['total_errors'] == 0

def test_register_error_handler(error_handler):
    """Test registering an error handler."""
    handler = Mock()
    error_handler.register_error_handler('TestError', handler)
    assert 'TestError' in error_handler._error_handlers
    assert handler in error_handler._error_handlers['TestError']

def test_unregister_error_handler(error_handler):
    """Test unregistering an error handler."""
    handler = Mock()
    error_handler.register_error_handler('TestError', handler)
    error_handler.unregister_error_handler('TestError', handler)
    assert 'TestError' not in error_handler._error_handlers

def test_register_error_notifier(error_handler):
    """Test registering an error notifier."""
    notifier = Mock()
    error_handler.register_error_notifier(notifier)
    assert notifier in error_handler._error_notifiers

def test_unregister_error_notifier(error_handler):
    """Test unregistering an error notifier."""
    notifier = Mock()
    error_handler.register_error_notifier(notifier)
    error_handler.unregister_error_notifier(notifier)
    assert notifier not in error_handler._error_notifiers

def test_handle_error(error_handler):
    """Test handling an error."""
    error = ValueError("Test error")
    context = {'test': 'context'}
    
    error_handler.handle_error(error, context)
    
    assert len(error_handler._error_history) == 1
    assert error_handler._error_counts['ValueError'] == 1
    assert error_handler._error_metrics['total_errors'] == 1
    assert error_handler._error_metrics['error_types']['ValueError'] == 1

def test_handle_error_with_handler(error_handler):
    """Test handling an error with a registered handler."""
    error = ValueError("Test error")
    handler = Mock()
    error_handler.register_error_handler('ValueError', handler)
    
    error_handler.handle_error(error)
    
    handler.assert_called_once_with(error, None)

def test_handle_error_with_notifier(error_handler):
    """Test handling an error with a registered notifier."""
    error = ValueError("Test error")
    notifier = Mock()
    error_handler.register_error_notifier(notifier)
    
    error_handler.handle_error(error)
    
    notifier.assert_called_once()

def test_error_threshold(error_handler):
    """Test error threshold exceeded."""
    notifier = Mock()
    error_handler.register_error_notifier(notifier)
    
    # Generate errors exceeding threshold
    for _ in range(4):
        error_handler.handle_error(ValueError("Test error"))
    
    # Check if threshold exceeded notification was sent
    assert any(
        call.args[0]['type'] == 'ErrorThresholdExceeded'
        for call in notifier.call_args_list
    )

def test_get_error_history(error_handler):
    """Test getting error history."""
    error1 = ValueError("Error 1")
    error2 = TypeError("Error 2")
    
    error_handler.handle_error(error1)
    error_handler.handle_error(error2)
    
    history = error_handler.get_error_history()
    assert len(history) == 2
    
    filtered_history = error_handler.get_error_history('ValueError')
    assert len(filtered_history) == 1
    assert filtered_history[0]['type'] == 'ValueError'

def test_get_error_metrics(error_handler):
    """Test getting error metrics."""
    error_handler.handle_error(ValueError("Test error"))
    metrics = error_handler.get_error_metrics()
    
    assert metrics['total_errors'] == 1
    assert metrics['error_types']['ValueError'] == 1
    assert metrics['last_error_time'] is not None
    assert metrics['error_rate'] > 0

def test_clear_error_history(error_handler):
    """Test clearing error history."""
    error_handler.handle_error(ValueError("Test error"))
    error_handler.clear_error_history()
    
    assert len(error_handler._error_history) == 0
    assert len(error_handler._error_counts) == 0
    assert error_handler._error_metrics['total_errors'] == 0
    assert len(error_handler._error_metrics['error_types']) == 0
    assert error_handler._error_metrics['error_rate'] == 0

@pytest.mark.asyncio
async def test_handle_errors_decorator_async(error_handler):
    """Test handle_errors decorator with async function."""
    @handle_errors(error_handler)
    async def test_func():
        raise ValueError("Test error")
    
    with pytest.raises(ValueError):
        await test_func()
    
    assert len(error_handler._error_history) == 1
    assert error_handler._error_counts['ValueError'] == 1

def test_handle_errors_decorator_sync(error_handler):
    """Test handle_errors decorator with sync function."""
    @handle_errors(error_handler)
    def test_func():
        raise ValueError("Test error")
    
    with pytest.raises(ValueError):
        test_func()
    
    assert len(error_handler._error_history) == 1
    assert error_handler._error_counts['ValueError'] == 1

@pytest.mark.asyncio
async def test_retry_on_error_decorator_async(error_handler):
    """Test retry_on_error decorator with async function."""
    attempts = 0
    
    @retry_on_error(error_handler)
    async def test_func():
        nonlocal attempts
        attempts += 1
        if attempts < 3:
            raise ValueError("Test error")
        return "success"
    
    result = await test_func()
    
    assert result == "success"
    assert attempts == 3
    assert len(error_handler._error_history) == 2  # Two failed attempts

def test_retry_on_error_decorator_sync(error_handler):
    """Test retry_on_error decorator with sync function."""
    attempts = 0
    
    @retry_on_error(error_handler)
    def test_func():
        nonlocal attempts
        attempts += 1
        if attempts < 3:
            raise ValueError("Test error")
        return "success"
    
    result = test_func()
    
    assert result == "success"
    assert attempts == 3
    assert len(error_handler._error_history) == 2  # Two failed attempts

@pytest.mark.asyncio
async def test_retry_on_error_max_retries_async(error_handler):
    """Test retry_on_error decorator with max retries exceeded."""
    @retry_on_error(error_handler)
    async def test_func():
        raise ValueError("Test error")
    
    with pytest.raises(ValueError):
        await test_func()
    
    assert len(error_handler._error_history) == 3  # Three failed attempts

def test_retry_on_error_max_retries_sync(error_handler):
    """Test retry_on_error decorator with max retries exceeded."""
    @retry_on_error(error_handler)
    def test_func():
        raise ValueError("Test error")
    
    with pytest.raises(ValueError):
        test_func()
    
    assert len(error_handler._error_history) == 3  # Three failed attempts 