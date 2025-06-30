from typing import Dict, Any, List, Optional, Callable
from datetime import datetime
from ..core.error_handling import SatoxError

class AssetSearch:
    def __init__(self):
        """Initialize the AssetSearch."""
        self._search_filters = {
            'type': self._filter_by_type,
            'owner': self._filter_by_owner,
            'name': self._filter_by_name,
            'date_range': self._filter_by_date_range,
            'metadata': self._filter_by_metadata
        }

    def _filter_by_type(self, assets: List[Dict[str, Any]], asset_type: str) -> List[Dict[str, Any]]:
        """Filter assets by type."""
        return [asset for asset in assets if asset.get('type') == asset_type]

    def _filter_by_owner(self, assets: List[Dict[str, Any]], owner: str) -> List[Dict[str, Any]]:
        """Filter assets by owner."""
        return [asset for asset in assets if asset.get('owner') == owner]

    def _filter_by_name(self, assets: List[Dict[str, Any]], name: str) -> List[Dict[str, Any]]:
        """Filter assets by name (case-insensitive partial match)."""
        name = name.lower()
        return [asset for asset in assets if name in asset.get('name', '').lower()]

    def _filter_by_date_range(self, assets: List[Dict[str, Any]], date_range: Dict[str, str]) -> List[Dict[str, Any]]:
        """Filter assets by creation date range."""
        start_date = datetime.fromisoformat(date_range.get('start', '1970-01-01T00:00:00'))
        end_date = datetime.fromisoformat(date_range.get('end', datetime.utcnow().isoformat()))
        
        return [
            asset for asset in assets
            if start_date <= datetime.fromisoformat(asset.get('created_at', '1970-01-01T00:00:00')) <= end_date
        ]

    def _filter_by_metadata(self, assets: List[Dict[str, Any]], metadata: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Filter assets by metadata fields."""
        return [
            asset for asset in assets
            if all(
                asset.get('metadata', {}).get(k) == v
                for k, v in metadata.items()
            )
        ]

    def search(self, assets: List[Dict[str, Any]], filters: Dict[str, Any]) -> List[Dict[str, Any]]:
        """
        Search assets using multiple filters.
        
        Args:
            assets: List of assets to search
            filters: Dictionary of filter criteria
            
        Returns:
            List[Dict[str, Any]]: Filtered list of assets
            
        Raises:
            SatoxError: If search fails
        """
        try:
            result = assets.copy()
            
            for filter_type, filter_value in filters.items():
                if filter_type in self._search_filters:
                    result = self._search_filters[filter_type](result, filter_value)
                else:
                    raise SatoxError(f"Unknown filter type: {filter_type}")
            
            return result
        except Exception as e:
            raise SatoxError(f"Search failed: {str(e)}")

    def add_custom_filter(self, name: str, filter_func: Callable[[List[Dict[str, Any]], Any], List[Dict[str, Any]]]):
        """
        Add a custom filter function.
        
        Args:
            name: Name of the filter
            filter_func: Filter function that takes assets list and filter value
            
        Raises:
            SatoxError: If adding filter fails
        """
        try:
            if name in self._search_filters:
                raise SatoxError(f"Filter {name} already exists")
            
            self._search_filters[name] = filter_func
        except Exception as e:
            raise SatoxError(f"Failed to add custom filter: {str(e)}")

    def remove_filter(self, name: str) -> bool:
        """
        Remove a filter function.
        
        Args:
            name: Name of the filter to remove
            
        Returns:
            bool: True if filter was removed
            
        Raises:
            SatoxError: If removing filter fails
        """
        try:
            if name not in self._search_filters:
                raise SatoxError(f"Filter {name} does not exist")
            
            del self._search_filters[name]
            return True
        except Exception as e:
            raise SatoxError(f"Failed to remove filter: {str(e)}")

    def get_available_filters(self) -> List[str]:
        """
        Get list of available filter names.
        
        Returns:
            List[str]: List of filter names
        """
        return list(self._search_filters.keys())

    def search_by_custom_criteria(self, assets: List[Dict[str, Any]], criteria_func: Callable[[Dict[str, Any]], bool]) -> List[Dict[str, Any]]:
        """
        Search assets using a custom criteria function.
        
        Args:
            assets: List of assets to search
            criteria_func: Function that takes an asset and returns True if it matches
            
        Returns:
            List[Dict[str, Any]]: Filtered list of assets
            
        Raises:
            SatoxError: If search fails
        """
        try:
            return [asset for asset in assets if criteria_func(asset)]
        except Exception as e:
            raise SatoxError(f"Custom search failed: {str(e)}") 