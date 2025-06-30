using System;
using System.Threading.Tasks;
using Xunit;
using SatoxSDK;

namespace SatoxSDK.Tests
{
    public class CoreTests
    {
        [Fact]
        public async Task TestSatoxManagerInitialization()
        {
            var manager = new SatoxManager();
            Assert.NotNull(manager);
            await manager.InitializeAsync();
            Assert.True(manager.IsInitialized);
        }

        [Fact]
        public async Task TestComponentRegistration()
        {
            var manager = new SatoxManager();
            await manager.InitializeAsync();
            
            var component = new TestComponent();
            manager.RegisterComponent(component);
            
            Assert.True(manager.IsComponentRegistered(component.Id));
        }

        [Fact]
        public async Task TestErrorHandling()
        {
            var manager = new SatoxManager();
            await manager.InitializeAsync();
            
            Assert.Throws<ArgumentException>(() => 
            {
                manager.RegisterComponent(null);
            });
        }

        [Fact]
        public async Task TestThreadSafety()
        {
            var manager = new SatoxManager();
            await manager.InitializeAsync();
            
            var tasks = new Task[10];
            for (int i = 0; i < 10; i++)
            {
                tasks[i] = Task.Run(() =>
                {
                    var component = new TestComponent();
                    manager.RegisterComponent(component);
                });
            }
            
            await Task.WhenAll(tasks);
            Assert.True(manager.IsInitialized);
        }
    }

    public class TestComponent : IComponent
    {
        public string Id => "TestComponent";
        public string Name => "TestComponent";
        public string Version => "1.0.0";
        public bool IsInitialized { get; private set; }

        public async Task<bool> InitializeAsync()
        {
            IsInitialized = true;
            await Task.CompletedTask;
            return true;
        }

        public async Task ShutdownAsync()
        {
            IsInitialized = false;
            await Task.CompletedTask;
        }
    }
} 