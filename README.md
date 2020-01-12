# Multithreaded_String_Searching
Implement multi-thread programming to fulfill client-sever model

## Overview 
![](https://i.imgur.com/kZRSOYH.png)


## Behavior
- Files stored in the server
- Each client sends a request to the server to search for one or
more strings in these files
- Server returns the search results

## Server
- Connected with clients via Internet sockets
- Multi-threaded: a main thread + a pool of worker thread
- Main thread
    - Responsible for receiving client requests
    - Insert the requests in the request queue
-  Worker threads
    - Remove entries from the request queue and search all files for the
queried strings
    - Return results to the clients
    
## How to build
- `make` 

## Correctness

- pass all testbench from `OSLab`

## How to execute program

- execute server
`./server -r ROOT -p PORT -n THREAD_NUMBER`

- execute client
`./client -h LOCALHOST -p PORT`

- send request in client
` “QUERY_STRING” [“QUERY_STRING”]*`
    
    - QUERY_STRING: The string(s) that the client tries to query
    - The maximum size of the QUERY_STRING is 128 bytes
    - support one or more requests
