# Docker Setup

This document provides instructions for setting up Satox SDK using Docker.

## Prerequisites
- Docker Engine 20.10+
- Docker Compose 2.0+
- At least 4GB RAM available
- 10GB free disk space

## Quick Start

### 1. Clone the Repository
```bash
git clone https://github.com/satox/satox-sdk.git
cd satox-sdk
```

### 2. Build the Docker Image
```bash
docker build -t satox-sdk .
```

### 3. Run with Docker Compose
```bash
docker-compose up -d
```

## Configuration

### Environment Variables
Create a `.env` file in the project root:
```bash
# Database Configuration
POSTGRES_DB=satox_db
POSTGRES_USER=satox_user
POSTGRES_PASSWORD=your_secure_password

# Redis Configuration
REDIS_PASSWORD=your_redis_password

# Quantum Security
QUANTUM_ENABLED=true
QUANTUM_KMS_URL=http://quantum-kms:8086

# Network Configuration
NETWORK_NAME=satoxcoin.docker-network
```

### Docker Compose Configuration
The `docker-compose.yml` file includes:
- Satox SDK service
- PostgreSQL database
- Redis cache
- Quantum security services
- Monitoring stack (Prometheus, Grafana)

## Services

### Core Services
- **satox-sdk**: Main SDK service
- **postgres**: PostgreSQL database
- **redis**: Redis cache
- **mongodb**: MongoDB document store

### Quantum Security Services
- **quantum-kms**: Key management service
- **quantum-encryption**: Encryption service
- **quantum-auth**: Authentication service
- **quantum-ca**: Certificate authority

### Monitoring Services
- **prometheus**: Metrics collection
- **grafana**: Monitoring dashboard
- **loki**: Log aggregation
- **alertmanager**: Alert management

## Usage

### Start All Services
```bash
docker-compose up -d
```

### View Logs
```bash
# All services
docker-compose logs -f

# Specific service
docker-compose logs -f satox-sdk
```

### Stop Services
```bash
docker-compose down
```

### Restart Services
```bash
docker-compose restart
```

## Development

### Build for Development
```bash
docker-compose -f docker-compose.dev.yml up -d
```

### Run Tests
```bash
docker-compose exec satox-sdk npm test
```

### Access Services
- **SDK API**: http://localhost:3000
- **Grafana**: http://localhost:3000
- **Prometheus**: http://localhost:9090
- **PostgreSQL**: localhost:5432
- **Redis**: localhost:6379

## Troubleshooting

### Common Issues
1. **Port conflicts**: Change ports in docker-compose.yml
2. **Memory issues**: Increase Docker memory limit
3. **Permission errors**: Check file permissions
4. **Network issues**: Verify Docker network configuration

### Debug Commands
```bash
# Check service status
docker-compose ps

# Check resource usage
docker stats

# Access service shell
docker-compose exec satox-sdk bash

# View service configuration
docker-compose config
```

## Production Deployment

### Security Considerations
- Use secrets management
- Enable TLS/SSL
- Configure firewalls
- Regular security updates

### Scaling
- Use Docker Swarm or Kubernetes
- Configure load balancing
- Implement health checks
- Monitor resource usage

## Resources
- [Docker Architecture](architecture.md)
- [Docker Compose](compose.md)
- [Production Deployment](production.md)
- [Monitoring](monitoring.md)
- [Troubleshooting](../troubleshooting/common-issues.md) 