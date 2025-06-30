# MongoDB Integration

This document describes how to use MongoDB with Satox SDK.

## Prerequisites
- MongoDB server running (local or remote)
- Connection string (see below)

## Configuration
- Set the following environment variable:
  ```sh
  export SATOX_MONGODB_URI="mongodb://satoxuser:yourpassword@localhost:27017/satoxdb"
  ```
- Or configure in your `config.json`:
  ```json
  {
    "mongodb_uri": "mongodb://satoxuser:yourpassword@localhost:27017/satoxdb"
  }
  ```

## Usage
- The SDK will automatically connect to MongoDB if the URI is set.
- All data will be stored in the specified database.

## Security
- Use strong passwords for database users.
- Restrict network access to trusted hosts.

## Troubleshooting
- Check MongoDB logs for connection errors.
- Ensure the user has the correct permissions.
- See `../troubleshooting/common-issues.md` for more help. 