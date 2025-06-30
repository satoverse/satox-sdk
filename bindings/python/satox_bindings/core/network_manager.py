"""Network Manager for Satox SDK.
Handles network communication and peer management.
"""

from typing import Optional, List, Dict
import json
import asyncio
import aiohttp

class NetworkManager:
    """Manages network communication and peer connections."""
    
    def __init__(self):
        """Initialize the network manager."""
        self._initialized = False
        self._last_error = ""
        self._peers = {}
        self._session = None

    async def initialize(self) -> bool:
        """Initialize the network manager."""
        try:
            self._session = aiohttp.ClientSession()
            self._initialized = True
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    async def shutdown(self) -> bool:
        """Shutdown the network manager."""
        try:
            if self._session:
                await self._session.close()
            self._initialized = False
            self._peers.clear()
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    async def add_peer(self, peer_id: str, address: str) -> bool:
        """Add a new peer to the network."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return False
        try:
            if peer_id in self._peers:
                self._last_error = "Peer already exists"
                return False
            
            self._peers[peer_id] = {
                "address": address,
                "connected": False,
                "last_seen": None
            }
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    async def remove_peer(self, peer_id: str) -> bool:
        """Remove a peer from the network."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return False
        try:
            if peer_id not in self._peers:
                self._last_error = "Peer not found"
                return False
            
            del self._peers[peer_id]
            return True
        except Exception as e:
            self._last_error = str(e)
            return False

    async def connect_to_peer(self, peer_id: str) -> bool:
        """Connect to a peer."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return False
        try:
            if peer_id not in self._peers:
                self._last_error = "Peer not found"
                return False
            
            peer = self._peers[peer_id]
            try:
                async with self._session.get(f"{peer['address']}/ping") as response:
                    if response.status == 200:
                        peer["connected"] = True
                        return True
                    else:
                        self._last_error = f"Failed to connect to peer: {response.status}"
                        return False
            except Exception as e:
                self._last_error = f"Failed to connect to peer: {str(e)}"
                return False
        except Exception as e:
            self._last_error = str(e)
            return False

    async def send_message(self, peer_id: str, message: Dict) -> bool:
        """Send a message to a peer."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return False
        try:
            if peer_id not in self._peers:
                self._last_error = "Peer not found"
                return False
            
            peer = self._peers[peer_id]
            if not peer["connected"]:
                self._last_error = "Peer not connected"
                return False
            
            try:
                async with self._session.post(
                    f"{peer['address']}/message",
                    json=message
                ) as response:
                    if response.status == 200:
                        return True
                    else:
                        self._last_error = f"Failed to send message: {response.status}"
                        return False
            except Exception as e:
                self._last_error = f"Failed to send message: {str(e)}"
                return False
        except Exception as e:
            self._last_error = str(e)
            return False

    def get_peers(self) -> List[Dict]:
        """Get all peers."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return []
        return [
            {"id": peer_id, **peer_data}
            for peer_id, peer_data in self._peers.items()
        ]

    def get_peer(self, peer_id: str) -> Optional[Dict]:
        """Get a peer by ID."""
        if not self._initialized:
            self._last_error = "Network manager not initialized"
            return None
        try:
            if peer_id not in self._peers:
                self._last_error = "Peer not found"
                return None
            
            return {"id": peer_id, **self._peers[peer_id]}
        except Exception as e:
            self._last_error = str(e)
            return None

    def get_last_error(self) -> str:
        """Get the last error message."""
        return self._last_error

    def clear_last_error(self) -> None:
        """Clear the last error message."""
        self._last_error = ""
