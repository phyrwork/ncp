# ncp - 'nc' parallel

I wrote this utility to try and take advantage of bonded interfaces when sending large data streams between hosts in my computer lab.

The idea is to split the data stream so that it can be transmitted on multiple interfaces simultaneously and then reassembled at the other end.

This implementation relies on L3+4 transmit hashing by creating a number of connections on different ports.

A connection is established on a single, known port and then sockets for the streams are established by the server and the port numbers transmitted to the client to complete the connections.

## Usage:

```
/* receive */
ncp recv [port] > out
 
/* transmit */
ncp send [host] [port] < in
```
 
## Known issues:
 
This is just a proof of concept/simple tool for me to use so there's a lot that hasn't been done properly including
- data verification
- some error checking
- any proper error handling
- configurable stream count / block size

Issues:
- selecting a block size greater than ~16K causes socket errors when attempting to transmit
 
## How it works
1. Client and server 'negotiate' the connection
2. Stream connections are created and assigned to stream threads
3. Data read from stdin in chunks up to a selected block size
4. Block is assigned a sequence number and queued for transmission by a client stream thread
5. Block is transmitted and received by a server stream thread
6. Block is queued for processing by server join (reassembly) thread
7. Server join thread maintains a list of cached blocks and writes out blocks in-sequence when available 
