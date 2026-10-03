#ifndef _CLI_
#define _CLI_

#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "DAC.h"

#define  CLI_MAX_LINE_LENGTH (512) // Max number of characters in one command line
#define  CLI_MAX_TOKENS (20) // Max number of words in one command line

typedef enum cmd_type_t
{
    CMD_UNKNOWN,
    CMD_WRITE_SINGLE,
    CMD_WRITE_ALL,
    CMD_ZERO,
    CMD_DEBUG_ON,
    CMD_DEBUG_OFF,
    CMD_READ_CONFIG,
    CMD_HELP
}cmd_type_t;

typedef struct parsed_cmd_t
{
    cmd_type_t command;
    dac_instance_t dac_instance;
    uint16_t channel_values[DAC_CHANNELS_COUNT];// DAC new values
    bool channel_to_update[DAC_CHANNELS_COUNT]; // DAC channels ready to update
    bool status; // true -> Ok, false -> Fail
    bool debug_enable;
}parsed_cmd_t;


void read_line(char *buffer, uint32_t max_len);
void parse_command(char *buffer, uint32_t str_len);
bool execute_command(parsed_cmd_t *cmd);
void cli_run(void);

#endif