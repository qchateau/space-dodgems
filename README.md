# Space Dodgems

This is a mini-game created with boost.beast, C++20 coroutines and websockets.

[I want to play](https://space-dodgems.qchateau.fr/)

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

## Use behind a reverse-proxy

Here is a sample of the location blocks you can add
to an nginx server to use space-dodgems behind nginx
acting as a reverse-proxy.

Note that it works for both HTTP and HTTPS.

```
server {
        ...

        location = /space-dodgems {
                return 302 /space-dodgems/;
        }

        location /space-dodgems {
                proxy_pass http://127.0.0.1:8080/;
                proxy_http_version 1.1;
                proxy_set_header Upgrade $http_upgrade;
                proxy_set_header Connection "upgrade";
        }

        ...
}
```
