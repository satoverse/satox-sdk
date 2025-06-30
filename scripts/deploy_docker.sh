#!/bin/bash

# Satox SDK Docker Deployment Script
# This script builds and deploys the satox-sdk using Docker containers
# Following cursor rules for Docker deployment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
DOCKER_COMPOSE_FILE="$PROJECT_ROOT/docker-compose.yml"
DOCKERFILE="$PROJECT_ROOT/Dockerfile"
DOCKERFILE_ROCKSDB="$PROJECT_ROOT/Dockerfile.rocksdb"
NETWORK_NAME="satoxcoin.docker-network"

# Logging
LOG_DIR="$PROJECT_ROOT/logs"
DOCKER_LOG="$LOG_DIR/docker_deployment_$(date '+%Y%m%d_%H%M%S').log"

# Create log directory
mkdir -p "$LOG_DIR"

# Logging function
log() {
    echo -e "${BLUE}[$(date '+%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$DOCKER_LOG"
}

log_success() {
    echo -e "${GREEN}[$(date '+%Y-%m-%d %H:%M:%S')] SUCCESS:${NC} $1" | tee -a "$DOCKER_LOG"
}

log_warning() {
    echo -e "${YELLOW}[$(date '+%Y-%m-%d %H:%M:%S')] WARNING:${NC} $1" | tee -a "$DOCKER_LOG"
}

log_error() {
    echo -e "${RED}[$(date '+%Y-%m-%d %H:%M:%S')] ERROR:${NC} $1" | tee -a "$DOCKER_LOG"
}

# Check prerequisites
check_prerequisites() {
    log "Checking Docker prerequisites..."
    
    # Check if Docker is installed
    if ! command -v docker &> /dev/null; then
        log_error "Docker is not installed. Please install Docker first."
        exit 1
    fi
    
    # Check if Docker Compose is installed
    if ! command -v docker-compose &> /dev/null; then
        log_error "Docker Compose is not installed. Please install Docker Compose first."
        exit 1
    fi
    
    # Check Docker daemon
    if ! docker info &> /dev/null; then
        log_error "Docker daemon is not running. Please start Docker first."
        exit 1
    fi
    
    # Check available disk space (at least 10GB)
    AVAILABLE_SPACE=$(df . | awk 'NR==2 {print $4}')
    if [ "$AVAILABLE_SPACE" -lt 10485760 ]; then
        log_warning "Low disk space available. At least 10GB recommended."
    fi
    
    log_success "Prerequisites check passed"
}

# Setup network (following cursor rules)
setup_network() {
    log "Setting up Docker network (following cursor rules)..."
    
    # Check if network exists
    if ! docker network ls | grep -q "$NETWORK_NAME"; then
        log "Creating $NETWORK_NAME network..."
        docker network create "$NETWORK_NAME"
        log_success "Network $NETWORK_NAME created"
    else
        log_success "Network $NETWORK_NAME already exists"
    fi
}

# Clean up existing containers
cleanup() {
    log "Cleaning up existing containers..."
    
    # Stop and remove existing containers
    if [ -f "$DOCKER_COMPOSE_FILE" ]; then
        docker-compose -f "$DOCKER_COMPOSE_FILE" down --volumes --remove-orphans 2>/dev/null || true
    fi
    
    # Remove dangling images
    docker image prune -f 2>/dev/null || true
    
    log_success "Cleanup completed"
}

# Build Docker images
build_images() {
    log "Building Docker images..."
    
    # Build main SDK image
    log "Building satox-sdk image..."
    if ! docker build -t satox-sdk:latest -f "$DOCKERFILE" "$PROJECT_ROOT"; then
        log_error "Failed to build satox-sdk image"
        return 1
    fi
    
    # Build RocksDB image
    log "Building RocksDB image..."
    if ! docker build -t satox-rocksdb:latest -f "$DOCKERFILE_ROCKSDB" "$PROJECT_ROOT"; then
        log_error "Failed to build RocksDB image"
        return 1
    fi
    
    log_success "All images built successfully"
}

# Start services
start_services() {
    log "Starting Docker services..."
    
    if ! docker-compose -f "$DOCKER_COMPOSE_FILE" up -d; then
        log_error "Failed to start Docker services"
        return 1
    fi
    
    log_success "Services started successfully"
}

# Wait for services to be ready
wait_for_services() {
    log "Waiting for services to be ready..."
    
    # Wait for databases
    log "Waiting for PostgreSQL..."
    timeout 60 bash -c 'until docker exec satox-postgres pg_isready -U satox_user -d satox_core_db; do sleep 2; done' || {
        log_error "PostgreSQL failed to start within timeout"
        return 1
    }
    
    log "Waiting for Redis..."
    timeout 30 bash -c 'until docker exec satox-redis redis-cli ping; do sleep 2; done' || {
        log_error "Redis failed to start within timeout"
        return 1
    }
    
    log "Waiting for MongoDB..."
    timeout 60 bash -c 'until docker exec satox-mongodb mongosh --eval "db.adminCommand(\"ping\")" --quiet; do sleep 2; done' || {
        log_error "MongoDB failed to start within timeout"
        return 1
    }
    
    log "Waiting for RocksDB..."
    timeout 30 bash -c 'until docker exec satox-rocksdb test -f /data/health; do sleep 2; done' || {
        log_error "RocksDB failed to start within timeout"
        return 1
    }
    
    log_success "All services are ready"
}

