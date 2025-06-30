from typing import Dict, Any, List, Set, Optional
from ..core.error_handling import SatoxError

class AssetPermissions:
    def __init__(self):
        """Initialize the AssetPermissions."""
        self._permissions: Dict[str, Dict[str, Set[str]]] = {}
        self._default_permissions = {
            'owner': {'read', 'write', 'transfer', 'delete'},
            'user': {'read'},
            'admin': {'read', 'write', 'transfer'}
        }

    def set_permissions(self, asset_id: str, permissions: Dict[str, Set[str]]) -> bool:
        """
        Set permissions for an asset.
        
        Args:
            asset_id: ID of the asset
            permissions: Dictionary mapping roles to sets of permissions
            
        Returns:
            bool: True if permissions were set successfully
            
        Raises:
            SatoxError: If setting permissions fails
        """
        try:
            self._validate_permissions(permissions)
            self._permissions[asset_id] = permissions
            return True
        except Exception as e:
            raise SatoxError(f"Failed to set permissions: {str(e)}")

    def get_permissions(self, asset_id: str) -> Dict[str, Set[str]]:
        """
        Get permissions for an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            Dict[str, Set[str]]: Dictionary of permissions
            
        Raises:
            SatoxError: If getting permissions fails
        """
        try:
            return self._permissions.get(asset_id, self._default_permissions.copy())
        except Exception as e:
            raise SatoxError(f"Failed to get permissions: {str(e)}")

    def add_permission(self, asset_id: str, role: str, permission: str) -> bool:
        """
        Add a permission for a role on an asset.
        
        Args:
            asset_id: ID of the asset
            role: Role to add permission for
            permission: Permission to add
            
        Returns:
            bool: True if permission was added successfully
            
        Raises:
            SatoxError: If adding permission fails
        """
        try:
            if asset_id not in self._permissions:
                self._permissions[asset_id] = self._default_permissions.copy()
            
            if role not in self._permissions[asset_id]:
                self._permissions[asset_id][role] = set()
            
            self._permissions[asset_id][role].add(permission)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to add permission: {str(e)}")

    def remove_permission(self, asset_id: str, role: str, permission: str) -> bool:
        """
        Remove a permission for a role on an asset.
        
        Args:
            asset_id: ID of the asset
            role: Role to remove permission from
            permission: Permission to remove
            
        Returns:
            bool: True if permission was removed successfully
            
        Raises:
            SatoxError: If removing permission fails
        """
        try:
            if asset_id in self._permissions and role in self._permissions[asset_id]:
                self._permissions[asset_id][role].discard(permission)
            return True
        except Exception as e:
            raise SatoxError(f"Failed to remove permission: {str(e)}")

    def check_permission(self, asset_id: str, role: str, permission: str) -> bool:
        """
        Check if a role has a specific permission on an asset.
        
        Args:
            asset_id: ID of the asset
            role: Role to check
            permission: Permission to check
            
        Returns:
            bool: True if role has the permission
            
        Raises:
            SatoxError: If checking permission fails
        """
        try:
            permissions = self.get_permissions(asset_id)
            return role in permissions and permission in permissions[role]
        except Exception as e:
            raise SatoxError(f"Failed to check permission: {str(e)}")

    def get_roles(self, asset_id: str) -> Set[str]:
        """
        Get all roles for an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            Set[str]: Set of roles
            
        Raises:
            SatoxError: If getting roles fails
        """
        try:
            return set(self.get_permissions(asset_id).keys())
        except Exception as e:
            raise SatoxError(f"Failed to get roles: {str(e)}")

    def get_role_permissions(self, asset_id: str, role: str) -> Set[str]:
        """
        Get all permissions for a role on an asset.
        
        Args:
            asset_id: ID of the asset
            role: Role to get permissions for
            
        Returns:
            Set[str]: Set of permissions
            
        Raises:
            SatoxError: If getting role permissions fails
        """
        try:
            permissions = self.get_permissions(asset_id)
            return permissions.get(role, set())
        except Exception as e:
            raise SatoxError(f"Failed to get role permissions: {str(e)}")

    def _validate_permissions(self, permissions: Dict[str, Set[str]]) -> bool:
        """
        Validate a permissions dictionary.
        
        Args:
            permissions: Dictionary of permissions to validate
            
        Returns:
            bool: True if permissions are valid
            
        Raises:
            SatoxError: If validation fails
        """
        try:
            valid_permissions = {'read', 'write', 'transfer', 'delete'}
            
            for role, perms in permissions.items():
                if not isinstance(role, str):
                    raise SatoxError(f"Invalid role type: {type(role)}")
                if not isinstance(perms, set):
                    raise SatoxError(f"Invalid permissions type for role {role}: {type(perms)}")
                if not all(isinstance(p, str) for p in perms):
                    raise SatoxError(f"Invalid permission type in role {role}")
                if not all(p in valid_permissions for p in perms):
                    raise SatoxError(f"Invalid permission in role {role}")
            
            return True
        except Exception as e:
            raise SatoxError(f"Permission validation failed: {str(e)}")

    def clear_permissions(self, asset_id: str) -> bool:
        """
        Clear all permissions for an asset.
        
        Args:
            asset_id: ID of the asset
            
        Returns:
            bool: True if permissions were cleared successfully
            
        Raises:
            SatoxError: If clearing permissions fails
        """
        try:
            if asset_id in self._permissions:
                del self._permissions[asset_id]
            return True
        except Exception as e:
            raise SatoxError(f"Failed to clear permissions: {str(e)}")

    def copy_permissions(self, source_asset_id: str, target_asset_id: str) -> bool:
        """
        Copy permissions from one asset to another.
        
        Args:
            source_asset_id: ID of the source asset
            target_asset_id: ID of the target asset
            
        Returns:
            bool: True if permissions were copied successfully
            
        Raises:
            SatoxError: If copying permissions fails
        """
        try:
            source_permissions = self.get_permissions(source_asset_id)
            self._permissions[target_asset_id] = {
                role: perms.copy()
                for role, perms in source_permissions.items()
            }
            return True
        except Exception as e:
            raise SatoxError(f"Failed to copy permissions: {str(e)}") 