## Rock Paper Scissors AI

Enable NATS
```
docker run -d --network=host --name nats-main -p 4222:4222 -p 6222:6222 -p 8222:8222 nats 
```

Listen to NATS subject
```
docker run --rm -it --network=host natsio/nats-box:latest

```