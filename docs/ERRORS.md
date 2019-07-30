# List of error codes and their descriptions
## Handler
| Error code | Description |
| ---------- | ----------- |
| 0          | Syntax error |
| 1          | Bad X-Auth-Key |
| 2          | Bad URL |
| 122        | Bad action |
| 123        | Unknown action |

## LibVirtD communication
### Connection 
| Error code | Description |
| ---------- | ----------- |
| 10         | Failed to open connection to LibVirtD |

### Domains
| Error code | Description |
| ---------- | ----------- |
| 100        | Cannot find VM with a such name |
| 101        | Cannot find VM with a such UUID |
| 102        | No UUID specified |
| 103        | No name specified |
| 200        | Could not shut down the VM |
| 201        | Domain is not running |
| 202        | Could not start the VM |
| 203        | Domain is already running |
| 204        | No status actions specified |
| 205        | Renaming failed |
| 206        | Setting available memory failed |
| 207        | Setting maximum available memory failed |
| 208        | Setting autostart policy failed |
| 298        | PATCH data has to be an array of actions |
| 299        | Invalid domain state value |

# Networks

| Error code | Description |
| ---------- | ----------- |
| 500        | Error occurred while getting network status |
| 501        | Cannot find network with a such name |
| 502        | Cannot find network with a such UUID |
| 503        | Error occurred while getting network autostart |