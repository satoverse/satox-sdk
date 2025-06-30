/**
 * @file firebase_example.ts
 * @brief TypeScript example usage of Firebase integration with Satox SDK
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 */

import { DatabaseManager, DatabaseType } from '../../bindings/typescript/src/database';
import * as fs from 'fs';

async function main() {
    console.log('=== Satox SDK Firebase Integration Example (TypeScript) ===');
    
    try {
        // Initialize database manager
        const dbManager = new DatabaseManager();
        
        // Load Firebase configuration
        const configData = fs.readFileSync('config/cloud/firebase_config.json', 'utf8');
        const config = JSON.parse(configData);
        
        // Connect to Firebase
        const connectionId = await dbManager.connect(DatabaseType.FIREBASE, config);
        if (!connectionId) {
            console.error('Error: Failed to connect to Firebase');
            return;
        }
        
        console.log(`Successfully connected to Firebase with ID: ${connectionId}`);
        
        // Example: Firestore operations
        console.log('\n--- Firestore Operations ---');
        
        // Create a document
        const createDocQuery = JSON.stringify({
            operation: 'set',
            collection: 'users',
            document_id: 'user123',
            data: {
                name: 'John Doe',
                email: 'john@example.com',
                age: 30,
                created_at: '2025-01-20T10:00:00Z'
            }
        });
        
        const createResult = await dbManager.executeQuery(connectionId, createDocQuery);
        if (createResult.success) {
            console.log('Document created successfully');
        } else {
            console.error('Error creating document:', createResult.error);
        }
        
        // Get a document
        const getDocQuery = JSON.stringify({
            operation: 'get',
            collection: 'users',
            document_id: 'user123'
        });
        
        const getResult = await dbManager.executeQuery(connectionId, getDocQuery);
        if (getResult.success) {
            console.log('Document retrieved successfully');
            console.log('Document data:', JSON.stringify(getResult.rows, null, 2));
        } else {
            console.error('Error retrieving document:', createResult.error);
        }
        
        // Update a document
        const updateDocQuery = JSON.stringify({
            operation: 'update',
            collection: 'users',
            document_id: 'user123',
            data: {
                age: 31,
                updated_at: '2025-01-20T11:00:00Z'
            }
        });
        
        const updateResult = await dbManager.executeQuery(connectionId, updateDocQuery);
        if (updateResult.success) {
            console.log('Document updated successfully');
        } else {
            console.error('Error updating document:', updateResult.error);
        }
        
        // Query documents
        const queryDocQuery = JSON.stringify({
            operation: 'query',
            collection: 'users',
            data: {
                where: {
                    field: 'age',
                    operator: '>=',
                    value: 25
                }
            }
        });
        
        const queryResult = await dbManager.executeQuery(connectionId, queryDocQuery);
        if (queryResult.success) {
            console.log('Query executed successfully');
            console.log('Query results:', JSON.stringify(queryResult.rows, null, 2));
        } else {
            console.error('Error querying documents:', queryResult.error);
        }
        
        // Delete a document
        const deleteDocQuery = JSON.stringify({
            operation: 'delete',
            collection: 'users',
            document_id: 'user123'
        });
        
        const deleteResult = await dbManager.executeQuery(connectionId, deleteDocQuery);
        if (deleteResult.success) {
            console.log('Document deleted successfully');
        } else {
            console.error('Error deleting document:', deleteResult.error);
        }
        
        // Example: Transaction
        console.log('\n--- Transaction Example ---');
        const transactionQueries = [
            JSON.stringify({
                operation: 'set',
                collection: 'users',
                document_id: 'user1',
                data: { name: 'Alice', email: 'alice@example.com' }
            }),
            JSON.stringify({
                operation: 'set',
                collection: 'users',
                document_id: 'user2',
                data: { name: 'Bob', email: 'bob@example.com' }
            })
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
            console.log('Disconnected from Firebase');
        } else {
            console.error('Error disconnecting');
        }
        
    } catch (error) {
        console.error('Exception:', error);
    }
    
    console.log('=== Example completed ===');
}

main().catch(console.error);
