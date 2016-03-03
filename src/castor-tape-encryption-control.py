#! /usr/bin/env python

# ******************************************************************************
#
#  This an external script to be used with and from CASTOR project.
#  See http://castor.web.cern.ch/castor
#
#  Copyright (C) 2016  CERN
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Affero General Public License
#  as published by the Free Software Foundation.
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU Affero General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#
#
#  @author Stefanos Laskaridis, steve.laskaridis@cern.ch
# ****************************************************************************/

import argparse
import json
import os


def extract_pool_name(vid):
    """
    Extract the pool name based the vid of the tape.
    It's results are based on the output of `vmgrlisttape` tool.
    :param vid: The VID of the tape whose pool we are searching for.
    :return: The pool name of the given VID or the return code of the `vmgrlisttape`
    command if != 0.
    """
    command = ['vmgrlisttape', '-V', vid]
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pool_cmd_out, pool_cmd_err = proc.communicate()
    if proc.returncode == 0:
        return pool_cmd_out.split()[5]
    else:
        return proc.returncode


def cleanup_and_exit(drive):
    """
    Function clearing the encryption parameters from the drive. Exits the program with code -1
    after completion.
    :param drive: The drive whose encryption parameters are to be cleared.
    """
    print "Reverting drive to non-encrypting state."
    if disable_encryption(drive) == 0:
        exit(-1)
    else:
        print "Disabling encryption failed."
        exit(-1)


def disable_encryption(drive):
    """
    Disable the encryption and clear all encryption parameters from the drive.
    Calls C++ backend in order to communicate with the drive via the SCSI interface.
    :param drive: The drive whose encryption parameters are to be cleared.
    :return: The backend's process return code.
    """
    if drive:
        command = ['../bin/spout_cmd', '-d', drive, '-n']
        proc = subprocess.Popen(command, close_fds=True)
        proc.communicate()
        return proc.returncode


def enable_encryption(key, drive):
    """
    Enable the encryption and pass encryption parameters to the drive.
    Calls C++ backend in order to communicate with the drive via the SCSI interface.
    :param key: The key with which the drive will be encrypting/decrypting data.
    :param drive: The drive whose encryption parameters are to be cleared.
    :return: The backend's process return code.
    """
    if key:
        command = ['../bin/spout_cmd', '-d', drive, '-k', key]
        proc = subprocess.Popen(command, close_fds=True)
        proc.communicate()


# Entry point of execution.
if __name__ == '__main__':

    # Parse CLI arguments. Adheres to the specification under /docs/interface_specification.md
    parser = argparse.ArgumentParser()
    action_group = parser.add_mutually_exclusive_group(required=True)
    action_group.add_argument('--enable', dest='enable', action='store_true',
                              help="Enable the encryption for the target device.")
    action_group.add_argument('--disable', dest='disable', action='store_true',
                              help="Clear any encryption parameters from the target device.")
    parser.add_argument('-D', '--device', dest='drive', required=True,
                        help="The drive target for controlling its encryption state")
    parser.add_argument('-V', '--vid', dest='vid',
                              help="The ID of the volume we wish to perform"
                                   "some encryption operation. Needed to "
                                   "determine and retrieve the relevant key.")
    args = parser.parse_args()
    if args.enable and args.vid is None:
        parser.error("Action '--enable' requires passing a VID.")

    if not os.path.exists(args.drive):
        exit(-1)

    import subprocess

    # Enable encryption
    if args.enable:
        print "Turning encryption on in drive."

        # Get pool of the tape
        pool_name = extract_pool_name(args.vid)
        if type(pool_name) == int:
            print "Could not determine the pool to which this VID belongs."gi
            exit(-1)

        # Get encryption key from file
        key = ""
        key_filename = '/etc/castor/encryption/keys/tape-encryption-keys.json'
        if os.path.exists(key_filename):
            with open(key_filename) as f:
                try:
                    key = json.load(f)[pool_name]
                except ValueError:
                    print ' '.join([key_filename, 'is not a valid JSON file.'])
                    exit(-1)
                except KeyError:
                    print ' '.join(['Pool name', pool_name,
                                    'is not available in the JSON key store.'])
                    exit(-1)
                except:
                    print ' '.join(['Unknown error while reading the file', key_filename])
                    exit(-1)
        else:
            print ' '.join(['File', key_filename, 'does not exist.'])
            exit(-1)

        if enable_encryption(key, args.drive) == 0:
            print 'Encryption enabled. ' \
                  'Using key from file {filename}.'.format(filename=key_filename)
        else:
            print 'Enabling encryption failed.'
            cleanup_and_exit(args.drive)

    # Disable encryption
    if args.disable:
        if disable_encryption(args.drive) == 0:
            print 'Encryption disabled. Cleared encryption parameters from tape.'
        else:
            print 'Disabling encryption failed.'
