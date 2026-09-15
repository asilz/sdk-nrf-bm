#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <ff.h>
#include <bm/bm_buttons.h>
#include <board-config.h>

LOG_MODULE_REGISTER(sqspi, CONFIG_SAMPLE_SQSPI_LOG_LEVEL);

static FATFS FatFs;   /* Work area (filesystem object) for logical drive */
static FIL file;
static BYTE work[FF_MAX_SS];

static void button_handler(uint8_t pin, uint8_t action)
{
    	FRESULT err;
    	UINT byte_count;

    	if(action != BM_BUTTONS_RELEASE){
    		return;
    	}

    	if(bm_buttons_is_pressed(BOARD_PIN_BTN_2)){
    	    	switch (pin)
    	    	{
    	    	case BOARD_PIN_BTN_0:
    	    		err = f_open(&file, "main.txt", FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
    	    		if(err != FR_OK){
    	    			LOG_ERR("Failed to open file, err = %d", err);
    	    		} else {
    	    			LOG_INF("Opened file");
				nrf_gpio_pin_write(BOARD_PIN_LED_1, BOARD_LED_ACTIVE_STATE);
    	    		}
    	    		break;
    	    	case BOARD_PIN_BTN_1:
    	    		err = f_close(&file);
    	    		if(err != FR_OK){
    	    			LOG_ERR("Failed to close file, err = %d", err);
    	    		} else {
    	    			LOG_INF("Closed file");
				nrf_gpio_pin_write(BOARD_PIN_LED_1, !BOARD_LED_ACTIVE_STATE);
    	    		}
    	    		break;
    	    	case BOARD_PIN_BTN_3:
    	    	    	err = f_mount(&FatFs, "", 0);
    	    	    	if(err != FR_OK){
    	    	    		LOG_ERR("Failed to mount drive, err = %d", err);
    	    	    	} else{
    	    	    		LOG_INF("Mounted file system");
				nrf_gpio_pin_write(BOARD_PIN_LED_0, BOARD_LED_ACTIVE_STATE);
    	    	    	}
    	    	    	break;
    	    	default:
    			break;
    	    	}
    	}
    	else{
    	    	switch (pin)
    	    	{
    	    	case BOARD_PIN_BTN_0:
    	        	uint32_t read_buf;
    	        	err = f_read(&file, &read_buf, sizeof(read_buf), &byte_count);
    	        	if(err != FR_OK){
    	        		LOG_ERR("Failed to read from file, err = %d", err);
    	        	}
    	        	err = f_lseek(&file, 0);
    	        	if(err != FR_OK){
    	        		LOG_ERR("Failed to seek to start of file, err = %d", err);
    	        	}
    	        	LOG_INF("Read Value 0x%X", read_buf);
    	        	break;
    	    	case BOARD_PIN_BTN_1:
			static const uint32_t magic = 0xCAFEBABE;
    	        	err = f_write(&file, &magic, sizeof(magic), &byte_count);
    	        	if(err != FR_OK){
    	        		LOG_ERR("Failed to write to file, err = %d", err);
    	        	}
    	        	err = f_lseek(&file, 0);
    	        	if(err != FR_OK){
    	        		LOG_ERR("Failed to seek to start of file, err = %d", err);
    	        	}
    	        	LOG_INF("Wrote Value 0x%X", magic);
    	        	break;
    	    	case BOARD_PIN_BTN_3:
			nrf_gpio_pin_write(BOARD_PIN_LED_2, BOARD_LED_ACTIVE_STATE);
    	        	err = f_mkfs("", NULL, work, sizeof(work));
			nrf_gpio_pin_write(BOARD_PIN_LED_2, !BOARD_LED_ACTIVE_STATE);
    	        	if(err != FR_OK){
    	        		LOG_ERR("Failed to create FAT volume on logical drive, err = %d", err);
    	        	} else {
    	        		LOG_INF("Created FAT volume on logical drive");
    	        	}
    	        	break;
    	    	default:
    	        	break;
    	    	}
    	}
}

int main(void) {

    	int err;

    	LOG_INF("sQSPI Sample started.");

    	static const struct bm_buttons_config configs[] = {
		{
			.pin_number = BOARD_PIN_BTN_0,
			.active_state = BM_BUTTONS_ACTIVE_LOW,
			.pull_config = BM_BUTTONS_PIN_PULLUP,
			.handler = button_handler,
		},
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

	err = bm_buttons_init(configs, ARRAY_SIZE(configs), BM_BUTTONS_DETECTION_DELAY_MIN_US);
	if (err) {
		LOG_ERR("Failed to initialize buttons, err %d", err);
		goto idle;
	}

	err = bm_buttons_enable();
	if (err) {
		LOG_ERR("Failed to enable buttons, err %d", err);
		goto idle;
	}

    	nrf_gpio_cfg_output(BOARD_PIN_LED_0);
    	nrf_gpio_cfg_output(BOARD_PIN_LED_1);
	nrf_gpio_cfg_output(BOARD_PIN_LED_2);
	nrf_gpio_pin_write(BOARD_PIN_LED_0, !BOARD_LED_ACTIVE_STATE);
    	nrf_gpio_pin_write(BOARD_PIN_LED_1, !BOARD_LED_ACTIVE_STATE);
	nrf_gpio_pin_write(BOARD_PIN_LED_2, !BOARD_LED_ACTIVE_STATE);

idle:
    	while(1){
		log_flush();
    	}

    	return 0;
}