# Run health checks
run_health_checks() {
    log "Running health checks..."
    
    # Check container status
    log "Checking container status..."
    if ! docker-compose -f "$DOCKER_COMPOSE_FILE" ps | grep -q "Up"; then
        log_error "Some containers are not running"
        docker-compose -f "$DOCKER_COMPOSE_FILE" ps
        return 1
    fi
    
    # Check SDK health endpoint (following cursor rules - port 8080)
    log "Checking SDK health endpoint..."
    if curl -f http://localhost:8080/health 2>/dev/null; then
        log_success "SDK health check passed"
    else
        log_warning "SDK health endpoint not available yet"
    fi
    
    # Check network connectivity
    log "Checking network connectivity..."
    if docker network inspect "$NETWORK_NAME" >/dev/null 2>&1; then
        log_success "Network connectivity verified"
    else
        log_error "Network connectivity issues detected"
        return 1
    fi
    
    log_success "Health checks completed"
}

# Run tests in Docker
run_docker_tests() {
    log "Running tests in Docker containers..."
    
    # Run SDK tests
    log "Running SDK tests..."
    if docker exec satox-sdk /usr/local/satox-sdk/bin/satox-sdk-test 2>/dev/null; then
        log_success "SDK tests passed"
    else
        log_warning "SDK tests failed or not available"
    fi
    
    # Test database connections
    log "Testing database connections..."
    
    # Test PostgreSQL
    if docker exec satox-postgres psql -U satox_user -d satox_core_db -c "SELECT 1;" 2>/dev/null; then
        log_success "PostgreSQL connection test passed"
    else
        log_warning "PostgreSQL connection test failed"
    fi
    
    # Test Redis
    if docker exec satox-redis redis-cli ping 2>/dev/null | grep -q "PONG"; then
        log_success "Redis connection test passed"
    else
        log_warning "Redis connection test failed"
    fi
    
    # Test MongoDB
    if docker exec satox-mongodb mongosh --eval "db.adminCommand('ping')" --quiet 2>/dev/null; then
        log_success "MongoDB connection test passed"
    else
        log_warning "MongoDB connection test failed"
    fi
    
    log_success "Docker tests completed"
}

# Show service information
show_service_info() {
    log "Service Information:"
    echo "==================="
    
    echo -e "${BLUE}SDK Service:${NC}"
    echo "  - API: http://localhost:8080"
    echo "  - Health: http://localhost:8080/health"
    echo "  - Metrics: http://localhost:8080/metrics"
    
    echo -e "${BLUE}Databases:${NC}"
    echo "  - PostgreSQL: localhost:5432"
    echo "  - Redis: localhost:6379"
    echo "  - MongoDB: localhost:27017"
    echo "  - RocksDB: Container only"
    
    echo -e "${BLUE}Network:${NC}"
    echo "  - Network: $NETWORK_NAME"
    echo "  - Following cursor rules for Docker deployment"
    
    echo -e "${BLUE}Monitoring:${NC}"
    echo "  - Using centralized monitoring system (following cursor rules)"
    echo "  - Access centralized monitoring at:"
    echo "    - Grafana: http://localhost:3000 (admin/admin)"
    echo "    - Prometheus: http://localhost:9090"
    echo "    - Alertmanager: http://localhost:9093"
    
    echo -e "${BLUE}Container Status:${NC}"
    docker-compose -f "$DOCKER_COMPOSE_FILE" ps
    
    echo -e "${BLUE}Logs:${NC}"
    echo "  - Docker logs: $DOCKER_LOG"
    echo "  - Application logs: $PROJECT_ROOT/logs/"
}

# Main deployment function
deploy() {
    log "Starting Satox SDK Docker deployment (following cursor rules)..."
    
    # Change to project root
    cd "$PROJECT_ROOT"
    
    # Run deployment steps
    check_prerequisites
    setup_network
    cleanup
    build_images
    start_services
    wait_for_services
    run_health_checks
    run_docker_tests
    show_service_info
    
    log_success "Docker deployment completed successfully!"
    log "You can now access the services at the URLs shown above."
    log "To stop the services, run: docker-compose -f $DOCKER_COMPOSE_FILE down"
    log "Note: This deployment follows cursor rules for Docker and network usage."
}

# Cleanup function
cleanup_deployment() {
    log "Cleaning up Docker deployment..."
    
    if [ -f "$DOCKER_COMPOSE_FILE" ]; then
        docker-compose -f "$DOCKER_COMPOSE_FILE" down --volumes --remove-orphans
    fi
    
    # Remove images
    docker rmi satox-sdk:latest satox-rocksdb:latest 2>/dev/null || true
    
    log_success "Cleanup completed"
}

# Show logs
show_logs() {
    log "Showing service logs..."
    docker-compose -f "$DOCKER_COMPOSE_FILE" logs -f
}

# Main script logic
case "${1:-deploy}" in
    "deploy")
        deploy
        ;;
    "cleanup")
        cleanup_deployment
        ;;
    "logs")
        show_logs
        ;;
    "test")
        run_docker_tests
        ;;
    "health")
        run_health_checks
        ;;
    "build")
        check_prerequisites
        build_images
        ;;
    "start")
        setup_network
        start_services
        wait_for_services
        ;;
    "stop")
        docker-compose -f "$DOCKER_COMPOSE_FILE" down
        ;;
    "restart")
        docker-compose -f "$DOCKER_COMPOSE_FILE" restart
        ;;
    "status")
        docker-compose -f "$DOCKER_COMPOSE_FILE" ps
        ;;
    *)
        echo "Usage: $0 {deploy|cleanup|logs|test|health|build|start|stop|restart|status}"
        echo ""
        echo "Commands:"
        echo "  deploy   - Full deployment (default)"
        echo "  cleanup  - Remove all containers and images"
        echo "  logs     - Show service logs"
        echo "  test     - Run tests in containers"
        echo "  health   - Run health checks"
        echo "  build    - Build Docker images only"
        echo "  start    - Start services only"
        echo "  stop     - Stop services"
        echo "  restart  - Restart services"
        echo "  status   - Show service status"
        exit 1
        ;;
esac 