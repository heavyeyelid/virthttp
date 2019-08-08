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
| 100        | Cannot find domain with a such name |
| 101        | Cannot find domain with a such UUID |
| 102        | No UUID specified |
| 103        | No name specified |
| 200        | Could not shut down the domain |
| 201        | Domain is not running |
| 202        | Could not start the domain |
| 203        | Domain is already active |
| 204        | No status actions specified |
| 205        | Renaming failed |
| 206        | Setting available memory failed |
| 207        | Setting maximum available memory failed |
| 208        | Setting autostart policy failed |
| 209        | Could not destroy the domain |
| 210        | Domain is not active |
| 211        | Domain is not suspended |
| 212        | Cannot resume the domain |
| 213        | Cannot reboot the domain |
| 214        | Cannot reset the domain |
| 215        | Cannot suspend the domain |
| 298        | PATCH data has to be an array of actions |
| 300        | Invalid power management value |
| 301        | Invalid flag |

# Networks

| Error code | Description |
| ---------- | ----------- |
| 500        | Error occurred while getting network status |
| 501        | Cannot find network with a such name |
| 502        | Cannot find network with a such UUID |
| 503        | Error occurred while getting network autostart |