const { expect } = require('chai');
const { SatoxManager } = require('../dist/satox-wasm');

describe('SatoxManager (WASM)', () => {
    let manager;

    beforeEach(async () => {
        manager = new SatoxManager();
        await manager.initialize();
    });

    it('should initialize successfully', async () => {
        expect(manager.isInitialized()).to.be.true;
    });

    it('should register components', async () => {
        const component = {
            getType: () => 'TestComponent',
            initialize: () => {},
            isInitialized: () => true
        };
        
        await manager.registerComponent(component);
        expect(await manager.isComponentRegistered('TestComponent')).to.be.true;
    });

    it('should handle errors properly', async () => {
        try {
            await manager.registerComponent(null);
            expect.fail('Should have thrown an error');
        } catch (error) {
            expect(error.message).to.include('Invalid component');
        }
    });

    it('should handle concurrent operations', async () => {
        const promises = Array(10).fill().map(async () => {
            const component = {
                getType: () => `TestComponent${Math.random()}`,
                initialize: () => {},
                isInitialized: () => true
            };
            await manager.registerComponent(component);
        });
        
        await Promise.all(promises);
        expect(manager.isInitialized()).to.be.true;
    });
}); 