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

JSON_FILENAME='/etc/castor/encryption/keys/tape-encryption-keys.json'
BACKEND_BINARY_PATH = '../bin/spout_cmd'

def extract_pool_name(vid):
    """
    Extract the pool name based the vid of the tape.
    It's results are based on the output of `vmgrlisttape` tool.
    :param vid: The VID of the tape whose pool we are searching for.
    :return: The pool name of the given VID or the return code of the `vmgrlisttape`
    command if != 0. Otherwise the return value of the command.
    """
    command = ['vmgrlisttape', '-V', vid]
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pool_cmd_out, pool_cmd_err = proc.communicate()
    if proc.returncode == 0:
        return pool_cmd_out.split()[5].strip()
    else:
        return proc.returncode

def extract_key_id_from_tag(vid):
    """
    Function extracting the key_id from the tag of the given VID from the VMGR database.
    :param vid: The VID of the tape whose tag is to be returned.
    :return: The key_id that is stored in the tag column of the VMGR database if command !=0.
    Otherwise the return value of the command.
    """
    command = ['vmgrgettag', '-V', vid]
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    cmd_out, cmd_err = proc.communicate()
    if proc.returncode == 0:
        return str(cmd_out).strip()
    # If tag is empty, return empty string
    elif cmd_err.strip() == "{vid}: No such file or directory".format(vid=vid):
        return ""
    else:
        return proc.returncode

def update_key_id_to_tag(key_id, vid):
    """
    Function updating the key_id to the tag of a specific vid to the VMGR database .
    :param key_id: The key_id whose value will be copied to the tag field.
    :param vid: The VID of the tape whose tag will be updated.
    :return:
    """
    if key_id:
        command = ['vmgrsettag', '--tag', key_id, '-V', vid]
    else:
        command = ['vmgrdeltag', '-V', vid]
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    cmd_out, cmd_err = proc.communicate()
    if proc.returncode == 0:
        return str(cmd_out)
    else:
        return proc.returncode


def get_json_hash(filename=JSON_FILENAME):
    """
    Load JSON hash into memory.
    :param filename: The filename of the json file in the disk.
    :return: The JSON hash loaded, else None.
    """
    json_hash = None
    if os.path.exists(filename):
        with open(filename) as f:
            try:
                json_hash = json.load(f)
            except ValueError:
                print ' '.join([filename, 'is not a valid JSON file.'])
            except KeyError:
                print ' '.join(['Pool name', pool_name,
                                'is not available in the JSON key store.'])
            except:
                print ' '.join(['Unknown error while reading the file', filename])
    else:
        print ' '.join(['File', filename, 'does not exist.'])
    return json_hash

def cleanup_and_exit(drive):
    """
    Function clearing the encryption parameters from the drive. Exits the program with code -1
    after completion.
    :param drive: The drive whose encryption parameters are to be cleared.
    """
    print "Reverting drive to non-encrypting state."
    res = disable_encryption(drive)
    if res == 0:
        exit(-1)
    else:
        print "Disabling encryption failed."
        if type(res) == str:
            print res
        exit(1)


def disable_encryption(drive, backend_path=BACKEND_BINARY_PATH):
    """
    Disable the encryption and clear all encryption parameters from the drive.
    Calls C++ backend in order to communicate with the drive via the SCSI interface.
    :param drive: The drive whose encryption parameters are to be cleared.
    :return: The backend's process return code.
    """
    if drive:
        command = [backend_path, '-d', drive, '-n']
        try:
            proc = subprocess.Popen(command, close_fds=True,
                                    stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            cmd_out, cmd_err = proc.communicate()
            return proc.returncode
        except OSError as e:
            return "{message}: {filename}".format(message=str(e), filename=os.path.realpath(backend_path))



def enable_encryption(key, drive, backend_path=BACKEND_BINARY_PATH):
    """
    Enable the encryption and pass encryption parameters to the drive.
    Calls C++ backend in order to communicate with the drive via the SCSI interface.
    :param key: The key with which the drive will be encrypting/decrypting data.
    :param drive: The drive whose encryption parameters are to be cleared.
    :return: The backend's process return code if there is a key, None otherwise.
    """
    if key:
        command = [backend_path, '-d', drive, '-k', key]
        try:
            proc = subprocess.Popen(command, close_fds=True,
                                    stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            cmd_out, cmd_err = proc.communicate()
            return proc.returncode
        except OSError as e:
            return "{message}: {filename}".format(message=str(e), filename=os.path.realpath(backend_path))
    else:
        return None


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
        key = None

        # Get tag value for the tape
        key_id = extract_key_id_from_tag(args.vid)
        if type(key_id) == int:
            print "Could not acquire the key id of the tape."
            cleanup_and_exit(args.drive)

        # Get json hash
        json_hash = get_json_hash()
        if not json_hash:
            cleanup_and_exit(args.drive)

        if key_id:
            # Search for the key_id in the JSON file
            try:
                key = json_hash[key_id]
            except KeyError:
                print "Could not find the key with key_id={key_id} " \
                      "in {filename}.".format(key_id=key_id, filename=JSON_FILENAME)
                cleanup_and_exit(args.drive)
        else:
            # Get pool of the tape
            pool_name = extract_pool_name(args.vid)
            if type(pool_name) == int:
                print "Could not determine the pool to which this VID belongs."
                cleanup_and_exit(args.drive)
            # Search for the latest pool key in the JSON file
            pool_keys_list = sorted(filter(lambda x: x.startswith(pool_name), json_hash.keys()))
            if pool_keys_list:
                key_id = pool_keys_list[-1]
                key = json_hash[key_id]
                # update the key_id to the volume's tag
                update_key_id_to_tag(key_id, args.vid)
            else:
                pass # do not encrypt

        res = enable_encryption(key, args.drive)
        if res == 0:
            print 'Encryption enabled. ' \
                  'Using key with id={id}.'.format(id=key_id)
            exit(0)
        elif res is None:
            print 'Encryption not enabled.'
            exit(0)
        else:
            print 'Enabling encryption failed.'
            if type(res) == str:
                print res
            cleanup_and_exit(args.drive)

    # Disable encryption
    if args.disable:
        res = disable_encryption(args.drive)
        if res == 0:
            print 'Encryption disabled. Cleared encryption parameters from tape.'
            exit(0)
        else:
            print 'Disabling encryption failed.'
            if type(res) == str:
                print res
            exit(1)
