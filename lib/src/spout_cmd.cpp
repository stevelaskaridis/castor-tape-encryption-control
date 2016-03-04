/******************************************************************************

  This a file to be used from castor-tape-encryption-control script, in turn
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

  This file was originally written by:

  Marco van Wieringen, March 2012 for:

  BAREOS® - Backup Archiving REcovery Open Sourced

  Copyright (C) 2011-2012 Planets Communications B.V.
  Copyright (C) 2013-2013 Bareos GmbH & Co. KG

  @author Stefanos Laskaridis, steve.laskaridis@cern.ch
  ****************************************************************************/

#include <cstring>
#include <iostream>
#include "scsi_lli.hpp"
#include "spout_cmd.hpp"

/*
 * Store a value as 2 bytes MSB/LSB
 */
static inline void set_2_byte_value(unsigned char *field, int value)
{
   field[0] = (unsigned char)((value & 0xff00) >> 8);
   field[1] = (unsigned char)(value & 0x00ff);
}

/*
 * Store a value as 4 bytes MSB/LSB
 */
static inline void set_4_byte_value(unsigned char *field, int value)
{
   field[0] = (unsigned char)((value & 0xff000000) >> 24);
   field[1] = (unsigned char)((value & 0x00ff0000) >> 16);
   field[2] = (unsigned char)((value & 0x0000ff00) >> 8);
   field[3] = (unsigned char)(value & 0x000000ff);
}

/*
 * Clear an encryption key used by a tape drive
 * using a lowlevel SCSI interface.
 *
 * The device is send a:
 * - SPOUT Security Protocol OUT SCSI CDB. (0xB5)
 * - SPOUT Send Encryption Key Page. (0x10)
 *
 * The Send Encryption Key page has the encryption
 * and decryption set to disabled and the key is empty.
 */
bool clear_scsi_encryption_key(int fd, const char *device_name)
{
   /*
    * Create a SPOUT Set Encryption Key CDB and
    *        a SPOUT Clear Encryption Mode Page
    */
   SPP_SCSI_CDB cdb;
   SPP_PAGE_BUFFER cmd_page;
   SPP_PAGE_SDE *sps;
   int cmd_page_len, cdb_len;

   /*
    * Put a SPOUT Set Data Encryption page into the start of
    * the generic cmd_page structure.
    */
   memset(&cmd_page, 0, sizeof(cmd_page));
   sps = (SPP_PAGE_SDE *)&cmd_page;
   set_2_byte_value(sps->pageCode, SPOUT_SET_DATA_ENCRYPTION_PAGE);
   set_2_byte_value(sps->length, sizeof(SPP_PAGE_SDE)-3);
   sps->nexusScope = SPP_NEXUS_SC_LOCAL; // oracle compatibility
   sps->encryptionMode = SPP_ENCR_MODE_DISABLE;
   sps->decryptionMode = SPP_DECR_MODE_DISABLE;
   sps->algorithmIndex = 0x01;
   sps->kadFormat = SPP_KAD_KEY_FORMAT_NORMAL;
   set_2_byte_value(sps->keyLength, SPP_KEY_LENGTH);

   /*
    * Set the length to the size of the SPP_PAGE_SDE we just filled.
    */
   cmd_page_len = sizeof(SPP_PAGE_SDE);

    // Print SPS hex
   std::cout << "SPS: "; print_struct_to_hex((void *)sps, sizeof(SPP_PAGE_SDE));

   /*
    * Set the actual size of the cmd_page - 4 into the cmd_page length field
    * (without the pageCode and pageLength)
    */
   set_2_byte_value(cmd_page.length, cmd_page_len - 4);

   /*
    * Fill the SCSI CDB.
    */
   cdb_len = sizeof(cdb);
   memset(&cdb, 0, cdb_len);
   cdb.opcode = SCSI_SPOUT_OPCODE;
   cdb.scp = SPP_SP_PROTOCOL_TDE;
   set_2_byte_value(cdb.scp_specific, SPOUT_SET_DATA_ENCRYPTION_PAGE);
   set_4_byte_value(cdb.allocation_length, cmd_page_len);

    // Print CDB hex
   std::cout << "CDB: "; print_struct_to_hex((void *)&cdb, sizeof(SPP_SCSI_CDB));

   /*
    * Clear the encryption key.
    */
   return send_scsi_cmd_page(fd, device_name,
                            (void *)&cdb, cdb_len,
                            (void *)&cmd_page, cmd_page_len);
}

/*
 * Set an encryption key used by a tape drive
 * using a lowlevel SCSI interface.
 *
 * The device is send a:
 * - SPOUT Security Protocol OUT SCSI CDB. (0xB5)
 * - SPOUT Send Encryption Key Page. (0x10)
 *
 * The Send Encryption Key page has the encryption
 * and decryption set to not disabled and the key is filled.
 */
bool set_scsi_encryption_key(int fd, const char *device_name, const char *encryption_key)
{
   /*
    * Create a SPOUT Set Encryption Key CDB and
    *        a SPOUT Send Encryption Key Page
    */
   SPP_SCSI_CDB cdb;
   SPP_PAGE_BUFFER cmd_page;
   SPP_PAGE_SDE *sps;
   int cmd_page_len, cdb_len;

   /*
    * Put a SPOUT Set Data Encryption page into the start of
    * the generic cmd_page structure.
    */
   memset(&cmd_page, 0, sizeof(cmd_page));
   sps = (SPP_PAGE_SDE *)&cmd_page;
   set_2_byte_value(sps->pageCode, SPOUT_SET_DATA_ENCRYPTION_PAGE);
   set_2_byte_value(sps->length, sizeof(SPP_PAGE_SDE)-3);
   //   sps->nexusScope = SPP_NEXUS_SC_ALL_I_T_NEXUS; //ibm
   sps->nexusScope = SPP_NEXUS_SC_LOCAL; //oracle
   sps->encryptionMode = SPP_ENCR_MODE_ENCRYPT;
   sps->decryptionMode = SPP_DECR_MODE_MIXED;
   sps->algorithmIndex = 0x01;
   sps->kadFormat = SPP_KAD_KEY_FORMAT_NORMAL;
   set_2_byte_value(sps->keyLength, SPP_KEY_LENGTH);
   strncpy((char *)sps->keyData, encryption_key, SPP_KEY_LENGTH);

    // Print SPS hex
   std::cout << "SPS: "; print_struct_to_hex((void *)sps, sizeof(SPP_PAGE_SDE));

   /*
    * Set the length to the size of the SPP_PAGE_SDE we just filled.
    */
   cmd_page_len = sizeof(SPP_PAGE_SDE);

   /*
    * Set the actual size of the cmd_page - 4 into the cmd_page length field
    * (without the pageCode and pageLength)
    */
   set_2_byte_value(cmd_page.length, cmd_page_len - 4);

   /*
    * Fill the SCSI CDB.
    */
   cdb_len = sizeof(cdb);
   memset(&cdb, 0, cdb_len);
   cdb.opcode = SCSI_SPOUT_OPCODE;
   cdb.scp = SPP_SP_PROTOCOL_TDE;
   set_2_byte_value(cdb.scp_specific, SPOUT_SET_DATA_ENCRYPTION_PAGE);
   set_4_byte_value(cdb.allocation_length, cmd_page_len);

    // Print CDB hex
   std::cout << "CDB: "; print_struct_to_hex((void *)&cdb, sizeof(SPP_SCSI_CDB));

   /*
    * Set the new encryption key.
    */
   return send_scsi_cmd_page(fd, device_name,
                            (void *)&cdb, cdb_len,
                            (void *)&cmd_page, cmd_page_len);
}