# Everloop

The Everloop driver controls the LED array of the MATRIX Creator.
It follows the [MALOS protocol](#protocol).

### 0MQ Port
```
20021
```

### Protocol buffers

```
message EverloopImage {
  repeated LedValue led = 1;
}
```

The message `EverloopImage` needs to have exactly 35 messages of type `LedValue` in the repeated field `led`,
corresponding to each of the LEDs present in the Creator. The LEDs are counted starting from the left, clock-wise
as shown in the picture.

![Everloop leds](creator-front-everloop-leds.png)

The message LedValue holds the value for each LED.

```
message LedValue {
  uint32 red = 1;
  uint32 green = 2;
  uint32 blue = 3;
  uint32 white = 4;
}
```


### Keep-alives

This driver doesn't need keepalive messages as it doesn't send data to the subscribed programs.

### Errors

### Read

### Write

### JS example
