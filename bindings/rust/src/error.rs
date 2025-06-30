use thiserror::Error;

/// Error types for the Satox SDK
#[derive(Error, Debug)]
pub enum SatoxError {
    #[error("SDK initialization failed")]
    InitializationFailed,
    
    #[error("Connection to blockchain network failed")]
    ConnectionFailed,
    
    #[error("Disconnection from blockchain network failed")]
    DisconnectionFailed,
    
    #[error("Invalid configuration: {0}")]
    InvalidConfiguration(String),
    
    #[error("Network error: {0}")]
    NetworkError(String),
    
    #[error("Blockchain error: {0}")]
    BlockchainError(String),
    
    #[error("Security error: {0}")]
    SecurityError(String),
    
    #[error("Asset error: {0}")]
    AssetError(String),
    
    #[error("NFT error: {0}")]
    NftError(String),
    
    #[error("IPFS error: {0}")]
    IpfsError(String),
    
    #[error("Serialization error: {0}")]
    SerializationError(String),
    
    #[error("Deserialization error: {0}")]
    DeserializationError(String),
    
    #[error("Unknown error: {0}")]
    Unknown(String),
}

impl From<std::ffi::NulError> for SatoxError {
    fn from(err: std::ffi::NulError) -> Self {
        SatoxError::SerializationError(err.to_string())
    }
}

impl From<serde_json::Error> for SatoxError {
    fn from(err: serde_json::Error) -> Self {
        SatoxError::SerializationError(err.to_string())
    }
}

/// Result type for Satox SDK operations
pub type SatoxResult<T> = Result<T, SatoxError>; 