# Deployment Guide

## System Requirements

### Hardware Requirements

#### Minimum Requirements
- **CPU**: 2 cores (x86_64 or ARM64)
- **RAM**: 4 GB
- **Storage**: 10 GB available space
- **Network**: 100 Mbps

#### Recommended Requirements
- **CPU**: 8+ cores (x86_64 or ARM64)
- **RAM**: 16 GB
- **Storage**: 50 GB SSD
- **Network**: 1 Gbps

#### Production Requirements
- **CPU**: 16+ cores (x86_64 or ARM64)
- **RAM**: 32+ GB
- **Storage**: 100+ GB NVMe SSD
- **Network**: 10 Gbps

### Operating System Support

| OS | Version | Architecture | Support Level |
|---|---|---|---|
| Ubuntu | 20.04, 22.04 | x86_64, ARM64 | Full |
| Debian | 11, 12 | x86_64, ARM64 | Full |
| RHEL/CentOS | 8, 9 | x86_64 | Full |
| macOS | 11+, 12+, 13+ | x86_64, ARM64 | Full |
| Windows | 10, 11, Server 2019+ | x86_64 | Partial |
| Alpine Linux | 3.16+ | x86_64, ARM64 | Container only |

### Software Dependencies

#### Build Dependencies
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libpq-dev

# RHEL/CentOS
sudo yum install -y \
    gcc-c++ \
    cmake3 \
    git \
    openssl-devel \
    postgresql-devel

# macOS
brew install \
    cmake \
    postgresql \
    openssl
```

#### Runtime Dependencies
- PostgreSQL 12+ (for database features)
- OpenSSL 1.1.1+ (for SSL/TLS)
- libc++ or libstdc++ (C++20 support)

## Installation

### From Source

#### 1. Clone Repository
```bash
git clone https://github.com/yourorg/messaging_system.git
cd messaging_system
git submodule update --init --recursive
```

#### 2. Configure Build
```bash
mkdir build && cd build

# Standard build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/messaging_system

# Custom configuration
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/messaging_system \
    -DUSE_DATABASE=ON \
    -DUSE_PYTHON_BINDING=OFF \
    -DENABLE_LTO=ON \
    -DENABLE_SIMD=ON
```

#### 3. Build and Install
```bash
# Build
cmake --build . --parallel $(nproc)

# Run tests
ctest --output-on-failure

# Install
sudo cmake --install .
```

### Using Package Managers

#### APT Repository (Ubuntu/Debian)
```bash
# Add repository
echo "deb https://repo.messaging-system.io/apt stable main" | \
    sudo tee /etc/apt/sources.list.d/messaging-system.list

# Add GPG key
curl -fsSL https://repo.messaging-system.io/apt/gpg.key | \
    sudo apt-key add -

# Install
sudo apt-get update
sudo apt-get install messaging-system
```

#### YUM Repository (RHEL/CentOS)
```bash
# Add repository
sudo tee /etc/yum.repos.d/messaging-system.repo <<EOF
[messaging-system]
name=Messaging System Repository
baseurl=https://repo.messaging-system.io/yum/\$releasever/\$basearch/
enabled=1
gpgcheck=1
gpgkey=https://repo.messaging-system.io/yum/gpg.key
EOF

# Install
sudo yum install messaging-system
```

### Docker Installation

#### Pull Official Image
```bash
docker pull messaging-system/server:latest
docker pull messaging-system/worker:latest
```

#### Build Custom Image
```dockerfile
# Dockerfile
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpq-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source
WORKDIR /build
COPY . .

# Build
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --parallel

