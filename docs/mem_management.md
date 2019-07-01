# Memory Management in NimBLE

## Scope
For its first draft, this document focuses on running/configuring NimBLE for
running on top of RIOT on nRF-targets using both the host and controller.

## Overview: Related Configuration Options

## General Memory Management Concept
?


## Scenario: Beacon (incl. scan responses)
Assumption:
- no extended advertisements
- no data packet length extension used

Traffic pattern:
- advertising only

Buffers used:
- internal msys buffer

Min buffer size needed per packet: 75 bytes
- 24 bytes `mbuf` overhead: `sizeof(struct os_mbuf) + sizeof(struct os_mbuf_pkthdr)`
- 12 bytes controller overhead: `sizeof(struct ble_mbuf_hdr)`
- 39 bytes BLE MTU (2 bytes link layer header + 37 bytes MTU)

Min number of buffers needed: 2?
- 1 for advertising packet
- 1 for scan response packet


## Scenario: GATT Server
Assumption:
- no extended advertisements
- no data packet length extension used
- only a single connection allowed

Traffic pattern:
- advertising
XOR
- incoming GATT requests
- outgoing GATT responses

Buffers used:
- internal msys buffer

Min buffers size needed per packet:
- when advertising `75` bytes, see scenario above
- when connected (GATT traffic): `75` bytes
    -


## Scenario: IP-over-BLE (L2CAP COC)


Incoming packets:
- L2CAP fragments are written into msys buffer
- once packet is complete,


Example configuration:
- IPv6 MTU 1280 bytes, L2CAP MTU: 250 bytes
per connection:


- 3 concurrent connections,

min:
-
