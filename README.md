# Space Dodgems

This is a mini-game created with boost.beast, C++20 coroutines and websockets.

## How to run

### Release environment

The backend server comes pre-built in the image

```bash
docker-compose up -d
```

### Dev environment

Sources are mounted and the backend is built on every backend restart.

```bash
docker-compose -f docker-compose.yml -f docker-compose-dev.yml up -d
```
