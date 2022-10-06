## Rock Paper Scissors AI

Enable NATS
```
docker run -d --name nats-main -p 4222:4222 -p 6222:6222 -p 8222:8222 nats 
```

____

Run RPS detection
Detects what move the user made and publishes it to the NATS subject 'human_move'
```
docker run --network=host  sanjayrancher/rps-move-detector rtsp://localhost:8554/rps nats://localhost:4222
```

___
Listen to NATS subject
```
python3 nats-sub.py human_move -s nats://localhost:4222

```

