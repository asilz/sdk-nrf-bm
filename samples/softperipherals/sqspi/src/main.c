
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

LOG_MODULE_REGISTER(sqspi, CONFIG_SAMPLE_SQSPI_LOG_LEVEL);

ISR_DIRECT_DECLARE(sqspi_direct_isr)
{
	nrf_sqspi_irq_handler();
	return 0;
}

static nrf_sqspi_t qspi = {.p_reg = (void *)DT_REG_ADDR(DT_NODELABEL(flpr_vri_ram)), .drv_inst_idx = 0};
static uint32_t magic = 0xCAFEBABE;
static const nrf_sqspi_xfer_t xfer = {.dir = NRF_SQSPI_XFER_DIR_TX, .cmd = 0, .address = 0, .p_data = &magic, .data_length = sizeof(magic), .cmd_length = 0, .addr_length = 0, .dummy_length = 0};

static void cb(nrf_sqspi_t const *p_qspi, nrf_sqspi_evt_t *p_event, void *p_context)
{
    nrfx_err_t err;
    if(p_event->type == NRF_SQSPI_EVT_XFER_DONE){

        if(p_event->data.xfer_done != NRF_SQSPI_RESULT_OK){
            LOG_ERR("xfer done error, err = %d", p_event->data.xfer_done);
        }
        err = nrf_sqspi_xfer(&qspi, &xfer, 1, 0);
        if(err != NRFX_SUCCESS){
            LOG_ERR("nrf_sqspi_xfer failed, err = %d", err);
        }
    }
}

int main(void) {

    nrfx_err_t err;

    BM_IRQ_DIRECT_CONNECT(SP_VPR_IRQn, 4, sqspi_direct_isr, 0);

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
					    .mspi_lines = NRF_SQSPI_SPI_LINES_SINGLE,
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

    err = nrf_sqspi_xfer(&qspi, &xfer, 1, 0);
    if(err != NRFX_SUCCESS){
        LOG_ERR("nrf_sqspi_xfer failed, err = %d", err);
    }
    LOG_INF("sQSPI Sample started.");
    while(1){
        log_flush();
    }

    return 0;
}
