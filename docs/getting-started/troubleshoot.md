# Troubleshoot

## If something unexpected happens during compilation
This should be considered the last resource, but if any other method fail, try to perform a `make clean`.

## Remove all containers

```bash
docker-compose down --remove-orphans
```
