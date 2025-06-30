# IPFS Module Documentation

The IPFS module provides functionality for interacting with the InterPlanetary File System (IPFS), allowing you to store and retrieve files in a decentralized manner.

## Installation

```bash
pip install bindings
```

## Basic Usage

### Initialization

```python
from satox_bindings.ipfs import IPFSManager

# Create and initialize IPFS manager
ipfs_manager = IPFSManager()
ipfs_manager.initialize()
```

### Adding Files to IPFS

```python
# Add a file to IPFS
content = ipfs_manager.add_file("path/to/file.txt")
print(f"File added to IPFS with CID: {content.cid}")

# Add a file with metadata
metadata = {
    "description": "My file",
    "tags": ["document", "important"]
}
content = ipfs_manager.add_file("path/to/file.txt", metadata=metadata)
```

### Retrieving Files from IPFS

```python
# Get a file from IPFS
result = ipfs_manager.get_file("QmHash...", "output.txt")
if result:
    print("File downloaded successfully")
```

### Pinning Content

```python
# Pin content to keep it available
result = ipfs_manager.pin_content("QmHash...")
if result:
    print("Content pinned successfully")

# Unpin content when no longer needed
result = ipfs_manager.unpin_content("QmHash...")
if result:
    print("Content unpinned successfully")
```

## Advanced Usage

### Working with IPFSContent

```python
# Get content details
content = ipfs_manager.add_file("file.txt")
print(f"CID: {content.cid}")
print(f"Name: {content.name}")
print(f"Size: {content.size}")
print(f"Content Type: {content.content_type}")
print(f"Created At: {content.created_at}")
print(f"Metadata: {content.metadata}")

# Convert to dictionary
content_dict = content.to_dict()

# Create from dictionary
new_content = IPFSContent.from_dict(content_dict)
```

### Error Handling

```python
# Check for errors
content = ipfs_manager.add_file("non_existent.txt")
if content is None:
    print(f"Error: {ipfs_manager.get_last_error()}")

# Clear error
ipfs_manager.clear_last_error()
```

## Best Practices

1. Always check for errors after operations
2. Pin important content to ensure availability
3. Use metadata to organize and describe your content
4. Handle large files appropriately
5. Consider content addressing when designing your application

## API Reference

### IPFSManager

#### Methods

- `initialize() -> bool`: Initialize the IPFS manager
- `add_file(file_path: str, metadata: Optional[Dict] = None) -> Optional[IPFSContent]`: Add a file to IPFS
- `get_file(cid: str, output_path: str) -> bool`: Get a file from IPFS
- `pin_content(cid: str) -> bool`: Pin content in IPFS
- `unpin_content(cid: str) -> bool`: Unpin content from IPFS
- `get_last_error() -> str`: Get the last error message
- `clear_last_error() -> None`: Clear the last error message

### IPFSContent

#### Properties

- `cid: str`: Content identifier
- `name: str`: File name
- `size: int`: File size in bytes
- `content_type: str`: MIME type
- `created_at: datetime`: Creation timestamp
- `metadata: Dict`: Additional metadata

#### Methods

- `to_dict() -> Dict`: Convert to dictionary
- `from_dict(data: Dict) -> IPFSContent`: Create from dictionary 