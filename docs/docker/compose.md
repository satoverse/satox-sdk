# Docker Compose Setup for Satox Core SDK

## Overview

Docker Compose provides a simple way to run the complete Satox Core SDK stack with all dependencies in isolated containers. This includes the SDK, databases, and supporting services.

## Quick Start

### Prerequisites

```bash
# Install Docker and Docker Compose
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
sudo usermod -aG docker $USER

# Install Docker Compose
sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose
```

### Basic Setup

```bash
# Clone the repository
git clone https://github.com/satoverse/satox-sdk.git
cd satox-sdk

# Start all services
docker-compose up -d

# Check service status
docker-compose ps

# View logs
docker-compose logs -f satox-sdk
```

## Docker Compose Configuration

### docker-compose.yml

```yaml
version: '3.8'

services:
  # Satox Core SDK
  satox-sdk:
    build:
      context: .
      dockerfile: Dockerfile
    container_name: satox-sdk
    ports:
      - "8080:8080"  # API port
      - "9090:9090"  # Metrics port
    environment:
      - NODE_ENV=production
      - DATABASE_URL=postgresql://satox_user:password@postgres:5432/satox_sdk
      - REDIS_URL=redis://redis:6379
      - MONGODB_URL=mongodb://mongo:27017/satox_sdk
    volumes:
      - ./data:/app/data
      - ./logs:/app/logs
      - ./config:/app/config
    depends_on:
      - postgres
      - redis
      - mongo
    networks:
      - satox-network
    restart: unless-stopped

  # PostgreSQL Database
  postgres:
    image: postgres:15-alpine
    container_name: satox-postgres
    environment:
      - POSTGRES_DB=satox_sdk
      - POSTGRES_USER=satox_user
      - POSTGRES_PASSWORD=secure_password
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ./docker/postgres/init.sql:/docker-entrypoint-initdb.d/init.sql
    ports:
      - "5432:5432"
    networks:
      - satox-network
    restart: unless-stopped

  # Redis Cache
  redis:
    image: redis:7-alpine
    container_name: satox-redis
    command: redis-server --appendonly yes --requirepass redis_password
    volumes:
      - redis_data:/data
    ports:
      - "6379:6379"
    networks:
      - satox-network
    restart: unless-stopped

  # MongoDB
  mongo:
    image: mongo:6
    container_name: satox-mongo
    environment:
      - MONGO_INITDB_ROOT_USERNAME=admin
      - MONGO_INITDB_ROOT_PASSWORD=secure_password
      - MONGO_INITDB_DATABASE=satox_sdk
    volumes:
      - mongo_data:/data/db
      - ./docker/mongo/init.js:/docker-entrypoint-initdb.d/init.js
    ports:
      - "27017:27017"
    networks:
      - satox-network
    restart: unless-stopped

  # IPFS Node
  ipfs:
    image: ipfs/kubo:latest
    container_name: satox-ipfs
    ports:
      - "4001:4001"  # P2P port
      - "5001:5001"  # API port
      - "8080:8080"  # Gateway port
    volumes:
      - ipfs_data:/data/ipfs
    networks:
      - satox-network
    restart: unless-stopped

  # Monitoring Stack
  prometheus:
    image: prom/prometheus:latest
    container_name: satox-prometheus
    ports:
      - "9091:9090"
    volumes:
      - ./docker/prometheus/prometheus.yml:/etc/prometheus/prometheus.yml
      - prometheus_data:/prometheus
    networks:
      - satox-network
    restart: unless-stopped

  grafana:
    image: grafana/grafana:latest
    container_name: satox-grafana
    ports:
      - "3000:3000"
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin_password
    volumes:
      - grafana_data:/var/lib/grafana
      - ./docker/grafana/dashboards:/etc/grafana/provisioning/dashboards
    networks:
      - satox-network
    restart: unless-stopped

volumes:
  postgres_data:
  redis_data:
  mongo_data:
  ipfs_data:
  prometheus_data:
  grafana_data:

networks:
  satox-network:
    driver: bridge
```

