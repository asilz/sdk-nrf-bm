/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "platform.h"
#include "storage.h"

#include <nrf_sp_qspi.h>
#include <nrf_sqspi.h>
#include <drivers/nrfx_errors.h>
#include <nrfx_gpiote.h>
#include <stdbool.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/irq.h>
#include <bm/bm_irq.h>
#include <hal/nrf_vpr.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_spu.h>
#include <hal/nrf_memconf.h>
#include <softperipheral_meta.h>
#include <softperipheral_regif.h>
#include <hal/nrf_oscillators.h>

/* Example: Mapping of physical drive number for each drive */
#define DEV_FLASH	0	/* Map FTL to physical drive 0 */
#define DEV_MMC		1	/* Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Map USB MSD to physical drive 2 */

#define MX25_CMD_PAGE_PROGRAM 0x02U
#define MX25_CMD_READ         0x03U
#define MX25_CMD_RDSR         0x05U
#define MX25_CMD_WREN         0x06U
#define MX25_CMD_SECTOR_ERASE 0x20U

#define MX25_SECTOR_COUNT 2048
#define MX25_SECTOR_SIZE 4096
#define MX25_BLOCK_SIZE 8

#define MX25_CMD_HEADER(opcode, addr)								   \
	(opcode),										   \
	(uint8_t)((addr) >> 16),								   \
	(uint8_t)((addr) >> 8),									   \
	(uint8_t)(addr)


ISR_DIRECT_DECLARE(sqspi_direct_isr)
{
	nrf_sqspi_irq_handler();
	return 0;
}

static nrf_sqspi_t qspi = {.p_reg = (void *)DT_REG_ADDR(DT_NODELABEL(flpr_vri_ram)), .drv_inst_idx = 0};

static void cb(nrf_sqspi_t const *p_qspi, nrf_sqspi_evt_t *p_event, void *p_context)
{
	if(p_event->type == NRF_SQSPI_EVT_XFER_DONE){
	        if(p_event->data.xfer_done != NRF_SQSPI_RESULT_OK){
	            LOG_ERR("xfer done error, err = %d", p_event->data.xfer_done);
	        }
	}
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	DSTATUS ret = 0;

	if (pdrv != MX25_FLASH){
		return RES_PARERR;
	}

	if(!nrf_sqspi_init_check(&qspi)){
		ret |= STA_NOINIT;
	}
	return ret;
}



