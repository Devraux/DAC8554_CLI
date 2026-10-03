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
    size_t length = 0;
    size_t cursor_pos = 0;
    static char last_command[CLI_MAX_LINE_LENGTH];

    while(length < max_len - 1)
    {
        int c = getchar();

        if(c == '\r' || c == '\n')
            break;

        // Escape sequence
        if(c == 27)
        {
            int c1 = getchar();
            if(c1 == '[')
            {
                int c2 = getchar();

                switch(c2)
                {
                    case 'A':   // Up arrow
                    {
                        if(last_command[0] != '\0')
                        {
                            // Clear current line
                            printf("\r\033[K");
                            printf("DAC> ");

                            strcpy(buffer, last_command);

                            length = strlen(buffer);
                            cursor_pos = length;

                            printf("%s", buffer);
                            fflush(stdout);
                        }
                    }
                    break;

                    case 'D':   // Left arrow
                        if(cursor_pos > 0)
                        {
                            cursor_pos--;
                            printf("\033[D");
                            fflush(stdout);
                        }
                        break;

                    case 'C':   // Right arrow
                        if(cursor_pos < length)
                        {
                            cursor_pos++;
                            printf("\033[C");
                            fflush(stdout);
                        }
                        break;

                    case 'H':   // Home
                        while(cursor_pos > 0)
                        {
                            cursor_pos--;
                            printf("\033[D");
                        }
                        fflush(stdout);
                        break;

                    case 'F':   // End
                        while(cursor_pos < length)
                        {
                            cursor_pos++;
                            printf("\033[C");
                        }
                        fflush(stdout);
                        break;

                    case '3':   // Delete: ESC [ 3 ~
                    {
                        int c3 = getchar();

                        if(c3 == '~' && cursor_pos < length)
                        {
                            memmove(&buffer[cursor_pos],
                                    &buffer[cursor_pos + 1],
                                    length - cursor_pos);

                            length--;

                            printf("\033[K");

                            for(size_t i = cursor_pos; i < length; i++)
                            {
                                putchar(buffer[i]);
                            }

                            printf("\033[%zuD", length - cursor_pos);
                            fflush(stdout);
                        }
                    }
                    break;

                    default:
                        break;
                }
            }

            continue;
        }

        //Backspace
        if(c == '\b' || c == 127)
        {
            if(cursor_pos > 0)
            {
                memmove(&buffer[cursor_pos - 1],
                        &buffer[cursor_pos],
                        length - cursor_pos);

                cursor_pos--;
                length--;

                printf("\b");

                for(size_t i = cursor_pos; i < length; i++)
                {
                    putchar(buffer[i]);
                }

                putchar(' ');

                printf("\033[%zuD", length - cursor_pos + 1);

                fflush(stdout);
            }

            continue;
        }

        // Ignore non-printable characters
        if(c < 32 || c > 126)
        {
            continue;
        }

        // Insert character at cursor position
        if(length < max_len - 1)
        {
            memmove(&buffer[cursor_pos + 1],
                    &buffer[cursor_pos],
                    length - cursor_pos);

            buffer[cursor_pos] = (char)c;

            length++;
            cursor_pos++;

            for(size_t i = cursor_pos - 1; i < length; i++)
            {
                putchar(buffer[i]);
            }

            if(cursor_pos < length)
            {
                printf("\033[%zuD", length - cursor_pos);
            }

            fflush(stdout);
        }
    }

    buffer[length] = '\0';

    // Save command for history
    if(length > 0)
    {
        strcpy(last_command, buffer);
    }

    printf("\r\n");
}

