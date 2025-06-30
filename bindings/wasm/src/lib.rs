use wasm_bindgen::prelude::*;
use satox_core::{BlockchainManager, NFTManager, IPFSManager};

#[wasm_bindgen]
pub struct BlockchainClient {
    manager: BlockchainManager,
}

#[wasm_bindgen]
impl BlockchainClient {
    #[wasm_bindgen(constructor)]
    pub fn new(config: JsValue) -> Result<BlockchainClient, JsValue> {
        let config = serde_wasm_bindgen::from_value(config)?;
        Ok(BlockchainClient {
            manager: BlockchainManager::new(config),
        })
    }

    #[wasm_bindgen]
    pub async fn connect(&self) -> Result<(), JsValue> {
        self.manager.connect().await.map_err(|e| JsValue::from_str(&e.to_string()))
    }

    #[wasm_bindgen]
    pub async fn get_block_height(&self) -> Result<u64, JsValue> {
        self.manager.get_block_height().await.map_err(|e| JsValue::from_str(&e.to_string()))
    }

    #[wasm_bindgen]
    pub async fn get_block(&self, hash: &str) -> Result<JsValue, JsValue> {
        let block = self.manager.get_block(hash).await.map_err(|e| JsValue::from_str(&e.to_string()))?;
        Ok(serde_wasm_bindgen::to_value(&block)?)
    }
}

#[wasm_bindgen]
pub struct NFTClient {
    manager: NFTManager,
}

#[wasm_bindgen]
impl NFTClient {
    #[wasm_bindgen(constructor)]
    pub fn new(config: JsValue) -> Result<NFTClient, JsValue> {
        let config = serde_wasm_bindgen::from_value(config)?;
        Ok(NFTClient {
            manager: NFTManager::new(config),
        })
    }

    #[wasm_bindgen]
    pub async fn create_nft(&self, metadata: JsValue) -> Result<String, JsValue> {
        let metadata = serde_wasm_bindgen::from_value(metadata)?;
        self.manager.create_nft(metadata).await.map_err(|e| JsValue::from_str(&e.to_string()))
    }

    #[wasm_bindgen]
    pub async fn get_nft(&self, token_id: &str) -> Result<JsValue, JsValue> {
        let nft = self.manager.get_nft(token_id).await.map_err(|e| JsValue::from_str(&e.to_string()))?;
        Ok(serde_wasm_bindgen::to_value(&nft)?)
    }
}

#[wasm_bindgen]
pub struct IPFSClient {
    manager: IPFSManager,
}

#[wasm_bindgen]
impl IPFSClient {
    #[wasm_bindgen(constructor)]
    pub fn new(config: JsValue) -> Result<IPFSClient, JsValue> {
        let config = serde_wasm_bindgen::from_value(config)?;
        Ok(IPFSClient {
            manager: IPFSManager::new(config),
        })
    }

    #[wasm_bindgen]
    pub async fn add(&self, data: &[u8]) -> Result<String, JsValue> {
        self.manager.add(data).await.map_err(|e| JsValue::from_str(&e.to_string()))
    }

    #[wasm_bindgen]
    pub async fn get(&self, cid: &str) -> Result<Vec<u8>, JsValue> {
        self.manager.get(cid).await.map_err(|e| JsValue::from_str(&e.to_string()))
    }
} 