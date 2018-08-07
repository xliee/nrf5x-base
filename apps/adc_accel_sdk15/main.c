// Blink app
//
// Blinks all pins on Buckler

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_uarte.h"
#include "app_uart.h"
#include "nrfx_saadc.h"


#define UART_RX              NRF_GPIO_PIN_MAP(0, 8)
#define UART_TX              NRF_GPIO_PIN_MAP(0, 6)
#define UART_TX_BUF_SIZE     256
#define UART_RX_BUF_SIZE     256

#define LED0 NRF_GPIO_PIN_MAP(0,16)
#define LED1 NRF_GPIO_PIN_MAP(0,17)
#define LED2 NRF_GPIO_PIN_MAP(0,19)

//#define ADCX NRF_GPIO_PIN_MAP(0,29)
//#define ADCY NRF_GPIO_PIN_MAP(0,30)
//#define ADCZ NRF_GPIO_PIN_MAP(0,31)
#define ADCX NRF_SAADC_INPUT_AIN5;
#define ADCY NRF_SAADC_INPUT_AIN6;
#define ADCZ NRF_SAADC_INPUT_AIN7;

static nrf_saadc_value_t m_buffer_pool[2][1];

void saadc_callback(nrfx_saadc_evt_t const * p_event) {
  nrf_gpio_pin_toggle(LED2);
  /*
  ret_code_t err_code;

  //Capture offset calibration complete event
  if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {

    //Toggle LED2 to indicate SAADC buffer full		
    LEDS_INVERT(BSP_LED_1_MASK);

    //Evaluate if offset calibration should be performed. Configure the SAADC_CALIBRATION_INTERVAL constant to change the calibration frequency
    if((m_adc_evt_counter % SAADC_CALIBRATION_INTERVAL) == 0) {
      // Abort all ongoing conversions. Calibration cannot be run if SAADC is busy
      nrfx_saadc_abort();
      // Set flag to trigger calibration in main context when SAADC is stopped
      m_saadc_calibrate = true;
    }


    if (m_saadc_calibrate == false) {
      //Set buffer so the SAADC can write to it again. 
      err_code = nrfx_saadc_buffer_convert(p_event->data.done.p_buffer, SAADC_SAMPLES_IN_BUFFER);
      APP_ERROR_CHECK(err_code);
    }

    m_adc_evt_counter++;

  } else if (p_event->type == NRF_DRV_SAADC_EVT_CALIBRATEDONE) {
    //Toggle LED3 to indicate SAADC calibration complete
    LEDS_INVERT(BSP_LED_2_MASK);

    //Set buffer so the SAADC can write to it again. 
    err_code = nrfx_saadc_buffer_convert(m_buffer_pool[0], SAADC_SAMPLES_IN_BUFFER);
    //APP_ERROR_CHECK(err_code);
    //Need to setup both buffers, as they were both removed with the call to nrfx_saadc_abort before calibration.
    err_code = nrfx_saadc_buffer_convert(m_buffer_pool[1], SAADC_SAMPLES_IN_BUFFER);
    //APP_ERROR_CHECK(err_code);
  }
  */
}

void uart_error_handle (app_uart_evt_t * p_event) {
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    } else if (p_event->evt_type == APP_UART_FIFO_ERROR) {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

void uart_init(void) {
  uint32_t err_code;

  const app_uart_comm_params_t comm_params =
  {
    UART_RX,
    UART_TX,
    0,
    0,
    APP_UART_FLOW_CONTROL_DISABLED,
    false,
    NRF_UARTE_BAUDRATE_115200
  };
  APP_UART_FIFO_INIT(&comm_params,
                     UART_RX_BUF_SIZE,
                     UART_TX_BUF_SIZE,
                     uart_error_handle,
                     APP_IRQ_PRIORITY_LOW,
                     err_code);
  if (err_code != NRF_SUCCESS) {
    nrf_gpio_pin_clear(LED1);
  }
  //if (err_code == NRF_ERROR_INVALID_ADDR) {
  //  nrf_gpio_pin_clear(LED2);
  //}
  APP_ERROR_CHECK(err_code);

}

int main(void) {
  uint32_t err_code;

  uint32_t leds[3] = {LED0, LED1, LED2};

  // Initialize.
  for (int i=0; i<3; i++) {
    uint32_t LED = leds[i];
    nrf_gpio_cfg_output(LED);
    nrf_gpio_pin_set(LED);
  }
  nrf_gpio_pin_clear(LED0);
  uart_init();

  printf("Testing ADC\n");

  // initialize ADC
  nrfx_saadc_config_t saadc_config;
  saadc_config.low_power_mode = false; // changed from example code
  saadc_config.resolution = NRF_SAADC_RESOLUTION_12BIT;
  saadc_config.oversample = NRF_SAADC_OVERSAMPLE_DISABLED;
  saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;
  err_code = nrfx_saadc_init(&saadc_config, saadc_callback);
  printf("\tnrfx_saadc_init: %d\n", err_code);

  // configure ADC
  // The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
  nrf_saadc_channel_config_t channel_config;
  channel_config.reference = NRF_SAADC_REFERENCE_INTERNAL;
  channel_config.gain = NRF_SAADC_GAIN1_6;
  channel_config.acq_time = NRF_SAADC_ACQTIME_10US;
  channel_config.mode = NRF_SAADC_MODE_SINGLE_ENDED;
  channel_config.pin_p = ADCX;
  channel_config.pin_n = NRF_SAADC_INPUT_DISABLED;
  channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
  channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
  err_code = nrfx_saadc_channel_init(0, &channel_config);
  printf("\tnrfx_saadc_channel_init: %d\n", err_code);

  while (1) {
    //nrfx_saadc_buffer_convert(m_buffer_pool[0], SAADC_SAMPLES_IN_BUFFER);
    //nrfx_saadc_buffer_convert(m_buffer_pool[1], SAADC_SAMPLES_IN_BUFFER);
    nrf_saadc_value_t val;
    err_code = nrfx_saadc_sample_convert(0, &val);
    printf("\tnrfx_saadc_sample_convert: %d\n", err_code);

    printf("\tsample value = %d\n\n", val);


    if (val < ((2 << 12)/2)) {
      nrf_gpio_pin_clear(LED1);
      nrf_gpio_pin_set(LED2);
    } else {
      nrf_gpio_pin_set(LED1);
      nrf_gpio_pin_clear(LED2);
    }
    nrf_delay_ms(500);
  }
}