/*-----------------------------------------------------------------------*/
/* Initialise a drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number to identify the drive */
)
{
	nrfx_err_t err;
	DSTATUS ret = 0;

	if (pdrv != MX25_FLASH){
		return RES_PARERR;
	}

    	M_IRQ_DIRECT_CONNECT(SP_VPR_IRQn, 4, sqspi_direct_isr, 0);

    	nrf_sqspi_cfg_t cfg = {
        	.skip_gpio_cfg = false,
        	.skip_pmux_cfg = false,
        	.pins = {
        	    .sck = NRF_PIN_PORT_TO_PIN_NUMBER(1, 2),
        	    .strobe = NRF_SQSPI_PINS_UNUSED,
        	    .io = {
        	        NRF_PIN_PORT_TO_PIN_NUMBER(2, 2),
        	        NRF_PIN_PORT_TO_PIN_NUMBER(4, 2),
        	        NRF_PIN_PORT_TO_PIN_NUMBER(3, 2),
        	        NRF_PIN_PORT_TO_PIN_NUMBER(0, 2),
        	    }
        	}
    	};

    	nrf_spu_periph_perm_secattr_set(NRF_SPU00, nrf_address_slave_get((uint32_t)NRF_MEMCONF), true);

    	nrf_sqspi_dev_cfg_t qspi_dev_config = {.csn_pin = NRF_PIN_PORT_TO_PIN_NUMBER(5, 2),
    	    					.sck_freq_khz = 1000,
						.spi_cpolpha = NRF_SQSPI_SPI_CPOLPHA_0,
						.mspi_lines = NRF_SQSPI_SPI_LINES_QUAD_1_1_4,
						.mspi_ddr = NRF_SQSPI_SPI_DDR_SINGLE,
						.spi_clk_stretch = false};
    	NRF_SPU00_S->PERIPH[0xC].PERM = (SPU_PERIPH_PERM_SECATTR_Secure << SPU_PERIPH_PERM_SECATTR_Pos);

    	err = nrf_sqspi_init(&qspi, &cfg);
    	if (err != NRFX_SUCCESS){
    	    return err;
    	}

    	nrf_sqspi_data_fmt_t sqspi_data_fmt = {
			.cmd_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
			.addr_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
			.data_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
			.data_bit_reorder_unit = 8,
			.data_container = 32,
			.data_swap_unit = 8,
			.data_padding = NRF_SQSPI_DATA_FMT_PAD_RAW,
		};
    	err = nrf_sqspi_dev_data_fmt_set(&qspi, &sqspi_data_fmt);
    	if (err != NRFX_SUCCESS) {
			LOG_ERR("nrf_sqspi_dev_data_fmt_set() failed: %08x", err);
		}

    	if (!nrf_sqspi_init_check(&qspi)){
    	    LOG_WRN("sqspi not inited");
    	}


    	err = nrf_sqspi_dev_cfg(&qspi, &qspi_dev_config, cb, NULL);
    	if(err != NRFX_SUCCESS){
    	    LOG_ERR("nrf_sqspi_dev_cfg failed, err = %d", err);
    	}

    	if (cfg.skip_gpio_cfg == false) { //This overrides the driver's default configuration
    	    // Set drive strength E0E1, as it is non-standard.
    	    nrf_gpio_cfg(qspi_dev_config.csn_pin, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
    	                 NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    nrf_gpio_cfg(cfg.pins.sck, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
    	                 NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    for (int i = 0; i < 4; i++) {
    	        nrf_gpio_cfg(cfg.pins.io[i], NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT,
    	                     NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    }
    	}
    	if (cfg.skip_pmux_cfg == false) { //This overrides the driver's default configuration
    	    // Set pin source as FLPR.
    	    nrf_gpio_pin_control_select(cfg.pins.sck, NRF_GPIO_PIN_SEL_VPR);
    	    nrf_gpio_pin_control_select(qspi_dev_config.csn_pin, NRF_GPIO_PIN_SEL_VPR);
    	    for (int i = 0; i < NRF_SQSPI_MAX_NUM_DATA_LINES; i++) {
    	        nrf_gpio_pin_control_select(cfg.pins.io[i], NRF_GPIO_PIN_SEL_VPR);
    	    }
    	}

    	err = nrf_sqspi_activate(&qspi);
    	if(err != NRFX_SUCCESS){
    	    LOG_ERR("nrf_sqspi_activate failed, err = %d", err);
    	}

		return ret;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nunber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,		/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if (pdrv != MX25_FLASH){
		return RES_PARERR;
	}

	static const nrf_sqspi_xfer_t xfer = {.dir = NRF_SQSPI_XFER_DIR_TXRX, .cmd = MX25_CMD_READ, .address = MX25_SECTOR_SIZE * sector, .p_data = buff, .data_length = MX25_SECTOR_SIZE * count, .cmd_length = 8, .addr_length = 24, .dummy_length = 0};

	nrfx_err_t err = nrf_sqspi_xfer(&qspi, &xfer, 1, 0);
    	if(err != NRFX_SUCCESS){
    	    return RES_NOTRDY;
    	}


	return RET_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count		/* Number of sectors to write */
)
{
	if (pdrv != MX25_FLASH){
		return RES_PARERR;
	}
	for (UINT i = 0; i < count; i++){
		uint32_t sector_addr = (sector + i) * MX25_SECTOR_SIZE;
		nrf_sqspi_xfer_t erase = {.dir = NRF_SQSPI_XFER_DIR_TX,
					.cmd = MX25_CMD_SECTOR_ERASE, .cmd_length = 8,
					.address = sector_addr, .addr_length = 24};
		nrfx_err_t err = nrf_sqspi_xfer(&qspi, &erase, 1, 0);

		if(err != NRFX_SUCCESS){
        		return RES_NOTRDY;
    		}
		nrf_sqspi_xfer_t write = {.dir = NRF_SQSPI_XFER_DIR_TX,
					.cmd = MX25_CMD_PAGE_PROGRAM, .cmd_length = 8,
					.address = sector_addr, .addr_length = 24,
					.p_data = buff, .data_length = sector * count};

		err = nrf_sqspi_xfer(&qspi, &write, 1, 0);
		if(err != NRFX_SUCCESS){
        		return RES_NOTRDY;
    		}

	}
	return RET_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (cmd) {
	case CTRL_SYNC :
		break;
	case GET_SECTOR_COUNT :
		*((LBA_t *)buff) = MX25_SECTOR_COUNT;
		break;
	case GET_SECTOR_SIZE :
		*((WORD *)buff) = MX25_SECTOR_SIZE;
		break;
	case GET_BLOCK_SIZE :
		*((DWORD *)buff) = MX25_BLOCK_SIZE;
		break;
	case CTRL_TRIM :
		break;
	default:
		break;
	}

	return RET_OK;
}
