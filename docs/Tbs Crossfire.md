# The TBS CROSSFIRE RX

## Basics
Betaflight 4.0
Only Rssi by percentage using Aux Channel

Betaflight 4.1
Direct Crossfire RX support without the need to use an Aux channel.

- Link Quality, based on the percentage of signal data received at the end-point [0 to 300%] CROSSFIRE runs different RF Profiles at time.  Any LQ above 80 is fine. 

  | 300-200 | MODE | HZ   | Note |
  | ------- | ---- | ---- | ---- |
  | 300-200 | 2    | 150  |      |
  | 199-100 | 1    | 50   |      |
  | 99-1    | 0    | 4    |      |

  

  Alarms :  min value to blink and display LQ LOW*    

  Stats: Display min value from last arm.

  Debug mode :  CRSF_LINK_STATISTICS_UPLINK - (2) uplink LQ - (3) RF mode  

- Rssi Dbm  Active antenna's Rssi . The value that is returned by Crossfire equipment is that it is the negative of the actual dBm value of the input signal, so it goes from 0 (1 mW) down to -130 (10^-13 mW)  in the OSD display is the negative value.

  

  Alarms :  min value to blink and display LQ LOW*    

  Stats: Display min value from last arm.

  Debug mode :  CRSF_LINK_STATISTICS_UPLINK - (0) Rssi 1 (1) Rssi 2

- Rssi by percentage *not recommended 
  See link xxx



## Configuration 4.1 FOR Direct Crossfire RX support:

1. in Betaflight configurator, go to the 'Configuration' tab, in 'Receiver' box, select 'crsf' for 'Receiver mode':

2. Enable  osd elements 

3.  Set alarm value for LQ - 80 

4. optional enable statistics .

   

    


## Tips & Tricks

- Direct Crossfire RX support:    rssi or rssi channel (todo pic) **should NOT be set.**

  


## Acknowledgements
