# Container Monitoring

This document describes the monitoring and observability setup for Satox SDK containers.

## Overview
- Prometheus for metrics collection
- Grafana for visualization
- Loki for log aggregation
- Alertmanager for alerting
- Node Exporter for system metrics

## Monitoring Stack

### Prometheus
```yaml
# prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "alert_rules.yml"

scrape_configs:
  - job_name: 'satox-sdk'
    static_configs:
      - targets: ['satox-sdk:3000']
    metrics_path: '/metrics'
    scrape_interval: 10s

  - job_name: 'node-exporter'
    static_configs:
      - targets: ['node-exporter:9100']

  - job_name: 'postgres'
    static_configs:
      - targets: ['postgres-exporter:9187']

  - job_name: 'redis'
    static_configs:
      - targets: ['redis-exporter:9121']
```

### Grafana Dashboards
```json
{
  "dashboard": {
    "title": "Satox SDK Overview",
    "panels": [
      {
        "title": "SDK Performance",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(satox_requests_total[5m])",
            "legendFormat": "Requests/sec"
          }
        ]
      },
      {
        "title": "Database Connections",
        "type": "graph",
        "targets": [
          {
            "expr": "pg_stat_database_numbackends",
            "legendFormat": "Active Connections"
          }
        ]
      }
    ]
  }
}
```

### Alert Rules
```yaml
# alert_rules.yml
groups:
  - name: satox-sdk
    rules:
      - alert: HighErrorRate
        expr: rate(satox_errors_total[5m]) > 0.1
        for: 2m
        labels:
          severity: warning
        annotations:
          summary: "High error rate detected"
          description: "Error rate is {{ $value }} errors per second"

      - alert: DatabaseConnectionHigh
        expr: pg_stat_database_numbackends > 80
        for: 1m
        labels:
          severity: warning
        annotations:
          summary: "High database connections"
          description: "Database has {{ $value }} active connections"

      - alert: MemoryUsageHigh
        expr: (node_memory_MemTotal_bytes - node_memory_MemAvailable_bytes) / node_memory_MemTotal_bytes > 0.9
        for: 5m
        labels:
          severity: critical
        annotations:
          summary: "High memory usage"
          description: "Memory usage is {{ $value | humanizePercentage }}"
```

## Metrics Collection

### Application Metrics
```cpp
// SDK metrics collection
class MetricsCollector {
public:
    void incrementRequests() {
        requests_total++;
    }
    
    void incrementErrors() {
        errors_total++;
    }
    
    void recordLatency(double duration) {
        request_duration_seconds.observe(duration);
    }
    
    void setDatabaseConnections(int connections) {
        db_connections.set(connections);
    }
};
```

### System Metrics
- CPU usage
- Memory consumption
- Disk I/O
- Network traffic
- Container resource usage

### Database Metrics
- Connection count
- Query performance
- Lock statistics
- Transaction rates
- Cache hit ratios

## Log Aggregation

### Loki Configuration
```yaml
# loki-config.yml
auth_enabled: false

server:
  http_listen_port: 3100

ingester:
  lifecycler:
    address: 127.0.0.1
    ring:
      kvstore:
        store: inmemory
      replication_factor: 1
    final_sleep: 0s
  chunk_idle_period: 5m
  chunk_retain_period: 30s

schema_config:
  configs:
    - from: 2020-05-15
      store: boltdb-shipper
      object_store: filesystem
      schema: v11
      index:
        prefix: index_
        period: 24h
```

### Log Collection
```yaml
# docker-compose.yml
services:
  promtail:
    image: grafana/promtail:latest
    volumes:
      - /var/log:/var/log
      - ./promtail-config.yml:/etc/promtail/config.yml
    command: -config.file=/etc/promtail/config.yml
    networks:
      - monitoring
```

## Alerting

### Alertmanager Configuration
```yaml
# alertmanager.yml
global:
  smtp_smarthost: 'localhost:587'
  smtp_from: 'alertmanager@satox.com'

route:
  group_by: ['alertname']
  group_wait: 10s
  group_interval: 10s
  repeat_interval: 1h
  receiver: 'team-satox'

receivers:
  - name: 'team-satox'
    email_configs:
      - to: 'team@satox.com'
    slack_configs:
      - api_url: 'https://hooks.slack.com/services/...'
        channel: '#alerts'
```

### Alert Channels
- Email notifications
- Slack integration
- PagerDuty escalation
- Webhook notifications
- SMS alerts (critical)

## Health Checks

### Container Health
```yaml
# Health check configuration
healthcheck:
  test: ["CMD", "curl", "-f", "http://localhost:3000/health"]
  interval: 30s
  timeout: 10s
  retries: 3
  start_period: 40s
```

### Service Health
```cpp
// Health check endpoint
class HealthCheck {
public:
    bool isHealthy() {
        return database.isConnected() && 
               redis.isConnected() && 
               quantumServices.isAvailable();
    }
    
    std::string getStatus() {
        return isHealthy() ? "healthy" : "unhealthy";
    }
};
```

## Performance Monitoring

### Key Metrics
- Request latency (p50, p95, p99)
- Throughput (requests per second)
- Error rates
- Resource utilization
- Database performance

### Performance Baselines
- Response time < 200ms (p95)
- Error rate < 0.1%
- CPU usage < 80%
- Memory usage < 85%
- Database connections < 80%

## Troubleshooting

### Common Monitoring Issues
1. **High latency**: Check database queries, network
2. **Memory leaks**: Monitor memory usage over time
3. **High error rates**: Check application logs
4. **Database issues**: Monitor connection pool

### Debug Commands
```bash
# Check Prometheus targets
curl http://prometheus:9090/api/v1/targets

# Check Grafana health
curl http://grafana:3000/api/health

# View Loki logs
curl -G -s "http://loki:3100/loki/api/v1/query_range" \
  --data-urlencode 'query={job="satox-sdk"}'

# Check Alertmanager
curl http://alertmanager:9093/api/v1/alerts
```

## Resources
- [Docker Setup](setup.md)
- [Production Deployment](production.md)
- [Troubleshooting](../troubleshooting/common-issues.md) 