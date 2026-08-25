
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



#define FLASH_ADDRESS 0x0U

LOG_MODULE_REGISTER(sqspi, CONFIG_SAMPLE_SQSPI_LOG_LEVEL);

ISR_DIRECT_DECLARE(sqspi_direct_isr)
{
	nrf_sqspi_irq_handler();
	return 0;
}

static nrf_sqspi_t qspi = {.p_reg = (void *)DT_REG_ADDR(DT_NODELABEL(flpr_vri_ram)), .drv_inst_idx = 0};
static uint32_t magic = 0xCAFEBABE;
static const nrf_sqspi_xfer_t xfer = {.dir = NRF_SQSPI_XFER_DIR_TX, .cmd = 0, .address = 0xBEEFF00D, .p_data = &magic, .data_length = sizeof(magic), .cmd_length = 0, .addr_length = 32, .dummy_length = 0};

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

    LOG_INF("sQSPI Sample started.");
    while(1){
        log_flush();
    }

    return 0;
}
