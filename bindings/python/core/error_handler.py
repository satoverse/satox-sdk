"""
Centralized error handling system for Satox SDK.
"""

import logging
import traceback
from typing import Dict, Any, Optional, Callable, List
from datetime import datetime
import json
import sys
from functools import wraps
import asyncio

from satox.core.exceptions import SatoxError

class ErrorHandler:
    """Centralized error handling system."""
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        """Initialize error handler.
        
        Args:
            config: Optional configuration dictionary
        """
        self.config = config or {}
        self.logger = logging.getLogger(__name__)
        self._error_handlers: Dict[str, List[Callable]] = {}
        self._error_counts: Dict[str, int] = {}
        self._error_history: List[Dict[str, Any]] = []
        self._max_history = self.config.get('max_error_history', 1000)
        self._max_retries = self.config.get('max_retries', 3)
        self._retry_delay = self.config.get('retry_delay', 1)
        self._error_threshold = self.config.get('error_threshold', 100)
        self._error_window = self.config.get('error_window', 3600)  # 1 hour
        self._error_notifiers: List[Callable] = []
        self._error_metrics: Dict[str, Any] = {
            'total_errors': 0,
            'error_types': {},
            'last_error_time': None,
            'error_rate': 0
        }

    def register_error_handler(self, error_type: str, handler: Callable) -> None:
        """Register an error handler.
        
        Args:
            error_type: Type of error to handle
            handler: Error handler function
        """
        if error_type not in self._error_handlers:
            self._error_handlers[error_type] = []
        self._error_handlers[error_type].append(handler)

    def unregister_error_handler(self, error_type: str, handler: Callable) -> None:
        """Unregister an error handler.
        
        Args:
            error_type: Type of error
            handler: Error handler function
        """
        if error_type in self._error_handlers and handler in self._error_handlers[error_type]:
            self._error_handlers[error_type].remove(handler)

    def register_error_notifier(self, notifier: Callable) -> None:
        """Register an error notifier.
        
        Args:
            notifier: Error notifier function
        """
        self._error_notifiers.append(notifier)

    def unregister_error_notifier(self, notifier: Callable) -> None:
        """Unregister an error notifier.
        
        Args:
            notifier: Error notifier function
        """
        if notifier in self._error_notifiers:
            self._error_notifiers.remove(notifier)

    def handle_error(self, error: Exception, context: Optional[Dict[str, Any]] = None) -> None:
        """Handle an error.
        
        Args:
            error: Exception to handle
            context: Optional context information
        """
        try:
            error_type = type(error).__name__
            error_time = datetime.now()
            
            # Update error metrics
            self._metrics['total_errors'] += 1
            self._metrics['error_types'][error_type] = self._metrics['error_types'].get(error_type, 0) + 1
            self._metrics['last_error_time'] = error_time
            
            # Calculate error rate
            if self._metrics['last_error_time']:
                time_diff = (error_time - self._metrics['last_error_time']).total_seconds()
                if time_diff > 0:
                    self._metrics['error_rate'] = 1 / time_diff
            
            # Create error record
            error_record = {
                'type': error_type,
                'message': str(error),
                'traceback': traceback.format_exc(),
                'time': error_time.isoformat(),
                'context': context or {}
            }
            
            # Add to history
            self._error_history.append(error_record)
            if len(self._error_history) > self._max_history:
                self._error_history.pop(0)
            
            # Update error counts
            self._error_counts[error_type] = self._error_counts.get(error_type, 0) + 1
            
            # Log error
            self.logger.error(
                f"Error occurred: {error_type} - {str(error)}",
                extra={
                    'error_type': error_type,
                    'error_message': str(error),
                    'traceback': traceback.format_exc(),
                    'context': context
                }
            )
            
            # Call error handlers
            if error_type in self._error_handlers:
                for handler in self._error_handlers[error_type]:
                    try:
                        handler(error, context)
                    except Exception as e:
                        self.logger.error(f"Error in error handler: {str(e)}")
            
            # Notify error
            self._notify_error(error_record)
            
            # Check error threshold
            self._check_error_threshold()
            
        except Exception as e:
            self.logger.error(f"Error in error handler: {str(e)}")

    def get_error_history(self, error_type: Optional[str] = None) -> List[Dict[str, Any]]:
        """Get error history.
        
        Args:
            error_type: Optional error type to filter by
            
        Returns:
            List of error records
        """
        if error_type:
            return [record for record in self._error_history if record['type'] == error_type]
        return self._error_history.copy()

    def get_error_metrics(self) -> Dict[str, Any]:
        """Get error metrics.
        
        Returns:
            Dictionary of error metrics
        """
        return self._metrics.copy()

    def clear_error_history(self) -> None:
        """Clear error history."""
        self._error_history.clear()
        self._error_counts.clear()
        self._metrics['total_errors'] = 0
        self._metrics['error_types'].clear()
        self._metrics['error_rate'] = 0

    def _notify_error(self, error_record: Dict[str, Any]) -> None:
        """Notify error to registered notifiers.
        
        Args:
            error_record: Error record to notify
        """
        for notifier in self._error_notifiers:
            try:
                notifier(error_record)
            except Exception as e:
                self.logger.error(f"Error in error notifier: {str(e)}")

    def _check_error_threshold(self) -> None:
        """Check if error threshold has been exceeded."""
        current_time = datetime.now()
        window_start = current_time.timestamp() - self._error_window
        
        # Count errors in window
        error_count = sum(
            1 for record in self._error_history
            if datetime.fromisoformat(record['time']).timestamp() > window_start
        )
        
        if error_count > self._error_threshold:
            self.logger.warning(
                f"Error threshold exceeded: {error_count} errors in {self._error_window} seconds"
            )
            # Notify threshold exceeded
            self._notify_error({
                'type': 'ErrorThresholdExceeded',
                'message': f"Error threshold exceeded: {error_count} errors in {self._error_window} seconds",
                'time': current_time.isoformat(),
                'error_count': error_count,
                'window': self._error_window
            })

