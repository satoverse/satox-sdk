using System;
using System.Threading.Tasks;
using Satox.Bindings.Core;

namespace Satox.Core
{
    public class CoreManager : ICoreManager
    {
        private readonly ICoreManager _manager;

        public CoreManager()
        {
            _manager = new Satox.Bindings.Core.CoreManager();
        }

        public bool Initialize() => _manager.Initialize();
        public bool Shutdown() => _manager.Shutdown();
        public string GetVersion() => _manager.GetVersion();
        public string GetBuildInfo() => _manager.GetBuildInfo();
        public Task<bool> ValidateConfiguration() => _manager.ValidateConfiguration();
        public Task<bool> UpdateConfiguration() => _manager.UpdateConfiguration();
        public Task<bool> BackupData() => _manager.BackupData();
        public Task<bool> RestoreData() => _manager.RestoreData();
        public Task<bool> PerformMaintenance() => _manager.PerformMaintenance();
        public Task<bool> CheckHealth() => _manager.CheckHealth();
        public Task<bool> ResetToDefaults() => _manager.ResetToDefaults();
        public void Dispose() => _manager.Dispose();
    }
} 