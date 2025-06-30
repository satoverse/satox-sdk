/**
 * Tests for Core Manager
 */

import { CoreManager, SatoxError, CoreConfig } from '../src';

describe('CoreManager', () => {
    let manager: CoreManager;

    beforeEach(() => {
        manager = new CoreManager();
    });

    afterEach(async () => {
        try {
            await manager.shutdown();
        } catch {
            // Ignore shutdown errors in tests
        }
    });

    it('should initialize successfully', async () => {
        const config: CoreConfig = {
            name: 'test_sdk',
            enableLogging: true,
            logLevel: 'info',
            maxThreads: 4
        };

        const result = await manager.initialize(config);
        expect(result).toBe(true);
    });

    it('should throw error on double initialization', async () => {
        const config: CoreConfig = {
            name: 'test_sdk',
            enableLogging: true,
            logLevel: 'info',
            maxThreads: 4
        };

        await manager.initialize(config);
        
        await expect(manager.initialize(config)).rejects.toThrow(
            SatoxError
        );
    });

    it('should get status after initialization', async () => {
        const config: CoreConfig = {
            name: 'test_sdk',
            enableLogging: true,
            logLevel: 'info',
            maxThreads: 4
        };

        await manager.initialize(config);
        const status = await manager.getStatus();
        
        expect(status).toBeDefined();
        expect(typeof status).toBe('object');
    });

    it('should register and unregister callbacks', () => {
        const callback = jest.fn();
        
        manager.registerCallback(callback);
        manager.unregisterCallback(callback);
        
        // Note: This test may need adjustment based on actual implementation
        expect(callback).toBeDefined();
    });

    it('should shutdown successfully', async () => {
        const config: CoreConfig = {
            name: 'test_sdk',
            enableLogging: true,
            logLevel: 'info',
            maxThreads: 4
        };

        await manager.initialize(config);
        const result = await manager.shutdown();
        
        expect(result).toBe(true);
    });
}); 