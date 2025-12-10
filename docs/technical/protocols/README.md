# Communication Protocols

Overview of network communication protocols used in the Haptech system.

## Overview

Haptech uses **gRPC** with **Protocol Buffers** (protobuf) for client-server communication. This provides:
- **Type-safe** communication with schema validation
- **Efficient serialization** (binary format)
- **Streaming** for real-time data collection
- **Cross-language** support (Python client, C++ server)
- **Versioning** via backward-compatible schema evolution

## Architecture

```
┌──────────────────────────────┐
│     Python Client             │
│   python_client/main.py      │
│                               │
│  • Parse XML configuration   │
│  • Build TrialRequest        │
│  • Send via gRPC             │
│  • Stream TrialDataPoints    │
│  • Write to CSV              │
└──────────────┬───────────────┘
               │
               │ gRPC/HTTP2
               │ 
               │
               ▼
┌──────────────────────────────┐
│      C++ Core Server         │
│   FrankaTrialServiceImpl     │
│                               │
│  • Receive TrialRequest      │
│  • Configure scene           │
│  • Run simulation            │
│  • Stream TrialDataPoints    │
└──────────────────────────────┘
```

## Protocol Definition

The protocol is defined in `proto/franka_trial.proto` using Protocol Buffers v3 syntax:

```protobuf
syntax = "proto3";

service FrankaTrialService {
    rpc RunTrial(TrialRequest) returns (stream TrialResponseStream);
}
```

### Key Features

1. **Service Definition**: Single `RunTrial` RPC endpoint
2. **Request/Response**: Structured messages with nested fields
3. **Streaming**: Server streams data back to client in real-time
4. **Type Safety**: Compiler-checked message structure
5. **Versioning**: Add new fields without breaking compatibility

## gRPC Benefits

### vs REST/HTTP

| Feature | gRPC | REST |
|---------|------|------|
| Protocol | HTTP/2 | HTTP/1.1 |
| Encoding | Binary (protobuf) | Text (JSON) |
| Streaming | Bidirectional | Server-sent events |
| Schema | Required (.proto) | Optional (OpenAPI) |
| Performance | Fast | Slower |
| Code Generation | Automatic | Manual/varies |

### vs Raw Sockets

| Feature | gRPC | Raw TCP |
|---------|------|---------|
| Serialization | Built-in (protobuf) | Manual |
| Connection Management | Automatic | Manual |
| Error Handling | Status codes | Custom |
| Compression | Built-in | Manual |
| TLS/SSL | Built-in | Manual |

## Protocol Buffers

### Compilation

Protobuf definitions are compiled to source code:

```bash
# C++ code generation
protoc --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` franka_trial.proto

# Python code generation
python -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. franka_trial.proto
```

**Generated Files**:
- C++: `franka_trial.pb.{h,cc}`, `franka_trial.grpc.pb.{h,cc}`
- Python: `franka_trial_pb2.py`, `franka_trial_pb2_grpc.py`

### Message Structure

```protobuf
message TrialRequest {
    string trial_id = 1;
    repeated Object objects = 2;
    repeated TrialCondition startConditions = 3;
    repeated TrialCondition endConditions = 4;
    SceneSettings sceneSettings = 5;
    DataCollectionConfig dataCollection = 6;
}
```

**Field Numbers**: Permanent identifiers (never change)
- Adding new fields: Always use new numbers
- Removing fields: Mark as `reserved`
- Renaming fields: Safe (only number matters for serialization)

### Default Values

Protocol Buffers v3 uses implicit defaults:
- `string`: `""`
- `int32`, `double`: `0`
- `bool`: `false`
- `repeated`: empty list
- `message`: not set

```cpp
// Check if optional field is set
if (request.has_scenesettings()) {
    // Field was provided
} else {
    // Use default behavior
}
```

## Communication Flow

### Trial Execution Sequence

```
Client                          Server
  │                               │
  ├─ Connect to localhost:50051 ──→
  │                               │
  ├─ Send TrialRequest ──────────→
  │                               ├─ Parse request
  │                               ├─ Validate parameters
  │                               ├─ Configure scene
  │                               ├─ Add objects
  │                               ├─ Set up conditions
  │                               │
  │←─ STARTED status ─────────────┤
  │                               │
  │                               ├─ Enable haptics
  │                               ├─ Start data collection
  │                               │
  │←─ TrialDataPoint (t=0.01) ────┤
  │←─ TrialDataPoint (t=0.02) ────┤
  │←─ TrialDataPoint (t=0.03) ────┤
  │    ...                         │
  │                               ├─ Check end conditions
  │                               ├─ Condition met!
  │                               ├─ Disable haptics
  │                               │
  │←─ COMPLETED status ────────────┤
  │                               │
  ├─ Close connection ────────────→
  │                               ├─ Reset scene
  │                               ├─ Ready for next trial
```

### Error Handling

```
Client                          Server
  │                               │
  ├─ Send TrialRequest ──────────→
  │                               ├─ Trial already running!
  │                               │
  │←─ ERROR status ────────────────┤
  │   message: "There is already a trial running."
  │                               │
  ├─ Close connection ────────────→
