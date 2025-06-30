use std::sync::{Arc, Mutex, RwLock, atomic::{AtomicBool, AtomicU64, Ordering}};
use std::collections::HashMap;
use std::time::{Duration, Instant};
use std::any::Any;
use std::fmt;
use tokio::sync::mpsc;
use serde::{Serialize, Deserialize};

/// Thread state enumeration
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ThreadState {
    Uninitialized,
    Initializing,
    Initialized,
    ShuttingDown,
    Shutdown,
    Error,
}

impl fmt::Display for ThreadState {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ThreadState::Uninitialized => write!(f, "uninitialized"),
            ThreadState::Initializing => write!(f, "initializing"),
            ThreadState::Initialized => write!(f, "initialized"),
            ThreadState::ShuttingDown => write!(f, "shutting_down"),
            ThreadState::Shutdown => write!(f, "shutdown"),
            ThreadState::Error => write!(f, "error"),
        }
    }
}

/// Thread safety statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThreadSafeStats {
    pub total_operations: u64,
    pub concurrent_operations: u64,
    pub max_concurrent_operations: u64,
    pub average_operation_time: f64,
    pub total_operation_time: f64,
    pub lock_contention_count: u64,
    pub last_operation_time: f64,
}

impl Default for ThreadSafeStats {
    fn default() -> Self {
        Self {
            total_operations: 0,
            concurrent_operations: 0,
            max_concurrent_operations: 0,
            average_operation_time: 0.0,
            total_operation_time: 0.0,
            lock_contention_count: 0,
            last_operation_time: 0.0,
        }
    }
}

/// Event handler type
pub type EventHandler = Arc<dyn Fn(serde_json::Value) + Send + Sync>;

/// Thread-safe manager base class for Rust bindings
pub struct ThreadSafeManager {
    name: String,
    state: Arc<RwLock<ThreadState>>,
    initialized: AtomicBool,
    disposed: AtomicBool,
    last_error: Arc<Mutex<String>>,
    error_count: AtomicU64,
    
    // Statistics
    stats: Arc<RwLock<ThreadSafeStats>>,
    operation_start_times: Arc<Mutex<HashMap<u64, Instant>>>,
    active_operations: AtomicU64,
    operation_counter: AtomicU64,
    
    // Event handling
    event_handlers: Arc<RwLock<HashMap<String, Vec<EventHandler>>>>,
    event_stats: Arc<RwLock<HashMap<String, u64>>>,
    
    // Resource management
    resources: Arc<RwLock<HashMap<String, Arc<dyn Any + Send + Sync>>>>,
    
    // Configuration
    config: Arc<RwLock<HashMap<String, serde_json::Value>>>,
    
    // Cache
    cache: Arc<RwLock<HashMap<String, (serde_json::Value, Instant)>>>,
    
    // Performance monitoring
    performance_enabled: AtomicBool,
    start_time: Instant,
}

impl ThreadSafeManager {
    /// Create a new thread-safe manager
    pub fn new(name: &str) -> Self {
        Self {
            name: name.to_string(),
            state: Arc::new(RwLock::new(ThreadState::Uninitialized)),
            initialized: AtomicBool::new(false),
            disposed: AtomicBool::new(false),
            last_error: Arc::new(Mutex::new(String::new())),
            error_count: AtomicU64::new(0),
            
            stats: Arc::new(RwLock::new(ThreadSafeStats::default())),
            operation_start_times: Arc::new(Mutex::new(HashMap::new())),
            active_operations: AtomicU64::new(0),
            operation_counter: AtomicU64::new(0),
            
            event_handlers: Arc::new(RwLock::new(HashMap::new())),
            event_stats: Arc::new(RwLock::new(HashMap::new())),
            
            resources: Arc::new(RwLock::new(HashMap::new())),
            config: Arc::new(RwLock::new(HashMap::new())),
            cache: Arc::new(RwLock::new(HashMap::new())),
            
            performance_enabled: AtomicBool::new(true),
            start_time: Instant::now(),
        }
    }
    
