#include "cli.h"

static char input_buf[CLI_MAX_LINE_LENGTH];  //This variables should be private
static char *input_tokenized[CLI_MAX_TOKENS]; //This variables should be private
static parsed_cmd_t parsed_cmd = {0}; // Stay public static

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
    uint8_t inst = 0;
    if (strcmp(input_tokenized[0], "write_single") == 0)
    {
        for (uint32_t i = 1; i < token_count; i++)
        {
            if (strcmp(input_tokenized[i], "--inst") == 0 && (i + 1) < token_count)
            {
                char *val_str = input_tokenized[i + 1];
                inst = atoi(val_str);

                if (inst >= 0 && inst <= 2)
                    parsed_cmd.dac_instance = inst;
                else
                    parsed_cmd.status = false;
                
                i++;
            }
            
            else if (strcmp(input_tokenized[i], "--ch") == 0 && (i + 2) < token_count)
            {
                char *ch_letter = input_tokenized[i + 1];
                char *val_str = input_tokenized[i + 2];
                int32_t ch_idx = -1;

                if (ch_letter[0] >= 'a' && ch_letter[0] <= 'd')
                    ch_idx = ch_letter[0] - 'a';
                else if (ch_letter[0] >= 'A' && ch_letter[0] <= 'D')
                    ch_idx = ch_letter[0] - 'A';

                if (ch_idx >= 0 && ch_idx < DAC_CHANNELS_COUNT)
                {
                    parsed_cmd.channel_values[ch_idx] = strtoul(val_str, NULL, 16);
                    parsed_cmd.channel_to_update[ch_idx] = true;
                }
                else
                    parsed_cmd.status = false;
            

                i += 2; 
            }
        }

        parsed_cmd.selected_cmd = CMD_WRITE_SINGLE;
    }
    
    else if (strcmp(input_tokenized[0], "write_all") == 0)
    {
        for (uint32_t i = 1; i < token_count; i++)
        {
            if(strcmp(input_tokenized[i], "--inst") == 0 && (i + 1) < token_count) 
            {
                char *val_str = input_tokenized[i + 1];
                uint8_t inst = (uint8_t)atoi(val_str);

                if (inst == 0)
                    parsed_cmd.dac_instance = DAC_INST_0;
                else if (inst == 1) 
                    parsed_cmd.dac_instance = DAC_INST_1;
                else if (inst == 2) 
                    parsed_cmd.dac_instance = DAC_INST_2;
                else 
                    parsed_cmd.status = false;

                i += 1;
            }

            else if (strcmp(input_tokenized[i], "--ch") == 0 && (i + 2) < token_count)
            {
                char *ch_letter = input_tokenized[i + 1];
                char *val_str = input_tokenized[i + 2];
                int32_t ch_idx = -1;

                if (ch_letter[0] >= 'a' && ch_letter[0] <= 'd')
                    ch_idx = ch_letter[0] - 'a';

                else if (ch_letter[0] >= 'A' && ch_letter[0] <= 'D')
                    ch_idx = ch_letter[0] - 'A';

                if (ch_idx >= 0 && ch_idx < DAC_CHANNELS_COUNT)
                {
                    parsed_cmd.channel_values[ch_idx] = strtoul(val_str, NULL, 16);
                    parsed_cmd.channel_to_update[ch_idx] = true;
                }
                else
                {
                    parsed_cmd.status = false;
                }

                i += 2;
            }
        }

        parsed_cmd.selected_cmd = CMD_WRITE_SINGLE;
    }






    else if (strcmp(input_tokenized[0], "help") == 0 || strcmp(input_tokenized[0],  "?") == 0 || strcmp(input_tokenized[0], "h") == 0)
    {
      parsed_cmd.selected_cmd = CMD_HELP;
    }

    else if (strcmp(input_tokenized[0], "debug") == 0)
    {   
        if (strcmp(input_tokenized[1], "0") == 0)
        {
            parsed_cmd.selected_cmd = CMD_DEBUG_OFF;
            parsed_cmd.debug_mode= false;
        }
        else if(strcmp(input_tokenized[1], "1") == 0)
        {
            parsed_cmd.selected_cmd = CMD_DEBUG_ON;
            parsed_cmd.debug_mode= false;
        }
    }

    else if(strcmp(input_tokenized[0], "read_config") == 0)
    {
        parsed_cmd.selected_cmd = CMD_READ_CONFIG;
    }
    
    else 
    {
        parsed_cmd.selected_cmd = CMD_UNKNOWN;
    }
}


bool execute_command(parsed_cmd_t *cmd)
{
    switch(parsed_cmd.selected_cmd)
    {
        case (CMD_UNKNOWN):
            printf("Undefined command try again\n");
        break;

        case (CMD_WRITE_SINGLE):
        break;

        case (CMD_WRITE_ALL):
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
    
    
}

void cli_run(void)
{
    // CLI main loop
    while(true)
    {
        read_line(input_buf, CLI_MAX_LINE_LENGTH);      // Wait for user input 
        parse_command(input_buf, strlen(input_buf));  // Parse user input
        execute_command(&parsed_cmd);                   // Validate and update hardware with new user input
    }
}