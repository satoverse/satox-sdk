from typing import Dict, List, Optional, Any, Set
from ..core.satox_error import SatoxError
import logging
from datetime import datetime

class Authorization:
    """Handles access control and authorization."""
    
    def __init__(self):
        """Initialize the authorization handler."""
        self.roles: Dict[str, Dict[str, Any]] = {}  # role_name -> role_info
        self.permissions: Dict[str, Set[str]] = {}  # role_name -> set of permissions
        self.user_roles: Dict[str, Set[str]] = {}  # username -> set of roles
        self.logger = logging.getLogger(__name__)
        
    def create_role(self, role_name: str, description: str) -> None:
        """Create a new role.
        
        Args:
            role_name: Name of the role
            description: Role description
            
        Raises:
            SatoxError: If role already exists
        """
        if role_name in self.roles:
            raise SatoxError(f"Role {role_name} already exists")
            
        self.roles[role_name] = {
            'name': role_name,
            'description': description,
            'created_at': datetime.now()
        }
        
        self.permissions[role_name] = set()
        
    def delete_role(self, role_name: str) -> None:
        """Delete a role.
        
        Args:
            role_name: Name of the role
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        # Remove role from all users
        for username in self.user_roles:
            self.user_roles[username].discard(role_name)
            
        # Remove role
        del self.roles[role_name]
        del self.permissions[role_name]
        
    def assign_role(self, username: str, role_name: str) -> None:
        """Assign a role to a user.
        
        Args:
            username: Username
            role_name: Name of the role
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        if username not in self.user_roles:
            self.user_roles[username] = set()
            
        self.user_roles[username].add(role_name)
        
    def remove_role(self, username: str, role_name: str) -> None:
        """Remove a role from a user.
        
        Args:
            username: Username
            role_name: Name of the role
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        if username in self.user_roles:
            self.user_roles[username].discard(role_name)
            
    def add_permission(self, role_name: str, permission: str) -> None:
        """Add a permission to a role.
        
        Args:
            role_name: Name of the role
            permission: Permission to add
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        self.permissions[role_name].add(permission)
        
    def remove_permission(self, role_name: str, permission: str) -> None:
        """Remove a permission from a role.
        
        Args:
            role_name: Name of the role
            permission: Permission to remove
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        self.permissions[role_name].discard(permission)
        
    def check_permission(self, username: str, permission: str) -> bool:
        """Check if a user has a permission.
        
        Args:
            username: Username
            permission: Permission to check
            
        Returns:
            True if user has permission, False otherwise
        """
        if username not in self.user_roles:
            return False
            
        # Check all user roles
        for role_name in self.user_roles[username]:
            if role_name in self.permissions and permission in self.permissions[role_name]:
                return True
                
        return False
        
    def get_user_roles(self, username: str) -> List[str]:
        """Get roles assigned to a user.
        
        Args:
            username: Username
            
        Returns:
            List of role names
        """
        return list(self.user_roles.get(username, set()))
        
    def get_role_permissions(self, role_name: str) -> List[str]:
        """Get permissions for a role.
        
        Args:
            role_name: Name of the role
            
        Returns:
            List of permissions
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        return list(self.permissions[role_name])
        
    def get_role_info(self, role_name: str) -> Dict[str, Any]:
        """Get information about a role.
        
        Args:
            role_name: Name of the role
            
        Returns:
            Role information
            
        Raises:
            SatoxError: If role does not exist
        """
        if role_name not in self.roles:
            raise SatoxError(f"Role {role_name} does not exist")
            
        return self.roles[role_name].copy()
        
    def get_all_roles(self) -> List[Dict[str, Any]]:
        """Get information about all roles.
        
        Returns:
            List of role information
        """
        return [role.copy() for role in self.roles.values()]
        
    def get_user_permissions(self, username: str) -> List[str]:
        """Get all permissions for a user.
        
        Args:
            username: Username
            
        Returns:
            List of permissions
        """
        if username not in self.user_roles:
            return []
            
        permissions = set()
        
        # Collect permissions from all roles
        for role_name in self.user_roles[username]:
            if role_name in self.permissions:
                permissions.update(self.permissions[role_name])
                
        return list(permissions) 