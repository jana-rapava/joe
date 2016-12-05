# Protocol for file transfer:

**Purpose**: Transfere a file from the client to the server.

     ----------                           ----------
    |  Server  | _______________________ |  Client  |
     ----------                           ----------
  
     Server               Client
        |                    |
        |   HELLO,file_name  |     First message from the client to the server containe 2 frames,
        | <----------------- |     One with the string "HELLO" and one with the name of the file.
        |                    |
        |   READY / ERROR    |     If the server is ready, the server send one message with one frame
        | -----------------> |     with the string "READY" otherwise the string "ERROR" and close the
        |                    |     connection.
        |                    |
    
# Messages
Client: "HELLO", "filename"
Server response: "READY" or "ERROR", "descriptive text"

Client: "CHUNK", "filename", "offset", "size", "checksum", "data"
Server response: "READY" or "ERROR", "descriptive text"

Client: "CLOSE", "filename", "size"
Server response: "READY" or "ERROR", "descriptive text"

# Protocol rules
  1. First message: HELLO
  2. Then 0-* messages CHUNK. The filename must match the one in HELLO. The chunks in transit (=with no answer yet), as defined by offset and size, must not overlap.
  3. Last message: CLOSE. The filename must match the one in HELLO. All chunks must have been already transfered (no gaps). The size must match the size of the file transfered (size is equal to offset+size of the chunk with highest offset).
  
## Error handling:
  1. HELLO: ERROR means no new transfer has been set up. If any file is already in transit, it's not affected.
  2. CHUNK: ERROR means this particular chunk is rejected. It does not affect any other chunk.
  3. CLOSE: ERROR means abort of the transfer of this filename. After ERROR, a new HELLO is necessary for any further transfer of this file.

## What should work:
  1. Handle of multiple files in transfer from the same client.
  2. Hanlde of multiple clients.
  3. Reject attempts to transfer the same file from multiple requests (same client or different clients)

# Problems:
  1. Try to communicate using czmq pull & push functions
  2. Try to use czmq functions router (for server) and dealer (for client) on your local machine using the protocol
  3. Try to implement by teams one server and one client using the protocol
  4. **Try to implement a local server&client using czmq actor model and the protocol**
