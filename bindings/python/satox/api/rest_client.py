"""REST API Client for Satox."""

import aiohttp
import json
from typing import Any, Dict, List, Optional, Union
from satox_bindings.core.error_handling import SatoxError
from satox_bindings.core.config_manager import ConfigManager

class RESTClient:
    """REST API Client for interacting with Satox services."""

    def __init__(self, config_manager: ConfigManager):
        """Initialize the REST client.
        
        Args:
            config_manager: Configuration manager instance
        """
        self.config = config_manager
        self.session: Optional[aiohttp.ClientSession] = None
        self.base_url = self.config.get("api.rest.base_url")
        self.timeout = self.config.get("api.rest.timeout", 30)
        self.max_retries = self.config.get("api.rest.max_retries", 3)
        self.retry_delay = self.config.get("api.rest.retry_delay", 1)

    async def initialize(self) -> bool:
        """Initialize the REST client.
        
        Returns:
            bool: True if initialization was successful
        """
        try:
            self.session = aiohttp.ClientSession(
                timeout=aiohttp.ClientTimeout(total=self.timeout)
            )
            return True
        except Exception as e:
            raise SatoxError(f"Failed to initialize REST client: {str(e)}")

    async def shutdown(self) -> bool:
        """Shutdown the REST client.
        
        Returns:
            bool: True if shutdown was successful
        """
        try:
            if self.session:
                await self.session.close()
            return True
        except Exception as e:
            raise SatoxError(f"Failed to shutdown REST client: {str(e)}")

    async def _make_request(
        self,
        method: str,
        endpoint: str,
        params: Optional[Dict[str, Any]] = None,
        data: Optional[Dict[str, Any]] = None,
        headers: Optional[Dict[str, str]] = None,
        retry_count: int = 0
    ) -> Dict[str, Any]:
        """Make an HTTP request.
        
        Args:
            method: HTTP method
            endpoint: API endpoint
            params: Query parameters
            data: Request body
            headers: Request headers
            retry_count: Current retry attempt
            
        Returns:
            Dict[str, Any]: Response data
            
        Raises:
            SatoxError: If the request fails
        """
        if not self.session:
            raise SatoxError("REST client not initialized")

        headers = headers or {}
        headers.update({
            "Content-Type": "application/json",
            "Accept": "application/json"
        })

        url = f"{self.base_url.rstrip('/')}/{endpoint.lstrip('/')}"

        try:
            async with self.session.request(
                method,
                url,
                params=params,
                json=data,
                headers=headers
            ) as response:
                if response.status == 429 and retry_count < self.max_retries:
                    await asyncio.sleep(self.retry_delay * (2 ** retry_count))
                    return await self._make_request(
                        method,
                        endpoint,
                        params,
                        data,
                        headers,
                        retry_count + 1
                    )

                response.raise_for_status()
                return await response.json()

        except aiohttp.ClientError as e:
            if retry_count < self.max_retries:
                await asyncio.sleep(self.retry_delay * (2 ** retry_count))
                return await self._make_request(
                    method,
                    endpoint,
                    params,
                    data,
                    headers,
                    retry_count + 1
                )
            raise SatoxError(f"REST request failed: {str(e)}")

    # Asset endpoints
    async def get_asset(self, asset_id: str) -> Dict[str, Any]:
        """Get asset information.
        
        Args:
            asset_id: Asset identifier
            
        Returns:
            Dict[str, Any]: Asset information
        """
        return await self._make_request("GET", f"assets/{asset_id}")

    async def list_assets(
        self,
        page: int = 1,
        per_page: int = 10,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """List assets.
        
        Args:
            page: Page number
            per_page: Items per page
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of assets
        """
        params = {
            "page": page,
            "per_page": per_page,
            **(filter or {})
        }
        return await self._make_request("GET", "assets", params=params)

    async def create_asset(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Create a new asset.
        
        Args:
            data: Asset data
            
        Returns:
            Dict[str, Any]: Created asset
        """
        return await self._make_request("POST", "assets", data=data)

    async def update_asset(self, asset_id: str, data: Dict[str, Any]) -> Dict[str, Any]:
        """Update an asset.
        
        Args:
            asset_id: Asset identifier
            data: Asset data
            
        Returns:
            Dict[str, Any]: Updated asset
        """
        return await self._make_request("PUT", f"assets/{asset_id}", data=data)

    async def delete_asset(self, asset_id: str) -> bool:
        """Delete an asset.
        
        Args:
            asset_id: Asset identifier
            
        Returns:
            bool: True if deletion was successful
        """
        await self._make_request("DELETE", f"assets/{asset_id}")
        return True

    # NFT endpoints
    async def get_nft(self, nft_id: str) -> Dict[str, Any]:
        """Get NFT information.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            Dict[str, Any]: NFT information
        """
        return await self._make_request("GET", f"nfts/{nft_id}")

    async def list_nfts(
        self,
        page: int = 1,
        per_page: int = 10,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """List NFTs.
        
        Args:
            page: Page number
            per_page: Items per page
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of NFTs
        """
        params = {
            "page": page,
            "per_page": per_page,
            **(filter or {})
        }
        return await self._make_request("GET", "nfts", params=params)

    async def create_nft(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Create a new NFT.
        
        Args:
            data: NFT data
            
        Returns:
            Dict[str, Any]: Created NFT
        """
        return await self._make_request("POST", "nfts", data=data)

    async def update_nft(self, nft_id: str, data: Dict[str, Any]) -> Dict[str, Any]:
        """Update an NFT.
        
        Args:
            nft_id: NFT identifier
            data: NFT data
            
        Returns:
            Dict[str, Any]: Updated NFT
        """
        return await self._make_request("PUT", f"nfts/{nft_id}", data=data)

    async def delete_nft(self, nft_id: str) -> bool:
        """Delete an NFT.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            bool: True if deletion was successful
        """
        await self._make_request("DELETE", f"nfts/{nft_id}")
        return True

    # Transaction endpoints
    async def get_transaction(self, tx_id: str) -> Dict[str, Any]:
        """Get transaction information.
        
        Args:
            tx_id: Transaction identifier
            
        Returns:
            Dict[str, Any]: Transaction information
        """
        return await self._make_request("GET", f"transactions/{tx_id}")

    async def list_transactions(
        self,
        page: int = 1,
        per_page: int = 10,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """List transactions.
        
        Args:
            page: Page number
            per_page: Items per page
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of transactions
        """
        params = {
            "page": page,
            "per_page": per_page,
            **(filter or {})
        }
        return await self._make_request("GET", "transactions", params=params)

    async def create_transaction(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Create a new transaction.
        
        Args:
            data: Transaction data
            
        Returns:
            Dict[str, Any]: Created transaction
        """
        return await self._make_request("POST", "transactions", data=data)

    # Block endpoints
    async def get_block(self, block_id: str) -> Dict[str, Any]:
        """Get block information.
        
        Args:
            block_id: Block identifier
            
        Returns:
            Dict[str, Any]: Block information
        """
        return await self._make_request("GET", f"blocks/{block_id}")

    async def list_blocks(
        self,
        page: int = 1,
        per_page: int = 10,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """List blocks.
        
        Args:
            page: Page number
            per_page: Items per page
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of blocks
        """
        params = {
            "page": page,
            "per_page": per_page,
            **(filter or {})
        }
        return await self._make_request("GET", "blocks", params=params)

    # Wallet endpoints
    async def get_wallet(self, wallet_id: str) -> Dict[str, Any]:
        """Get wallet information.
        
        Args:
            wallet_id: Wallet identifier
            
        Returns:
            Dict[str, Any]: Wallet information
        """
        return await self._make_request("GET", f"wallets/{wallet_id}")

    async def list_wallets(
        self,
        page: int = 1,
        per_page: int = 10,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """List wallets.
        
        Args:
            page: Page number
            per_page: Items per page
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of wallets
        """
        params = {
            "page": page,
            "per_page": per_page,
            **(filter or {})
        }
        return await self._make_request("GET", "wallets", params=params)

    async def create_wallet(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """Create a new wallet.
        
        Args:
            data: Wallet data
            
        Returns:
            Dict[str, Any]: Created wallet
        """
        return await self._make_request("POST", "wallets", data=data)

    async def update_wallet(self, wallet_id: str, data: Dict[str, Any]) -> Dict[str, Any]:
        """Update a wallet.
        
        Args:
            wallet_id: Wallet identifier
            data: Wallet data
            
        Returns:
            Dict[str, Any]: Updated wallet
        """
        return await self._make_request("PUT", f"wallets/{wallet_id}", data=data)

    async def delete_wallet(self, wallet_id: str) -> bool:
        """Delete a wallet.
        
        Args:
            wallet_id: Wallet identifier
            
        Returns:
            bool: True if deletion was successful
        """
        await self._make_request("DELETE", f"wallets/{wallet_id}")
        return True

    # Network endpoints
    async def get_network_status(self) -> Dict[str, Any]:
        """Get network status information.
        
        Returns:
            Dict[str, Any]: Network status information
        """
        return await self._make_request("GET", "network/status")

    async def get_node_info(self) -> Dict[str, Any]:
        """Get node information.
        
        Returns:
            Dict[str, Any]: Node information
        """
        return await self._make_request("GET", "network/node")

    async def get_peers(self) -> Dict[str, Any]:
        """Get peer information.
        
        Returns:
            Dict[str, Any]: Peer information
        """
        return await self._make_request("GET", "network/peers")

    async def get_sync_status(self) -> Dict[str, Any]:
        """Get sync status information.
        
        Returns:
            Dict[str, Any]: Sync status information
        """
        return await self._make_request("GET", "network/sync") 