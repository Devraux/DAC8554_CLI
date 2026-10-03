#ifndef _DAC8554_
#define _DAC8554_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>   
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define DAC_INSTANCE_COUNT (3) // Number of DAC instances - DAC0/1/2
#define DAC_CHANNELS_COUNT (4) // Number of DAC channels (A/B/C/D)

#define DAC_SYNC_PIN (0)
#define DAC_SCL_PIN (2)
#define DAC_DIN_PIN (3)
#define DAC_SPI_INSTANCE (spi0)

typedef struct dac_state_t
{
    uint32_t ch_a_val;
    uint32_t ch_b_val;
    uint32_t ch_c_val;
    uint32_t ch_d_val;

    uint32_t current_command;
} dac_state_t;

typedef enum dac_instance_t
{
    DAC_INST_0 = 0,
    DAC_INST_1 = 1,
    DAC_INST_2 = 2
} dac_instance_t;

typedef enum dac_channel_t
{
    CHANNEL_A = 0b00, // DAC buffer A
    CHANNEL_B = 0b01, // DAC buffer B
    CHANNEL_C = 0b10, // DAC buffer C
    CHANNEL_D = 0b11,  // DAC buffer D
    CHANNEL_COUNT
}dac_channel_t;

bool dac_init(void);
bool dac_write_single(dac_instance_t dac_inst, dac_channel_t dac_channel, uint16_t output_value);
bool dac_write_all(dac_instance_t dac_inst, uint16_t dac_ch_a_val, uint16_t dac_ch_b_val, uint16_t dac_ch_c_val, uint16_t dac_ch_d_val);
void dac_get_state(dac_state_t *state, dac_instance_t dac_instance);

#endif