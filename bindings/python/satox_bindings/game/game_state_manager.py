from typing import Dict, Any, Optional, List
import json
import time
from datetime import datetime
from ..core.error_handling import SatoxError

class GameStateManager:
    def __init__(self):
        """Initialize the GameStateManager."""
        self._game_states: Dict[str, Dict[str, Any]] = {}
        self._state_history: Dict[str, List[Dict[str, Any]]] = {}
        self._max_history_size = 100

    def create_game_state(self, game_id: str, initial_state: Dict[str, Any]) -> Dict[str, Any]:
        """
        Create a new game state.
        
        Args:
            game_id: Unique identifier for the game
            initial_state: Initial state data
            
        Returns:
            Dict[str, Any]: Created game state
            
        Raises:
            SatoxError: If game state already exists
        """
        if game_id in self._game_states:
            raise SatoxError(f"Game state already exists for ID {game_id}")

        state = {
            'id': game_id,
            'state': initial_state,
            'created_at': datetime.now().isoformat(),
            'updated_at': datetime.now().isoformat(),
            'version': 1
        }
        
        self._game_states[game_id] = state
        self._state_history[game_id] = [state.copy()]
        
        return state

    def get_game_state(self, game_id: str) -> Optional[Dict[str, Any]]:
        """
        Get a game state by ID.
        
        Args:
            game_id: ID of the game
            
        Returns:
            Optional[Dict[str, Any]]: Game state if found
        """
        return self._game_states.get(game_id)

    def update_game_state(self, game_id: str, new_state: Dict[str, Any]) -> Dict[str, Any]:
        """
        Update a game state.
        
        Args:
            game_id: ID of the game
            new_state: New state data
            
        Returns:
            Dict[str, Any]: Updated game state
            
        Raises:
            SatoxError: If game state not found
        """
        if game_id not in self._game_states:
            raise SatoxError(f"Game state not found for ID {game_id}")

        current_state = self._game_states[game_id]
        current_state['state'].update(new_state)
        current_state['updated_at'] = datetime.now().isoformat()
        current_state['version'] += 1

        # Add to history
        self._state_history[game_id].append(current_state.copy())
        
        # Trim history if needed
        if len(self._state_history[game_id]) > self._max_history_size:
            self._state_history[game_id] = self._state_history[game_id][-self._max_history_size:]

        return current_state

    def delete_game_state(self, game_id: str) -> bool:
        """
        Delete a game state.
        
        Args:
            game_id: ID of the game
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If game state not found
        """
        if game_id not in self._game_states:
            raise SatoxError(f"Game state not found for ID {game_id}")

        del self._game_states[game_id]
        del self._state_history[game_id]
        return True

    def list_game_states(self) -> List[str]:
        """
        Get a list of all game IDs.
        
        Returns:
            List[str]: List of game IDs
        """
        return list(self._game_states.keys())

    def get_state_history(self, game_id: str, limit: Optional[int] = None) -> List[Dict[str, Any]]:
        """
        Get the state history for a game.
        
        Args:
            game_id: ID of the game
            limit: Optional limit on number of history entries
            
        Returns:
            List[Dict[str, Any]]: List of state history entries
            
        Raises:
            SatoxError: If game state not found
        """
        if game_id not in self._state_history:
            raise SatoxError(f"Game state not found for ID {game_id}")

        history = self._state_history[game_id]
        if limit is not None:
            history = history[-limit:]
        return history

    def revert_to_state(self, game_id: str, version: int) -> Dict[str, Any]:
        """
        Revert a game state to a previous version.
        
        Args:
            game_id: ID of the game
            version: Version number to revert to
            
        Returns:
            Dict[str, Any]: Reverted game state
            
        Raises:
            SatoxError: If game state not found or version invalid
        """
        if game_id not in self._state_history:
            raise SatoxError(f"Game state not found for ID {game_id}")

        history = self._state_history[game_id]
        for state in reversed(history):
            if state['version'] == version:
                self._game_states[game_id] = state.copy()
                return state

        raise SatoxError(f"Version {version} not found in history for game {game_id}")

    def get_state_diff(self, game_id: str, version1: int, version2: int) -> Dict[str, Any]:
        """
        Get the difference between two state versions.
        
        Args:
            game_id: ID of the game
            version1: First version number
            version2: Second version number
            
        Returns:
            Dict[str, Any]: State differences
            
        Raises:
            SatoxError: If game state not found or versions invalid
        """
        if game_id not in self._state_history:
            raise SatoxError(f"Game state not found for ID {game_id}")

        history = self._state_history[game_id]
        state1 = None
        state2 = None

        for state in history:
            if state['version'] == version1:
                state1 = state
            elif state['version'] == version2:
                state2 = state

        if not state1 or not state2:
            raise SatoxError(f"One or both versions not found in history for game {game_id}")

        # Calculate differences
        diff = {}
        for key in set(state1['state'].keys()) | set(state2['state'].keys()):
            if key not in state1['state']:
                diff[key] = {'added': state2['state'][key]}
            elif key not in state2['state']:
                diff[key] = {'removed': state1['state'][key]}
            elif state1['state'][key] != state2['state'][key]:
                diff[key] = {
                    'old': state1['state'][key],
                    'new': state2['state'][key]
                }

        return {
            'game_id': game_id,
            'version1': version1,
            'version2': version2,
            'differences': diff
        }

    def set_max_history_size(self, size: int):
        """
        Set the maximum number of history entries to keep.
        
        Args:
            size: Maximum history size
        """
        if size < 1:
            raise SatoxError("History size must be at least 1")
        self._max_history_size = size

    def get_max_history_size(self) -> int:
        """
        Get the maximum number of history entries kept.
        
        Returns:
            int: Maximum history size
        """
        return self._max_history_size

    def clear_history(self, game_id: str):
        """
        Clear the state history for a game.
        
        Args:
            game_id: ID of the game
            
        Raises:
            SatoxError: If game state not found
        """
        if game_id not in self._state_history:
            raise SatoxError(f"Game state not found for ID {game_id}")

        current_state = self._game_states[game_id]
        self._state_history[game_id] = [current_state.copy()] 