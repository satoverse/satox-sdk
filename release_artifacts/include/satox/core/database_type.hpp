/**
 * @brief Supported database types, including cloud providers
 */
enum class DatabaseType {
    SQLITE,
    POSTGRESQL,
    MYSQL,
    REDIS,
    MONGODB,
    ROCKSDB,
    SUPABASE,
    FIREBASE,
    AWS,         ///< Amazon Web Services (RDS, DynamoDB, etc.)
    AZURE,       ///< Microsoft Azure (SQL, CosmosDB, etc.)
    GOOGLE_CLOUD ///< Google Cloud Platform (Cloud SQL, Firestore, etc.)
}; 