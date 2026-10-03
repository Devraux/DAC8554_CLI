#include "DAC.h"

static dac_state_t dac_state[DAC_INSTANCE_COUNT] = {0};

// Maps logical DAC index to Hardware Address Bits (A1, A0)
static uint8_t get_dac_address_bits(dac_instance_t dac_inst)
{
    switch (dac_inst)
    {
        case DAC_INST_0:
            return 0b00; // A1=0, A0=0
        break;

        case DAC_INST_1:
            return 0b10; // A1=1, A0=0
        break;

        case DAC_INST_2:
            return 0b11; // A1=1, A0=1
        break;

        default:
            return 0b00;
    }
}

bool dac_init(void)
{
    uint32_t baudrate = 0;
    gpio_set_function(DAC_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(DAC_DIN_PIN, GPIO_FUNC_SPI);
    
    gpio_init(DAC_SYNC_PIN);
    gpio_set_dir(DAC_SYNC_PIN, GPIO_OUT);
    gpio_put(DAC_SYNC_PIN, 1);

    baudrate = spi_init(DAC_SPI_INSTANCE, 1 * 1000 * 1000);
    spi_set_format(DAC_SPI_INSTANCE, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

    return (baudrate > 0);
}

bool dac_write_single(dac_instance_t dac_inst, dac_channel_t dac_channel, uint16_t output_value)
{
    uint32_t status = 0;
    if(dac_inst != DAC_INST_0 && dac_inst != DAC_INST_1 && dac_inst != DAC_INST_2)
        return false;
    if(dac_channel != CHANNEL_A && dac_channel != CHANNEL_B && dac_channel != CHANNEL_C && dac_channel != CHANNEL_D)
        return false;

    uint8_t write_buffer[3] = {0}; // 24-bit register DAC command register

    switch(dac_inst)
    {
        case (DAC_INST_0):
            write_buffer[0] &= ~((1 << 7) | (1 << 6)); // set hardcoded device address 0b00
            break;

        case (DAC_INST_1):
            write_buffer[0] |= (1 << 7); // set hardcoded device address 0b10
            write_buffer[0] &=  ~(1 << 6); // set hardcoded device address 0b10
            break;

        case (DAC_INST_2):
            write_buffer[0] |= ((1 << 7) | (1 << 6)); // set hardcoded device address 0b11
            break;        
    }
    
    write_buffer[0] &= ~(1 << 5); // set load type - single channel update
    write_buffer[0] |= (1 << 4);  // set load type - single channel update

    switch (dac_channel)
    {
        case CHANNEL_A:
            write_buffer[0] &= ~((1 << 2) | (1 << 1));
            dac_state[dac_inst].ch_a_val = output_value;
            break;

        case CHANNEL_B:
            write_buffer[0] |= (1 << 1);
            write_buffer[0] &= ~(1 << 2);
            dac_state[dac_inst].ch_b_val = output_value;
            break;

        case CHANNEL_C:
            write_buffer[0] |= (1 << 2);
            write_buffer[0] &= ~(1 << 1);
            dac_state[dac_inst].ch_c_val = output_value;
            break;

        case CHANNEL_D:
            write_buffer[0] |= ((1 << 2) | (1 << 1));
            dac_state[dac_inst].ch_d_val = output_value;
            break;
    }

    write_buffer[1] = (output_value >> 8) & 0xFF;
    write_buffer[2] = (output_value & 0xFF);

    gpio_put(DAC_SYNC_PIN, 0);
    status = spi_write_blocking(DAC_SPI_INSTANCE, write_buffer, 3);
    gpio_put(DAC_SYNC_PIN, 1);

    uint32_t command = 0;
    command |= write_buffer[0]; command = command << 8;
    command |= write_buffer[1]; command = command << 8;
    command |= write_buffer[2]; 
    dac_state[dac_inst].current_command = command;

    return (status > 0);
}

bool dac_write_all(dac_instance_t dac_inst, uint16_t dac_ch_a_val, uint16_t dac_ch_b_val, uint16_t dac_ch_c_val, uint16_t dac_ch_d_val)
{
uint32_t status = 0;
    if(dac_inst != DAC_INST_0 && dac_inst != DAC_INST_1 && dac_inst != DAC_INST_2)
        return false;

    uint8_t write_buffer[3] = {0}; // 24-bit register DAC command register

    switch(dac_inst)
    {
        case (DAC_INST_0):
            write_buffer[0] &= ~((1 << 7) | (1 << 6)); // set hardcoded device address 0b00
            break;

        case (DAC_INST_1):
            write_buffer[0] |= (1 << 7); // set hardcoded device address 0b10
            write_buffer[0] &=  ~(1 << 6); // set hardcoded device address 0b10
            break;

        case (DAC_INST_2):
            write_buffer[0] |= ((1 << 7) | (1 << 6)); // set hardcoded device address 0b11
            break;        
    }

    write_buffer[0] &= ~(1 << 5); // set load type - all channel update mode
    write_buffer[0] &= ~(1 << 4); // set load type - all channel update mode

    for (dac_channel_t i = 0; i < CHANNEL_COUNT - 1; i++)
    {   
        switch(i)
        {
            case (CHANNEL_A):
                write_buffer[0] &= ~((1 << 2) | (1 << 1)); // Select channel A
                dac_state[dac_inst].ch_a_val = dac_ch_a_val;
                gpio_put(DAC_SYNC_PIN, 0);
                write_buffer[1] = (dac_ch_a_val >> 8) & 0xFF; 
                write_buffer[2] = (dac_ch_a_val & 0xFF);
                status = spi_write_blocking(DAC_SPI_INSTANCE, write_buffer, 3);
                gpio_put(DAC_SYNC_PIN, 1);
            break;

            case (CHANNEL_B):
                write_buffer[0] |= (1 << 1);  // Select channel B
                write_buffer[0] &= ~(1 << 2); // Select channel B
                dac_state[dac_inst].ch_b_val = dac_ch_b_val;
                write_buffer[1] = (dac_ch_b_val >> 8) & 0xFF; 
                write_buffer[2] = (dac_ch_b_val & 0xFF);
                gpio_put(DAC_SYNC_PIN, 0);
                status = spi_write_blocking(DAC_SPI_INSTANCE, write_buffer, 3);
                gpio_put(DAC_SYNC_PIN, 1);
            break;

            case (CHANNEL_C):
                write_buffer[0] |= (1 << 2); // Select channel C
                write_buffer[0] &= ~(1 << 1);// Select channel C
                dac_state[dac_inst].ch_c_val = dac_ch_c_val;
                write_buffer[1] = (dac_ch_c_val >> 8) & 0xFF; 
                write_buffer[2] = (dac_ch_c_val & 0xFF);
                gpio_put(DAC_SYNC_PIN, 0);
                status = spi_write_blocking(DAC_SPI_INSTANCE, write_buffer, 3);
                gpio_put(DAC_SYNC_PIN, 1);
            break;
        }

        if(status == 0)
            return false;
    }

    // Wrtie buffer D and update all outputs
    write_buffer[0] |= (1 << 5); // set load type - single channel update
    write_buffer[0] &= ~(1 << 4);  // set load type - single channel update
    write_buffer[0] |= ((1 << 2) | (1 << 1)); // Select channel D
    dac_state[dac_inst].ch_d_val = dac_ch_d_val;
    write_buffer[1] = (dac_ch_d_val >> 8) & 0xFF; // Dodano wpisanie wartości danych
    write_buffer[2] = (dac_ch_d_val & 0xFF);
    gpio_put(DAC_SYNC_PIN, 0);
    status = spi_write_blocking(DAC_SPI_INSTANCE, write_buffer, 3);
    gpio_put(DAC_SYNC_PIN, 1);

    return (status > 0);
}

void dac_get_state(dac_state_t *state, dac_instance_t dac_instance)
{
    if (state == NULL || dac_instance >= DAC_INSTANCE_COUNT)
        return; 

    memcpy(state, &dac_state[dac_instance], sizeof(dac_state_t));
}