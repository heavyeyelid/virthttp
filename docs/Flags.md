# Domains
## Power management flags

| Action	| Flags	|
| ------	| -----	|
| destroy	| _graceful_	|
| reboot	| _see shutdown flags_	|
| reset	| _N/A_	|
| resume	| _N/A_	|
| shutdown	| _acpi\_power\_btn, guest_agent, initctl, signal, paravirt_	|
| start	| _paused, autodestroy, bypass\_cache, force_boot, validate_	|
| suspend	| _N/A_	|

These flags can be a _string_, or an _array of strings_ to combine flags.

N.B. : If wrong flag specified, API will return an "Invalid flag" error.