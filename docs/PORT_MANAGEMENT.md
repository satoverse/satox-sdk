# Satox Core SDK Port Management

## Port Range Allocation: 4000-4099 (Core SDK and Shared Services)

### Purpose
This document defines the port allocation strategy for the Satox Core SDK to avoid conflicts with existing Docker services and maintain organization.

### Current Docker Services (Avoid These Ports)
- **3000**: Grafana (satox-grafana)
- **8080**: cAdvisor (satox-cadvisor)
- **9090**: Prometheus (satox-prometheus)
- **3100**: Loki (satox-loki)
- **9093**: Alertmanager (satox-alertmanager)
- **9100**: Node Exporter (satox-node-exporter)
- **7777**: SatoxCoin RPC Port (Reserved for blockchain node communication)
- **60777**: SatoxCoin P2P Port (Reserved for peer-to-peer network communication)

### Core SDK Port Allocation

#### Development & Testing (4000-4009)
- **4000**: Core SDK API Server
- **4001**: Core SDK Health Check
- **4002**: Core SDK Metrics
- **4003**: Core SDK Documentation
- **4004**: Core SDK Testing
- **4005**: Core SDK Examples
- **4006**: Core SDK Admin Panel
- **4007**: Core SDK WebSocket
- **4008**: Core SDK gRPC
- **4009**: Core SDK GraphQL

#### Production Services (5000-5009)
- **5000**: Core SDK Production API
- **5001**: Core SDK Production Health
- **5002**: Core SDK Production Metrics
- **5003**: Core SDK Production Admin
- **5004**: Core SDK Production WebSocket
- **5005**: Core SDK Production gRPC
- **5006**: Core SDK Production GraphQL

#### Reserved for Future Use (4010-4099)
- **4010-4099**: Additional Core SDK Services

### Usage Guidelines

#### Development
```bash
# Start Core SDK development server
cd satox-sdk && npm run dev -- --port 4000

# Start Core SDK testing server
cd satox-sdk && npm run test -- --port 4004

# Start Core SDK documentation server
cd satox-sdk && npm run docs -- --port 4003
```

#### Production
```bash
# Start Core SDK production server
cd satox-sdk && npm start -- --port 5000

# Start Core SDK production health check
cd satox-sdk && npm run health -- --port 5001
```

#### Docker Services
```yaml
# Example docker-compose.yml for Core SDK
version: '3.8'
services:
  satox-sdk:
    ports:
      - "4000:4000"  # API Server
      - "4001:4001"  # Health Check
      - "4002:4002"  # Metrics
    networks:
      - satoxcoin.docker-network
```

### Network Configuration
- **Network Name**: `satoxcoin.docker-network`
- **Network Type**: External bridge network
- **Service Discovery**: Container names as hostnames
- **Inter-service Communication**: HTTP/HTTPS, gRPC, WebSocket

### Database Ports (Internal Only)
- **PostgreSQL**: 5432 (internal), 5433 (host)
- **Redis**: 6379 (internal), 16379 (host)
- **MongoDB**: 27017 (internal), 27018 (host)
- **RocksDB**: Container only (no external port)

### Monitoring and Observability
- **Metrics**: Prometheus on port 4002
- **Health Checks**: HTTP endpoint on port 4001
- **Logging**: Centralized via Loki
- **Tracing**: Distributed tracing enabled
- **Alerting**: Alertmanager integration

### Security Configuration
- **SSL/TLS**: Configurable (default: disabled for development)
- **Authentication**: JWT-based with quantum security
- **Authorization**: Role-based access control
- **Rate Limiting**: Enabled with configurable limits
- **CORS**: Enabled for cross-origin requests

### Performance Configuration
- **Caching**: Redis-based with configurable TTL
- **Connection Pooling**: Configurable pool sizes
- **Load Balancing**: Nginx reverse proxy support
- **Compression**: Gzip compression enabled
- **Timeout Handling**: Configurable timeouts

### Benefits
1. **Conflict Avoidance**: No conflicts with existing Docker monitoring services
2. **Organization**: Clear separation of concerns by port ranges
3. **Scalability**: Room for growth and additional services
4. **Documentation**: Clear reference for developers and DevOps
5. **Consistency**: Standardized approach across all SatoxCoin projects

### Current Status
- ✅ **Core SDK API Server**: Running on port 4000
- ✅ **Core SDK Health Check**: Running on port 4001
- ✅ **Core SDK Metrics**: Running on port 4002
- 🔄 **Core SDK Documentation**: Planned for port 4003
- 🔄 **Core SDK Testing**: Planned for port 4004
- 🔄 **Core SDK Examples**: Planned for port 4005
- 🔄 **Core SDK Admin Panel**: Planned for port 4006
- 🔄 **Core SDK WebSocket**: Planned for port 4007
- 🔄 **Core SDK gRPC**: Planned for port 4008
- 🔄 **Core SDK GraphQL**: Planned for port 4009

### Migration Notes
- **From Previous Ports**: Migrated from 8080-8081 to 4000-4001
- **Network**: Now using `satoxcoin.docker-network` for inter-service communication
- **Environment Variables**: Updated to use new port allocations
- **Health Checks**: Updated to use new health check port
- **Documentation**: Updated to reflect new port assignments

### Notes
- Always check port availability before starting new services
- Update this document when adding new services
- Use environment variables for port configuration in production
- Consider using reverse proxy (nginx) for production deployments
- Follow the port allocation template for consistency across all SDKs 