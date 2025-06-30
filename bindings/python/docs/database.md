# Database Module Documentation

The database module provides a flexible and easy-to-use interface for database operations, supporting both synchronous and asynchronous operations.

## Installation

```bash
pip install bindings
```

## Basic Usage

### Initialization

```python
from satox_bindings.database import DatabaseManager

# Create and initialize database manager
db_manager = DatabaseManager("path/to/database.db")
await db_manager.initialize()
```

### Table Management

```python
# Create a table
columns = {
    "id": "INTEGER",
    "name": "TEXT",
    "age": "INTEGER"
}
result = await db_manager.create_table(
    "users",
    columns,
    primary_key="id"
)

# Create a table with foreign keys
foreign_keys = [
    {
        "column": "user_id",
        "references": "users",
        "on": "id"
    }
]
result = await db_manager.create_table(
    "posts",
    {
        "id": "INTEGER",
        "user_id": "INTEGER",
        "content": "TEXT"
    },
    primary_key="id",
    foreign_keys=foreign_keys
)
```

### CRUD Operations

```python
# Insert data
data = {
    "id": 1,
    "name": "John Doe",
    "age": 30
}
result = await db_manager.insert("users", data)

# Update data
data = {"age": 31}
where = {"id": 1}
result = await db_manager.update("users", data, where)

# Delete data
where = {"id": 1}
result = await db_manager.delete("users", where)

# Query data
results = await db_manager.query(
    "users",
    columns=["id", "name", "age"],
    where={"age": 30},
    order_by="name",
    limit=10
)
```

## Advanced Usage

### Working with Models

```python
from satox_bindings.database import DatabaseModel

class User(DatabaseModel):
    def __init__(self, id: int, name: str, age: int):
        super().__init__("users")
        self.id = id
        self.name = name
        self.age = age
    
    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "age": self.age
        }
    
    @classmethod
    def from_dict(cls, data):
        return cls(
            data["id"],
            data["name"],
            data["age"]
        )

# Create and save model
user = User(1, "John Doe", 30)
await db_manager.insert("users", user.to_dict())

# Query and recreate model
results = await db_manager.query("users", where={"id": 1})
if results:
    user = User.from_dict(results[0])
```

### Error Handling

```python
# Check for errors
result = await db_manager.insert("non_existent", {"id": 1})
if not result:
    print(f"Error: {db_manager.get_last_error()}")

# Clear error
db_manager.clear_last_error()
```

## Best Practices

1. Always use parameterized queries to prevent SQL injection
2. Create indexes for frequently queried columns
3. Use transactions for multiple related operations
4. Handle errors appropriately
5. Close database connections when done

## API Reference

### DatabaseManager

#### Methods

- `initialize() -> bool`: Initialize the database manager
- `create_table(table_name: str, columns: Dict[str, str], primary_key: Optional[str] = None, foreign_keys: Optional[List[Dict[str, str]]] = None) -> bool`: Create a table
- `insert(table_name: str, data: Dict[str, Any]) -> bool`: Insert data
- `update(table_name: str, data: Dict[str, Any], where: Dict[str, Any]) -> bool`: Update data
- `delete(table_name: str, where: Dict[str, Any]) -> bool`: Delete data
- `query(table_name: str, columns: Optional[List[str]] = None, where: Optional[Dict[str, Any]] = None, order_by: Optional[str] = None, limit: Optional[int] = None) -> List[Dict[str, Any]]`: Query data
- `get_last_error() -> str`: Get the last error message
- `clear_last_error() -> None`: Clear the last error message

### DatabaseModel

#### Methods

- `to_dict() -> Dict`: Convert to dictionary
- `from_dict(data: Dict) -> DatabaseModel`: Create from dictionary 