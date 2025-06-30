using System;
using System.Runtime.InteropServices;

namespace Satox.Core
{
    /// <summary>
    /// SatoxManager provides core functionality for the Satox SDK.
    /// </summary>
    public class SatoxManager : IDisposable
    {
        private bool _initialized;
        private bool _disposed;

        /// <summary>
        /// Creates a new instance of SatoxManager.
        /// </summary>
        public SatoxManager()
        {
            _initialized = false;
            _disposed = false;
        }

        /// <summary>
        /// Initializes the Satox manager.
        /// </summary>
        public void Initialize()
        {
            if (_initialized)
                return;

            var status = NativeMethods.satox_manager_initialize();
            if (status != 0)
                throw new SatoxError("Failed to initialize Satox manager");

            _initialized = true;
        }

        /// <summary>
        /// Shuts down the Satox manager.
        /// </summary>
        public void Shutdown()
        {
            if (!_initialized)
                return;

            var status = NativeMethods.satox_manager_shutdown();
            if (status != 0)
                throw new SatoxError("Failed to shutdown Satox manager");

            _initialized = false;
        }

        /// <summary>
        /// Gets the version of the Satox SDK.
        /// </summary>
        /// <returns>The version string.</returns>
        public string GetVersion()
        {
            EnsureInitialized();

            try
            {
                IntPtr versionPtr = IntPtr.Zero;
                var status = NativeMethods.satox_manager_get_version(ref versionPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get version");

                var version = Marshal.PtrToStringAnsi(versionPtr);
                NativeMethods.satox_manager_free_buffer(versionPtr);

                return version;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get version: {ex.Message}");
            }
        }

        /// <summary>
        /// Registers a component with the Satox manager.
        /// </summary>
        /// <param name="componentName">The name of the component to register.</param>
        /// <param name="component">The component instance.</param>
        public void RegisterComponent(string componentName, object component)
        {
            EnsureInitialized();
            ValidateInput(componentName, nameof(componentName));
            ValidateInput(component, nameof(component));

            try
            {
                var status = NativeMethods.satox_manager_register_component(componentName, component);
                if (status != 0)
                    throw new SatoxError("Failed to register component");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to register component: {ex.Message}");
            }
        }

        /// <summary>
        /// Unregisters a component from the Satox manager.
        /// </summary>
        /// <param name="componentName">The name of the component to unregister.</param>
        public void UnregisterComponent(string componentName)
        {
            EnsureInitialized();
            ValidateInput(componentName, nameof(componentName));

            try
            {
                var status = NativeMethods.satox_manager_unregister_component(componentName);
                if (status != 0)
                    throw new SatoxError("Failed to unregister component");
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to unregister component: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a registered component by name.
        /// </summary>
        /// <typeparam name="T">The type of the component.</typeparam>
        /// <param name="componentName">The name of the component to get.</param>
        /// <returns>The component instance.</returns>
        public T GetComponent<T>(string componentName)
        {
            EnsureInitialized();
            ValidateInput(componentName, nameof(componentName));

            try
            {
                IntPtr componentPtr = IntPtr.Zero;
                var status = NativeMethods.satox_manager_get_component(componentName, ref componentPtr);

                if (status != 0)
                    throw new SatoxError("Failed to get component");

                var component = Marshal.GetObjectForIUnknown(componentPtr) as T;
                if (component == null)
                    throw new SatoxError("Failed to cast component to requested type");

                return component;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get component: {ex.Message}");
            }
        }

        /// <summary>
        /// Checks if a component is registered.
        /// </summary>
        /// <param name="componentName">The name of the component to check.</param>
        /// <returns>True if the component is registered, false otherwise.</returns>
        public bool IsComponentRegistered(string componentName)
        {
            EnsureInitialized();
            ValidateInput(componentName, nameof(componentName));

            try
            {
                int isRegistered = 0;
                var status = NativeMethods.satox_manager_is_component_registered(componentName, ref isRegistered);
                return status == 0 && isRegistered != 0;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to check component registration: {ex.Message}");
            }
        }

        /// <summary>
        /// Gets a list of registered components.
        /// </summary>
        /// <returns>An array of component names.</returns>
        public string[] GetRegisteredComponents()
        {
            EnsureInitialized();

            try
            {
                IntPtr componentsPtr = IntPtr.Zero;
                int count = 0;
                var status = NativeMethods.satox_manager_get_registered_components(ref componentsPtr, ref count);

                if (status != 0)
                    throw new SatoxError("Failed to get registered components");

                var components = new string[count];
                var currentPtr = componentsPtr;

                for (int i = 0; i < count; i++)
                {
                    components[i] = Marshal.PtrToStringAnsi(Marshal.ReadIntPtr(currentPtr));
                    currentPtr = IntPtr.Add(currentPtr, IntPtr.Size);
                }

                NativeMethods.satox_manager_free_buffer(componentsPtr);
                return components;
            }
            catch (Exception ex)
            {
                throw new SatoxError($"Failed to get registered components: {ex.Message}");
            }
        }

        private void EnsureInitialized()
        {
            if (!_initialized)
                throw new SatoxError("Satox manager is not initialized");
        }

        private void ValidateInput(object input, string paramName)
        {
            if (input == null)
                throw new ArgumentNullException(paramName);
        }

        private void ValidateInput(string input, string paramName)
        {
            if (string.IsNullOrEmpty(input))
                throw new ArgumentException("Value cannot be null or empty", paramName);
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    if (_initialized)
                    {
                        Shutdown();
                    }
                }

                _disposed = true;
            }
        }

        ~SatoxManager()
        {
            Dispose(false);
        }

        private static class NativeMethods
        {
            [DllImport("satox_core")]
            public static extern int satox_manager_initialize();

            [DllImport("satox_core")]
            public static extern int satox_manager_shutdown();

            [DllImport("satox_core")]
            public static extern int satox_manager_get_version(ref IntPtr version);

            [DllImport("satox_core")]
            public static extern int satox_manager_register_component(string componentName, object component);

            [DllImport("satox_core")]
            public static extern int satox_manager_unregister_component(string componentName);

            [DllImport("satox_core")]
            public static extern int satox_manager_get_component(string componentName, ref IntPtr component);

            [DllImport("satox_core")]
            public static extern int satox_manager_is_component_registered(string componentName, ref int isRegistered);

            [DllImport("satox_core")]
            public static extern int satox_manager_get_registered_components(ref IntPtr components, ref int count);

            [DllImport("satox_core")]
            public static extern void satox_manager_free_buffer(IntPtr buffer);
        }
    }
} 