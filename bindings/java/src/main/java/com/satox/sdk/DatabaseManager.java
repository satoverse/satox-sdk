package com.satox.sdk;

/**
 * Database Manager for the Satox SDK
 */
public class DatabaseManager {
    private final long handle;
    
    DatabaseManager(long handle) {
        this.handle = handle;
    }
    
    public int createDatabase(String name) {
        return SatoxSDK.createDatabase(handle, name);
    }
    
    public int deleteDatabase(String name) {
        return SatoxSDK.deleteDatabase(handle, name);
    }
    
    public String listDatabases() {
        return SatoxSDK.listDatabases(handle);
    }
    
    public int createCollection(String dbName, String collectionName) {
        return SatoxSDK.createCollection(handle, dbName, collectionName);
    }
    
    public int deleteCollection(String dbName, String collectionName) {
        return SatoxSDK.deleteCollection(handle, dbName, collectionName);
    }
    
    public String insertDocument(String dbName, String collectionName, String document) {
        return SatoxSDK.insertDocument(handle, dbName, collectionName, document);
    }
    
    public int updateDocument(String dbName, String collectionName, String query, String update) {
        return SatoxSDK.updateDocument(handle, dbName, collectionName, query, update);
    }
    
    public int deleteDocument(String dbName, String collectionName, String query) {
        return SatoxSDK.deleteDocument(handle, dbName, collectionName, query);
    }
    
    public String findDocuments(String dbName, String collectionName, String query) {
        return SatoxSDK.findDocuments(handle, dbName, collectionName, query);
    }
    
    public String getDocument(String dbName, String collectionName, String id) {
        return SatoxSDK.getDocument(handle, dbName, collectionName, id);
    }
    
    long getHandle() {
        return handle;
    }
} 