/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/atomic.h>

#include <board-config.h>
#include <bm/bm_buttons.h>
#include <bm/bm_irq.h>
#include <bm/bm_spi_mngr.h>
#include <drivers/nrfx_errors.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_memconf.h>
#include <hal/nrf_spu.h>
#include <hal/nrf_vpr.h>
#include <nrf_sp_qspi.h>
#include <nrf_sqspi.h>
#include <nrfx_gpiote.h>
#include <hal/nrf_gpio.h>
#include <nrfx_spim.h>
#include <softperipheral_regif.h>

#define MX25_WIP_BIT 1 << 0
#define MX25_WEL_BIT 1 << 1

LOG_MODULE_REGISTER(sample, CONFIG_SAMPLE_SQSPI_LOG_LEVEL);

/* MX25R6435F opcodes:
 * READ, PAGE PROGRAM and SECTOR ERASE all use the same header: 1 opcode byte + 3 address bytes.
 * WREN (Write Enable) must precede any program or erase.
 * RDSR (Read Status Register). The Write In Progress (WIP) bit tells if an erase or program
 * operation has finished.
 */
#define MX25_CMD_WRSR         	0x01U
#define MX25_CMD_PAGE_PROGRAM 	0x02U
#define MX25_CMD_READ         	0x03U
#define MX25_CMD_RDSR         	0x05U
#define MX25_CMD_WREN         	0x06U
#define MX25_CMD_RDCR         	0x15U
#define MX25_CMD_SECTOR_ERASE 	0x20U
#define MX25_CMD_4PAGE_PROGRAM 	0x38U
#define MX25_CMD_4READ		0xEBU

#define MX25_SECTOR_COUNT 		2048
#define MX25_SECTOR_SIZE 		4096
#define MX25_BLOCK_SIZE 		8
#define MX25_PAGE_SIZE 			256

#define MX25_WIP_BIT 1 << 0
#define MX25_WEL_BIT 1 << 1

#define MX25_CMD_HEADER_LEN   4U

/* Max payload bytes per MX25 Page Program command. */
#define MX25_PAGE_PROGRAM_LEN (255U - MX25_CMD_HEADER_LEN)

/* Status register bit: Write In Progress. Set while an erase/program runs internally. */
#define MX25_SR_WIP           0x01U

static nrf_sqspi_t qspi = {
	.p_reg = (void *)DT_REG_ADDR(DT_NODELABEL(flpr_vri_ram)),
	.drv_inst_idx = 0
};
static volatile uint8_t ready_reg = 0;
volatile bool xfer_done;

ISR_DIRECT_DECLARE(sqspi_direct_isr)
{
	nrf_sqspi_irq_handler();
	return 0;
}

static void xfer_done_cb(nrf_sqspi_t const *p_qspi, nrf_sqspi_evt_t *p_event, void *p_context)
{
	if(p_event->type == NRF_SQSPI_EVT_XFER_DONE){
	        if(p_event->data.xfer_done != NRF_SQSPI_RESULT_OK){
	            LOG_ERR("xfer done error, err = %d", p_event->data.xfer_done);
	        }
		xfer_done = true;
	}
}

