# Deployment Guide

## Overview

This guide covers different deployment options for the DigiPets server.

## Docker Deployment (Recommended)

### Prerequisites
- Docker 20.10+
- Docker Compose 2.0+

### Quick Start

1. **Clone the repository**
```bash
git clone https://github.com/Gameaday/digi-pets-20.git
cd digi-pets-20
```

2. **Start the server**
```bash
docker-compose up -d
```

3. **Check status**
```bash
docker-compose ps
docker-compose logs -f
```

4. **Test the API**
```bash
curl http://localhost:8080/health
```

5. **Stop the server**
```bash
docker-compose down
```

### Custom Configuration

Edit `docker-compose.yml` to change settings:

```yaml
services:
  digipets-server:
    build: .
    ports:
      - "8080:8080"  # Change host port here
    volumes:
      - ./data:/app/data  # Persistent storage
    environment:
      - PORT=8080
```

### Data Persistence

Pet data is stored in `./data/pets.json` on the host machine. This ensures pets survive container restarts.

To backup your data:
```bash
cp data/pets.json data/pets.json.backup
```

## Native Build Deployment

### Linux

1. **Install dependencies**
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libssl-dev
```

2. **Build**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

3. **Run**
```bash
./server/digipets_server 8080
```

### macOS

1. **Install dependencies**
```bash
brew install cmake openssl
```

2. **Build**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=$(brew --prefix openssl)
cmake --build . -j$(sysctl -n hw.ncpu)
```

3. **Run**
```bash
./server/digipets_server 8080
```

### Windows

1. **Install dependencies**
   - Visual Studio 2019+ with C++ support
   - CMake 3.20+

2. **Build**
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

3. **Run**
```powershell
.\server\Release\digipets_server.exe 8080
```

## Systemd Service (Linux)

For production deployments on Linux, create a systemd service:

1. **Create service file** `/etc/systemd/system/digipets.service`
```ini
[Unit]
Description=DigiPets Server
After=network.target

[Service]
Type=simple
User=digipets
WorkingDirectory=/opt/digipets
ExecStart=/opt/digipets/digipets_server 8080
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

2. **Create user and setup**
```bash
sudo useradd -r -s /bin/false digipets
sudo mkdir -p /opt/digipets
sudo cp build/server/digipets_server /opt/digipets/
sudo chown -R digipets:digipets /opt/digipets
```

3. **Enable and start service**
```bash
sudo systemctl daemon-reload
sudo systemctl enable digipets
sudo systemctl start digipets
sudo systemctl status digipets
```

## Cloud Deployment

### Docker Hub

1. **Build and tag image**
```bash
docker build -t yourusername/digipets-server:latest .
```

2. **Push to Docker Hub**
```bash
docker login
docker push yourusername/digipets-server:latest
```

3. **Deploy on any cloud provider**
```bash
docker run -d -p 8080:8080 -v $(pwd)/data:/app/data yourusername/digipets-server:latest
```

### AWS ECS

1. **Create task definition** (JSON)
```json
{
  "family": "digipets-server",
  "containerDefinitions": [
    {
      "name": "digipets-server",
      "image": "yourusername/digipets-server:latest",
      "portMappings": [
        {
          "containerPort": 8080,
          "protocol": "tcp"
        }
      ],
      "memory": 512,
      "cpu": 256
    }
  ]
}
```

2. **Create service** via AWS Console or CLI

### Google Cloud Run

```bash
# Build and push to Google Container Registry
gcloud builds submit --tag gcr.io/PROJECT-ID/digipets-server

# Deploy to Cloud Run
gcloud run deploy digipets-server \
  --image gcr.io/PROJECT-ID/digipets-server \
  --platform managed \
  --region us-central1 \
  --allow-unauthenticated \
  --port 8080
```

### Azure Container Instances

```bash
# Create resource group
az group create --name digipets-rg --location eastus

# Deploy container
az container create \
  --resource-group digipets-rg \
  --name digipets-server \
  --image yourusername/digipets-server:latest \
  --dns-name-label digipets-unique-name \
  --ports 8080
```

## Reverse Proxy (Nginx)

For production, use a reverse proxy like Nginx:

```nginx
server {
    listen 80;
    server_name digipets.example.com;

    location / {
        proxy_pass http://localhost:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

## SSL/TLS Configuration

Use Let's Encrypt with Certbot:

```bash
sudo apt-get install certbot python3-certbot-nginx
sudo certbot --nginx -d digipets.example.com
```

## Monitoring

### Health Check Endpoint

```bash
# Check if server is running
curl http://localhost:8080/health
```

### Docker Healthcheck

Already included in `docker-compose.yml`:
```yaml
healthcheck:
  test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
  interval: 30s
  timeout: 10s
  retries: 3
```

### Logs

Docker:
```bash
docker-compose logs -f
```

Systemd:
```bash
sudo journalctl -u digipets -f
```

## Performance Tuning

### Environment Variables

```bash
# Number of worker threads (default: auto)
export DIGIPETS_THREADS=4

# Update interval in seconds (default: 300)
export DIGIPETS_UPDATE_INTERVAL=300
```

### Resource Limits (Docker)

```yaml
services:
  digipets-server:
    deploy:
      resources:
        limits:
          cpus: '0.5'
          memory: 512M
        reservations:
          cpus: '0.25'
          memory: 256M
```

## Backup and Restore

### Backup

```bash
# Backup pets data
cp pets.json pets.json.backup.$(date +%Y%m%d)

# Or with Docker volume
docker cp digipets-server:/app/pets.json ./backup/
```

### Restore

```bash
# Restore pets data
cp pets.json.backup pets.json

# Or with Docker
docker cp ./backup/pets.json digipets-server:/app/
docker-compose restart
```

## Troubleshooting

### Server Won't Start

1. Check port availability
```bash
sudo lsof -i :8080
```

2. Check logs
```bash
docker-compose logs
```

3. Verify file permissions
```bash
ls -la pets.json
```

### Connection Refused

1. Check firewall
```bash
sudo ufw status
sudo ufw allow 8080
```

2. Verify server is running
```bash
docker-compose ps
curl http://localhost:8080/health
```

### High Memory Usage

1. Check number of pets
```bash
curl http://localhost:8080/api/pets | jq length
```

2. Increase container memory limit
```yaml
deploy:
  resources:
    limits:
      memory: 1G
```

## Security Best Practices

1. **Don't expose port 8080 directly to the internet**
   - Use a reverse proxy (Nginx, Apache)
   - Enable SSL/TLS

2. **Limit container capabilities**
```yaml
security_opt:
  - no-new-privileges:true
cap_drop:
  - ALL
```

3. **Run as non-root user** (already configured in Dockerfile)

4. **Keep dependencies updated**
```bash
docker-compose pull
docker-compose up -d
```

5. **Use environment variables for secrets** (when authentication is added)

## Scaling

For high traffic, consider:

1. **Horizontal scaling** with load balancer
2. **Database backend** instead of JSON files
3. **Redis cache** for session management
4. **WebSocket support** for real-time updates

## Support

For deployment issues, check:
- GitHub Issues
- Documentation at `/docs/`
- API reference at `/docs/API.md`
