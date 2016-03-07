# SCSI Encryption SPOUT backend

composed by [Stefanos Laskaridis](mailto:steve.laskaridis@cern.ch)

## Aim

In order to be able to pass/clear the encryption parameters to/from the drive, we implement a C++ backend that defines the relevant methods.
This program **should not** be invoked by hand.

## Interface

### Arguments

* `-d <drive>` - The drive towards the SCSI calls are headed.
* `-k <key>` - The key for enabling encryption. Max. 256bit. If less, right padding with zeros.
* `-n` - Disable encryption, clear the encryption parameters from the drive.

### Return codes

|  Return code | Description  |
|   ---        | ---          |
|  **0**       | **Success**  |
|  **-1**      | **Error**    |

## Build

In order to build the backend, follow the instructions below:

1. Navigate to castor-tape-encryption-control/lib
2. Create `bin/` directory:
```
mkdir bin/
cd bin/
```
3. cmake
```
cmake ../
```
4. Build the project
```
make
```

