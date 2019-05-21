# The TBS CROSSFIRE RX

## Basics
Betaflight 4.0
Only Rssi by percentage using Aux Channel

Betaflight 4.1
Direct RX support without the need to use an Aux channel.
- Link Quality 0-300 : TODO define lix quality and modes here
-300 -200 mode 2
-199-100 mode 1
-99-1 mode 0
Support for alarms and stats.
- Rssi Dbm
Now the actual meaning of the 'RSSI dBm' value that is returned by Crossfire equipment is that it is the negative of the actual dBm value of the input signal, so it goes from 0 (1 mW) down to -130 (10^-13 mW). So in OSD, what we display is the negative value.
Support for alarms and stats.
- Rssi by percentage
See link xxx
## The TBS CROSSFIRE RX

- Crossfire link statitics



## Configuration

1. in Betaflight configurator, go to the 'Configuration' tab, in 'Receiver' box, select 'SPI RX support' for 'Receiver mode':


## Tips & Tricks

- the RX bind information is stored in the following CLI parameters: `frsky_spi_tx_id` (internal TX id), `frsky_spi_offset` (frequency offset), `frsky_spi_bind_hop_data` (frequency hop sequence), and `frsky_x_rx_num` (RX number; FrSky X only). These are printed as part of a CLI `diff` / `dump`, and will be restored after a firmware update, making it unnecessary to do another bind after the update;
- resetting the above parameters to defaults will 'erase' the binding information;
- the CLI parameter `frsky_spi_autobind` can be enabled to configure the FrSky SPI RX to attempt a bind on every power up. This is mostly useful for demonstration models that should bind to whatever TX is powered up in the vicinity.


## Acknowledgements
