# Encryption Script

composed by [Stefanos Laskaridis](mailto:steve.laskaridis@cern.ch)

## Aim

The aim of this script is to enable/disable encryption while reading/writing to some tape.

As setting encryption on/off is only altering the state of the drive, this script can be called by other Hierchical Storage Management Systems in order enable/disable the encryption capability.

## Components

* **castor-tape-encryption-control.py**: Scripting frontend that provides the interface towards the outside world.
* **lib/spout_cmd.{h,c}pp**: C++ backend implementing the SCSI inteface to the device and making the ioctl call.

## Prerequisites

**IMPORTANT NOTE**: This script is implementing AME (Application Managed Encryption). Before running, you **must** enable the encryption capability for the specified drive from the vendor's inteface.


## Related Links

| Title | URL |
| --- | --- |
| Jira Ticket | [https://its.cern.ch/jira/browse/CASTOR-5350](https://its.cern.ch/jira/browse/CASTOR-5350) |
| Twiki page | [https://twiki.cern.ch/twiki/bin/view/DSSGroup/EncryptionProject](https://twiki.cern.ch/twiki/bin/view/DSSGroup/EncryptionProject) |
