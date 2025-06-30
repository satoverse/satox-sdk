const { SatoxManager } = require('../src/core');
const { expect } = require('chai');

describe('SatoxManager', () => {
    let manager;

    beforeEach(() => {
        manager = new SatoxManager();
    });

    it('should initialize successfully', () => {
        manager.initialize();
        expect(manager.isInitialized()).to.be.true;
    });

    it('should register components', () => {
        manager.initialize();
        
        const component = {
            getType: () => 'TestComponent',
            initialize: () => {},
            isInitialized: () => true
        };
        
        manager.registerComponent(component);
        expect(manager.isComponentRegistered('TestComponent')).to.be.true;
    });

    it('should handle errors properly', () => {
        manager.initialize();
        
        expect(() => {
            manager.registerComponent(null);
        }).to.throw('Invalid component');
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
        expect(manager.isInitialized()).to.be.true;
    });
}); 