# Runtime image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libpq5 \
    && rm -rf /var/lib/apt/lists/*

# Copy binaries
COPY --from=builder /build/build/bin/* /usr/local/bin/
COPY --from=builder /build/build/lib/* /usr/local/lib/

# Configuration
ENV LD_LIBRARY_PATH=/usr/local/lib
EXPOSE 8080 9090

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:9090/health || exit 1

# Run
CMD ["/usr/local/bin/messaging_server"]
```

Build and run:
```bash
docker build -t my-messaging-system .
docker run -d \
    --name messaging \
    -p 8080:8080 \
    -p 9090:9090 \
    -v /etc/messaging:/etc/messaging \
    my-messaging-system
```

## Configuration

### Configuration File Structure

```ini
; /etc/messaging/config.ini

[system]
instance_id = prod-server-01
datacenter = us-east-1
environment = production
log_level = info

[network]
bind_address = 0.0.0.0
listen_port = 8080
max_connections = 10000
connection_timeout_ms = 5000
keepalive_interval_s = 30
enable_ssl = true
ssl_cert_file = /etc/messaging/certs/server.crt
ssl_key_file = /etc/messaging/certs/server.key

[database]
host = db.internal.example.com
port = 5432
database = messaging_prod
username = messaging_user
password = ${DB_PASSWORD}
pool_size = 50
max_pool_size = 100
connection_timeout_ms = 3000
idle_timeout_s = 300

[message_bus]
worker_threads = 16
queue_size = 100000
batch_size = 100
enable_persistence = true
persistence_path = /var/lib/messaging/queue

[monitoring]
enable = true
metrics_port = 9090
metrics_path = /metrics
enable_tracing = true
jaeger_endpoint = http://jaeger:14268/api/traces

[security]
enable_auth = true
auth_type = jwt
jwt_secret = ${JWT_SECRET}
token_expiry_minutes = 60
enable_rate_limiting = true
rate_limit_window_s = 60
rate_limit_requests = 1000

[clustering]
enable = true
cluster_name = messaging-prod
discovery_method = consul
consul_endpoint = consul.service.consul:8500
heartbeat_interval_s = 5
election_timeout_s = 10
```

### Environment Variables

```bash
# System
export MESSAGING_INSTANCE_ID=prod-01
export MESSAGING_ENVIRONMENT=production
export MESSAGING_LOG_LEVEL=info

# Network
export MESSAGING_PORT=8080
export MESSAGING_SSL_CERT=/etc/messaging/certs/server.crt
export MESSAGING_SSL_KEY=/etc/messaging/certs/server.key

# Database
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=messaging_prod
export DB_USER=messaging_user
export DB_PASSWORD=secure_password_here

# Security
export JWT_SECRET=your_jwt_secret_key_here
export API_KEY=your_api_key_here

# Performance
export WORKER_THREADS=16
export MAX_CONNECTIONS=10000
export QUEUE_SIZE=100000
```

### Secrets Management

#### Using HashiCorp Vault
```bash
# Store secrets in Vault
vault kv put secret/messaging/prod \
    db_password="secure_password" \
    jwt_secret="jwt_secret_key" \
    api_key="api_key_value"

# Retrieve secrets at runtime
export DB_PASSWORD=$(vault kv get -field=db_password secret/messaging/prod)
export JWT_SECRET=$(vault kv get -field=jwt_secret secret/messaging/prod)
```

#### Using Kubernetes Secrets
```yaml
apiVersion: v1
kind: Secret
metadata:
  name: messaging-secrets
type: Opaque
data:
  db-password: <base64-encoded-password>
  jwt-secret: <base64-encoded-secret>
```

## Deployment Scenarios

### Single Server Deployment

Suitable for development and small-scale production.

```bash
# Install
sudo apt-get install messaging-system

# Configure
sudo vi /etc/messaging/config.ini

# Start service
sudo systemctl start messaging-system
sudo systemctl enable messaging-system

# Check status
sudo systemctl status messaging-system
sudo journalctl -u messaging-system -f
```

### High Availability Deployment

Multi-server setup with load balancing and failover.

#### Architecture
```
                    ┌──────────────┐
                    │Load Balancer │
                    └──────┬───────┘
                           │
         ┌─────────────────┼─────────────────┐
         │                 │                 │
    ┌────▼────┐      ┌────▼────┐      ┌────▼────┐
    │Server 1 │      │Server 2 │      │Server 3 │
    └────┬────┘      └────┬────┘      └────┬────┘
         │                 │                 │
         └─────────────────┼─────────────────┘
                           │
                    ┌──────▼──────┐
                    │  PostgreSQL  │
                    │   Cluster    │
                    └─────────────┘
```

#### HAProxy Configuration
```
# /etc/haproxy/haproxy.cfg

global
    maxconn 50000
    log /dev/log local0
    user haproxy
    group haproxy
    daemon

defaults
    mode tcp
    log global
    option tcplog
    option dontlognull
    timeout connect 5000ms
    timeout client 50000ms
    timeout server 50000ms

frontend messaging_frontend
    bind *:8080
    default_backend messaging_backend

backend messaging_backend
    balance roundrobin
    option tcp-check

    server msg1 10.0.1.10:8080 check
    server msg2 10.0.1.11:8080 check
    server msg3 10.0.1.12:8080 check
```

### Kubernetes Deployment

#### Namespace and ConfigMap
```yaml
# namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: messaging-system

---
# configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: messaging-config
  namespace: messaging-system
data:
  config.ini: |
    [system]
    environment = production
    log_level = info

    [network]
    listen_port = 8080
    max_connections = 10000

    [database]
    host = postgres-service
    port = 5432
    database = messaging
```

#### Deployment
```yaml
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: messaging-server
  namespace: messaging-system
spec:
  replicas: 3
  selector:
    matchLabels:
      app: messaging-server
  template:
    metadata:
      labels:
        app: messaging-server
    spec:
      containers:
      - name: messaging
        image: messaging-system/server:latest
        ports:
        - containerPort: 8080
          name: messaging
        - containerPort: 9090
          name: metrics
        env:
        - name: DB_PASSWORD
          valueFrom:
            secretKeyRef:
              name: messaging-secrets
              key: db-password
        - name: JWT_SECRET
          valueFrom:
            secretKeyRef:
              name: messaging-secrets
              key: jwt-secret
        volumeMounts:
        - name: config
          mountPath: /etc/messaging
        - name: data
          mountPath: /var/lib/messaging
        resources:
          requests:
            memory: "2Gi"
            cpu: "1000m"
          limits:
            memory: "4Gi"
            cpu: "2000m"
        livenessProbe:
          httpGet:
            path: /health
            port: 9090
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /ready
            port: 9090
          initialDelaySeconds: 5
          periodSeconds: 5
      volumes:
      - name: config
        configMap:
          name: messaging-config
      - name: data
        persistentVolumeClaim:
          claimName: messaging-data
```

#### Service and Ingress
```yaml
# service.yaml
apiVersion: v1
kind: Service
metadata:
  name: messaging-service
  namespace: messaging-system
spec:
  selector:
    app: messaging-server
  ports:
  - name: messaging
    port: 8080
    targetPort: 8080
  - name: metrics
    port: 9090
    targetPort: 9090
  type: ClusterIP

---
# ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: messaging-ingress
  namespace: messaging-system
  annotations:
    kubernetes.io/ingress.class: nginx
    cert-manager.io/cluster-issuer: letsencrypt-prod
spec:
  tls:
  - hosts:
    - messaging.example.com
    secretName: messaging-tls
  rules:
  - host: messaging.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: messaging-service
            port:
              number: 8080
```

#### Horizontal Pod Autoscaler
```yaml
# hpa.yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: messaging-hpa
  namespace: messaging-system
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: messaging-server
  minReplicas: 3
  maxReplicas: 20
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
  - type: Resource
    resource:
      name: memory
      target:
        type: Utilization
        averageUtilization: 80
```

### Docker Compose Deployment

```yaml
# docker-compose.yml
version: '3.8'

services:
  postgres:
    image: postgres:15-alpine
    environment:
      POSTGRES_DB: messaging
      POSTGRES_USER: messaging_user
      POSTGRES_PASSWORD: secure_password
    volumes:
      - postgres_data:/var/lib/postgresql/data
    networks:
      - messaging_network
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U messaging_user"]
      interval: 10s
      timeout: 5s
      retries: 5

  redis:
    image: redis:7-alpine
    command: redis-server --appendonly yes
    volumes:
      - redis_data:/data
    networks:
      - messaging_network
    healthcheck:
      test: ["CMD", "redis-cli", "ping"]
      interval: 10s
      timeout: 5s
      retries: 5

  messaging-server:
    image: messaging-system/server:latest
    depends_on:
      postgres:
        condition: service_healthy
      redis:
        condition: service_healthy
    environment:
      DB_HOST: postgres
      DB_PORT: 5432
      DB_NAME: messaging
      DB_USER: messaging_user
      DB_PASSWORD: secure_password
      REDIS_HOST: redis
      REDIS_PORT: 6379
      WORKER_THREADS: 8
    ports:
      - "8080:8080"
      - "9090:9090"
    volumes:
      - ./config:/etc/messaging
      - messaging_data:/var/lib/messaging
    networks:
      - messaging_network
    deploy:
      replicas: 3
      resources:
        limits:
          cpus: '2'
          memory: 2G
        reservations:
          cpus: '1'
          memory: 1G
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:9090/health"]
      interval: 30s
      timeout: 10s
      retries: 3

  nginx:
    image: nginx:alpine
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
    ports:
      - "80:80"
      - "443:443"
    depends_on:
      - messaging-server
    networks:
      - messaging_network

volumes:
  postgres_data:
  redis_data:
  messaging_data:

networks:
  messaging_network:
    driver: bridge
```

## Performance Tuning

### System Tuning

#### Linux Kernel Parameters
```bash
# /etc/sysctl.d/99-messaging.conf

# Network
net.core.somaxconn = 65535
net.core.netdev_max_backlog = 65536
net.ipv4.tcp_max_syn_backlog = 65536
net.ipv4.tcp_fin_timeout = 15
net.ipv4.tcp_tw_reuse = 1
net.ipv4.tcp_keepalive_time = 300
net.ipv4.tcp_keepalive_probes = 5
net.ipv4.tcp_keepalive_intvl = 15

# Memory
vm.swappiness = 10
vm.dirty_ratio = 15
vm.dirty_background_ratio = 5

# File descriptors
fs.file-max = 2097152
fs.nr_open = 2097152

# Apply settings
sudo sysctl -p /etc/sysctl.d/99-messaging.conf
```

#### ulimit Configuration
```bash
# /etc/security/limits.d/messaging.conf

messaging soft nofile 1048576
messaging hard nofile 1048576
messaging soft nproc 65536
messaging hard nproc 65536
messaging soft memlock unlimited
messaging hard memlock unlimited
```

### Application Tuning

#### Thread Pool Configuration
```ini
[thread_pool]
; Number of worker threads (2 * CPU cores recommended)
worker_threads = 32

; Thread pool priorities
realtime_threads = 4
batch_threads = 16
background_threads = 8

; CPU affinity (bind threads to specific cores)
enable_cpu_affinity = true
cpu_affinity_mask = 0xFFFFFFFF
```

#### Memory Management
```ini
[memory]
; Object pool sizes
message_pool_size = 10000
buffer_pool_size = 1000
connection_pool_size = 100

; Arena allocator settings
arena_size_mb = 128
arena_count = 4

; Garbage collection
gc_interval_seconds = 60
gc_threshold_mb = 1024
```

#### Network Optimization
```ini
[network_optimization]
; TCP settings
tcp_nodelay = true
tcp_quickack = true
socket_buffer_size = 1048576

; Connection pooling
connection_pool_size = 100
connection_idle_timeout_s = 300
connection_max_age_s = 3600

; Batch processing
enable_batching = true
batch_size = 100
batch_timeout_ms = 10
```

### Database Tuning

#### PostgreSQL Configuration
```conf
# postgresql.conf

# Connection settings
max_connections = 200
superuser_reserved_connections = 3

# Memory settings
shared_buffers = 4GB
effective_cache_size = 12GB
maintenance_work_mem = 1GB
work_mem = 64MB

# Write performance
checkpoint_segments = 32
checkpoint_completion_target = 0.9
wal_buffers = 16MB

# Query tuning
random_page_cost = 1.1
effective_io_concurrency = 200
```

#### Connection Pool Tuning
```ini
[database_pool]
min_connections = 10
max_connections = 100
connection_timeout_ms = 3000
idle_timeout_s = 300
validation_query = SELECT 1
validation_interval_s = 30
```

## Monitoring Setup

### Prometheus Integration

#### Prometheus Configuration
```yaml
# prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

scrape_configs:
  - job_name: 'messaging-system'
    static_configs:
      - targets:
        - 'messaging-server-1:9090'
        - 'messaging-server-2:9090'
        - 'messaging-server-3:9090'
```

#### Metrics Exposed
```
# System metrics
messaging_system_uptime_seconds
messaging_system_cpu_usage_percent
messaging_system_memory_usage_bytes
messaging_system_thread_count

# Message metrics
messaging_messages_processed_total
messaging_messages_failed_total
messaging_message_processing_duration_seconds
messaging_message_queue_size

# Network metrics
messaging_connections_active
messaging_connections_total
messaging_bytes_sent_total
messaging_bytes_received_total

# Database metrics
messaging_db_connections_active
messaging_db_queries_total
messaging_db_query_duration_seconds
messaging_db_errors_total
```

### Grafana Dashboard

```json
{
  "dashboard": {
    "title": "Messaging System Dashboard",
    "panels": [
      {
        "title": "Message Throughput",
        "targets": [
          {
            "expr": "rate(messaging_messages_processed_total[5m])"
          }
        ]
      },
      {
        "title": "Error Rate",
        "targets": [
          {
            "expr": "rate(messaging_messages_failed_total[5m])"
          }
        ]
      },
      {
        "title": "Active Connections",
        "targets": [
          {
            "expr": "messaging_connections_active"
          }
        ]
      },
      {
        "title": "CPU Usage",
        "targets": [
          {
            "expr": "messaging_system_cpu_usage_percent"
          }
        ]
      }
    ]
  }
}
```

### Logging Configuration

#### Structured Logging
```ini
[logging]
; Log format
format = json

; Log levels
level = info
levels.network = debug
levels.database = warn

; Output destinations
console.enabled = true
file.enabled = true
file.path = /var/log/messaging/messaging.log
file.rotate_size_mb = 100
file.rotate_count = 10

; Remote logging
syslog.enabled = true
syslog.address = syslog.example.com:514
syslog.protocol = tcp
```

#### Log Aggregation (ELK Stack)
```yaml
# filebeat.yml
filebeat.inputs:
- type: log
  enabled: true
  paths:
    - /var/log/messaging/*.log
  json.keys_under_root: true
  json.add_error_key: true

output.elasticsearch:
  hosts: ["elasticsearch:9200"]
  index: "messaging-%{+yyyy.MM.dd}"

processors:
  - add_host_metadata:
      when.not.contains:
        tags: forwarded
```

## Scaling Strategies

### Horizontal Scaling

#### Load Balancer Configuration
```nginx
# nginx.conf
upstream messaging_backend {
    least_conn;

    server server1.example.com:8080 weight=5;
    server server2.example.com:8080 weight=5;
    server server3.example.com:8080 weight=5;

    # Backup servers
    server backup1.example.com:8080 backup;
    server backup2.example.com:8080 backup;

    # Health check
    check interval=3000 rise=2 fall=3 timeout=1000;
}
```

#### Auto-scaling Rules
```yaml
# AWS Auto Scaling
AutoScalingGroup:
  MinSize: 3
  MaxSize: 20
  DesiredCapacity: 5
  TargetGroupARNs:
    - !Ref TargetGroup
  MetricsCollection:
    - Granularity: 1Minute
  ScalingPolicies:
    - PolicyName: ScaleUp
      ScalingAdjustment: 2
      AdjustmentType: ChangeInCapacity
      Cooldown: 300
      MetricAggregationType: Average
      StepAdjustments:
        - MetricIntervalLowerBound: 0
          ScalingAdjustment: 2
```

### Vertical Scaling

#### Resource Allocation
```bash
# CPU affinity for high-performance
taskset -c 0-15 /usr/local/bin/messaging_server

# Memory allocation
numactl --cpubind=0 --membind=0 /usr/local/bin/messaging_server

# I/O scheduling
echo noop > /sys/block/sda/queue/scheduler
```

### Database Scaling

#### Read Replicas
```ini
[database_replicas]
master.host = db-master.example.com
master.port = 5432

replica1.host = db-replica1.example.com
replica1.port = 5432
replica1.weight = 1

replica2.host = db-replica2.example.com
replica2.port = 5432
replica2.weight = 1

# Read preference
read_preference = replica
failover_to_master = true
```

#### Sharding Strategy
```ini
[database_sharding]
enable = true
shard_count = 4
shard_key = user_id
distribution = consistent_hash

shard1.range = 0-25
shard1.host = db-shard1.example.com

shard2.range = 26-50
shard2.host = db-shard2.example.com

shard3.range = 51-75
shard3.host = db-shard3.example.com

shard4.range = 76-100
shard4.host = db-shard4.example.com
```

## Backup and Recovery

### Backup Strategy

#### Application State Backup
```bash
#!/bin/bash
# backup.sh

# Configuration
BACKUP_DIR="/backup/messaging"
DATE=$(date +%Y%m%d_%H%M%S)
RETENTION_DAYS=30

# Create backup directory
mkdir -p $BACKUP_DIR/$DATE

# Backup configuration
cp -r /etc/messaging $BACKUP_DIR/$DATE/config

# Backup persistent queues
cp -r /var/lib/messaging/queue $BACKUP_DIR/$DATE/queue

# Backup database
pg_dump -h localhost -U messaging_user messaging > \
    $BACKUP_DIR/$DATE/database.sql

# Compress backup
tar -czf $BACKUP_DIR/messaging_$DATE.tar.gz \
    -C $BACKUP_DIR $DATE

# Remove uncompressed files
rm -rf $BACKUP_DIR/$DATE

# Clean old backups
find $BACKUP_DIR -name "*.tar.gz" -mtime +$RETENTION_DAYS -delete

# Upload to S3
aws s3 cp $BACKUP_DIR/messaging_$DATE.tar.gz \
    s3://backup-bucket/messaging/
```

#### Database Backup
```sql
-- Continuous archiving with PostgreSQL
-- postgresql.conf

archive_mode = on
archive_command = 'test ! -f /backup/wal/%f && cp %p /backup/wal/%f'
archive_timeout = 300

-- Base backup
pg_basebackup -h localhost -D /backup/base -U replicator -W -P
```

### Recovery Procedures

#### Service Recovery
```bash
#!/bin/bash
# recovery.sh

# Stop services
systemctl stop messaging-system

# Restore from backup
BACKUP_FILE=$1
RESTORE_DIR="/tmp/restore"

# Extract backup
tar -xzf $BACKUP_FILE -C $RESTORE_DIR

# Restore configuration
cp -r $RESTORE_DIR/*/config/* /etc/messaging/

# Restore queue data
cp -r $RESTORE_DIR/*/queue/* /var/lib/messaging/queue/

# Restore database
psql -h localhost -U messaging_user messaging < \
    $RESTORE_DIR/*/database.sql

