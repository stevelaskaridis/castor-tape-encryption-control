# Encryption Script Specification

created by [Stefanos Laskaridis](steve.laskaridis@cern.ch),  
defined by [Vladimír Bahyl](Vladimir.Bahyl@cern.ch)


## Input - Script arguments inteface

The command line arguments of the script should be the following:

* --vid/-V <VID> - **compulsory**
* --drive/-D <drive_name> - **compulsory**
* --encryption/-E <on|off> - **compulsory**
* --help - **optional**

## Output - Logging interface

The output of the script should adhere to the syslog's layout, and log the following:

Encryption on:
  * Date
  * VID
  * Drive Name
  * Pool Name
  * Message: "Encryption enabled. Using key from file <file path>."

Encryption off:
  * Date
  * VID
  * Drive Name
  * Pool Name
  * Message: "Encryption disabled. Cleared encryption parameters from tape."
