## Computer Move Service

Modules can request on NATS subject `get_computer_move`
  * CMS will reply to the request with the computer remove {rock, paper, scissors}
  * CMS will also publish the computer move to the NATS subject `computer_move`
  
#### Run Computer Move Service
```
docker run --network=host  sanjayrancher/rps-generate-move:v2  nats://localhost:4222
```

#### Example request computer generated move. Set 'always_win' to get an move that wins against the last 5 captured user frames
```
python3 example_requester.py nats://localhost:4222 always_win
```



Consume the NATS subject `computer_move` when running the example requester to view payload: `python3 nats-sub.py computer_move -s nats://localhost:4222`