import { SatoxManager, Component, Config } from '../src/satox';
import { TransactionManager, TransactionConfig, TransactionResult } from '../src/transaction';
import { PostQuantumAlgorithms } from '../src/quantum/post_quantum_algorithms';

describe('SatoxManager', () => {
    let satoxManager: SatoxManager;
    let config: Config;

    beforeEach(() => {
        config = {
            networkId: 'mainnet',
            apiEndpoint: 'http://localhost:7777',
            debugMode: true,
            maxRetries: 3,
            timeout: 5000,
            customConfig: {}
        };
        satoxManager = new SatoxManager(config);
    });

    afterEach(async () => {
        await satoxManager.shutdown();
    });

    // Basic Functionality Tests
    test('initialization and shutdown', async () => {
        await expect(satoxManager.initialize()).resolves.not.toThrow();
        expect(satoxManager.isInitialized()).toBe(true);
        await expect(satoxManager.shutdown()).resolves.not.toThrow();
        expect(satoxManager.isInitialized()).toBe(false);
    });

    // Component Management Tests
    test('component registration and retrieval', async () => {
        const component: Component = {
            initialize: async () => {},
            shutdown: async () => {},
            getName: () => 'test-component'
        };

        satoxManager.registerComponent(component);
        expect(satoxManager.getComponent('test-component')).toBe(component);
        expect(satoxManager.getComponentCount()).toBe(1);
        expect(satoxManager.listComponents()).toContain('test-component');
    });

    // Configuration Tests
    test('configuration management', () => {
        expect(satoxManager.getConfig()).toEqual(config);
        const newConfig = { ...config, debugMode: false };
        satoxManager.updateConfig(newConfig);
        expect(satoxManager.getConfig()).toEqual(newConfig);
    });

    // Transaction Tests
    test('transaction operations', async () => {
        const txConfig: TransactionConfig = {
            fee: 0.001,
            maxSize: 1000000,
            enableRBF: true
        };

        const txManager = new TransactionManager(txConfig);
        await txManager.initialize();

        const result = await txManager.createTransaction({
            inputs: [],
            outputs: [{ address: 'test-address', amount: 1.0 }]
        });

        expect(result).toBeInstanceOf(TransactionResult);
        expect(result.success).toBe(true);
    });

    // Quantum Tests
    test('quantum operations', async () => {
        const algorithms = new PostQuantumAlgorithms();
        await algorithms.initialize();

        expect(algorithms.isAlgorithmAvailable('FALCON-512')).toBe(true);
        expect(algorithms.isAlgorithmRecommended('FALCON-512')).toBe(true);
        expect(algorithms.getDefaultAlgorithm()).toBe('FALCON-512');
    });

    // Error Handling Tests
    test('error handling', async () => {
        const invalidComponent: Component = {
            initialize: async () => { throw new Error('Test error'); },
            shutdown: async () => {},
            getName: () => 'invalid-component'
        };

        satoxManager.registerComponent(invalidComponent);
        await expect(satoxManager.initialize()).rejects.toThrow('Failed to initialize component invalid-component: Test error');
    });

    // Concurrency Tests
    test('concurrent operations', async () => {
        const promises = Array(10).fill(null).map(async () => {
            const component: Component = {
                initialize: async () => {},
                shutdown: async () => {},
                getName: () => `component-${Math.random()}`
            };
            satoxManager.registerComponent(component);
            await component.initialize();
        });

        await Promise.all(promises);
        expect(satoxManager.getComponentCount()).toBe(10);
    });

    // Performance Tests
    test('performance', async () => {
        const start = performance.now();
        const promises = Array(1000).fill(null).map(async () => {
            const component: Component = {
                initialize: async () => {},
                shutdown: async () => {},
                getName: () => `component-${Math.random()}`
            };
            satoxManager.registerComponent(component);
            await component.initialize();
        });

        await Promise.all(promises);
        const duration = performance.now() - start;
        expect(duration).toBeLessThan(5000); // Should complete in less than 5 seconds
    });

    // Recovery Tests
    test('recovery after failure', async () => {
        await satoxManager.initialize();
        const component: Component = {
            initialize: async () => {},
            shutdown: async () => {},
            getName: () => 'test-component'
        };

        satoxManager.registerComponent(component);
        await satoxManager.shutdown();
        await satoxManager.initialize();

        expect(satoxManager.getComponent('test-component')).toBeDefined();
        expect(satoxManager.isInitialized()).toBe(true);
    });
}); 