def handle_errors(error_handler: ErrorHandler):
    """Decorator to handle errors in functions.
    
    Args:
        error_handler: Error handler instance
        
    Returns:
        Decorated function
    """
    def decorator(func):
        @wraps(func)
        async def async_wrapper(*args, **kwargs):
            try:
                return await func(*args, **kwargs)
            except Exception as e:
                error_handler.handle_error(e, {
                    'function': func.__name__,
                    'args': args,
                    'kwargs': kwargs
                })
                raise
        
        @wraps(func)
        def sync_wrapper(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except Exception as e:
                error_handler.handle_error(e, {
                    'function': func.__name__,
                    'args': args,
                    'kwargs': kwargs
                })
                raise
        
        return async_wrapper if asyncio.iscoroutinefunction(func) else sync_wrapper
    return decorator

def retry_on_error(error_handler: ErrorHandler, max_retries: Optional[int] = None):
    """Decorator to retry functions on error.
    
    Args:
        error_handler: Error handler instance
        max_retries: Optional maximum number of retries
        
    Returns:
        Decorated function
    """
    def decorator(func):
        @wraps(func)
        async def async_wrapper(*args, **kwargs):
            retries = 0
            max_attempts = max_retries or error_handler._max_retries
            
            while retries < max_attempts:
                try:
                    return await func(*args, **kwargs)
                except Exception as e:
                    retries += 1
                    if retries == max_attempts:
                        error_handler.handle_error(e, {
                            'function': func.__name__,
                            'args': args,
                            'kwargs': kwargs,
                            'retries': retries
                        })
                        raise
                    
                    error_handler.handle_error(e, {
                        'function': func.__name__,
                        'args': args,
                        'kwargs': kwargs,
                        'retries': retries,
                        'will_retry': True
                    })
                    
                    await asyncio.sleep(error_handler._retry_delay * retries)
        
        @wraps(func)
        def sync_wrapper(*args, **kwargs):
            retries = 0
            max_attempts = max_retries or error_handler._max_retries
            
            while retries < max_attempts:
                try:
                    return func(*args, **kwargs)
                except Exception as e:
                    retries += 1
                    if retries == max_attempts:
                        error_handler.handle_error(e, {
                            'function': func.__name__,
                            'args': args,
                            'kwargs': kwargs,
                            'retries': retries
                        })
                        raise
                    
                    error_handler.handle_error(e, {
                        'function': func.__name__,
                        'args': args,
                        'kwargs': kwargs,
                        'retries': retries,
                        'will_retry': True
                    })
                    
                    time.sleep(error_handler._retry_delay * retries)
        
        return async_wrapper if asyncio.iscoroutinefunction(func) else sync_wrapper
    return decorator 