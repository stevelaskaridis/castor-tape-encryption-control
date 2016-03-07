# Encryption Script

composed by [Stefanos Laskaridis](mailto:steve.laskaridis@cern.ch)

## Aim

The aim of this script is to enable/disable encryption while reading/writing to some tape.

As setting encryption on/off is only altering the state of the drive, this script can be called by other Hierachical Storage Management Systems in order enable/disable the encryption capability.

## Components

* **castor-tape-encryption-control.py**: Scripting frontend that provides the interface towards the outside world.
* **lib/{spout_cmd.cpp,include/spout_cmd.hpp}**: C++ backend implementing the SCSI encryption interface to the device.
* **lib/{scsi_lli.cpp,include/scsi_lli.hpp}**: C++ backend implementing underlying SCSI communication with the device and making the ioctl calls.
* **lib/include/scsi_constants.hpp**: Constants used throughout the C++ backend.
* **lib/main.cpp**: C++ backend entry point of execution.
* **bin/spout_cmd.cpp**: Executable created from C++ backend. Called by the python script. Expected to be found after compilation.

## Prerequisites

**IMPORTANT NOTE**: This script is implementing AME (Application Managed Encryption). Before running, you **must** enable the encryption capability for the specified drive from the vendor's interface.

## Related Links

| Title | URL |
| --- | --- |
| Jira Ticket | [https://its.cern.ch/jira/browse/CASTOR-5350](https://its.cern.ch/jira/browse/CASTOR-5350) |
| Twiki page | [https://twiki.cern.ch/twiki/bin/view/DSSGroup/EncryptionProject](https://twiki.cern.ch/twiki/bin/view/DSSGroup/EncryptionProject) |