# Start services
systemctl start messaging-system

# Verify recovery
curl http://localhost:9090/health
```

#### Disaster Recovery Plan
```yaml
# Disaster Recovery Configuration
DR:
  RPO: 15 minutes  # Recovery Point Objective
  RTO: 1 hour      # Recovery Time Objective

  Primary_Site:
    Region: us-east-1
    Database: rds-primary.amazonaws.com

  Secondary_Site:
    Region: us-west-2
    Database: rds-secondary.amazonaws.com
    Replication_Lag: 5 seconds

  Failover_Procedure:
    1. Detect primary failure
    2. Promote secondary database
    3. Update DNS to point to secondary
    4. Start services in secondary region
    5. Verify system health
```

## Security Hardening

### Network Security

#### Firewall Rules
```bash
# iptables rules
iptables -A INPUT -p tcp --dport 8080 -m conntrack --ctstate NEW,ESTABLISHED -j ACCEPT
iptables -A INPUT -p tcp --dport 9090 -s 10.0.0.0/8 -j ACCEPT  # Metrics only from internal
iptables -A INPUT -p tcp --dport 5432 -s 10.0.0.0/8 -j ACCEPT  # Database from internal
iptables -A INPUT -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
iptables -A INPUT -j DROP

# Save rules
iptables-save > /etc/iptables/rules.v4
```

#### SSL/TLS Configuration
```ini
[security.ssl]
enabled = true
min_version = TLSv1.3
cert_file = /etc/messaging/certs/server.crt
key_file = /etc/messaging/certs/server.key
ca_file = /etc/messaging/certs/ca.crt
verify_client = true
cipher_suites = TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256
```

### Application Security

#### Authentication
```ini
[security.auth]
type = jwt
algorithm = RS256
public_key_file = /etc/messaging/keys/public.pem
private_key_file = /etc/messaging/keys/private.pem
token_expiry = 3600
refresh_token_expiry = 86400
```

#### Rate Limiting
```ini
[security.rate_limit]
enabled = true
window_seconds = 60
max_requests = 1000
burst_size = 100
blacklist_threshold = 10
blacklist_duration = 3600
```

### Compliance

#### Audit Logging
```ini
[audit]
enabled = true
log_file = /var/log/messaging/audit.log
log_format = json
log_level = info

