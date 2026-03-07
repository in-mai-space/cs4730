# Compilation and Running Instructions

To compile the program, run:

```bash
make
````

This will generate the required binary files.

```bash
./server_binary <port> <factory_id> <num_peers> [<peer_id> <peer_ip> <peer_port>] ...
```

* `<port>` — Port number for the server to listen on
* `<factory_id>` — Unique ID for this server
* `<num_peers>` — Number of peer servers
* `[<peer_id> <peer_ip> <peer_port>] ...` — Optional details of each peer server

## Running the Client

```bash
./client_binary <ip-address> <port> <num_customers> <num_orders> <request_type>
```

* `<ip-address>` — IP address of the server to connect to
* `<port>` — Server port to connect to
* `<num_customers>` — Number of customer threads to simulate
* `<num_orders>` — Number of orders per customer
* `<request_type>` — Type of request to perform

