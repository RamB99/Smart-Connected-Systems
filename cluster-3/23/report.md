# Skill 23: Accelerometer

Author: Raghurama Bukkarayasamudram

## Date: 2020-10-23

## Summary

In this skill, I wired the accelerometer to ESP32 using SDA and SCL. Created function that read acceleration values and converted to roll and pitch.

The conversion function used to transform the roll and pitch values are as follows: roll = atan2(y,z)_57.3
pitch = atan2((-1 _ x) , sqrt(y _ y + z _ z)) \* 57.3

## Video

[![](http://img.youtube.com/vi/sk_7ht5dsJw/0.jpg)](http://www.youtube.com/watch?v=sk_7ht5dsJw "Skill 23: Accelerometer")

## Modules, Tools, Source Used Including Attribution

- BU-EC444/code-examples/i2c-accel/

## Supporting Artifacts

---
