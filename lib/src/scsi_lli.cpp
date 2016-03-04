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

  This file was originally written by:

  Marco van Wieringen, March 2012 for:

  BAREOS® - Backup Archiving REcovery Open Sourced

  Copyright (C) 2011-2012 Planets Communications B.V.
  Copyright (C) 2013-2013 Bareos GmbH & Co. KG

  @author Stefanos Laskaridis, steve.laskaridis@cern.ch
  ****************************************************************************/

#ifndef SG_INCLUDED
#define SG_INCLUDED 1

#include <cstdlib>
#include <scsi/sg.h>

#endif
#ifndef SCSI_INCLUDED
#define SCSI_INCLUDED 1

#include <scsi/scsi.h>

#endif
#ifndef SCSI_LLI_INCLUDED
#define SCSI_LLI_INCLUDED 1

#include "scsi_lli.hpp"

#endif

#ifndef SCSI_CONSTANTS_INCLUDED
#define SCSI_CONSTANTS_INCLUDED 1

#include "scsi_constants.hpp"

#endif

#ifndef SIOC_REQSENSE
#define SIOC_REQSENSE _IOR ('C',0x02,SCSI_PAGE_SENSE)
#endif

#include <sys/fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <cstring>
#include <unistd.h>

/*
 * Core interface function to lowlevel SCSI interface.
 */
static inline bool do_scsi_cmd_page(int fd, const char *device_name,
                                    void *cdb, unsigned int cdb_len,
                                    void *cmd_page, unsigned int cmd_page_len,
                                    int direction) {
    int rc;
    sg_io_hdr_t io_hdr;
    SCSI_PAGE_SENSE sense;
    bool opened_device = false;
    bool retval = false;

    /*
     * See if we need to open the device_name or if we got an open filedescriptor.
     */
    if (fd == -1) {
        fd = open(device_name, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            std::cerr << "Failed to open " << device_name << std::endl;
            return false;
        }
        opened_device = true;
    }

    memset(&sense, 0, sizeof(sense));
    memset(&io_hdr, 0, sizeof(io_hdr));
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = cdb_len;
    io_hdr.mx_sb_len = sizeof(sense);
    io_hdr.dxfer_direction = direction;
    io_hdr.dxfer_len = cmd_page_len;
    io_hdr.dxferp = (char *) cmd_page;
    io_hdr.cmdp = (unsigned char *) cdb;
    io_hdr.sbp = (unsigned char *) &sense;

    rc = ioctl(fd, SG_IO, &io_hdr);
    if (rc < 0) {
        std::cerr << "Unable to perform SG_IO ioctl on fd " << fd << std::endl;
        goto bail_out;
    }

    if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
        std::cerr << "Failed with info " << io_hdr.info << ", mask status " << io_hdr.masked_status
        << ", msg status " << io_hdr.msg_status << ", host status driver status " <<
        io_hdr.driver_status << std::endl;
        goto bail_out;
    }

    retval = true;

    bail_out:
    /*
     * See if we opened the device in this function, if so close it.
     */
    if (opened_device) {
        close(fd);
    }

    std::cout << "\n\n" << std::endl;
    std::cout << "Sense data" << std::endl;
    std::cout << "==========" << std::endl;
    std::cout << "Sense code: " << senseKeysText[(int)sense.senseKey] << std::endl;
    std::cout << "Additional sense code: " << additionalSenseCodeText[(int)sense.addSenseCode<<8+(int) sense.addSenseCodeQual].text << std::endl;

    return retval;
}

/*
 * Receive a lowlevel SCSI cmd from a SCSI device using the Linux SG_IO ioctl interface.
 */
bool recv_scsi_cmd_page(int fd, const char *device_name,
                        void *cdb, unsigned int cdb_len,
                        void *cmd_page, unsigned int cmd_page_len) {
    return do_scsi_cmd_page(fd, device_name, cdb, cdb_len, cmd_page, cmd_page_len, SG_DXFER_FROM_DEV);
}

/*
 * Send a lowlevel SCSI cmd to a SCSI device using the Linux SG_IO ioctl interface.
 */
bool send_scsi_cmd_page(int fd, const char *device_name,
                        void *cdb, unsigned int cdb_len,
                        void *cmd_page, unsigned int cmd_page_len) {
    return do_scsi_cmd_page(fd, device_name, cdb, cdb_len, cmd_page, cmd_page_len, SG_DXFER_TO_DEV);
}

/*
 * Check if the SCSI sense is EOD.
 */
bool check_scsi_at_eod(int fd) {
    int rc;
    SCSI_PAGE_SENSE sense;

    memset(&sense, 0, sizeof(sense));

    rc = ioctl(fd, SIOC_REQSENSE, &sense);
    if (rc != 0) {
        return false;
    }

    return sense.senseKey == 0x08 && /* BLANK CHECK. */
           sense.addSenseCode == 0x00 && /* End of data (Combination of asc and ascq)*/
           sense.addSenseCodeQual == 0x05;
}

void print_struct_to_hex(void* struct_ptr, unsigned int struct_size) {
    unsigned char * char_ptr = (unsigned char *) struct_ptr;
    for(int i=0;i<struct_size;++i) {
        printf("%02x ", char_ptr[i]);
    }
    std::cout << std::endl;
}
