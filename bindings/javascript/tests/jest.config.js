module.exports = {
    testEnvironment: 'node',
    transform: {
        '^.+\\.(js|jsx|ts|tsx)$': ['babel-jest', { presets: ['@babel/preset-env', '@babel/preset-typescript'] }]
    },
    moduleFileExtensions: ['js', 'jsx', 'ts', 'tsx'],
    testMatch: ['**/*.test.js', '**/*.test.ts'],
    transformIgnorePatterns: [
        'node_modules/(?!(chai)/)'
    ],
    globals: {
        'ts-jest': {
            tsconfig: 'tsconfig.json'
        }
    }
}; 