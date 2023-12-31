[![PlatformIO CI](https://github.com/matthewturner/massive-clock/actions/workflows/platformio.yml/badge.svg)](https://github.com/matthewturner/massive-clock/actions/workflows/platformio.yml)

# Massive Clock Synchronizer

Synchronizes the time of the massive clock with NTP time servers.

## Commands

Issue the `request-sync` command to update time, retrieve the latest schedule and push to the clock:

`>request-sync!`

## Installing ESP8266 driver

[Download](https://sparks.gogo.co.nz/ch340.html)

## Installing Platform IO

Install command line tools by following the installation instructions for [Windows](https://docs.platformio.org/en/latest/core/installation.html#windows)

## Serial Monitor

Deploy code to your Arduino and run the following command in a terminal:

```powershell
 pio device monitor --eol=CRLF --echo --filter=send_on_enter
```

## Unit Testing

Run the following command after installing Platform IO:

```powershell
pio test -e native
```
