#include "cli.h"

static char input_buf[CLI_MAX_LINE_LENGTH];  //This variables should be private
static char *input_tokenized[CLI_MAX_TOKENS]; //This variables should be private
static parsed_cmd_t parsed_cmd =
{
    .command = CMD_UNKNOWN,
    .dac_instance = DAC_INST_0,
    .channel_values = 0,
    .channel_to_update = false,
    .status = true,
    .debug_enable = false
};

void read_line(char *buffer, uint32_t max_len)
{
    size_t index = 0;
    
    while (index < max_len - 1) {
        int c = getchar();

        if (c == '\r' || c == '\n') {
            if (index > 0) { 
                break;
            }
            continue; 
        }

        if (c == '\b' || c == 127) {
            if (index > 0) {
                index--;
                printf("\b \b"); 
                fflush(stdout);
            }
            continue;
        }

        if (c < 32 || c > 126) {
            continue;
        }

        buffer[index++] = (char)c;
        putchar(c); 
        fflush(stdout);
    }

    buffer[index] = '\0';
    printf("\r\n");
}

void parse_command(char *buffer, uint32_t str_len)
{
    if(buffer == NULL)
        return;
    if(str_len >= CLI_MAX_LINE_LENGTH)
        return;
    
    // Tokenize user input
    char *saveptr;
    uint32_t token_count = 0;
    char *token = strtok_r(buffer, " \r\n", &saveptr);
    while (token != NULL && token_count < CLI_MAX_TOKENS)
    {
        input_tokenized[token_count++] = token;
        token = strtok_r(NULL, " \r\n", &saveptr); // Tokenization is looking for " " or "\r" or "\n"
    }

    if (token_count == 0)
        return;

    // Check command type and save arguments
    int inst = 0;
    if(strcmp(input_tokenized[0], "write_single") == 0)
    {
        bool instance_provided = false;
        bool channel_provided = false;

        for(uint32_t i = 1; i < token_count; i++)
        {
            if(strcmp(input_tokenized[i], "--inst") == 0)
            {
                if((i + 1) >= token_count)
                {
                    parsed_cmd.status = false;
                    break;
                }

                inst = atoi(input_tokenized[i + 1]);

                if(inst >= 0 && inst <= 2)
                {
                    parsed_cmd.dac_instance = inst;
                    instance_provided = true;
                }
                else
                {
                    parsed_cmd.status = false;
                    break;
                }

                i++;
            }
            else if(strcmp(input_tokenized[i], "--ch") == 0)
            {
                if((i + 2) >= token_count)
                {
                    parsed_cmd.status = false;
                    break;
                }

                if(channel_provided)
                {
                    parsed_cmd.status = false;
                    break;
                }

                char *ch_letter = input_tokenized[i + 1];
                char *val_str = input_tokenized[i + 2];

                if(strlen(ch_letter) != 1)
                {
                    parsed_cmd.status = false;
                    break;
                }

                int32_t ch_idx = -1;

                if(ch_letter[0] >= 'a' && ch_letter[0] <= 'd')
                    ch_idx = ch_letter[0] - 'a';
                else if(ch_letter[0] >= 'A' && ch_letter[0] <= 'D')
                    ch_idx = ch_letter[0] - 'A';

                if(ch_idx < 0 || ch_idx >= DAC_CHANNELS_COUNT)
                {
                    parsed_cmd.status = false;
                    break;
                }

                char *endptr;
                unsigned long value = strtoul(val_str, &endptr, 16);

                if(*endptr != '\0' || value > sizeof(uint16_t))
                {
                    parsed_cmd.status = false;
                    break;
                }

                parsed_cmd.channel_values[ch_idx] = value;
                parsed_cmd.channel_to_update[ch_idx] = true;
                channel_provided = true;

                i += 2;
            }
            else
            {
                parsed_cmd.status = false;
                break;
            }
        }

        if(!instance_provided)
            parsed_cmd.status = false;

        if(!channel_provided)
            parsed_cmd.status = false;

        parsed_cmd.command = CMD_WRITE_SINGLE;
    }
        
    else if(strcmp(input_tokenized[0], "write_all") == 0)
    {
        bool instance_provided = false;
        bool channel_provided = false;

        for(uint32_t i = 1; i < token_count; i++)
        {
            if(strcmp(input_tokenized[i], "--inst") == 0)
            {
                if((i + 1) >= token_count)
                {
                    parsed_cmd.status = false;
                    break;
                }

                int inst = atoi(input_tokenized[i + 1]);

                if(inst >= 0 && inst <= 2)
                {
                    parsed_cmd.dac_instance = inst;
                    instance_provided = true;
                }
                else
                {
                    parsed_cmd.status = false;
                    break;
                }

                i++;
            }
            else if(strcmp(input_tokenized[i], "--ch") == 0)
            {
                if((i + 2) >= token_count)
                {
                    parsed_cmd.status = false;
                    break;
                }

                char *ch_letter = input_tokenized[i + 1];
                char *val_str = input_tokenized[i + 2];

                if(strlen(ch_letter) != 1)
                {
                    parsed_cmd.status = false;
                    break;
                }

                int32_t ch_idx = -1;

                if(ch_letter[0] >= 'a' && ch_letter[0] <= 'd')
                    ch_idx = ch_letter[0] - 'a';
                else if(ch_letter[0] >= 'A' && ch_letter[0] <= 'D')
                    ch_idx = ch_letter[0] - 'A';

                if(ch_idx < 0 || ch_idx >= DAC_CHANNELS_COUNT)
                {
                    parsed_cmd.status = false;
                    break;
                }

                if(parsed_cmd.channel_to_update[ch_idx])
                {
                    parsed_cmd.status = false;
                    break;
                }

                char *endptr;
                unsigned long value = strtoul(val_str, &endptr, 16);

                if(*endptr != '\0' || value > UINT16_MAX)
                {
                    parsed_cmd.status = false;
                    break;
                }

                parsed_cmd.channel_values[ch_idx] = value;
                parsed_cmd.channel_to_update[ch_idx] = true;

                channel_provided = true;

                i += 2;
            }
            else
            {
                parsed_cmd.status = false;
                break;
            }
        }

        if(!instance_provided)
            parsed_cmd.status = false;

        if(!channel_provided)
            parsed_cmd.status = false;

        parsed_cmd.command = CMD_WRITE_ALL;
    }

    else if (strcmp(input_tokenized[0], "help") == 0 || strcmp(input_tokenized[0],  "?") == 0 || strcmp(input_tokenized[0], "h") == 0)
    {
      parsed_cmd.command = CMD_HELP;
    }

    else if (strcmp(input_tokenized[0], "debug") == 0)
    {   
        if (strcmp(input_tokenized[1], "0") == 0)
        {
            parsed_cmd.command = CMD_DEBUG_OFF;
            parsed_cmd.debug_enable= false;
        }
        else if(strcmp(input_tokenized[1], "1") == 0)
        {
            parsed_cmd.command = CMD_DEBUG_ON;
            parsed_cmd.debug_enable= false;
        }
    }

    else if(strcmp(input_tokenized[0], "read_config") == 0)
    {
        parsed_cmd.command = CMD_READ_CONFIG;
    }
    
    else 
    {
        parsed_cmd.command = CMD_UNKNOWN;
    }
}