events_to_log = [
    "authentication",
    "authorization",
    "configuration_change",
    "data_access",
    "data_modification",
    "system_start",
    "system_stop"
]
```

#### Data Encryption
```ini
[security.encryption]
data_at_rest = true
algorithm = AES-256-GCM
key_rotation_days = 90
key_store = /etc/messaging/keys/
```

## Troubleshooting Deployment

### Common Issues

#### Service Won't Start
```bash
# Check logs
journalctl -u messaging-system -n 100

# Check configuration
/usr/local/bin/messaging_server --validate-config

# Check permissions
ls -la /etc/messaging/
ls -la /var/lib/messaging/

# Check port availability
netstat -tulpn | grep 8080
```

#### High Memory Usage
```bash
# Check memory usage
ps aux | grep messaging
top -p $(pgrep messaging)

# Memory profiling
pmap -x $(pgrep messaging)

# Check for leaks
valgrind --leak-check=full /usr/local/bin/messaging_server
```

#### Performance Issues
```bash
# CPU profiling
perf top -p $(pgrep messaging)

# I/O analysis
iotop -p $(pgrep messaging)

# Network analysis
ss -tunapl | grep messaging
tcpdump -i eth0 port 8080
```

### Health Checks

#### Application Health
```bash
# Basic health check
curl http://localhost:9090/health

# Detailed health check
curl http://localhost:9090/health/detailed

# Response format
{
  "status": "healthy",
  "uptime": 3600,
  "version": "1.0.0",
  "components": {
    "database": "healthy",
    "queue": "healthy",
    "cache": "healthy"
  }
}
```

#### System Health
```bash
#!/bin/bash
# health_check.sh

# Check service status
if ! systemctl is-active --quiet messaging-system; then
    echo "Service is not running"
    exit 1
fi

# Check port availability
if ! nc -z localhost 8080; then
    echo "Port 8080 is not accessible"
    exit 1
fi

# Check database connection
if ! psql -h localhost -U messaging_user -c "SELECT 1" > /dev/null 2>&1; then
    echo "Database is not accessible"
    exit 1
fi

echo "All health checks passed"
exit 0
```