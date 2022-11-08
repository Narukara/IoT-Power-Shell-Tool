### IoT-Power shell tool for Linux

#### Usage

```
./iotpst -p port [-o filename]
```

#### Sample output

```
Connected to /dev/ttyUSB0
Waiting for initialization...
Output OFF
Voltage = 4.990V
Current = 0.100A
Collecting data... press Ctrl+C to stop
^C
Done
```

#### Notice

1. `iotpst` needs to be in the same directory as `libiot_parser.so`.
2. Initialization often gets stuck.
