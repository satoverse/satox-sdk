module.exports = {
    testEnvironment: 'node',
    transform: {
        '^.+\\.(js|jsx|ts|tsx)$': ['babel-jest', { 
            presets: [
                ['@babel/preset-env', { targets: { node: 'current' } }],
                '@babel/preset-typescript'
            ] 
        }]
    },
    moduleFileExtensions: ['js', 'jsx', 'ts', 'tsx'],
    testMatch: ['**/*.test.js', '**/*.test.ts'],
    transformIgnorePatterns: [
        'node_modules/(?!(chai|bindings)/)'
    ],
    setupFilesAfterEnv: ['<rootDir>/tests/setup.js'],
    testTimeout: 10000,
    globals: {
        'ts-jest': {
            tsconfig: 'tsconfig.json'
        }
    }
}; 