#!/usr/bin/env python3
"""
@file firebase_example.py
@brief Python example usage of Firebase integration with Satox SDK
@copyright Copyright (c) 2025 Satoxcoin Core Developers
@license MIT License
"""

import json
import asyncio
from satox_bindings.database import DatabaseManager, DatabaseType

async def main():
    print("=== Satox SDK Firebase Integration Example (Python) ===")
    
    try:
        # Initialize database manager
        db_manager = DatabaseManager()
        
        # Load Firebase configuration
        with open('config/cloud/firebase_config.json', 'r') as f:
            config = json.load(f)
        
        # Connect to Firebase
        connection_id = await db_manager.connect(DatabaseType.FIREBASE, config)
        if not connection_id:
            print("Error: Failed to connect to Firebase")
            return
        
        print(f"Successfully connected to Firebase with ID: {connection_id}")
        
        # Example: Firestore operations
        print("\n--- Firestore Operations ---")
        
        # Create a document
        create_doc_query = json.dumps({
            "operation": "set",
            "collection": "users",
            "document_id": "user123",
            "data": {
                "name": "John Doe",
                "email": "john@example.com",
                "age": 30,
                "created_at": "2025-01-20T10:00:00Z"
            }
        })
        
        create_result = await db_manager.execute_query(connection_id, create_doc_query)
        if create_result.success:
            print("Document created successfully")
        else:
            print(f"Error creating document: {create_result.error}")
        
        # Get a document
        get_doc_query = json.dumps({
            "operation": "get",
            "collection": "users",
            "document_id": "user123"
        })
        
        get_result = await db_manager.execute_query(connection_id, get_doc_query)
        if get_result.success:
            print("Document retrieved successfully")
            print(f"Document data: {json.dumps(get_result.rows, indent=2)}")
        else:
            print(f"Error retrieving document: {get_result.error}")
        
        # Update a document
        update_doc_query = json.dumps({
            "operation": "update",
            "collection": "users",
            "document_id": "user123",
            "data": {
                "age": 31,
                "updated_at": "2025-01-20T11:00:00Z"
            }
        })
        
        update_result = await db_manager.execute_query(connection_id, update_doc_query)
        if update_result.success:
            print("Document updated successfully")
        else:
            print(f"Error updating document: {update_result.error}")
        
        # Query documents
        query_doc_query = json.dumps({
            "operation": "query",
            "collection": "users",
            "data": {
                "where": {
                    "field": "age",
                    "operator": ">=",
                    "value": 25
                }
            }
        })
        
        query_result = await db_manager.execute_query(connection_id, query_doc_query)
        if query_result.success:
            print("Query executed successfully")
            print(f"Query results: {json.dumps(query_result.rows, indent=2)}")
        else:
            print(f"Error querying documents: {query_result.error}")
        
        # Delete a document
        delete_doc_query = json.dumps({
            "operation": "delete",
            "collection": "users",
            "document_id": "user123"
        })
        
        delete_result = await db_manager.execute_query(connection_id, delete_doc_query)
        if delete_result.success:
            print("Document deleted successfully")
        else:
            print(f"Error deleting document: {delete_result.error}")
        
        # Example: Transaction
        print("\n--- Transaction Example ---")
        transaction_queries = [
            json.dumps({
                "operation": "set",
                "collection": "users",
                "document_id": "user1",
                "data": {"name": "Alice", "email": "alice@example.com"}
            }),
            json.dumps({
                "operation": "set",
                "collection": "users",
                "document_id": "user2",
                "data": {"name": "Bob", "email": "bob@example.com"}
            })
        ]
        
        transaction_result = await db_manager.execute_transaction(connection_id, transaction_queries)
        if transaction_result.success:
            print("Transaction executed successfully")
        else:
            print(f"Error executing transaction: {transaction_result.error}")
        
        # Disconnect
        disconnect_result = await db_manager.disconnect(connection_id)
        if disconnect_result:
            print("Disconnected from Firebase")
        else:
            print("Error disconnecting")
        
    except Exception as e:
        print(f"Exception: {e}")
    
    print("=== Example completed ===")

if __name__ == "__main__":
    asyncio.run(main())
