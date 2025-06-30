using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Satox.Core
{
    public interface IComponent
    {
        string Id { get; }
        string Name { get; }
        string Version { get; }
        Task<bool> InitializeAsync();
        Task ShutdownAsync();
    }

    public class SatoxConfig
    {
        public string LogLevel { get; set; } = "info";
        public List<string> Components { get; set; } = new List<string>();
    }

    public class SatoxManager
    {
        private readonly Dictionary<string, IComponent> _components;
        private readonly SatoxConfig _config;
        private bool _initialized;
        private string _lastError;

        public event EventHandler Initialized;
        public event EventHandler<IComponent> ComponentRegistered;
        public event EventHandler<IComponent> ComponentUnregistered;
        public event EventHandler Shutdown;
        public event EventHandler<SatoxConfig> ConfigUpdated;

        public SatoxManager(SatoxConfig config = null)
        {
            _components = new Dictionary<string, IComponent>();
            _config = config ?? new SatoxConfig();
            _initialized = false;
            _lastError = string.Empty;
        }

        public async Task<bool> InitializeAsync()
        {
            try
            {
                if (_initialized)
                {
                    return true;
                }

                // Initialize components
                foreach (var component in _components.Values)
                {
                    var success = await component.InitializeAsync();
                    if (!success)
                    {
                        _lastError = $"Failed to initialize component: {component.Name}";
                        return false;
                    }
                }

                _initialized = true;
                OnInitialized();
                return true;
            }
            catch (Exception ex)
            {
                _lastError = $"Initialization error: {ex.Message}";
                return false;
            }
        }

        public bool RegisterComponent(IComponent component)
        {
            try
            {
                if (_components.ContainsKey(component.Id))
                {
                    _lastError = $"Component already registered: {component.Id}";
                    return false;
                }

                _components.Add(component.Id, component);
                OnComponentRegistered(component);
                return true;
            }
            catch (Exception ex)
            {
                _lastError = $"Registration error: {ex.Message}";
                return false;
            }
        }

        public T GetComponent<T>(string id) where T : IComponent
        {
            return _components.TryGetValue(id, out var component) ? (T)component : default;
        }

        public bool IsComponentRegistered(string id)
        {
            return _components.ContainsKey(id);
        }

        public bool UnregisterComponent(string id)
        {
            try
            {
                if (!_components.TryGetValue(id, out var component))
                {
                    _lastError = $"Component not found: {id}";
                    return false;
                }

                _components.Remove(id);
                OnComponentUnregistered(component);
                return true;
            }
            catch (Exception ex)
            {
                _lastError = $"Unregistration error: {ex.Message}";
                return false;
            }
        }

        public async Task ShutdownAsync()
        {
            try
            {
                if (!_initialized)
                {
                    return;
                }

                // Shutdown components in reverse order
                var components = new List<IComponent>(_components.Values);
                components.Reverse();
                foreach (var component in components)
                {
                    await component.ShutdownAsync();
                }

                _components.Clear();
                _initialized = false;
                OnShutdown();
            }
            catch (Exception ex)
            {
                _lastError = $"Shutdown error: {ex.Message}";
                throw;
            }
        }

        public string GetLastError()
        {
            return _lastError;
        }

        public void ClearLastError()
        {
            _lastError = string.Empty;
        }

        public bool IsInitialized()
        {
            return _initialized;
        }

        public IReadOnlyList<IComponent> GetComponents()
        {
            return new List<IComponent>(_components.Values);
        }

        public SatoxConfig GetConfig()
        {
            return new SatoxConfig
            {
                LogLevel = _config.LogLevel,
                Components = new List<string>(_config.Components)
            };
        }

        public void UpdateConfig(SatoxConfig config)
        {
            _config.LogLevel = config.LogLevel;
            _config.Components = new List<string>(config.Components);
            OnConfigUpdated(_config);
        }

        protected virtual void OnInitialized()
        {
            Initialized?.Invoke(this, EventArgs.Empty);
        }

        protected virtual void OnComponentRegistered(IComponent component)
        {
            ComponentRegistered?.Invoke(this, component);
        }

        protected virtual void OnComponentUnregistered(IComponent component)
        {
            ComponentUnregistered?.Invoke(this, component);
        }

        protected virtual void OnShutdown()
        {
            Shutdown?.Invoke(this, EventArgs.Empty);
        }

        protected virtual void OnConfigUpdated(SatoxConfig config)
        {
            ConfigUpdated?.Invoke(this, config);
        }
    }

    // Export the manager instance
    public static class Satox
    {
        public static readonly SatoxManager Manager = new SatoxManager();
    }
} 