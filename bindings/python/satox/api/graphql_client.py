"""GraphQL API Client for Satox."""

import aiohttp
import json
from typing import Any, Dict, List, Optional, Union
from satox_bindings.core.error_handling import SatoxError
from satox_bindings.core.config_manager import ConfigManager

class GraphQLClient:
    """GraphQL API Client for interacting with Satox services."""

    def __init__(self, config_manager: ConfigManager):
        """Initialize the GraphQL client.
        
        Args:
            config_manager: Configuration manager instance
        """
        self.config = config_manager
        self.session: Optional[aiohttp.ClientSession] = None
        self.base_url = self.config.get("api.graphql.base_url")
        self.timeout = self.config.get("api.graphql.timeout", 30)
        self.max_retries = self.config.get("api.graphql.max_retries", 3)
        self.retry_delay = self.config.get("api.graphql.retry_delay", 1)

    async def initialize(self) -> bool:
        """Initialize the GraphQL client.
        
        Returns:
            bool: True if initialization was successful
        """
        try:
            self.session = aiohttp.ClientSession(
                timeout=aiohttp.ClientTimeout(total=self.timeout)
            )
            return True
        except Exception as e:
            raise SatoxError(f"Failed to initialize GraphQL client: {str(e)}")

    async def shutdown(self) -> bool:
        """Shutdown the GraphQL client.
        
        Returns:
            bool: True if shutdown was successful
        """
        try:
            if self.session:
                await self.session.close()
            return True
        except Exception as e:
            raise SatoxError(f"Failed to shutdown GraphQL client: {str(e)}")

    async def _execute_query(
        self,
        query: str,
        variables: Optional[Dict[str, Any]] = None,
        operation_name: Optional[str] = None,
        headers: Optional[Dict[str, str]] = None,
        retry_count: int = 0
    ) -> Dict[str, Any]:
        """Execute a GraphQL query.
        
        Args:
            query: GraphQL query string
            variables: Query variables
            operation_name: Operation name
            headers: Request headers
            retry_count: Current retry attempt
            
        Returns:
            Dict[str, Any]: Query response
            
        Raises:
            SatoxError: If the query fails
        """
        if not self.session:
            raise SatoxError("GraphQL client not initialized")

        headers = headers or {}
        headers.update({
            "Content-Type": "application/json",
            "Accept": "application/json"
        })

        payload = {
            "query": query,
            "variables": variables or {},
            "operationName": operation_name
        }

        try:
            async with self.session.post(
                self.base_url,
                json=payload,
                headers=headers
            ) as response:
                if response.status == 429 and retry_count < self.max_retries:
                    await asyncio.sleep(self.retry_delay * (2 ** retry_count))
                    return await self._execute_query(
                        query,
                        variables,
                        operation_name,
                        headers,
                        retry_count + 1
                    )

                response.raise_for_status()
                result = await response.json()

                if "errors" in result:
                    raise SatoxError(f"GraphQL query failed: {result['errors']}")

                return result["data"]

        except aiohttp.ClientError as e:
            if retry_count < self.max_retries:
                await asyncio.sleep(self.retry_delay * (2 ** retry_count))
                return await self._execute_query(
                    query,
                    variables,
                    operation_name,
                    headers,
                    retry_count + 1
                )
            raise SatoxError(f"GraphQL query failed: {str(e)}")

    async def get_asset(self, asset_id: str) -> Dict[str, Any]:
        """Get asset information.
        
        Args:
            asset_id: Asset identifier
            
        Returns:
            Dict[str, Any]: Asset information
        """
        query = """
        query GetAsset($id: ID!) {
            asset(id: $id) {
                id
                name
                symbol
                totalSupply
                decimals
                owner
                metadata
                createdAt
                updatedAt
            }
        }
        """
        return await self._execute_query(query, {"id": asset_id})

    async def get_assets(
        self,
        first: int = 10,
        after: Optional[str] = None,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """Get a list of assets.
        
        Args:
            first: Number of items to return
            after: Cursor for pagination
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of assets
        """
        query = """
        query GetAssets($first: Int!, $after: String, $filter: AssetFilter) {
            assets(first: $first, after: $after, filter: $filter) {
                edges {
                    node {
                        id
                        name
                        symbol
                        totalSupply
                        decimals
                        owner
                        metadata
                        createdAt
                        updatedAt
                    }
                    cursor
                }
                pageInfo {
                    hasNextPage
                    endCursor
                }
            }
        }
        """
        return await self._execute_query(query, {
            "first": first,
            "after": after,
            "filter": filter
        })

    async def get_nft(self, nft_id: str) -> Dict[str, Any]:
        """Get NFT information.
        
        Args:
            nft_id: NFT identifier
            
        Returns:
            Dict[str, Any]: NFT information
        """
        query = """
        query GetNFT($id: ID!) {
            nft(id: $id) {
                id
                tokenId
                contract
                owner
                metadata
                createdAt
                updatedAt
            }
        }
        """
        return await self._execute_query(query, {"id": nft_id})

    async def get_nfts(
        self,
        first: int = 10,
        after: Optional[str] = None,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """Get a list of NFTs.
        
        Args:
            first: Number of items to return
            after: Cursor for pagination
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of NFTs
        """
        query = """
        query GetNFTs($first: Int!, $after: String, $filter: NFTFilter) {
            nfts(first: $first, after: $after, filter: $filter) {
                edges {
                    node {
                        id
                        tokenId
                        contract
                        owner
                        metadata
                        createdAt
                        updatedAt
                    }
                    cursor
                }
                pageInfo {
                    hasNextPage
                    endCursor
                }
            }
        }
        """
        return await self._execute_query(query, {
            "first": first,
            "after": after,
            "filter": filter
        })

    async def get_transaction(self, tx_id: str) -> Dict[str, Any]:
        """Get transaction information.
        
        Args:
            tx_id: Transaction identifier
            
        Returns:
            Dict[str, Any]: Transaction information
        """
        query = """
        query GetTransaction($id: ID!) {
            transaction(id: $id) {
                id
                hash
                block
                sender
                receiver
                amount
                fee
                status
                timestamp
                data
            }
        }
        """
        return await self._execute_query(query, {"id": tx_id})

    async def get_transactions(
        self,
        first: int = 10,
        after: Optional[str] = None,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """Get a list of transactions.
        
        Args:
            first: Number of items to return
            after: Cursor for pagination
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of transactions
        """
        query = """
        query GetTransactions($first: Int!, $after: String, $filter: TransactionFilter) {
            transactions(first: $first, after: $after, filter: $filter) {
                edges {
                    node {
                        id
                        hash
                        block
                        sender
                        receiver
                        amount
                        fee
                        status
                        timestamp
                        data
                    }
                    cursor
                }
                pageInfo {
                    hasNextPage
                    endCursor
                }
            }
        }
        """
        return await self._execute_query(query, {
            "first": first,
            "after": after,
            "filter": filter
        })

    async def get_block(self, block_id: str) -> Dict[str, Any]:
        """Get block information.
        
        Args:
            block_id: Block identifier
            
        Returns:
            Dict[str, Any]: Block information
        """
        query = """
        query GetBlock($id: ID!) {
            block(id: $id) {
                id
                hash
                number
                parentHash
                timestamp
                transactions {
                    id
                    hash
                }
            }
        }
        """
        return await self._execute_query(query, {"id": block_id})

    async def get_blocks(
        self,
        first: int = 10,
        after: Optional[str] = None,
        filter: Optional[Dict[str, Any]] = None
    ) -> Dict[str, Any]:
        """Get a list of blocks.
        
        Args:
            first: Number of items to return
            after: Cursor for pagination
            filter: Filter criteria
            
        Returns:
            Dict[str, Any]: List of blocks
        """
        query = """
        query GetBlocks($first: Int!, $after: String, $filter: BlockFilter) {
            blocks(first: $first, after: $after, filter: $filter) {
                edges {
                    node {
                        id
                        hash
                        number
                        parentHash
                        timestamp
                        transactions {
                            id
                            hash
                        }
                    }
                    cursor
                }
                pageInfo {
                    hasNextPage
                    endCursor
                }
            }
        }
        """
        return await self._execute_query(query, {
            "first": first,
            "after": after,
            "filter": filter
        })

    async def get_wallet(self, wallet_id: str) -> Dict[str, Any]:
        """Get wallet information.
        
        Args:
            wallet_id: Wallet identifier
            
        Returns:
            Dict[str, Any]: Wallet information
        """
        query = """
        query GetWallet($id: ID!) {
            wallet(id: $id) {
                id
                address
                balance
                assets {
                    id
                    amount
                }
                nfts {
                    id
                    tokenId
                }
                createdAt
                updatedAt
            }
        }
        """
        return await self._execute_query(query, {"id": wallet_id})

    async def get_network_status(self) -> Dict[str, Any]:
        """Get network status information.
        
        Returns:
            Dict[str, Any]: Network status information
        """
        query = """
        query GetNetworkStatus {
            networkStatus {
                nodes
                peers
                blockHeight
                difficulty
                hashRate
                lastBlockTime
            }
        }
        """
        return await self._execute_query(query)

    async def get_node_info(self) -> Dict[str, Any]:
        """Get node information.
        
        Returns:
            Dict[str, Any]: Node information
        """
        query = """
        query GetNodeInfo {
            nodeInfo {
                id
                version
                network
                syncStatus
                peers
                uptime
                memoryUsage
                cpuUsage
            }
        }
        """
        return await self._execute_query(query) 