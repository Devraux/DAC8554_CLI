# DAC CLI

A simple CLI for controlling three TI DAC8554 DACs connected to a Raspberry Pi Pico.

## Commands

### Write single channel

Writes a value to one selected DAC channel:

```text
write_single --inst <0-2> --ch <A-D> <value>
```

Example:

```text
write_single --inst 2 --ch A 0x1234
```

### Write all channels

Writes values to all four channels of the selected DAC:

```text
write_all --inst <0-2> --ch <A-D> <value> --ch <A-D> <value> --ch <A-D> <value> --ch <A-D> <value>
```

Example:

```text
write_all --inst 1 --ch A 0x1234 --ch B 0x5678 --ch C 0x9ABC --ch D 0x9994
```

All four channels must be specified. Duplicate channels are not allowed.

### Zero all DACs

Sets all channels of all DAC instances to zero:

```text
zero
```

### Read configuration

Displays the values stored in the Raspberry Pi Pico software state for all DAC instances and channels:

```text
read_config
```

Example output:

```text
=================================-++-=================================
| DAC instance: 0, channel A value: 0x1234 ( 4660), Vout: 0.145627 V |
| DAC instance: 0, channel B value: 0x5678 (22136), Vout: 0.691761 V |
| DAC instance: 0, channel C value: 0x9ABC (39612), Vout: 1.237894 V |
| DAC instance: 0, channel D value: 0x9994 (39316), Vout: 1.228644 V |
=================================-++-=================================
=================================-++-=================================
| DAC instance: 1, channel A value: 0x1234 ( 4660), Vout: 0.145627 V |
| DAC instance: 1, channel B value: 0x5678 (22136), Vout: 0.691761 V |
| DAC instance: 1, channel C value: 0x9ABC (39612), Vout: 1.237894 V |
| DAC instance: 1, channel D value: 0x9994 (39316), Vout: 1.228644 V |
=================================-++-=================================
=================================-++-=================================
| DAC instance: 2, channel A value: 0x1234 ( 4660), Vout: 0.145627 V |
| DAC instance: 2, channel B value: 0x5678 (22136), Vout: 0.691761 V |
| DAC instance: 2, channel C value: 0x9ABC (39612), Vout: 1.237894 V |
| DAC instance: 2, channel D value: 0x9994 (39316), Vout: 1.228644 V |
=================================-++-=================================
```

`read_config` is a convenient way to verify the current DAC configuration. The displayed values are based on data stored in the Raspberry Pi Pico software state.

**Important:** `read_config` does not read the values from the DAC8554. The DAC8554 does not provide a readback mechanism for these values. Therefore, the command shows the values that were last written by the Pico.

The `Vout` value is calculated from the stored DAC code using the configured reference voltage.

### Debug

Enable or disable command status messages:

```text
debug 1
debug 0
```

When enabled, successful commands print:

```text
OK
```

and failed commands print:

```text
Error
```

### Help

Display available commands:

```text
help
h
?
```

## Case-insensitive commands

Commands, options and channel names are case-insensitive.

For example:

```text
WRITE_SINGLE --INST 0 --CH A 0x1234
```

is equivalent to:

```text
write_single --inst 0 --ch a 0x1234
```