```

## Data Rates

### Typical Frequencies

- **Control Commands**: On-demand (trial start/stop)
- **Data Streaming**: ~100 Hz (configurable, limited by network)
- **Haptics Loop**: 1000 Hz (internal only, not streamed)
- **Graphics Loop**: 60 Hz (internal only, not streamed)

## Connection Management

### Server Lifecycle

```cpp
void runServer() {
    std::string server_address("0.0.0.0:50051");
    FrankaTrialServiceImpl service;
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    
    // Block until shutdown
    server->Wait();
}
```

### Client Connection

```python
# Python client
import grpc
from franka_trial_pb2 import *
from franka_trial_pb2_grpc import *

# Create channel
channel = grpc.insecure_channel('localhost:50051')
stub = FrankaTrialServiceStub(channel)

# Call RPC
request = TrialRequest(trial_id="test_trial")
responses = stub.RunTrial(request)

# Iterate over streamed responses
for response in responses:
    if response.HasField('statusUpdate'):
        print(f"Status: {response.statusUpdate.message}")
    elif response.HasField('dataPoint'):
        # Process data...
        pass
```

### Connection Timeout

```python
# Set timeout for RPC call
responses = stub.RunTrial(request, timeout=300)  # 5 minutes
```

### Reconnection

gRPC automatically handles:
- Connection pooling
- Retry on transient failures (configurable)
- Keep-alive pings

```python
# Configure channel options
options = [
    ('grpc.keepalive_time_ms', 10000),
    ('grpc.keepalive_timeout_ms', 5000),
    ('grpc.keepalive_permit_without_calls', True),
]
channel = grpc.insecure_channel('localhost:50051', options=options)
```

## Security

### Current: Insecure (localhost only)

```cpp
// Server
builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

// Client
channel = grpc.insecure_channel('localhost:50051')
```

**Use Case**: Lab environment, local communication only

### TLS/SSL

If there is ever a need for secure communication, this is how you would do it.

```cpp
// Server with TLS
grpc::SslServerCredentialsOptions ssl_opts;
ssl_opts.pem_root_certs = ReadFile("ca.pem");
ssl_opts.pem_key_cert_pairs.push_back({
    ReadFile("server-key.pem"),
    ReadFile("server-cert.pem")
});
auto creds = grpc::SslServerCredentials(ssl_opts);
builder.AddListeningPort(server_address, creds);
```

```python
# Client with TLS
with open('ca.pem', 'rb') as f:
    ca_cert = f.read()
creds = grpc.ssl_channel_credentials(ca_cert)
channel = grpc.secure_channel('server:50051', creds)
```

### Authentication

```cpp
// Server with token authentication
class AuthInterceptor : public grpc::ServerInterceptor {
    void Intercept(grpc::ServerContext* context) {
        auto metadata = context->client_metadata();
        auto auth_it = metadata.find("authorization");
        if (auth_it == metadata.end() || !ValidateToken(auth_it->second)) {
            context->TryCancel();
        }
    }
};
```

## Debugging

### Enable Logging

```python
# Python client
import logging
logging.basicConfig(level=logging.DEBUG)

# Set gRPC log level
import os
os.environ['GRPC_VERBOSITY'] = 'DEBUG'
os.environ['GRPC_TRACE'] = 'all'
```

```cpp
// C++ server
setenv("GRPC_VERBOSITY", "DEBUG", 1);
setenv("GRPC_TRACE", "all", 1);
```

### Inspect Messages

```python
# Print message in human-readable format
from google.protobuf import text_format
print(text_format.MessageToString(request))
```

### Network Capture

```bash
# Capture gRPC traffic with tcpdump
tcpdump -i lo -s 0 -w capture.pcap port 50051

# Analyze with Wireshark (supports HTTP/2)
wireshark capture.pcap
```

## Versioning and Compatibility

### Adding New Fields (Safe)

```protobuf
message TrialRequest {
    string trial_id = 1;
    repeated Object objects = 2;
    
    // NEW: Added in v2, old clients ignore it
    string experiment_name = 10;
}
```

Old clients: Ignore unknown fields
New servers: Provide default if field missing

### Removing Fields (Safe with Reserved)

```protobuf
message TrialRequest {
    string trial_id = 1;
    // repeated Object objects = 2;  // REMOVED
    reserved 2;  // Prevent reuse
}
```

### Renaming Fields (Safe)

```protobuf
message TrialRequest {
    string trial_id = 1;  // Was: experiment_id (name change OK)
}
```

Field numbers are the contract, not names.

### Changing Types (Usually Unsafe)

```protobuf
// DON'T: Change type
// int32 timeout = 5;
double timeout = 5;  // BREAKS compatibility!
```

## See Also

- [gRPC API](grpc-api.md) - Detailed API documentation
- [Data Formats](data-formats.md) - Message structure reference
- [Franka Trial Server](../core/franka-trial-server.md) - Server implementation