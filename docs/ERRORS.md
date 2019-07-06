# List of error codes and their descriptions
## Handler
| Error code | Description |
| ---------- | ----------- |
| 0          | Syntax error |
| 1          | Bad X-Auth-Key |
| 2          | Bad URL |
| 122        | Bad action |
| 123        | Unknown action |

## libvirtd communication
| Error code | Description |
| ---------- | ----------- |
| 10         | Failed to open connection to LibVirtD |
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
| 250        | Error occurred while getting networks |
| 298        | PATCH data has to be an array of actions |
| 299        | Invalid domain state value |