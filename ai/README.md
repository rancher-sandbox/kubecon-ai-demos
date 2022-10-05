## Rock Paper Scissors AI

Enable NATS
```
docker run -d --name nats-main -p 4222:4222 -p 6222:6222 -p 8222:8222 nats 
```

Listen to NATS subject
```
python3 nats-sub.py human_move -s nats://localhost:4222

```