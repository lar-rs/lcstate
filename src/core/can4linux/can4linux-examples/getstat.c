/*
 * getstat.c implements showCANStat(fd) 
 *
 * Copyright (c) 2004 port GmbH Halle (Saale)
 * Copyright (c) 2013 Heinz-Jürgen Oertel <hj.oertel@t-online.de>
 *------------------------------------------------------------------
 * $Header: /z2/cvsroot/library/co_lib/drivers/shar_src/can_can4linux.c,v 2.4 2004/04/23 12:52:25 oe Exp $
 *
 *------------------------------------------------------------------
 *
 */

void		showCANStat(int fd);

/*******************************************************************/
/**
*++ \brief showCANStat - show CAN status values using printf()
*-- \brief showCANStat - zeigt CAN Status Werte mit printf()
*
* \retval
*++ nothing
*-- nichts
*
*/
void showCANStat(int fd)
{
can_statuspar_t status;
char *m;

    ioctl(fd, CAN_IOCTL_STATUS, &status);
    switch(status.type) {
        case  CAN_TYPE_SJA1000:
            m = "NXP sja1000";
            break;
        case  CAN_TYPE_FlexCAN:
            m = "Freescale FlexCan";
            break;
        case  CAN_TYPE_TouCAN:
            m = "Freescale TouCAN";
            break;
        case  CAN_TYPE_82527:
            m = "I82527";
            break;
        case  CAN_TYPE_TwinCAN:
            m = "Infineon TwinCAN";
            break;
        case  CAN_TYPE_BlackFinCAN:
            m = "AD BlackFinCAN 537";
            break;
        case  CAN_TYPE_AT91SAM9263:
            m = "ATMEL AT91SAM9263";
            break;
	case  CAN_TYPE_MCP2515:
            m = "Microchip MCP2515 (SPI)";
            break;
	case CAN_TYPE_XCANPS:
            m = "XILINX XCANPS";
            break;
	case CAN_TYPE_DCAN:
            m = "Bosch D_CAN";
            break;
	case CAN_TYPE_IFI_CAN_FD:
            m = "IFI CAN FD";
            break;
	case CAN_TYPE_ALLWINNER_CAN:
            m = "Allwinner CAN";
            break;
	case  CAN_TYPE_VIRTUAL:
            m = "Virtual CAN net";
            break;
    case CAN_TYPE_UNSPEC:
    default:
            m = "unknown";
            break;
    }

    printf(":: %s %4d %2d %2d %2d %2d %2d tx:%3d/%3d: rx:%3d/%3d:\n",
        m,
        status.baud,
        status.status,
        status.error_warning_limit,
        status.rx_errors,
        status.tx_errors,
        status.error_code,
        /* */
        status.tx_buffer_size,
        status.tx_buffer_used,
        status.rx_buffer_size,
        status.rx_buffer_used
        );
}
