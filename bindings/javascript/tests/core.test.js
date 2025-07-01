const { SatoxManager } = require('../src/core');

describe('SatoxManager', () => {
    let manager;

    beforeEach(() => {
        manager = new SatoxManager();
    });

    it('should initialize successfully', () => {
        manager.initialize();
        expect(manager.isInitialized()).toBe(true);
    });

    it('should register components', () => {
        manager.initialize();
        
        const component = {
            getType: () => 'TestComponent',
            initialize: () => {},
            isInitialized: () => true
        };
        
        manager.registerComponent(component);
        expect(manager.isComponentRegistered('TestComponent')).toBe(true);
    });

    it('should handle errors properly', () => {
        manager.initialize();
        
        expect(() => {
            manager.registerComponent(null);
        }).toThrow('Invalid component');
    });

    it('should handle concurrent operations', async () => {
        manager.initialize();
        
        const promises = Array(10).fill().map(async () => {
            const component = {
                getType: () => `TestComponent${Math.random()}`,
                initialize: () => {},
                isInitialized: () => true
            };
            manager.registerComponent(component);
        });
        
        await Promise.all(promises);
        expect(manager.isInitialized()).toBe(true);
    });
}); 