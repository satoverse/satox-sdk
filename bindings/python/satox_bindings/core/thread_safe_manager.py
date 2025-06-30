"""
Thread-Safe Manager Base Class for Satox SDK Python Bindings
Provides comprehensive thread safety using Python's threading primitives.
"""

import threading
import asyncio
import time
from typing import Any, Optional, Callable, TypeVar, Generic, Dict, List
from contextlib import contextmanager
from dataclasses import dataclass
from enum import Enum
import weakref

T = TypeVar('T')

class ThreadState(Enum):
    """Thread state enumeration."""
    UNINITIALIZED = "uninitialized"
    INITIALIZING = "initializing"
    INITIALIZED = "initialized"
    SHUTTING_DOWN = "shutting_down"
    SHUTDOWN = "shutdown"
    ERROR = "error"

@dataclass
class ThreadSafeStats:
    """Thread safety statistics."""
    total_operations: int = 0
    concurrent_operations: int = 0
    max_concurrent_operations: int = 0
    average_operation_time: float = 0.0
    total_operation_time: float = 0.0
    lock_contention_count: int = 0
    last_operation_time: float = 0.0

class ThreadSafeManager:
    """
    Base class for thread-safe managers in the Satox SDK.
    
    Provides comprehensive thread safety using:
    - threading.RLock for reentrant locking (allows same thread to acquire lock multiple times)
    - threading.Lock for exclusive operations
    - threading.Event for state synchronization
    - Atomic counters and state management
    - Thread-safe singleton patterns
    - Exception-safe operations
    - Proper resource cleanup
    - Performance monitoring
    """
    
    def __init__(self, name: str = "ThreadSafeManager"):
        """Initialize the thread-safe manager."""
        self._name = name
        # Use RLock for reentrant locking (same thread can acquire multiple times)
        self._lock = threading.RLock()
        # Separate lock for error handling to prevent deadlocks
        self._error_lock = threading.Lock()
        # Lock for statistics to prevent race conditions
        self._stats_lock = threading.Lock()
        # Event for state synchronization
        self._state_event = threading.Event()
        
        # Thread-safe state management
        self._state = ThreadState.UNINITIALIZED
        self._initialized = False
        self._disposed = False
        self._last_error = ""
        self._error_count = 0
        
        # Thread-safe statistics
        self._stats = ThreadSafeStats()
        self._operation_start_times: Dict[int, float] = {}
        
        # Thread-safe operation tracking
        self._active_operations = 0
        self._operation_counter = 0
        
        # Thread-safe event handling
        self._event_handlers: Dict[str, List[Callable]] = {}
        self._event_lock = threading.Lock()
        
        # Thread-safe resource management
        self._resources: Dict[str, Any] = {}
        self._resource_lock = threading.Lock()
        
        # Thread-safe configuration
        self._config: Dict[str, Any] = {}
        self._config_lock = threading.Lock()
        
        # Thread-safe cache
        self._cache: Dict[str, Any] = {}
        self._cache_lock = threading.Lock()
        self._cache_timestamps: Dict[str, float] = {}
        
        # Performance monitoring
        self._performance_enabled = True
        self._start_time = time.time()
        
    def _ensure_initialized(self) -> bool:
        """
        Ensure the manager is initialized.
        
        Returns:
            bool: True if initialized, False otherwise
        """
        with self._lock:
            if self._state == ThreadState.UNINITIALIZED:
                self._set_error("Manager not initialized")
                return False
            if self._state == ThreadState.SHUTDOWN:
                self._set_error("Manager has been disposed")
                return False
            if self._state == ThreadState.ERROR:
                self._set_error("Manager is in error state")
                return False
            return True
    
    def _ensure_not_disposed(self) -> bool:
        """
        Ensure the manager has not been disposed.
        
        Returns:
            bool: True if not disposed, False otherwise
        """
        with self._lock:
            if self._state == ThreadState.SHUTDOWN:
                self._set_error("Manager has been disposed")
                return False
            return True
    
    def _set_state(self, state: ThreadState) -> None:
        """Set the manager state in a thread-safe manner."""
        with self._lock:
            self._state = state
            self._state_event.set()
    
    def _get_state(self) -> ThreadState:
        """Get the current manager state in a thread-safe manner."""
        with self._lock:
            return self._state
    
    def _wait_for_state(self, state: ThreadState, timeout: float = 5.0) -> bool:
        """Wait for a specific state with timeout."""
        start_time = time.time()
        while time.time() - start_time < timeout:
            if self._get_state() == state:
                return True
            time.sleep(0.01)
        return False
    
    @contextmanager
    def _with_lock(self):
        """
        Context manager for acquiring the manager lock.
        
        Usage:
            with self._with_lock():
                # Thread-safe operations here
                pass
        """
        operation_id = self._begin_operation()
        try:
            self._lock.acquire()
            yield
        finally:
            self._lock.release()
            self._end_operation(operation_id)
    
    @contextmanager
    def _with_read_lock(self):
        """
        Context manager for read operations (same as write lock in this implementation).
        
        Usage:
            with self._with_read_lock():
                # Thread-safe read operations here
                pass
        """
        operation_id = self._begin_operation()
        try:
            self._lock.acquire()
            yield
        finally:
            self._lock.release()
            self._end_operation(operation_id)
    
    def _begin_operation(self) -> int:
        """Begin tracking an operation."""
        if not self._performance_enabled:
            return 0
        
        with self._stats_lock:
            operation_id = self._operation_counter
            self._operation_counter += 1
            self._active_operations += 1
            self._operation_start_times[operation_id] = time.time()
            self._stats.concurrent_operations = self._active_operations
            self._stats.max_concurrent_operations = max(
                self._stats.max_concurrent_operations, 
                self._active_operations
            )
            return operation_id
    
    def _end_operation(self, operation_id: int) -> None:
        """End tracking an operation."""
        if not self._performance_enabled or operation_id == 0:
            return
        
        with self._stats_lock:
            if operation_id in self._operation_start_times:
                operation_time = time.time() - self._operation_start_times[operation_id]
                del self._operation_start_times[operation_id]
                
                self._active_operations -= 1
                self._stats.total_operations += 1
                self._stats.total_operation_time += operation_time
                self._stats.average_operation_time = (
                    self._stats.total_operation_time / self._stats.total_operations
                )
                self._stats.last_operation_time = time.time()
                self._stats.concurrent_operations = self._active_operations
    
    def _set_error(self, error: str) -> None:
        """
        Set the last error message in a thread-safe manner.
        
        Args:
            error: Error message to set
        """
        with self._error_lock:
            self._last_error = error
            self._error_count += 1
            self._set_state(ThreadState.ERROR)
    
    def _clear_error(self) -> None:
        """Clear the last error message in a thread-safe manner."""
        with self._error_lock:
            self._last_error = ""
            if self._state == ThreadState.ERROR:
                self._set_state(ThreadState.INITIALIZED)
    
    def get_last_error(self) -> str:
        """
        Get the last error message in a thread-safe manner.
        
        Returns:
            str: Last error message
        """
        with self._error_lock:
            return self._last_error
    
    def get_error_count(self) -> int:
        """
        Get the total error count in a thread-safe manner.
        
        Returns:
            int: Total error count
        """
        with self._error_lock:
            return self._error_count
    
    def clear_last_error(self) -> None:
        """Clear the last error message in a thread-safe manner."""
        self._clear_error()
    
    def is_initialized(self) -> bool:
        """
        Check if the manager is initialized in a thread-safe manner.
        
        Returns:
            bool: True if initialized, False otherwise
        """
        with self._lock:
            return self._state == ThreadState.INITIALIZED
    
    def is_disposed(self) -> bool:
        """
        Check if the manager has been disposed in a thread-safe manner.
        
        Returns:
            bool: True if disposed, False otherwise
        """
        with self._lock:
            return self._state == ThreadState.SHUTDOWN
    
    def get_state(self) -> ThreadState:
        """
        Get the current manager state in a thread-safe manner.
        
        Returns:
            ThreadState: Current state
        """
        return self._get_state()
    
    def get_stats(self) -> ThreadSafeStats:
        """
        Get thread safety statistics in a thread-safe manner.
        
        Returns:
            ThreadSafeStats: Current statistics
        """
        with self._stats_lock:
            return ThreadSafeStats(
                total_operations=self._stats.total_operations,
                concurrent_operations=self._stats.concurrent_operations,
                max_concurrent_operations=self._stats.max_concurrent_operations,
                average_operation_time=self._stats.average_operation_time,
                total_operation_time=self._stats.total_operation_time,
                lock_contention_count=self._stats.lock_contention_count,
                last_operation_time=self._stats.last_operation_time
            )
    
    def reset_stats(self) -> None:
        """Reset thread safety statistics."""
        with self._stats_lock:
            self._stats = ThreadSafeStats()
            self._operation_start_times.clear()
            self._active_operations = 0
            self._operation_counter = 0
    
    def enable_performance_monitoring(self) -> None:
        """Enable performance monitoring."""
        with self._stats_lock:
            self._performance_enabled = True
    
    def disable_performance_monitoring(self) -> None:
        """Disable performance monitoring."""
        with self._stats_lock:
            self._performance_enabled = False
    
    def get_uptime(self) -> float:
        """
        Get manager uptime in seconds.
        
        Returns:
            float: Uptime in seconds
        """
        return time.time() - self._start_time
    
    def set_config(self, key: str, value: Any) -> None:
        """
        Set configuration value in a thread-safe manner.
        
        Args:
            key: Configuration key
            value: Configuration value
        """
        with self._config_lock:
            self._config[key] = value
    
    def get_config(self, key: str, default: Any = None) -> Any:
        """
        Get configuration value in a thread-safe manner.
        
        Args:
            key: Configuration key
            default: Default value if key not found
            
        Returns:
            Any: Configuration value
        """
        with self._config_lock:
            return self._config.get(key, default)
    
    def set_cache(self, key: str, value: Any, ttl: float = 300.0) -> None:
        """
        Set cache value in a thread-safe manner.
        
        Args:
            key: Cache key
            value: Cache value
            ttl: Time to live in seconds
        """
        with self._cache_lock:
            self._cache[key] = value
            self._cache_timestamps[key] = time.time() + ttl
    
    def get_cache(self, key: str, default: Any = None) -> Any:
        """
        Get cache value in a thread-safe manner.
        
        Args:
            key: Cache key
            default: Default value if key not found or expired
            
        Returns:
            Any: Cache value
        """
        with self._cache_lock:
            if key in self._cache:
                if time.time() < self._cache_timestamps.get(key, 0):
                    return self._cache[key]
                else:
                    # Expired, remove from cache
                    del self._cache[key]
                    if key in self._cache_timestamps:
                        del self._cache_timestamps[key]
            return default
    
    def clear_cache(self) -> None:
        """Clear all cached values."""
        with self._cache_lock:
            self._cache.clear()
            self._cache_timestamps.clear()
    
    def add_event_handler(self, event: str, handler: Callable) -> None:
        """
        Add event handler in a thread-safe manner.
        
        Args:
            event: Event name
            handler: Event handler function
        """
        with self._event_lock:
            if event not in self._event_handlers:
                self._event_handlers[event] = []
            self._event_handlers[event].append(handler)
    
    def remove_event_handler(self, event: str, handler: Callable) -> None:
        """
        Remove event handler in a thread-safe manner.
        
        Args:
            event: Event name
            handler: Event handler function to remove
        """
        with self._event_lock:
            if event in self._event_handlers:
                try:
                    self._event_handlers[event].remove(handler)
                except ValueError:
                    pass  # Handler not found
    
    def emit_event(self, event: str, *args, **kwargs) -> None:
        """
        Emit an event in a thread-safe manner.
        
        Args:
            event: Event name
            *args: Event arguments
            **kwargs: Event keyword arguments
        """
        with self._event_lock:
            if event in self._event_handlers:
                # Copy handlers to avoid modification during iteration
                handlers = self._event_handlers[event].copy()
        
        # Call handlers outside of lock to prevent deadlocks
        for handler in handlers:
            try:
                handler(*args, **kwargs)
            except Exception as e:
                # Log error but don't stop other handlers
                self._set_error(f"Error in event handler {handler}: {e}")
    
    def add_resource(self, name: str, resource: Any) -> None:
        """
        Add a resource in a thread-safe manner.
        
        Args:
            name: Resource name
            resource: Resource object
        """
        with self._resource_lock:
            self._resources[name] = resource
    
    def get_resource(self, name: str, default: Any = None) -> Any:
        """
        Get a resource in a thread-safe manner.
        
        Args:
            name: Resource name
            default: Default value if resource not found
            
        Returns:
            Any: Resource object
        """
        with self._resource_lock:
            return self._resources.get(name, default)
    
    def remove_resource(self, name: str) -> None:
        """
        Remove a resource in a thread-safe manner.
        
        Args:
            name: Resource name
        """
        with self._resource_lock:
            if name in self._resources:
                del self._resources[name]
    
    def clear_resources(self) -> None:
        """Clear all resources."""
        with self._resource_lock:
            self._resources.clear()
    
    def dispose(self) -> None:
        """
        Dispose of the manager and cleanup resources in a thread-safe manner.
        This method should be called when the manager is no longer needed.
        """
        with self._lock:
            if self._state == ThreadState.SHUTDOWN:
                return
            
            try:
                self._set_state(ThreadState.SHUTTING_DOWN)
                self._dispose_impl()
                self._clear_cache()
                self._clear_resources()
                self._set_state(ThreadState.SHUTDOWN)
            except Exception as e:
                self._set_error(f"Disposal failed: {str(e)}")
                self._set_state(ThreadState.ERROR)
    
    def _dispose_impl(self) -> None:
        """
        Implementation of disposal logic. Override in subclasses.
        """
        pass
    
    def __enter__(self):
        """Context manager entry."""
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit with automatic disposal."""
        self.dispose()


class ThreadSafeSingleton(ThreadSafeManager):
    """
    Thread-safe singleton pattern for managers.
    
    Ensures only one instance of the manager exists across all threads.
    """
    
    _instances: Dict[type, 'ThreadSafeSingleton'] = {}
    _instance_locks: Dict[type, threading.Lock] = {}
    _singleton_lock = threading.Lock()
    
    def __new__(cls, *args, **kwargs):
        """Create or return existing instance in a thread-safe manner."""
        if cls not in cls._instances:
            with cls._singleton_lock:
                if cls not in cls._instances:
                    cls._instances[cls] = super().__new__(cls)
                    cls._instance_locks[cls] = threading.Lock()
        return cls._instances[cls]
    
    def __init__(self, name: str = "ThreadSafeSingleton"):
        """Initialize the singleton manager."""
        # Only initialize once
        if not hasattr(self, '_name'):
            super().__init__(name)
    
    def _get_instance_lock(self) -> threading.Lock:
        """Get the instance-specific lock."""
        return self._instance_locks[self.__class__]
    
    @classmethod
    def get_instance(cls) -> 'ThreadSafeSingleton':
        """Get the singleton instance."""
        return cls()
    
    @classmethod
    def reset_instance(cls) -> None:
        """Reset the singleton instance (for testing)."""
        with cls._singleton_lock:
            if cls in cls._instances:
                del cls._instances[cls]
            if cls in cls._instance_locks:
                del cls._instance_locks[cls]


class ThreadSafeEventEmitter:
    """
    Thread-safe event emitter for manager events.
    """
    
    def __init__(self):
        """Initialize the thread-safe event emitter."""
        self._lock = threading.RLock()
        self._listeners: Dict[str, List[Callable]] = {}
        self._event_stats: Dict[str, int] = {}
    
    def on(self, event: str, callback: Callable) -> None:
        """
        Register an event listener in a thread-safe manner.
        
        Args:
            event: Event name
            callback: Callback function
        """
        with self._lock:
            if event not in self._listeners:
                self._listeners[event] = []
            self._listeners[event].append(callback)
    
    def off(self, event: str, callback: Callable) -> None:
        """
        Remove an event listener in a thread-safe manner.
        
        Args:
            event: Event name
            callback: Callback function to remove
        """
        with self._lock:
            if event in self._listeners:
                try:
                    self._listeners[event].remove(callback)
                except ValueError:
                    pass  # Callback not found
    
    def emit(self, event: str, *args, **kwargs) -> None:
        """
        Emit an event in a thread-safe manner.
        
        Args:
            event: Event name
            *args: Event arguments
            **kwargs: Event keyword arguments
        """
        with self._lock:
            if event in self._listeners:
                # Copy listeners to avoid modification during iteration
                listeners = self._listeners[event].copy()
                self._event_stats[event] = self._event_stats.get(event, 0) + 1
        
        # Call listeners outside of lock to prevent deadlocks
        for callback in listeners:
            try:
                callback(*args, **kwargs)
            except Exception as e:
                # Log error but don't stop other listeners
                print(f"Error in event listener {callback}: {e}")
    
    def clear_listeners(self, event: Optional[str] = None) -> None:
        """
        Clear event listeners in a thread-safe manner.
        
        Args:
            event: Event name to clear, or None to clear all events
        """
        with self._lock:
            if event is None:
                self._listeners.clear()
                self._event_stats.clear()
            elif event in self._listeners:
                self._listeners[event].clear()
                if event in self._event_stats:
                    del self._event_stats[event]
    
    def get_event_stats(self) -> Dict[str, int]:
        """
        Get event emission statistics.
        
        Returns:
            Dict[str, int]: Event statistics
        """
        with self._lock:
            return self._event_stats.copy()
    
    def get_listener_count(self, event: str) -> int:
        """
        Get the number of listeners for an event.
        
        Args:
            event: Event name
            
        Returns:
            int: Number of listeners
        """
        with self._lock:
            return len(self._listeners.get(event, []))


class ThreadSafeAsyncManager(ThreadSafeManager):
    """
    Thread-safe async manager for asynchronous operations.
    """
    
    def __init__(self, name: str = "ThreadSafeAsyncManager"):
        """Initialize the async manager."""
        super().__init__(name)
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._async_lock = asyncio.Lock()
        self._async_tasks: List[asyncio.Task] = []
        self._task_lock = threading.Lock()
    
    async def _ensure_async_initialized(self) -> bool:
        """Ensure the async manager is initialized."""
        if not self._loop:
            self._loop = asyncio.get_event_loop()
        return await asyncio.get_event_loop().run_in_executor(None, self._ensure_initialized)
    
    async def _with_async_lock(self):
        """Async context manager for acquiring the async lock."""
        await self._async_lock.acquire()
        try:
            yield
        finally:
            self._async_lock.release()
    
    async def _run_in_executor(self, func: Callable, *args, **kwargs) -> Any:
        """Run a function in the executor to avoid blocking the event loop."""
        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(None, func, *args, **kwargs)
    
    def _add_task(self, task: asyncio.Task) -> None:
        """Add an async task for tracking."""
        with self._task_lock:
            self._async_tasks.append(task)
    
    def _remove_task(self, task: asyncio.Task) -> None:
        """Remove an async task from tracking."""
        with self._task_lock:
            try:
                self._async_tasks.remove(task)
            except ValueError:
                pass
    
    def get_active_tasks(self) -> List[asyncio.Task]:
        """Get active async tasks."""
        with self._task_lock:
            return [task for task in self._async_tasks if not task.done()]
    
    async def wait_for_all_tasks(self, timeout: float = 30.0) -> None:
        """Wait for all active tasks to complete."""
        active_tasks = self.get_active_tasks()
        if active_tasks:
            await asyncio.wait_for(asyncio.gather(*active_tasks, return_exceptions=True), timeout)
    
    async def _dispose_async_impl(self) -> None:
        """Async implementation of disposal logic."""
        await self.wait_for_all_tasks()
    
    def dispose(self) -> None:
        """Dispose of the async manager."""
        with self._lock:
            if self._state == ThreadState.SHUTDOWN:
                return
            
            try:
                self._set_state(ThreadState.SHUTTING_DOWN)
                # Run async disposal in executor
                if self._loop and not self._loop.is_closed():
                    future = asyncio.run_coroutine_threadsafe(self._dispose_async_impl(), self._loop)
                    future.result(timeout=10.0)
                self._dispose_impl()
                self._set_state(ThreadState.SHUTDOWN)
            except Exception as e:
                self._set_error(f"Async disposal failed: {str(e)}")
                self._set_state(ThreadState.ERROR) 