    /// Ensure the manager is initialized
    pub fn ensure_initialized(&self) -> Result<(), String> {
        let state = self.get_state()?;
        match state {
            ThreadState::Uninitialized => {
                self.set_error("Manager not initialized".to_string());
                Err(self.get_last_error())
            }
            ThreadState::Shutdown => {
                self.set_error("Manager has been disposed".to_string());
                Err(self.get_last_error())
            }
            ThreadState::Error => {
                self.set_error("Manager is in error state".to_string());
                Err(self.get_last_error())
            }
            _ => Ok(()),
        }
    }
    
    /// Ensure the manager has not been disposed
    pub fn ensure_not_disposed(&self) -> Result<(), String> {
        let state = self.get_state()?;
        if state == ThreadState::Shutdown {
            self.set_error("Manager has been disposed".to_string());
            Err(self.get_last_error())
        } else {
            Ok(())
        }
    }
    
    /// Set the manager state
    pub fn set_state(&self, state: ThreadState) -> Result<(), String> {
        let mut state_guard = self.state.write().map_err(|e| format!("Failed to acquire state lock: {}", e))?;
        *state_guard = state;
        Ok(())
    }
    
    /// Get the current manager state
    pub fn get_state(&self) -> Result<ThreadState, String> {
        let state_guard = self.state.read().map_err(|e| format!("Failed to acquire state lock: {}", e))?;
        Ok(state_guard.clone())
    }
    
    /// Wait for a specific state with timeout
    pub fn wait_for_state(&self, target_state: ThreadState, timeout: Duration) -> Result<bool, String> {
        let start = Instant::now();
        while start.elapsed() < timeout {
            let current_state = self.get_state()?;
            if current_state == target_state {
                return Ok(true);
            }
            std::thread::sleep(Duration::from_millis(10));
        }
        Ok(false)
    }
    
    /// Begin tracking an operation
    pub fn begin_operation(&self) -> u64 {
        if !self.performance_enabled.load(Ordering::Relaxed) {
            return 0;
        }
        
        let operation_id = self.operation_counter.fetch_add(1, Ordering::Relaxed);
        let active_ops = self.active_operations.fetch_add(1, Ordering::Relaxed) + 1;
        
        // Update statistics
        if let Ok(mut stats) = self.stats.write() {
            stats.concurrent_operations = active_ops;
            stats.max_concurrent_operations = stats.max_concurrent_operations.max(active_ops);
        }
        
        // Record start time
        if let Ok(mut start_times) = self.operation_start_times.lock() {
            start_times.insert(operation_id, Instant::now());
        }
        
        operation_id
    }
    
    /// End tracking an operation
    pub fn end_operation(&self, operation_id: u64) {
        if !self.performance_enabled.load(Ordering::Relaxed) || operation_id == 0 {
            return;
        }
        
        let active_ops = self.active_operations.fetch_sub(1, Ordering::Relaxed) - 1;
        
        // Get operation duration
        let duration = if let Ok(mut start_times) = self.operation_start_times.lock() {
            if let Some(start_time) = start_times.remove(&operation_id) {
                start_time.elapsed().as_secs_f64()
            } else {
                0.0
            }
        } else {
            0.0
        };
        
        // Update statistics
        if let Ok(mut stats) = self.stats.write() {
            stats.total_operations += 1;
            stats.total_operation_time += duration;
            stats.average_operation_time = stats.total_operation_time / stats.total_operations as f64;
            stats.last_operation_time = self.start_time.elapsed().as_secs_f64();
            stats.concurrent_operations = active_ops;
        }
    }
    
    /// Set error message
    pub fn set_error(&self, error: String) {
        if let Ok(mut error_guard) = self.last_error.lock() {
            *error_guard = error;
        }
        self.error_count.fetch_add(1, Ordering::Relaxed);
        let _ = self.set_state(ThreadState::Error);
    }
    
