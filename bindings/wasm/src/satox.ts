/**
 * Satox SDK Core Module
 */

// Component interface
export interface Component {
    initialize(): Promise<void>;
    shutdown(): Promise<void>;
    getName(): string;
}

// Configuration interface
export interface Config {
    networkId: string;
    apiEndpoint: string;
    debugMode: boolean;
    maxRetries: number;
    timeout: number;
    customConfig: Record<string, any>;
}

// SatoxManager class
export class SatoxManager {
    private components: Map<string, Component>;
    private config: Config;

    constructor(config: Config) {
        this.components = new Map();
        this.config = config;
    }

    /**
     * Initialize the SatoxManager
     */
    async initialize(): Promise<void> {
        for (const component of this.components.values()) {
            try {
                await component.initialize();
            } catch (error) {
                throw new Error(`Failed to initialize component ${component.getName()}: ${error}`);
            }
        }
    }

    /**
     * Register a new component
     */
    registerComponent(component: Component): void {
        const name = component.getName();
        if (this.components.has(name)) {
            throw new Error(`Component ${name} already registered`);
        }
        this.components.set(name, component);
    }

    /**
     * Unregister a component
     */
    unregisterComponent(name: string): void {
        if (!this.components.has(name)) {
            throw new Error(`Component ${name} not found`);
        }
        this.components.delete(name);
    }

    /**
     * Get a component by name
     */
    getComponent(name: string): Component {
        const component = this.components.get(name);
        if (!component) {
            throw new Error(`Component ${name} not found`);
        }
        return component;
    }

    /**
     * Shutdown the SatoxManager and all components
     */
    async shutdown(): Promise<void> {
        for (const component of this.components.values()) {
            try {
                await component.shutdown();
            } catch (error) {
                throw new Error(`Failed to shutdown component ${component.getName()}: ${error}`);
            }
        }
    }

    /**
     * Get the current configuration
     */
    getConfig(): Config {
        return this.config;
    }

    /**
     * Update the configuration
     */
    updateConfig(config: Config): void {
        if (!config) {
            throw new Error('Config cannot be null');
        }
        this.config = config;
    }

    /**
     * Get the number of registered components
     */
    getComponentCount(): number {
        return this.components.size;
    }

    /**
     * Get a list of registered component names
     */
    listComponents(): string[] {
        return Array.from(this.components.keys());
    }
}

// Export the module
export default {
    SatoxManager,
    Component,
    Config
}; 