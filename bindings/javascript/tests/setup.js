// Jest setup file for Satox SDK JavaScript bindings tests

// Global test timeout
jest.setTimeout(10000);

// Mock the native bindings for testing
jest.mock('satox', () => ({
    post_quantum_algorithms_initialize: jest.fn(() => 0),
    post_quantum_algorithms_shutdown: jest.fn(() => 0),
    post_quantum_algorithms_get_algorithm_info: jest.fn(() => ({
        name: 'FALCON-512',
        security_level: 256,
        key_size: 32,
        signature_size: 64,
        is_recommended: 1,
        description: 'FALCON-512 post-quantum signature algorithm'
    })),
    post_quantum_algorithms_get_available_algorithms: jest.fn(() => ['FALCON-512', 'CRYSTALS-Kyber']),
    post_quantum_algorithms_is_algorithm_available: jest.fn(() => 1),
    post_quantum_algorithms_is_algorithm_recommended: jest.fn(() => 1),
    post_quantum_algorithms_get_default_algorithm: jest.fn(() => 'FALCON-512'),
    post_quantum_algorithms_destroy: jest.fn(() => 0),
    initialize: jest.fn(() => 0),
    shutdown: jest.fn(() => 0)
}));

// Suppress console warnings during tests
const originalWarn = console.warn;
console.warn = (...args) => {
    if (args[0] && typeof args[0] === 'string' && args[0].includes('Failed to initialize Satox SDK')) {
        return;
    }
    originalWarn(...args);
}; 