void parse_command(char *buffer, uint32_t str_len)
{
    if(buffer == NULL)
        return;
    if(str_len >= CLI_MAX_LINE_LENGTH)
        return;
    
    // Clear previous commands
    parsed_cmd.command = CMD_UNKNOWN;
    parsed_cmd.dac_instance = DAC_INST_0;
    parsed_cmd.status = true;
    memset(parsed_cmd.channel_values, 0, sizeof(parsed_cmd.channel_values));
    memset(parsed_cmd.channel_to_update, false, sizeof(parsed_cmd.channel_to_update));


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

    else if(strcmp(input_tokenized[0], "zero") == 0)
    {
        if(token_count != 1)
        {
            parsed_cmd.status = false;
            parsed_cmd.command = CMD_UNKNOWN;
        }
        else
        {
            parsed_cmd.command = CMD_ZERO;
        }
    }

    else if (strcmp(input_tokenized[0], "help") == 0 || strcmp(input_tokenized[0],  "?") == 0 || strcmp(input_tokenized[0], "h") == 0)
    {
      parsed_cmd.command = CMD_HELP;
    }

    else if(strcmp(input_tokenized[0], "debug") == 0)
    {
        if(token_count < 2)
        {
            parsed_cmd.status = false;
            parsed_cmd.command = CMD_UNKNOWN;
        }
        else if(strcmp(input_tokenized[1], "0") == 0)
        {
            parsed_cmd.command = CMD_DEBUG_OFF;
            parsed_cmd.debug_enable = false;
        }
        else if(strcmp(input_tokenized[1], "1") == 0)
        {
            parsed_cmd.command = CMD_DEBUG_ON;
            parsed_cmd.debug_enable = true;
        }
        else
        {
            parsed_cmd.status = false;
            parsed_cmd.command = CMD_UNKNOWN;
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

        case (CMD_ZERO):
            status = dac_write_all(DAC_INST_0, 0, 0, 0, 0);
            status = dac_write_all(DAC_INST_1, 0, 0, 0, 0);
            status = dac_write_all(DAC_INST_2, 0, 0, 0, 0);
        break;

        case (CMD_DEBUG_ON):
            // Debug flag already settled 
            status = true;
        break;

        case (CMD_DEBUG_OFF):
            // Debug flag already settled 
            status = true;
        break;

        case (CMD_READ_CONFIG):
            printf("DAC info\n");
            static dac_state_t state;
            for (uint8_t i = 0; i < DAC_INSTANCE_COUNT; i++)
            {
                dac_get_state(&state, i);
                printf("=================================-++-=================================\n");
                printf("| DAC instance: %d, channel A value: 0x%04X (%5u), Vout: %.6f V |\n",
                        i, state.ch_a_val, state.ch_a_val, (state.ch_a_val / 65535.0f) * DAC_V_REF);

                printf("| DAC instance: %d, channel B value: 0x%04X (%5u), Vout: %.6f V |\n",
                        i, state.ch_b_val, state.ch_b_val, (state.ch_b_val / 65535.0f) * DAC_V_REF);

                printf("| DAC instance: %d, channel C value: 0x%04X (%5u), Vout: %.6f V |\n",
                        i, state.ch_c_val, state.ch_c_val, (state.ch_c_val / 65535.0f) * DAC_V_REF);

                printf("| DAC instance: %d, channel D value: 0x%04X (%5u), Vout: %.6f V |\n",
                        i, state.ch_d_val, state.ch_d_val, (state.ch_d_val / 65535.0f) * DAC_V_REF);
                printf("=================================-++-=================================\n");
            }
            status = true;
        break;

        case CMD_HELP:
            printf("\r\n");
            printf("Available commands:\r\n");
            printf("\r\n");

            printf("  write_single --inst <0-2> --ch <A-D> <value>\r\n");
            printf("      Write a value to a single DAC channel.\r\n");
            printf("      Example: write_single --inst 2 --ch A 0x1234\r\n");
            printf("\r\n");

            printf("  write_all --inst <0-2> <A-D> <value> ...\r\n");
            printf("      Write values to all DAC channels.\r\n");
            printf("      Example: write_all --inst 1 --ch A 0x1234 --ch B 0x5678 --ch C 0x9ABC --ch D 0xDEF0\r\n");
            printf("\r\n");

            printf("  zero\r\n");
            printf("      Set all DAC outputs to zero.\r\n");
            printf("      Example: zero\r\n");
            printf("\r\n");

            printf("  debug <0|1>\r\n");
            printf("      Disable or enable debug mode.\r\n");
            printf("      Example: debug 1\r\n");
            printf("\r\n");

            printf("  read_config\r\n");
            printf("      Read current DAC configuration.\r\n");
            printf("\r\n");

            printf("  help | h | ?\r\n");
            printf("      Display this help message.\r\n");
            printf("\r\n");
            status = true;
        break;

        default:
            printf("Undefined command try again\n");

    }

    if(cmd->debug_enable)
    {
        if(status)
            printf("OK\n");
        else
            printf("Error\n");
    }


    return status;
}

void cli_run(void)
{
    while(true)
    {
        printf("DAC> ");
        fflush(stdout);

        read_line(input_buf, CLI_MAX_LINE_LENGTH);

        if(input_buf[0] == '\0')
            continue;
    
        parse_command(input_buf, strlen(input_buf));

        if(parsed_cmd.status)
            execute_command(&parsed_cmd);
        else
            printf("Invalid command\r\n");
    }
}