## Environment Configuration

### .env File

```bash
# Create .env file
cat > .env << EOF
# Database Configuration
POSTGRES_DB=satox_sdk
POSTGRES_USER=satox_user
POSTGRES_PASSWORD=secure_password
POSTGRES_HOST=postgres
POSTGRES_PORT=5432

# Redis Configuration
REDIS_HOST=redis
REDIS_PORT=6379
REDIS_PASSWORD=redis_password

# MongoDB Configuration
MONGO_HOST=mongo
MONGO_PORT=27017
MONGO_USER=admin
MONGO_PASSWORD=secure_password
MONGO_DATABASE=satox_sdk

# SDK Configuration
SDK_API_PORT=8080
SDK_METRICS_PORT=9090
SDK_LOG_LEVEL=info
SDK_ENVIRONMENT=production

# Security
JWT_SECRET=your_jwt_secret_here
ENCRYPTION_KEY=your_encryption_key_here

# Blockchain Configuration
BLOCKCHAIN_NETWORK=mainnet
BLOCKCHAIN_RPC_URL=https://rpc.satox.com
BLOCKCHAIN_RPC_USER=your_rpc_user
BLOCKCHAIN_RPC_PASSWORD=your_rpc_password
EOF
```

## Service Management

### Starting Services

```bash
# Start all services
docker-compose up -d

# Start specific services
docker-compose up -d postgres redis mongo

# Start with custom configuration
docker-compose -f docker-compose.yml -f docker-compose.prod.yml up -d
```

### Stopping Services

```bash
# Stop all services
docker-compose down

# Stop and remove volumes
docker-compose down -v

# Stop specific services
docker-compose stop postgres redis
```

### Monitoring Services

```bash
# View service status
docker-compose ps

# View logs for all services
docker-compose logs

# View logs for specific service
docker-compose logs -f satox-sdk

# View resource usage
docker stats
```

## Development Setup

### Development Configuration

```yaml
# docker-compose.dev.yml
version: '3.8'

services:
  satox-sdk:
    build:
      context: .
      dockerfile: Dockerfile.dev
    environment:
      - NODE_ENV=development
      - DEBUG=true
    volumes:
      - .:/app
      - /app/node_modules
    ports:
      - "8080:8080"
      - "9229:9229"  # Debug port

  # Development tools
  adminer:
    image: adminer:latest
    container_name: satox-adminer
    ports:
      - "8081:8080"
    networks:
      - satox-network

  redis-commander:
    image: rediscommander/redis-commander:latest
    container_name: satox-redis-commander
    environment:
      - REDIS_HOSTS=local:redis:6379
    ports:
      - "8082:8081"
    networks:
      - satox-network
```

### Hot Reload Development

```bash
# Start development environment
docker-compose -f docker-compose.yml -f docker-compose.dev.yml up -d

# Watch for changes
docker-compose -f docker-compose.yml -f docker-compose.dev.yml logs -f satox-sdk
```

## Production Setup

### Production Configuration

```yaml
# docker-compose.prod.yml
version: '3.8'

services:
  satox-sdk:
    image: satoverse/satox-sdk:latest
    deploy:
      replicas: 3
      resources:
        limits:
          cpus: '2'
          memory: 4G
        reservations:
          cpus: '1'
          memory: 2G
    environment:
      - NODE_ENV=production
      - LOG_LEVEL=warn
    volumes:
      - satox_logs:/app/logs
    networks:
      - satox-network

  # Load balancer
  nginx:
    image: nginx:alpine
    container_name: satox-nginx
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./docker/nginx/nginx.conf:/etc/nginx/nginx.conf
      - ./docker/nginx/ssl:/etc/nginx/ssl
    depends_on:
      - satox-sdk
    networks:
      - satox-network

volumes:
  satox_logs:
```

### SSL Configuration

