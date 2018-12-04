# ledserver

## Build:
cd src
make

## Run server:
./ledserver

## Run client in separate terminal window:
### Turn on|off led:
./led.sh set-led-state on|off
### Set led color:
./led.sh set-led-color red|green|blue
### Set led blink rate:
./led.sh set-led-rate 0|1|2|3|4|5
### Get led status:
./led.sh get-led-state

./led.sh get-led-color

./led.sh get-led-rate

### Shutdown led server:
./led.sh exit

