from typing import Dict, Any, Optional, List, Set
from datetime import datetime
from ..core.error_handling import SatoxError

class PlayerManager:
    def __init__(self):
        """Initialize the PlayerManager."""
        self._players: Dict[str, Dict[str, Any]] = {}
        self._game_players: Dict[str, Set[str]] = {}
        self._player_games: Dict[str, Set[str]] = {}

    def create_player(self, player_id: str, player_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Create a new player.
        
        Args:
            player_id: Unique identifier for the player
            player_data: Player data
            
        Returns:
            Dict[str, Any]: Created player data
            
        Raises:
            SatoxError: If player already exists
        """
        if player_id in self._players:
            raise SatoxError(f"Player already exists with ID {player_id}")

        player = {
            'id': player_id,
            'data': player_data,
            'created_at': datetime.now().isoformat(),
            'updated_at': datetime.now().isoformat(),
            'games': set()
        }
        
        self._players[player_id] = player
        self._player_games[player_id] = set()
        
        return player

    def get_player(self, player_id: str) -> Optional[Dict[str, Any]]:
        """
        Get a player by ID.
        
        Args:
            player_id: ID of the player
            
        Returns:
            Optional[Dict[str, Any]]: Player data if found
        """
        return self._players.get(player_id)

    def update_player(self, player_id: str, player_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Update a player's data.
        
        Args:
            player_id: ID of the player
            player_data: New player data
            
        Returns:
            Dict[str, Any]: Updated player data
            
        Raises:
            SatoxError: If player not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")

        player = self._players[player_id]
        player['data'].update(player_data)
        player['updated_at'] = datetime.now().isoformat()
        
        return player

    def delete_player(self, player_id: str) -> bool:
        """
        Delete a player.
        
        Args:
            player_id: ID of the player
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If player not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")

        # Remove player from all games
        for game_id in self._player_games[player_id]:
            self._game_players[game_id].remove(player_id)
            if not self._game_players[game_id]:
                del self._game_players[game_id]

        del self._players[player_id]
        del self._player_games[player_id]
        return True

    def list_players(self) -> List[str]:
        """
        Get a list of all player IDs.
        
        Returns:
            List[str]: List of player IDs
        """
        return list(self._players.keys())

    def add_player_to_game(self, player_id: str, game_id: str) -> bool:
        """
        Add a player to a game.
        
        Args:
            player_id: ID of the player
            game_id: ID of the game
            
        Returns:
            bool: True if player was added successfully
            
        Raises:
            SatoxError: If player not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")

        if game_id not in self._game_players:
            self._game_players[game_id] = set()

        self._game_players[game_id].add(player_id)
        self._player_games[player_id].add(game_id)
        return True

    def remove_player_from_game(self, player_id: str, game_id: str) -> bool:
        """
        Remove a player from a game.
        
        Args:
            player_id: ID of the player
            game_id: ID of the game
            
        Returns:
            bool: True if player was removed successfully
            
        Raises:
            SatoxError: If player or game not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")
        if game_id not in self._game_players:
            raise SatoxError(f"Game not found with ID {game_id}")

        self._game_players[game_id].remove(player_id)
        self._player_games[player_id].remove(game_id)

        if not self._game_players[game_id]:
            del self._game_players[game_id]
        return True

    def get_game_players(self, game_id: str) -> List[str]:
        """
        Get a list of players in a game.
        
        Args:
            game_id: ID of the game
            
        Returns:
            List[str]: List of player IDs
            
        Raises:
            SatoxError: If game not found
        """
        if game_id not in self._game_players:
            raise SatoxError(f"Game not found with ID {game_id}")

        return list(self._game_players[game_id])

    def get_player_games(self, player_id: str) -> List[str]:
        """
        Get a list of games a player is in.
        
        Args:
            player_id: ID of the player
            
        Returns:
            List[str]: List of game IDs
            
        Raises:
            SatoxError: If player not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")

        return list(self._player_games[player_id])

    def get_player_stats(self, player_id: str) -> Dict[str, Any]:
        """
        Get player statistics.
        
        Args:
            player_id: ID of the player
            
        Returns:
            Dict[str, Any]: Player statistics
            
        Raises:
            SatoxError: If player not found
        """
        if player_id not in self._players:
            raise SatoxError(f"Player not found with ID {player_id}")

        player = self._players[player_id]
        return {
            'id': player_id,
            'games_count': len(self._player_games[player_id]),
            'created_at': player['created_at'],
            'updated_at': player['updated_at'],
            'data': player['data']
        }

    def search_players(self, criteria: Dict[str, Any]) -> List[str]:
        """
        Search for players matching criteria.
        
        Args:
            criteria: Search criteria
            
        Returns:
            List[str]: List of matching player IDs
        """
        matching_players = []
        for player_id, player in self._players.items():
            matches = True
            for key, value in criteria.items():
                if key not in player['data'] or player['data'][key] != value:
                    matches = False
                    break
            if matches:
                matching_players.append(player_id)
        return matching_players

    def get_active_players(self) -> List[str]:
        """
        Get a list of players who are currently in at least one game.
        
        Returns:
            List[str]: List of active player IDs
        """
        return [player_id for player_id, games in self._player_games.items() if games]

    def get_inactive_players(self) -> List[str]:
        """
        Get a list of players who are not in any games.
        
        Returns:
            List[str]: List of inactive player IDs
        """
        return [player_id for player_id, games in self._player_games.items() if not games] 