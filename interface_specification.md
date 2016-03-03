# Encryption Script Specification

created by [Stefanos Laskaridis](steve.laskaridis@cern.ch),  
defined by [Vladimír Bahyl](Vladimir.Bahyl@cern.ch) and [Eric Cano](Eric.Cano@cern.ch)


## Input - Script arguments inteface

The command line arguments of the script should be the following:

* --vid/-V <VID> - **compulsory**
* --drivetype/-D <drive_name> - **compulsory**
* --enable || --disable - **compulsory**
* --help - **optional**


## Output - Logging interface

The output of the script should adhere to the syslog's layout, and log the following:

Encryption on (--enable option):
  * Date
  * VID
  * Drive Name
  * Pool Name
  * Message: "Encryption enabled. Using key from file <file path>."

Encryption off (--disable option):
  * Date
  * VID
  * Drive Name
  * Pool Name
  * Message: "Encryption disabled. Cleared encryption parameters from tape."