    /// Clear error message
    pub fn clear_error(&self) {
        if let Ok(mut error_guard) = self.last_error.lock() {
            *error_guard = String::new();
        }
        if let Ok(state) = self.get_state() {
            if state == ThreadState::Error {
                let _ = self.set_state(ThreadState::Initialized);
            }
        }
    }
    
    /// Get last error message
    pub fn get_last_error(&self) -> String {
        if let Ok(error_guard) = self.last_error.lock() {
            error_guard.clone()
        } else {
            "Failed to acquire error lock".to_string()
        }
    }
    
    /// Get error count
    pub fn get_error_count(&self) -> u64 {
        self.error_count.load(Ordering::Relaxed)
    }
    
    /// Check if initialized
    pub fn is_initialized(&self) -> bool {
        if let Ok(state) = self.get_state() {
            state == ThreadState::Initialized
        } else {
            false
        }
    }
    
    /// Check if disposed
    pub fn is_disposed(&self) -> bool {
        if let Ok(state) = self.get_state() {
            state == ThreadState::Shutdown
        } else {
            false
        }
    }
    
    /// Get thread safety statistics
    pub fn get_stats(&self) -> Result<ThreadSafeStats, String> {
        let stats_guard = self.stats.read().map_err(|e| format!("Failed to acquire stats lock: {}", e))?;
        Ok(stats_guard.clone())
    }
    
    /// Reset statistics
    pub fn reset_stats(&self) -> Result<(), String> {
        let mut stats_guard = self.stats.write().map_err(|e| format!("Failed to acquire stats lock: {}", e))?;
        *stats_guard = ThreadSafeStats::default();
        
        if let Ok(mut start_times) = self.operation_start_times.lock() {
            start_times.clear();
        }
        
        self.active_operations.store(0, Ordering::Relaxed);
        self.operation_counter.store(0, Ordering::Relaxed);
        
        Ok(())
    }
    
    /// Enable performance monitoring
    pub fn enable_performance_monitoring(&self) {
        self.performance_enabled.store(true, Ordering::Relaxed);
    }
    
    /// Disable performance monitoring
    pub fn disable_performance_monitoring(&self) {
        self.performance_enabled.store(false, Ordering::Relaxed);
    }
    
    /// Get uptime in seconds
    pub fn get_uptime(&self) -> f64 {
        self.start_time.elapsed().as_secs_f64()
    }
    
    /// Set configuration value
    pub fn set_config(&self, key: &str, value: serde_json::Value) -> Result<(), String> {
        let mut config_guard = self.config.write().map_err(|e| format!("Failed to acquire config lock: {}", e))?;
        config_guard.insert(key.to_string(), value);
        Ok(())
    }
    
    /// Get configuration value
    pub fn get_config(&self, key: &str) -> Option<serde_json::Value> {
        if let Ok(config_guard) = self.config.read() {
            config_guard.get(key).cloned()
        } else {
            None
        }
    }
    
    /// Set cache value with TTL
    pub fn set_cache(&self, key: &str, value: serde_json::Value, ttl: Duration) -> Result<(), String> {
        let mut cache_guard = self.cache.write().map_err(|e| format!("Failed to acquire cache lock: {}", e))?;
        let expiry = Instant::now() + ttl;
        cache_guard.insert(key.to_string(), (value, expiry));
        Ok(())
    }
    
    /// Get cache value
    pub fn get_cache(&self, key: &str) -> Option<serde_json::Value> {
        if let Ok(mut cache_guard) = self.cache.write() {
            if let Some((value, expiry)) = cache_guard.get(key) {
                if Instant::now() < *expiry {
                    return Some(value.clone());
                } else {
                    // Expired, remove from cache
                    cache_guard.remove(key);
                }
            }
        }
        None
    }
    