bool execute_command(parsed_cmd_t *cmd)
{
    bool status = false;
    switch(cmd->command)
    {
        case CMD_UNKNOWN:
            printf("Undefined command, try again\n");
            return false;
        break;

        case CMD_WRITE_SINGLE:
            for(uint8_t i = 0; i < DAC_CHANNELS_COUNT; i++)
            {
                if(cmd->channel_to_update[i])
                {
                    status = dac_write_single( cmd->dac_instance, i, cmd->channel_values[i]);
                    break;
                }
            }
        break;

        case (CMD_WRITE_ALL):
            status = dac_write_all(cmd->dac_instance,
                                   cmd->channel_values[0],
                                   cmd->channel_values[1],
                                   cmd->channel_values[2],
                                   cmd->channel_values[3]);
        break;

        case (CMD_DEBUG_ON):
            // Debug flag already settled 
        break;

        case (CMD_DEBUG_OFF):
            // Debug flag already settled 
        break;

        case (CMD_READ_CONFIG):
        break;

        case (CMD_HELP):
            printf("DAC info\n");
            static dac_state_t state;
            for (uint8_t i = 0; i < DAC_INSTANCE_COUNT; i++)
            {
                dac_get_state(&state, i);
                printf("DAC instance: %d, channel A value: %d\n", i, state.ch_a_val);
                printf("DAC instance: %d, channel B value: %d\n", i, state.ch_b_val);
                printf("DAC instance: %d, channel C value: %d\n", i, state.ch_c_val);
                printf("DAC instance: %d, channel D value: %d\n", i, state.ch_d_val);
                printf("=============-+-=============\n");
            }
        break;

        default:
            printf("Undefined command try again\n");

    }
    return status;
}

void cli_run(void)
{
    // CLI main loop
    while(true)
    {
        printf("DAC> ");
        fflush(stdout);
        
        read_line(input_buf, CLI_MAX_LINE_LENGTH);      // Wait for user input 
        parse_command(input_buf, strlen(input_buf));  // Parse user input
        execute_command(&parsed_cmd);                   // Validate and update hardware with new user input
    }
}