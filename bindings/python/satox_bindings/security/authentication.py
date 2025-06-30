from typing import Dict, Optional, Any
from ..core.satox_error import SatoxError
import bcrypt
import jwt
import logging
from datetime import datetime, timedelta
import os

class Authentication:
    """Handles user authentication."""
    
    def __init__(self, secret_key: Optional[str] = None):
        """Initialize the authentication handler.
        
        Args:
            secret_key: Secret key for JWT tokens (generated if not provided)
        """
        self.secret_key = secret_key or os.urandom(32).hex()
        self.users: Dict[str, Dict[str, Any]] = {}  # username -> user_info
        self.sessions: Dict[str, Dict[str, Any]] = {}  # session_id -> session_info
        self.logger = logging.getLogger(__name__)
        
    def register_user(self, username: str, password: str, email: str) -> None:
        """Register a new user.
        
        Args:
            username: Username
            password: Password
            email: Email address
            
        Raises:
            SatoxError: If username already exists
        """
        if username in self.users:
            raise SatoxError(f"Username {username} already exists")
            
        # Hash password
        salt = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(password.encode(), salt)
        
        # Store user
        self.users[username] = {
            'username': username,
            'password': hashed_password,
            'email': email,
            'created_at': datetime.now(),
            'last_login': None,
            'is_active': True
        }
        
    def authenticate_user(self, username: str, password: str) -> str:
        """Authenticate a user.
        
        Args:
            username: Username
            password: Password
            
        Returns:
            JWT token
            
        Raises:
            SatoxError: If authentication fails
        """
        if username not in self.users:
            raise SatoxError("Invalid username or password")
            
        user = self.users[username]
        
        if not user['is_active']:
            raise SatoxError("Account is disabled")
            
        # Verify password
        if not bcrypt.checkpw(password.encode(), user['password']):
            raise SatoxError("Invalid username or password")
            
        # Update last login
        user['last_login'] = datetime.now()
        
        # Generate token
        token = self._generate_token(username)
        
        # Store session
        self.sessions[token] = {
            'username': username,
            'created_at': datetime.now(),
            'expires_at': datetime.now() + timedelta(hours=24)
        }
        
        return token
        
    def verify_token(self, token: str) -> Dict[str, Any]:
        """Verify a JWT token.
        
        Args:
            token: JWT token
            
        Returns:
            Token payload
            
        Raises:
            SatoxError: If token is invalid
        """
        try:
            # Verify token
            payload = jwt.decode(token, self.secret_key, algorithms=['HS256'])
            
            # Check session
            if token not in self.sessions:
                raise SatoxError("Session not found")
                
            session = self.sessions[token]
            
            # Check expiration
            if datetime.now() > session['expires_at']:
                del self.sessions[token]
                raise SatoxError("Session expired")
                
            return payload
            
        except jwt.InvalidTokenError:
            raise SatoxError("Invalid token")
            
    def logout(self, token: str) -> None:
        """Logout a user.
        
        Args:
            token: JWT token
            
        Raises:
            SatoxError: If token is invalid
        """
        if token not in self.sessions:
            raise SatoxError("Session not found")
            
        del self.sessions[token]
        
    def change_password(self, username: str, old_password: str, new_password: str) -> None:
        """Change user password.
        
        Args:
            username: Username
            old_password: Current password
            new_password: New password
            
        Raises:
            SatoxError: If password change fails
        """
        if username not in self.users:
            raise SatoxError("User not found")
            
        user = self.users[username]
        
        # Verify old password
        if not bcrypt.checkpw(old_password.encode(), user['password']):
            raise SatoxError("Invalid password")
            
        # Hash new password
        salt = bcrypt.gensalt()
        hashed_password = bcrypt.hashpw(new_password.encode(), salt)
        
        # Update password
        user['password'] = hashed_password
        
    def disable_account(self, username: str) -> None:
        """Disable a user account.
        
        Args:
            username: Username
            
        Raises:
            SatoxError: If user not found
        """
        if username not in self.users:
            raise SatoxError("User not found")
            
        self.users[username]['is_active'] = False
        
        # Remove all sessions
        self.sessions = {
            token: session
            for token, session in self.sessions.items()
            if session['username'] != username
        }
        
    def enable_account(self, username: str) -> None:
        """Enable a user account.
        
        Args:
            username: Username
            
        Raises:
            SatoxError: If user not found
        """
        if username not in self.users:
            raise SatoxError("User not found")
            
        self.users[username]['is_active'] = True
        
    def get_user_info(self, username: str) -> Dict[str, Any]:
        """Get user information.
        
        Args:
            username: Username
            
        Returns:
            User information
            
        Raises:
            SatoxError: If user not found
        """
        if username not in self.users:
            raise SatoxError("User not found")
            
        user = self.users[username].copy()
        user.pop('password')  # Don't return password
        return user
        
    def _generate_token(self, username: str) -> str:
        """Generate a JWT token.
        
        Args:
            username: Username
            
        Returns:
            JWT token
        """
        payload = {
            'username': username,
            'exp': datetime.utcnow() + timedelta(hours=24)
        }
        
        return jwt.encode(payload, self.secret_key, algorithm='HS256')
        
    def cleanup_expired_sessions(self) -> None:
        """Remove expired sessions."""
        now = datetime.now()
        
        self.sessions = {
            token: session
            for token, session in self.sessions.items()
            if session['expires_at'] > now
        } 