    /// Clear cache
    pub fn clear_cache(&self) -> Result<(), String> {
        let mut cache_guard = self.cache.write().map_err(|e| format!("Failed to acquire cache lock: {}", e))?;
        cache_guard.clear();
        Ok(())
    }
    
    /// Add event handler
    pub fn add_event_handler(&self, event: &str, handler: EventHandler) -> Result<(), String> {
        let mut handlers_guard = self.event_handlers.write().map_err(|e| format!("Failed to acquire handlers lock: {}", e))?;
        handlers_guard.entry(event.to_string()).or_insert_with(Vec::new).push(handler);
        Ok(())
    }
    
    /// Remove event handler
    pub fn remove_event_handler(&self, event: &str, handler_index: usize) -> Result<(), String> {
        let mut handlers_guard = self.event_handlers.write().map_err(|e| format!("Failed to acquire handlers lock: {}", e))?;
        if let Some(handlers) = handlers_guard.get_mut(event) {
            if handler_index < handlers.len() {
                handlers.remove(handler_index);
            }
        }
        Ok(())
    }
    
    /// Emit event
    pub fn emit_event(&self, event: &str, data: serde_json::Value) -> Result<(), String> {
        // Get handlers and update stats
        let handlers: Vec<EventHandler> = {
            let handlers_guard = self.event_handlers.read().map_err(|e| format!("Failed to acquire handlers lock: {}", e))?;
            if let Some(handlers) = handlers_guard.get(event) {
                handlers.iter().cloned().collect()
            } else {
                return Ok(());
            }
        };
        let mut stats_guard = self.event_stats.write().map_err(|e| format!("Failed to acquire event stats lock: {}", e))?;
        *stats_guard.entry(event.to_string()).or_insert(0) += 1;
        drop(stats_guard);
        // Call handlers outside of lock to prevent deadlocks
        for handler in handlers {
            if let Err(e) = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| handler(data.clone()))) {
                self.set_error(format!("Error in event handler: {:?}", e));
            }
        }
        Ok(())
    }
    
    /// Add resource
    pub fn add_resource<T: Any + Send + Sync + 'static>(&self, name: &str, resource: T) -> Result<(), String> {
        let mut resources_guard = self.resources.write().map_err(|e| format!("Failed to acquire resources lock: {}", e))?;
        resources_guard.insert(name.to_string(), Arc::new(resource));
        Ok(())
    }
    
    /// Get resource
    pub fn get_resource<T: Any + Send + Sync + 'static>(&self, name: &str) -> Option<Arc<T>> {
        if let Ok(resources_guard) = self.resources.read() {
            if let Some(resource) = resources_guard.get(name) {
                resource.clone().downcast::<T>().ok()
            } else {
                None
            }
        } else {
            None
        }
    }
    
    /// Remove resource
    pub fn remove_resource(&self, name: &str) -> Result<(), String> {
        let mut resources_guard = self.resources.write().map_err(|e| format!("Failed to acquire resources lock: {}", e))?;
        resources_guard.remove(name);
        Ok(())
    }
    
    /// Clear resources
    pub fn clear_resources(&self) -> Result<(), String> {
        let mut resources_guard = self.resources.write().map_err(|e| format!("Failed to acquire resources lock: {}", e))?;
        resources_guard.clear();
        Ok(())
    }
    
    /// Dispose of the manager
    pub fn dispose(&self) -> Result<(), String> {
        let current_state = self.get_state()?;
        if current_state == ThreadState::Shutdown {
            return Ok(());
        }
        
        self.set_state(ThreadState::ShuttingDown)?;
        
        // Clear cache and resources
        self.clear_cache()?;
        self.clear_resources()?;
        
        // Call implementation-specific disposal
        self.dispose_impl()?;
        
        self.set_state(ThreadState::Shutdown)?;
        Ok(())
    }
    
    /// Implementation-specific disposal logic (override in subclasses)
    fn dispose_impl(&self) -> Result<(), String> {
        Ok(())
    }
}

