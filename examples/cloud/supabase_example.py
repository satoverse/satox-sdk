#!/usr/bin/env python3
"""
@file supabase_example.py
@brief Python example usage of Supabase integration with Satox SDK
@copyright Copyright (c) 2025 Satoxcoin Core Developers
@license MIT License
"""

import json
import asyncio
from satox_bindings.database import DatabaseManager, DatabaseType

async def main():
    print("=== Satox SDK Supabase Integration Example (Python) ===")
    
    try:
        # Initialize database manager
        db_manager = DatabaseManager()
        
        # Load Supabase configuration
        with open('config/cloud/supabase_config.json', 'r') as f:
            config = json.load(f)
        
        # Connect to Supabase
        connection_id = await db_manager.connect(DatabaseType.SUPABASE, config)
        if not connection_id:
            print("Error: Failed to connect to Supabase")
            return
        
        print(f"Successfully connected to Supabase with ID: {connection_id}")
        
        # Example: Create a table
        create_table_query = """
            CREATE TABLE IF NOT EXISTS users (
                id SERIAL PRIMARY KEY,
                username VARCHAR(255) UNIQUE NOT NULL,
                email VARCHAR(255) UNIQUE NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """
        
        create_result = await db_manager.execute_query(connection_id, create_table_query)
        if create_result.success:
            print("Table created successfully")
        else:
            print(f"Error creating table: {create_result.error}")
        
        # Example: Insert data
        insert_query = """
            INSERT INTO users (username, email) 
            VALUES ('john_doe', 'john@example.com')
            ON CONFLICT (username) DO NOTHING
        """
        
        insert_result = await db_manager.execute_query(connection_id, insert_query)
        if insert_result.success:
            print("Data inserted successfully")
        else:
            print(f"Error inserting data: {insert_result.error}")
        
        # Example: Query data
        select_query = "SELECT * FROM users WHERE username = 'john_doe'"
        select_result = await db_manager.execute_query(connection_id, select_query)
        if select_result.success:
            print("Query executed successfully")
            print(f"Result: {json.dumps(select_result.rows, indent=2)}")
        else:
            print(f"Error querying data: {select_result.error}")
        
        # Example: Transaction
        transaction_queries = [
            "INSERT INTO users (username, email) VALUES ('jane_doe', 'jane@example.com')",
            "UPDATE users SET email = 'jane.updated@example.com' WHERE username = 'jane_doe'"
        ]
        
        transaction_result = await db_manager.execute_transaction(connection_id, transaction_queries)
        if transaction_result.success:
            print("Transaction executed successfully")
        else:
            print(f"Error executing transaction: {transaction_result.error}")
        
        # Disconnect
        disconnect_result = await db_manager.disconnect(connection_id)
        if disconnect_result:
            print("Disconnected from Supabase")
        else:
            print("Error disconnecting")
        
    except Exception as e:
        print(f"Exception: {e}")
    
    print("=== Example completed ===")

if __name__ == "__main__":
    asyncio.run(main())