static void reconfigure_quad_spi(void){
	nrfx_err_t err;

	static const nrf_sqspi_dev_cfg_t qspi_dev_quad_config = {
		.csn_pin = BOARD_EXTERNAL_MEMORY_SPIM_PIN_CSN,
        	.sck_freq_khz = 1000,
		.spi_cpolpha = NRF_SQSPI_SPI_CPOLPHA_0,
		.mspi_lines = NRF_SQSPI_SPI_LINES_QUAD_1_4_4,
		.mspi_ddr = NRF_SQSPI_SPI_DDR_SINGLE,
		.spi_clk_stretch = false
	};

	err = nrf_sqspi_deactivate(&qspi);
	if(err != NRFX_SUCCESS){
        LOG_ERR("nrf_sqspi_deactivate failed, err = %d", err);
    	}

	static const nrf_sqspi_cfg_t cfg_quad = {
		.skip_gpio_cfg = false,
		.skip_pmux_cfg = false,
		.pins = {
        		.sck = BOARD_EXTERNAL_MEMORY_SPIM_PIN_SCK,
        		.strobe = NRF_SQSPI_PINS_UNUSED,
        		.io = {
        			BOARD_EXTERNAL_MEMORY_SPIM_PIN_MOSI,
        			BOARD_EXTERNAL_MEMORY_SPIM_PIN_MISO,
        			BOARD_EXTERNAL_MEMORY_PIN_WP,
        			BOARD_EXTERNAL_MEMORY_PIN_RST,
        		}
    		}
    	};

	if (cfg_quad.skip_gpio_cfg == false) { //This overrides the driver's default configuration
        	// Set drive strength E0E1, as it is non-standard.
        	nrf_gpio_cfg(qspi_dev_quad_config.csn_pin, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
        	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	nrf_gpio_cfg(cfg_quad.pins.sck, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
        	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	for (int i = 0; i < 4; i++) {
        	    nrf_gpio_cfg(cfg_quad.pins.io[i], NRF_GPIO_PIN_DIR_OUTPUT,
			 	 NRF_GPIO_PIN_INPUT_CONNECT,
        	                 NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	}
    	}
    	if (cfg_quad.skip_pmux_cfg == false) { //This overrides the driver's default configuration
        	// Set pin source as FLPR.
        	nrf_gpio_pin_control_select(cfg_quad.pins.sck, NRF_GPIO_PIN_SEL_VPR);
        	nrf_gpio_pin_control_select(qspi_dev_quad_config.csn_pin, NRF_GPIO_PIN_SEL_VPR);
        	for (int i = 0; i < NRF_SQSPI_MAX_NUM_DATA_LINES; i++) {
        	    nrf_gpio_pin_control_select(cfg_quad.pins.io[i], NRF_GPIO_PIN_SEL_VPR);
        	}
    	}

	err = nrf_sqspi_dev_cfg(&qspi, &qspi_dev_quad_config, xfer_done_cb, NULL);
    	if(err != NRFX_SUCCESS){
		LOG_ERR("nrf_sqspi_dev_cfg failed, err = %d", err);
    	}

	err = nrf_sqspi_activate(&qspi);
    	if(err != NRFX_SUCCESS){
    		LOG_ERR("nrf_sqspi_activate failed, err = %d", err);
    	}
}

static void reconfigure_single_spi(void){
	nrfx_err_t err;

	err = nrf_sqspi_deactivate(&qspi);
	if(err != NRFX_SUCCESS){
        	LOG_ERR("nrf_sqspi_deactivate failed, err = %d", err);
    	}

	static const nrf_sqspi_cfg_t cfg_single = {
	.skip_gpio_cfg = false,
	.skip_pmux_cfg = false,
	.pins = {
	    	.sck = BOARD_EXTERNAL_MEMORY_SPIM_PIN_SCK,
	    	.strobe = NRF_SQSPI_PINS_UNUSED,
	    	.mosi = BOARD_EXTERNAL_MEMORY_SPIM_PIN_MOSI,
		.miso = BOARD_EXTERNAL_MEMORY_SPIM_PIN_MISO
	}
    	};
	static const nrf_sqspi_dev_cfg_t qspi_dev_single_config = {
		.csn_pin = BOARD_EXTERNAL_MEMORY_SPIM_PIN_CSN,
        	.sck_freq_khz = 1000,
		.spi_cpolpha = NRF_SQSPI_SPI_CPOLPHA_0,
		.mspi_lines = NRF_SQSPI_SPI_LINES_SINGLE,
		.mspi_ddr = NRF_SQSPI_SPI_DDR_SINGLE,
		.spi_clk_stretch = false
	};

	if (cfg_single.skip_gpio_cfg == false) { //This overrides the driver's default configuration
        	// Set drive strength E0E1, as it is non-standard.
        	nrf_gpio_cfg(qspi_dev_single_config.csn_pin, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
        	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	nrf_gpio_cfg(cfg_single.pins.sck, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
        	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	for (int i = 0; i < 4; i++) {
        	    nrf_gpio_cfg(cfg_single.pins.io[i], NRF_GPIO_PIN_DIR_OUTPUT,
			         NRF_GPIO_PIN_INPUT_CONNECT,
        	                 NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
        	}
    	}
    	if (cfg_single.skip_pmux_cfg == false) { //This overrides the driver's default configuration
    	    	// Set pin source as FLPR.
    	    	nrf_gpio_pin_control_select(cfg_single.pins.sck, NRF_GPIO_PIN_SEL_VPR);
    	    	nrf_gpio_pin_control_select(qspi_dev_single_config.csn_pin, NRF_GPIO_PIN_SEL_VPR);
    	    	for (int i = 0; i < NRF_SQSPI_MAX_NUM_DATA_LINES; i++) {
    	    	    nrf_gpio_pin_control_select(cfg_single.pins.io[i], NRF_GPIO_PIN_SEL_VPR);
    	    	}
    	}

	err = nrf_sqspi_dev_cfg(&qspi, &qspi_dev_single_config, xfer_done_cb, NULL);
    	if(err != NRFX_SUCCESS){
    		LOG_ERR("nrf_sqspi_dev_cfg failed, err = %d", err);
    	}

	err = nrf_sqspi_activate(&qspi);
    	if(err != NRFX_SUCCESS){
    		LOG_ERR("nrf_sqspi_activate failed, err = %d", err);
    	}
}

static nrfx_err_t sqspi_xfer_blocking(const nrf_sqspi_xfer_t* xfer, size_t xfer_count){
	nrfx_err_t err;

	xfer_done = false;
	do {
		err = nrf_sqspi_xfer(&qspi, xfer, xfer_count, 0);
	} while(err == NRFX_ERROR_BUSY);

	while(!xfer_done);

	if (err != NRFX_SUCCESS){
		return err;
	}

	return NRFX_SUCCESS;
}

static nrfx_err_t mx25_wait_wip(void){
	nrf_sqspi_xfer_t xfer = {
		.dir = NRF_SQSPI_XFER_DIR_RX,
		.cmd = MX25_CMD_RDSR, .cmd_length = 8,
		.addr_length = 0, .dummy_length = 0,
		.p_data = &ready_reg, .data_length = sizeof(ready_reg)
	};
	nrfx_err_t err;

	do {
		err = sqspi_xfer_blocking(&xfer, 1);

		if (err != NRFX_SUCCESS){
			return err;
		}
	} while((ready_reg & MX25_WIP_BIT) != 0);
	return NRFX_SUCCESS;
}

static nrfx_err_t mx25_wait_wel(void){
	nrf_sqspi_xfer_t xfer = {
		.dir = NRF_SQSPI_XFER_DIR_RX,
		.cmd = MX25_CMD_RDSR,
		.cmd_length = 8,
		.addr_length = 0,
		.dummy_length = 0,
		.p_data = &ready_reg,
		.data_length = sizeof(ready_reg)
	};
	nrfx_err_t err;

	do {
		err = sqspi_xfer_blocking(&xfer, 1);

		if (err != NRFX_SUCCESS){
			return err;
		}
	} while(!(ready_reg & MX25_WEL_BIT));
	return NRFX_SUCCESS;
}

static nrfx_err_t mx25_write_enable(void){
	nrf_sqspi_xfer_t xfer = {
		.dir = NRF_SQSPI_XFER_DIR_TX,
		.cmd = MX25_CMD_WREN,
		.cmd_length = 8,
		.addr_length = 0,
		.dummy_length = 0,
		.data_length = 0
	};
	nrfx_err_t err;

	err = sqspi_xfer_blocking(&xfer, 1);

	if (err != NRFX_SUCCESS){
		return err;
	}
	err = mx25_wait_wel();
	return err;
}

static nrfx_err_t external_memory_init(void)
{
	nrfx_err_t err;
	static const nrf_sqspi_cfg_t cfg_single = {
    		.skip_gpio_cfg = false,
    		.skip_pmux_cfg = false,
    		.pins = {
    		    	.sck = BOARD_EXTERNAL_MEMORY_SPIM_PIN_SCK,
    		    	.strobe = NRF_SQSPI_PINS_UNUSED,
    		    	.mosi = BOARD_EXTERNAL_MEMORY_SPIM_PIN_MOSI,
			.miso = BOARD_EXTERNAL_MEMORY_SPIM_PIN_MISO
    		}
    	};
	static const nrf_sqspi_dev_cfg_t qspi_dev_single_config = {
		.csn_pin = BOARD_EXTERNAL_MEMORY_SPIM_PIN_CSN,
        	.sck_freq_khz = 1000,
		.spi_cpolpha = NRF_SQSPI_SPI_CPOLPHA_0,
		.mspi_lines = NRF_SQSPI_SPI_LINES_SINGLE,
		.mspi_ddr = NRF_SQSPI_SPI_DDR_SINGLE,
		.spi_clk_stretch = false
	};
	static const nrf_sqspi_data_fmt_t sqspi_data_fmt = {
		.cmd_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
		.addr_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
		.data_bit_order = NRF_SQSPI_DATA_FMT_BIT_ORDER_MSB_FIRST,
		.data_bit_reorder_unit = 8,
		.data_container = 32,
		.data_swap_unit = 8,
		.data_padding = NRF_SQSPI_DATA_FMT_PAD_RAW,
	};

	if (nrf_sqspi_init_check(&qspi)){
        	return 0;
    	}
    	nrf_spu_periph_perm_secattr_set(NRF_SPU00,
					nrf_address_slave_get((uint32_t)NRF_MEMCONF), true);
    	NRF_SPU00_S->PERIPH[0xC].PERM = (SPU_PERIPH_PERM_SECATTR_Secure <<
					 SPU_PERIPH_PERM_SECATTR_Pos);

    	err = nrf_sqspi_init(&qspi, &cfg_single);
    	if (err != NRFX_SUCCESS){
    	    return err;
    	}

    	err = nrf_sqspi_dev_data_fmt_set(&qspi, &sqspi_data_fmt);
    	if (err != NRFX_SUCCESS) {
		LOG_ERR("nrf_sqspi_dev_data_fmt_set() failed: %d", err);
		return err;
	}
    	if (!nrf_sqspi_init_check(&qspi)){
    	    	LOG_WRN("sqspi not inited");
    	}

    	err = nrf_sqspi_dev_cfg(&qspi, &qspi_dev_single_config, xfer_done_cb, NULL);
    	if(err != NRFX_SUCCESS){
    	    	LOG_ERR("nrf_sqspi_dev_cfg failed, err = %d", err);
		return err;
    	}

    	if (cfg_single.skip_gpio_cfg == false) { //This overrides the driver's default configuration
    	    	// Set drive strength E0E1, as it is non-standard.
    	    	nrf_gpio_cfg(qspi_dev_single_config.csn_pin, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
    	    	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    	nrf_gpio_cfg(cfg_single.pins.sck, NRF_GPIO_PIN_DIR_OUTPUT,
			     NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL,
    	    	             NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    	for (int i = 0; i < 4; i++) {
    	    	    nrf_gpio_cfg(cfg_single.pins.io[i], NRF_GPIO_PIN_DIR_OUTPUT,
				 NRF_GPIO_PIN_INPUT_CONNECT,
    	    	                 NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_E0E1, NRF_GPIO_PIN_NOSENSE);
    	    	}
    	}
    	if (cfg_single.skip_pmux_cfg == false) { //This overrides the driver's default configuration
    	    	// Set pin source as FLPR.
    	    	nrf_gpio_pin_control_select(cfg_single.pins.sck, NRF_GPIO_PIN_SEL_VPR);
    	    	nrf_gpio_pin_control_select(qspi_dev_single_config.csn_pin, NRF_GPIO_PIN_SEL_VPR);
    	    	for (int i = 0; i < NRF_SQSPI_MAX_NUM_DATA_LINES; i++) {
    	    	    nrf_gpio_pin_control_select(cfg_single.pins.io[i], NRF_GPIO_PIN_SEL_VPR);
    	    	}
    	}

    	err = nrf_sqspi_activate(&qspi);
    	if(err != NRFX_SUCCESS){
    	    	LOG_ERR("nrf_sqspi_activate failed, err = %d", err);
		return err;
    	}

	BM_IRQ_DIRECT_CONNECT(SP_VPR_IRQn, 2, sqspi_direct_isr, 0);

	uint8_t quad_enable[3] = {0};
	nrf_sqspi_xfer_t xfer_rdsr = {
		.dir = NRF_SQSPI_XFER_DIR_RX,
		.cmd = MX25_CMD_RDSR,
		.p_data = quad_enable,
		.data_length = sizeof(quad_enable) - 2,
		.cmd_length = 8,
		.addr_length = 0,
		.dummy_length = 0
	};

	err = sqspi_xfer_blocking(&xfer_rdsr, 1);
	if(err != NRFX_SUCCESS){
    		return err;
    	}
	nrf_sqspi_xfer_t xfer_rdcr = {
		.dir = NRF_SQSPI_XFER_DIR_RX,
		.cmd = MX25_CMD_RDCR,
		.p_data = quad_enable + 1,
		.data_length = sizeof(quad_enable) - 1,
		.cmd_length = 8,
		.addr_length = 0,
		.dummy_length = 0
	};

	err = sqspi_xfer_blocking(&xfer_rdcr, 1);
	if(err != NRFX_SUCCESS){
    		return err;
    	}

	quad_enable[0] |= 1 << 6;

	nrf_sqspi_xfer_t xfer_wrsr = {
		.dir = NRF_SQSPI_XFER_DIR_TX,
		.cmd = MX25_CMD_WRSR,
		.p_data = quad_enable,
		.data_length = sizeof(quad_enable),
		.cmd_length = 8,
		.addr_length = 0,
		.dummy_length = 0
	};

	err = sqspi_xfer_blocking(&xfer_wrsr, 1);
	if(err != NRFX_SUCCESS){
    		return err;
    	}

	return 0;
}

static nrfx_err_t flash_erase(void)
{
	nrfx_err_t err;

	err = mx25_wait_wip();
	if(err != NRFX_SUCCESS) return err;

	err = mx25_write_enable();
	if(err != NRFX_SUCCESS) return err;

	nrf_sqspi_xfer_t xfer_erase = {
		.dir = NRF_SQSPI_XFER_DIR_TX,
		.cmd = MX25_CMD_SECTOR_ERASE,
		.cmd_length = 8,
		.address = CONFIG_SAMPLE_SQSPI_FLASH_ADDR,
		.addr_length = 24,
	};

	err = sqspi_xfer_blocking(&xfer_erase, 1);
	if(err != NRFX_SUCCESS) return err;

	LOG_INF("Erased 4 KiB sector @ 0x%08lx", (unsigned long)CONFIG_SAMPLE_SQSPI_FLASH_ADDR);

	return NRFX_SUCCESS;
}

static nrfx_err_t flash_read(void)
{
	reconfigure_quad_spi();

	uint8_t read_rx[sizeof(CONFIG_SAMPLE_SQSPI_MSG)];
	nrf_sqspi_xfer_t xfer = {
		.dir = NRF_SQSPI_XFER_DIR_RX,
		.cmd = MX25_CMD_4READ,
		.address = CONFIG_SAMPLE_SQSPI_FLASH_ADDR,
		.p_data = read_rx,
		.data_length = sizeof(CONFIG_SAMPLE_SQSPI_MSG),
		.cmd_length = 8,
		.addr_length = 24,
		.dummy_length = 6
	};
	nrfx_err_t err = sqspi_xfer_blocking(&xfer, 1);
    	if(err != NRFX_SUCCESS){
		reconfigure_single_spi();
    	    	return err;
    	}

	LOG_HEXDUMP_INF(read_rx, sizeof(CONFIG_SAMPLE_SQSPI_MSG), "flash");

	reconfigure_single_spi();
	return err;
}

static nrfx_err_t flash_program(void)
{
	nrfx_err_t err;

	err = mx25_wait_wip();
	if(err != NRFX_SUCCESS) return err;
	err = mx25_write_enable();
	if(err != NRFX_SUCCESS) return err;
	reconfigure_quad_spi();
	nrf_sqspi_xfer_t write = {
		.dir = NRF_SQSPI_XFER_DIR_TX,
		.cmd = MX25_CMD_4PAGE_PROGRAM,
		.cmd_length = 8,
		.address = CONFIG_SAMPLE_SQSPI_FLASH_ADDR,
		.addr_length = 24,
		.p_data = CONFIG_SAMPLE_SQSPI_MSG,
		.data_length = sizeof(CONFIG_SAMPLE_SQSPI_MSG)
	};
	err = sqspi_xfer_blocking(&write, 1);
	reconfigure_single_spi();
	if(err != NRFX_SUCCESS) return err;


	err = mx25_wait_wip();

	LOG_INF("Flash operation complete");

	return err;
}

static void button_handler(uint8_t pin, enum bm_buttons_evt_type action)
{
	nrfx_err_t err;
	if (action != BM_BUTTONS_PRESS) {
		return;
	}

	switch (pin) {
	case BOARD_PIN_BTN_1:
		err = flash_erase();
		if(err != NRFX_SUCCESS){
			LOG_ERR("Flash erase failed, err = %d", err);
		}
		break;
	case BOARD_PIN_BTN_2:
		err = flash_read();
		if(err != NRFX_SUCCESS){
			LOG_ERR("Flash read failed, err = %d", err);
		}
		break;
	case BOARD_PIN_BTN_3:
		err = flash_program();
		if(err != NRFX_SUCCESS){
			LOG_ERR("Flash program failed, err = %d", err);
		}
		break;
	default:
		break;
	}
}

static int buttons_init(void)
{
	int err;

	static const struct bm_buttons_config cfg[] = {
		{
			.pin_number = BOARD_PIN_BTN_1,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler,
		},
		{
			.pin_number = BOARD_PIN_BTN_2,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler,
		},
		{
			.pin_number = BOARD_PIN_BTN_3,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler,
		},
	};

	err = bm_buttons_init(cfg, ARRAY_SIZE(cfg), BM_BUTTONS_DETECTION_DELAY_MIN_US);
	if (err) {
		return err;
	}

	return bm_buttons_enable();
}

int main(void)
{
	int err;

	err = external_memory_init();
	if (err) {
		LOG_ERR("SPI init failed, err %d", err);
		goto idle;
	}

	err = buttons_init();
	if (err) {
		LOG_ERR("Buttons init failed, err %d", err);
		goto idle;
	}

	nrf_gpio_cfg_output(BOARD_PIN_LED_0);
	nrf_gpio_pin_write(BOARD_PIN_LED_0, !BOARD_LED_ACTIVE_STATE);

	LOG_INF("sQSPI sample initialized");
	LOG_INF("Use following buttons to interact with external memory through SPI");
	LOG_INF("Button 1: Erase");
	LOG_INF("Button 2: Read");
	LOG_INF("Button 3: Program");

	nrf_gpio_pin_write(BOARD_PIN_LED_0, BOARD_LED_ACTIVE_STATE);

idle:
	while (true) {
		log_flush();
		k_cpu_idle();
	}
}
