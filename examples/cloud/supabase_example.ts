/**
 * @file supabase_example.ts
 * @brief TypeScript example usage of Supabase integration with Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { DatabaseManager, DatabaseType } from '../../bindings/typescript/src/database';
import * as fs from 'fs';

async function main() {
    console.log('=== Satox SDK Supabase Integration Example (TypeScript) ===');
    
    try {
        // Initialize database manager
        const dbManager = new DatabaseManager();
        
        // Load Supabase configuration
        const configData = fs.readFileSync('config/cloud/supabase_config.json', 'utf8');
        const config = JSON.parse(configData);
        
        // Connect to Supabase
        const connectionId = await dbManager.connect(DatabaseType.SUPABASE, config);
        if (!connectionId) {
            console.error('Error: Failed to connect to Supabase');
            return;
        }
        
        console.log(`Successfully connected to Supabase with ID: ${connectionId}`);
        
        // Example: Create a table
        const createTableQuery = `
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(255) UNIQUE NOT NULL,
                email VARCHAR(255) UNIQUE NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        `;
        
        const createResult = await dbManager.executeQuery(connectionId, createTableQuery);
        if (createResult.success) {
            console.log('Table created successfully');
        } else {
            console.error('Error creating table:', createResult.error);
        }
        
        // Example: Insert data
        const insertQuery = `
            INSERT INTO users (username, email) 
            VALUES ('john_doe', 'john@example.com')
            ON CONFLICT (username) DO NOTHING
        `;
        
        const insertResult = await dbManager.executeQuery(connectionId, insertQuery);
        if (insertResult.success) {
            console.log('Data inserted successfully');
        } else {
            console.error('Error inserting data:', insertResult.error);
        }
        
        // Example: Query data
        const selectQuery = "SELECT * FROM users WHERE username = 'john_doe'";
        const selectResult = await dbManager.executeQuery(connectionId, selectQuery);
        if (selectResult.success) {
            console.log('Query executed successfully');
            console.log('Result:', JSON.stringify(selectResult.rows, null, 2));
        } else {
            console.error('Error querying data:', selectResult.error);
        }
        
        // Example: Transaction
        const transactionQueries = [
            "INSERT INTO users (username, email) VALUES ('jane_doe', 'jane@example.com')",
            "UPDATE users SET email = 'jane.updated@example.com' WHERE username = 'jane_doe'"
        ];
        
        const transactionResult = await dbManager.executeTransaction(connectionId, transactionQueries);
        if (transactionResult.success) {
            console.log('Transaction executed successfully');
        } else {
            console.error('Error executing transaction:', transactionResult.error);
        }
        
        // Disconnect
        const disconnectResult = await dbManager.disconnect(connectionId);
        if (disconnectResult) {
            console.log('Disconnected from Supabase');
        } else {
            console.error('Error disconnecting');
        }
        
    } catch (error) {
        console.error('Exception:', error);
    }
    
    console.log('=== Example completed ===');
}

main().catch(console.error);
