from typing import Dict, Any, Optional, List, Callable
from datetime import datetime
from ..core.error_handling import SatoxError

class GameLogic:
    def __init__(self):
        """Initialize the GameLogic."""
        self._rules: Dict[str, Dict[str, Any]] = {}
        self._actions: Dict[str, Callable] = {}
        self._events: Dict[str, List[Callable]] = {}
        self._game_states: Dict[str, Dict[str, Any]] = {}

    def add_rule(self, rule_id: str, rule_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Add a game rule.
        
        Args:
            rule_id: Unique identifier for the rule
            rule_data: Rule data including conditions and effects
            
        Returns:
            Dict[str, Any]: Added rule data
            
        Raises:
            SatoxError: If rule already exists
        """
        if rule_id in self._rules:
            raise SatoxError(f"Rule already exists with ID {rule_id}")

        rule = {
            'id': rule_id,
            'data': rule_data,
            'created_at': datetime.now().isoformat(),
            'updated_at': datetime.now().isoformat()
        }
        
        self._rules[rule_id] = rule
        return rule

    def get_rule(self, rule_id: str) -> Optional[Dict[str, Any]]:
        """
        Get a rule by ID.
        
        Args:
            rule_id: ID of the rule
            
        Returns:
            Optional[Dict[str, Any]]: Rule data if found
        """
        return self._rules.get(rule_id)

    def update_rule(self, rule_id: str, rule_data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Update a rule's data.
        
        Args:
            rule_id: ID of the rule
            rule_data: New rule data
            
        Returns:
            Dict[str, Any]: Updated rule data
            
        Raises:
            SatoxError: If rule not found
        """
        if rule_id not in self._rules:
            raise SatoxError(f"Rule not found with ID {rule_id}")

        rule = self._rules[rule_id]
        rule['data'].update(rule_data)
        rule['updated_at'] = datetime.now().isoformat()
        
        return rule

    def delete_rule(self, rule_id: str) -> bool:
        """
        Delete a rule.
        
        Args:
            rule_id: ID of the rule
            
        Returns:
            bool: True if deletion was successful
            
        Raises:
            SatoxError: If rule not found
        """
        if rule_id not in self._rules:
            raise SatoxError(f"Rule not found with ID {rule_id}")

        del self._rules[rule_id]
        return True

    def list_rules(self) -> List[str]:
        """
        Get a list of all rule IDs.
        
        Returns:
            List[str]: List of rule IDs
        """
        return list(self._rules.keys())

    def register_action(self, action_id: str, action_func: Callable):
        """
        Register a game action.
        
        Args:
            action_id: Unique identifier for the action
            action_func: Function implementing the action
            
        Raises:
            SatoxError: If action already exists
        """
        if action_id in self._actions:
            raise SatoxError(f"Action already exists with ID {action_id}")

        self._actions[action_id] = action_func

    def unregister_action(self, action_id: str) -> bool:
        """
        Unregister a game action.
        
        Args:
            action_id: ID of the action
            
        Returns:
            bool: True if unregistration was successful
            
        Raises:
            SatoxError: If action not found
        """
        if action_id not in self._actions:
            raise SatoxError(f"Action not found with ID {action_id}")

        del self._actions[action_id]
        return True

    def execute_action(self, action_id: str, game_id: str, params: Dict[str, Any]) -> Any:
        """
        Execute a game action.
        
        Args:
            action_id: ID of the action
            game_id: ID of the game
            params: Action parameters
            
        Returns:
            Any: Action result
            
        Raises:
            SatoxError: If action not found or execution fails
        """
        if action_id not in self._actions:
            raise SatoxError(f"Action not found with ID {action_id}")

        try:
            return self._actions[action_id](game_id, params)
        except Exception as e:
            raise SatoxError(f"Failed to execute action {action_id}: {str(e)}")

    def register_event_handler(self, event_id: str, handler_func: Callable):
        """
        Register an event handler.
        
        Args:
            event_id: Unique identifier for the event
            handler_func: Function handling the event
        """
        if event_id not in self._events:
            self._events[event_id] = []
        self._events[event_id].append(handler_func)

    def unregister_event_handler(self, event_id: str, handler_func: Callable) -> bool:
        """
        Unregister an event handler.
        
        Args:
            event_id: ID of the event
            handler_func: Function to unregister
            
        Returns:
            bool: True if unregistration was successful
        """
        if event_id not in self._events:
            return False

        try:
            self._events[event_id].remove(handler_func)
            if not self._events[event_id]:
                del self._events[event_id]
            return True
        except ValueError:
            return False

    def trigger_event(self, event_id: str, game_id: str, event_data: Dict[str, Any]):
        """
        Trigger an event.
        
        Args:
            event_id: ID of the event
            game_id: ID of the game
            event_data: Event data
            
        Raises:
            SatoxError: If event handling fails
        """
        if event_id not in self._events:
            return

        for handler in self._events[event_id]:
            try:
                handler(game_id, event_data)
            except Exception as e:
                raise SatoxError(f"Failed to handle event {event_id}: {str(e)}")

    def set_game_state(self, game_id: str, state_data: Dict[str, Any]):
        """
        Set a game's state.
        
        Args:
            game_id: ID of the game
            state_data: State data
        """
        self._game_states[game_id] = state_data

    def get_game_state(self, game_id: str) -> Optional[Dict[str, Any]]:
        """
        Get a game's state.
        
        Args:
            game_id: ID of the game
            
        Returns:
            Optional[Dict[str, Any]]: Game state if found
        """
        return self._game_states.get(game_id)

    def delete_game_state(self, game_id: str) -> bool:
        """
        Delete a game's state.
        
        Args:
            game_id: ID of the game
            
        Returns:
            bool: True if deletion was successful
        """
        if game_id in self._game_states:
            del self._game_states[game_id]
            return True
        return False

    def evaluate_rule(self, rule_id: str, game_id: str, context: Dict[str, Any]) -> bool:
        """
        Evaluate a rule against a game state.
        
        Args:
            rule_id: ID of the rule
            game_id: ID of the game
            context: Evaluation context
            
        Returns:
            bool: True if rule conditions are met
            
        Raises:
            SatoxError: If rule not found or evaluation fails
        """
        if rule_id not in self._rules:
            raise SatoxError(f"Rule not found with ID {rule_id}")

        try:
            rule = self._rules[rule_id]
            conditions = rule['data'].get('conditions', [])
            
            for condition in conditions:
                if not self._evaluate_condition(condition, game_id, context):
                    return False
            return True
        except Exception as e:
            raise SatoxError(f"Failed to evaluate rule {rule_id}: {str(e)}")

    def _evaluate_condition(self, condition: Dict[str, Any], game_id: str, context: Dict[str, Any]) -> bool:
        """
        Evaluate a single condition.
        
        Args:
            condition: Condition to evaluate
            game_id: ID of the game
            context: Evaluation context
            
        Returns:
            bool: True if condition is met
        """
        condition_type = condition.get('type')
        if condition_type == 'state':
            return self._evaluate_state_condition(condition, game_id)
        elif condition_type == 'context':
            return self._evaluate_context_condition(condition, context)
        elif condition_type == 'custom':
            return self._evaluate_custom_condition(condition, game_id, context)
        return False

    def _evaluate_state_condition(self, condition: Dict[str, Any], game_id: str) -> bool:
        """Evaluate a state-based condition."""
        state = self._game_states.get(game_id, {})
        path = condition.get('path', '').split('.')
        value = state
        for key in path:
            if not isinstance(value, dict) or key not in value:
                return False
            value = value[key]
        return value == condition.get('value')

    def _evaluate_context_condition(self, condition: Dict[str, Any], context: Dict[str, Any]) -> bool:
        """Evaluate a context-based condition."""
        path = condition.get('path', '').split('.')
        value = context
        for key in path:
            if not isinstance(value, dict) or key not in value:
                return False
            value = value[key]
        return value == condition.get('value')

    def _evaluate_custom_condition(self, condition: Dict[str, Any], game_id: str, context: Dict[str, Any]) -> bool:
        """Evaluate a custom condition."""
        if 'evaluator' not in condition:
            return False
        try:
            return condition['evaluator'](game_id, context)
        except Exception:
            return False 