impl Drop for ThreadSafeManager {
    fn drop(&mut self) {
        let _ = self.dispose();
    }
}

/// Thread-safe singleton pattern
pub struct ThreadSafeSingleton<T>
where
    T: Clone + Send + Sync + 'static,
{
    instance: Arc<Mutex<Option<T>>>,
}

impl<T: Clone + Send + Sync + 'static> ThreadSafeSingleton<T> {
    /// Create a new singleton
    pub fn new() -> Self {
        Self {
            instance: Arc::new(Mutex::new(None)),
        }
    }
    
    /// Get or create the singleton instance
    pub fn get_or_create<F>(&self, creator: F) -> Result<Arc<T>, String>
    where
        F: FnOnce() -> Result<T, String>,
    {
        if let Ok(mut instance_guard) = self.instance.lock() {
            if let Some(instance) = instance_guard.as_ref() {
                return Ok(Arc::new((*instance).clone()));
            } else {
                let new_instance = creator()?;
                let arc_instance = Arc::new(new_instance);
                *instance_guard = Some((*arc_instance).clone());
                return Ok(arc_instance);
            }
        }
        Err("Failed to acquire singleton lock".to_string())
    }
    
    /// Get the singleton instance (panics if not created)
    pub fn get(&self) -> Result<Arc<T>, String> {
        if let Ok(instance_guard) = self.instance.lock() {
            if let Some(instance) = instance_guard.as_ref() {
                return Ok(Arc::new((*instance).clone()));
            }
        }
        Err("Singleton instance not created".to_string())
    }
    
    /// Reset the singleton instance (for testing)
    pub fn reset(&self) -> Result<(), String> {
        if let Ok(mut instance_guard) = self.instance.lock() {
            *instance_guard = None;
            Ok(())
        } else {
            Err("Failed to acquire singleton lock".to_string())
        }
    }
}

/// Thread-safe async manager
pub struct ThreadSafeAsyncManager {
    manager: ThreadSafeManager,
    runtime: tokio::runtime::Runtime,
    task_sender: mpsc::UnboundedSender<tokio::task::JoinHandle<()>>,
}

impl ThreadSafeAsyncManager {
    /// Create a new async manager
    pub fn new(name: &str) -> Result<Self, String> {
        let runtime = tokio::runtime::Runtime::new()
            .map_err(|e| format!("Failed to create runtime: {}", e))?;
        
        let (task_sender, mut task_receiver) = mpsc::unbounded_channel();
        
        // Spawn task cleanup
        let runtime_clone = runtime.handle().clone();
        runtime.spawn(async move {
            while let Some(task) = task_receiver.recv().await {
                if let Err(e) = task.await {
                    eprintln!("Task failed: {:?}", e);
                }
            }
        });
        
        Ok(Self {
            manager: ThreadSafeManager::new(name),
            runtime,
            task_sender,
        })
    }
    
    /// Spawn an async task
    pub fn spawn_task<F, R>(&self, task: F) -> Result<(), String>
    where
        F: std::future::Future<Output = R> + Send + 'static,
        R: Send + 'static,
    {
        let handle = self.runtime.spawn(async move {
            let _ = task.await;
        });
        self.task_sender
            .send(handle)
            .map_err(|e| format!("Failed to send task: {}", e))?;
        Ok(())
    }
    
    /// Get the underlying manager
    pub fn manager(&self) -> &ThreadSafeManager {
        &self.manager
    }
    
    /// Get the runtime handle
    pub fn runtime_handle(&self) -> tokio::runtime::Handle {
        self.runtime.handle().clone()
    }
}

impl Drop for ThreadSafeAsyncManager {
    fn drop(&mut self) {
        let _ = self.manager.dispose();
    }
} 