/******************************************************************************

  File to be used from castor-tape-encryption-control script, in turn
  used by the CASTOR project.
  See http://castor.web.cern.ch/castor

  Copyright (C) 2016  CERN
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Affero General Public License
  as published by the Free Software Foundation.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU Affero General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.



  @author Stefanos Laskaridis, steve.laskaridis@cern.ch
  ****************************************************************************/

#include "scsi_lli.hpp"
#include "spout_cmd.hpp"

#include <cstring>
#include <iostream>
#include <string>

void parse_args(const int argc, char const* const* argv,
                std::string& key, bool &encryption_flag,
                bool &disable_encryption, std::string &drive);

int main(int argc, char* argv[]) {

    std::string key = "";
    bool encryption_flag = false;
    bool disable_encryption = false;
    std::string drive = "";
    bool res = 0;

    /*
     * Parse cli args
     */
    parse_args(argc, argv, key, encryption_flag, disable_encryption, drive);

    const char *encryption_key = key.c_str();

    // Enabling encryption
    if (!key.empty()) {
        std::cout << "Trying to encrypt." << std::endl;
        res = set_scsi_encryption_key(-1, drive.c_str(), encryption_key);
    }

    // disable encryption
    if (disable_encryption) {
        std::cout << "Trying to decrypt." << std::endl;
        res = clear_scsi_encryption_key(-1, drive.c_str());
    }

    if(res)
        return 0;
    else
        return 1;
}
/**
 * Parses CLI arguments.
 * If compulsory arguments are missing, prints relevant message and exits the program with
 * return code -1.
 */
void parse_args(const int argc, char const* const* argv,
                std::string& key, bool &encryption_flag,
                bool &disable_encryption, std::string &drive) {

    for(int i=0;i<argc;++i) {
        if (strcmp(argv[i],"-d") == 0) {
            drive = argv[++i];
        }
        else if (strcmp(argv[i], "-n") == 0 && !encryption_flag) {
            disable_encryption = true;
            encryption_flag = true;
        }
        else if (strcmp(argv[i], "-k") == 0 && !encryption_flag) {
            key = argv[++i];
            encryption_flag = true;
        }
    }

    if (drive == "") {
        std::cout << "Argument -d (drive path) is required." << std::endl;
        exit(-1);
    }
}