```nginx
# docker/nginx/nginx.conf
events {
    worker_connections 1024;
}

http {
    upstream satox_backend {
        server satox-sdk:8080;
    }

    server {
        listen 80;
        server_name your-domain.com;
        return 301 https://$server_name$request_uri;
    }

    server {
        listen 443 ssl http2;
        server_name your-domain.com;

        ssl_certificate /etc/nginx/ssl/cert.pem;
        ssl_certificate_key /etc/nginx/ssl/key.pem;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers ECDHE-RSA-AES256-GCM-SHA512:DHE-RSA-AES256-GCM-SHA512;

        location / {
            proxy_pass http://satox_backend;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }
    }
}
```

## Backup and Recovery

### Automated Backups

```bash
#!/bin/bash
# backup_docker.sh

DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR="/var/backups/satox-docker"

mkdir -p $BACKUP_DIR

# Backup PostgreSQL
docker exec satox-postgres pg_dump -U satox_user satox_sdk > $BACKUP_DIR/postgres_$DATE.sql

# Backup Redis
docker exec satox-redis redis-cli --rdb /data/dump.rdb
docker cp satox-redis:/data/dump.rdb $BACKUP_DIR/redis_$DATE.rdb

# Backup MongoDB
docker exec satox-mongo mongodump --out /data/backup
docker cp satox-mongo:/data/backup $BACKUP_DIR/mongo_$DATE

# Compress backups
tar -czf $BACKUP_DIR/satox_backup_$DATE.tar.gz -C $BACKUP_DIR .
```

### Recovery Procedures

```bash
# Restore PostgreSQL
docker exec -i satox-postgres psql -U satox_user satox_sdk < backup_file.sql

# Restore Redis
docker cp backup_file.rdb satox-redis:/data/dump.rdb
docker exec satox-redis redis-cli --rdb /data/dump.rdb

# Restore MongoDB
docker cp backup_dir satox-mongo:/data/backup
docker exec satox-mongo mongorestore /data/backup
```

## Monitoring and Logging

### Log Aggregation

```yaml
# Add to docker-compose.yml
services:
  elasticsearch:
    image: docker.elastic.co/elasticsearch/elasticsearch:8.8.0
    container_name: satox-elasticsearch
    environment:
      - discovery.type=single-node
      - xpack.security.enabled=false
    volumes:
      - elasticsearch_data:/usr/share/elasticsearch/data
    networks:
      - satox-network

  kibana:
    image: docker.elastic.co/kibana/kibana:8.8.0
    container_name: satox-kibana
    ports:
      - "5601:5601"
    networks:
      - satox-network

  filebeat:
    image: docker.elastic.co/beats/filebeat:8.8.0
    container_name: satox-filebeat
    volumes:
      - ./docker/filebeat/filebeat.yml:/usr/share/filebeat/filebeat.yml
      - /var/lib/docker/containers:/var/lib/docker/containers:ro
    networks:
      - satox-network

volumes:
  elasticsearch_data:
```

### Health Checks

```yaml
services:
  satox-sdk:
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s

  postgres:
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U satox_user -d satox_sdk"]
      interval: 10s
      timeout: 5s
      retries: 5

  redis:
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 10s
      timeout: 5s
      retries: 5
```

## Troubleshooting

### Common Issues

**Issue**: Services not starting
**Solution**: Check logs with `docker-compose logs`, verify port conflicts

**Issue**: Database connection errors
**Solution**: Ensure database services are running, check credentials

**Issue**: Memory/CPU limits
**Solution**: Adjust resource limits in docker-compose.yml

**Issue**: Volume permissions
**Solution**: Check file permissions, use appropriate user/group

### Debug Commands

```bash
# Inspect container
docker inspect satox-sdk

# Execute commands in container
docker exec -it satox-sdk bash

# View container logs
docker logs satox-sdk

# Check resource usage
docker stats satox-sdk

# Network inspection
docker network inspect satox_satox-network
```

## References

- [Docker Compose Documentation](https://docs.docker.com/compose/)
- [Docker Best Practices](https://docs.docker.com/develop/dev-best-practices/)
- [Multi-stage Docker Builds](https://docs.docker.com/develop/dev-best-practices/multistage-build/)

---

*Last updated: $(date)* 