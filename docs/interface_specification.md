# Encryption Script Specification

created by [Stefanos Laskaridis](steve.laskaridis@cern.ch),  
defined by [Vladimír Bahyl](Vladimir.Bahyl@cern.ch) and [Eric Cano](Eric.Cano@cern.ch)


## Input 

### Script arguments interface

The command line arguments of the script should be the following:

* `--vid/-V <VID>` - **compulsory on encryption**
* `--device/-D  </dev/sgX>` - **compulsory**
* `--enable || --disable` - **compulsory**
* `--help` - **optional**

### Key format specification

The encryption key is defined by the pool that the tape belongs to and is located in the file:
**/etc/castor/encryption/tape-encryption-keys.json**

This is a JSON file with the following layout:

```JSON
{
    "<pool_name_v0>": "encryption_key_0",
    ...
    "<pool_name_vn>": "encryption_key_n",
}
```

## Output

## Logging interface

The output of the script is in JSON format, provided the correct arguments are passed:

```JSON
{
    "message": "<log_line>",
    "response": {
        "code": <return_code>,
        "description": "<short_description_of_return_code>"
    }
}
```

## Return codes

The program utilizes the following return codes:

|  Return code | Description                                                 |
|   ---        | ---                                                         |
|  **0**       | **Success**                                                 |
|  **-1**      | **Error**: Cleared encryption parameters from drive         |
|  **1**       | **Error**: Could not clear encryption parameters from drive |
|  **2**       | **Error**: Invalid arguments passed                         |

