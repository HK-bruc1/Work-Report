# MK SD NAND Product Datasheet

**Product List**
- MKDV1GCL-ABF
- MKDV2GCL-ABF
- MKDV4GCL-ABF
- MKDV8GCL-ABF

- WebSite: www.mkfounder.com
- FAE support: Support@mkfounder.com

---

## Revision History

| Version | Date       | Description |
|---------|------------|-------------|
| Rev 1.0 | 2023/12/12 | Original version |
| Rev 2.0 | 2024/01/15 | The description of the Consumption in Part 6.3 of the original specification is incorrect and has been modified |
| Rev 2.1 | 2025/06/20 | The operating temperature range has been changed from -20°C to 70°C to -25°C to 85°C |

---

## Notice

The datasheet is prepared and approved by MK Founder semiconductor co., LTD.
MK Founder reserves the right to change products or specifications without notice.

© 2024 MK Founder semiconductor co., LTD. All rights reserved.

---

## Table Of Contents

1. Product List
2. Features
3. Block Diagram
4. Physical Characteristics
   - 4.1 Temperature
5. Pin Assignments (SD Mode & SPI Mode)
6. Usage
   - 6.1 SD Bus Mode protocol
   - 6.2 Card Initialize
   - 6.3 Electrical Characteristics
7. Internal Information
   - 7.1 Registers
   - 7.1.1 OCR Register
   - 7.1.2 CID Register
   - 7.1.3 CSD Register
   - 7.1.4 RCA Register
   - 7.1.5 DSR Register
8. Power Scheme
   - 8.1 Power Up
   - 8.2 Power Up Time
   - 8.2.1 Power On or Power Cycle
   - 8.2.2 Power Supply Ramp Up
   - 8.2.3 Power Supply Ramp Up
9. Part Numbering
10. Package Dimensions
11. Reference Design

---

## 1. Product List

| Part Number   | Flash Type | Capacity | Actual Capacity | Read/Write (MByte/s) | Package     |
|---------------|-----------|----------|-----------------|----------------------|-------------|
| MKDV1GCL-ABF  | SLC       | 1Gbit    | 118MByte        | 13/4                 | LGA-8(6×8)  |
| MKDV2GCL-ABF  | SLC       | 2Gbit    | 238MByte        | 17/9                 | LGA-8(6×8)  |
| MKDV4GCL-ABF  | SLC       | 4Gbit    | 481MByte        | 19/9                 | LGA-8(6×8)  |
| MKDV8GCL-ABF  | SLC       | 8Gbit    | 964MByte        | 19/10                | LGA-8(6×8)  |

**Note:**
1. The SD NAND mode was four-wire during the test.
2. The test ambient temperature was room temperature 25°C.

---

## 2. Features

- Support up to 50 MHz clock frequency
- Supports SPI Mode
- Built-in HW ECC Engine and highly reliable NAND management mechanism
- Smaller package LGA-8
- Advanced thermal management features to maximize performance and data protection at extended temperatures
- Static, dynamic, and global wear leveling
- Bad block management, intelligent garbage collection and support for interleaving, cache, and multi-plane programming
- Read disturb management and dynamic data refresh
- Best-in-class power fail management
- Low power consumption, cost-effective solution
- Industrial grade temperature range: -25°C ~ +85°C
- Program/Erase: 60,000 cycles

---

## 3. Block Diagram

MK SD NAND is an embedded storage solution designed in the form of an LGA package that operates similarly to industry standard SD cards. SD NAND consists of NAND flash memory and a high-performance controller that supports ultra-High speed mode (UHS-I) and is fully compliant with the SD2.0 interface. With superior read/write performance, reliability, and host compatibility, MK SD NAND is designed to provide a high-performance, low-power, cost-effective, and value-added solution for SD-oriented applications.

---

## 4. Physical Characteristics

### 4.1 Temperature

**1) Operation Conditions**
- Temperature Range: -25°C to +85°C

**2) Storage Conditions**
- Temperature Range: -25°C to +85°C

---

## 5. Pin Assignments (SD Mode & SPI Mode)

*(TOP VIEW — Pin assignments diagram)*

**Note:**
- a. Type Key: S = power supply; I = input; O = output using push-pull drivers; PP = I/O using push-pull drivers.
- b. The extended DAT lines (DAT1-DAT3) are input on power up. They start to operate as DAT lines after the SET_BUS_WIDTH. Type Key: S = power supply; I = input; O = output using push-pull drivers; PP = I/O using push-pull drivers.
- c. At power up this line has a 50 kilohm pull-up enabled in the card. This resistor serves two functions: Card detection and Mode Selection. For Mode Selection, the host can drive the line high or let it be pulled high to select SD mode. If the host wants to select SPI mode it should drive the line low. For Card detection, the host detects that the line is pulled high. This pull-up should be disconnected by the user during regular data transfer, with SET_CLR_CARD_DETECT (ACMD42) command.

---

## 6. Usage

### 6.1 SD Bus Mode protocol

The SD bus allows the dynamic configuration of the number of data line from 1 to 4 Bi-directional data signal. After power up by default, the SD card will use only DAT0. After initialization, host can change the bus width.

Multiplied SD cards connections are available to the host. Common VDD, VSS and CLK signal connections are available in the multiple connections. However, Command, Respond and Data lines (DAT0-DAT3) shall be divided for each device from host.

This feature allows easy trade off between hardware cost and system performance. Communication over the SD bus is based on command and data bit stream initiated by a start bit and terminated by stop bit.

**Command**

Commands are transferred serially on the CMD line. A command is a token to starts an operation from host to the device. Commands are sent to an addressed single card (addressed Command) or to all connected cards (Broadcast command).

**Response**

Responses are transferred serially on the CMD line. A response is a token to answer to a previous received command. Responses are sent from an addressed single card or from all connected cards.

**Data**

Data can be transfer from the card to the host or vice versa. Data is transferred via the data lines.

**SD NAND (A)**

| Signal  | Description                          |
|---------|--------------------------------------|
| CLK     | Host card Clock signal               |
| CMD     | Bi-directional Command/Response Signal |
| DAT0-DAT3 | 4 Bi-directional data signal       |
| VDD     | Power supply                         |
| VSS     | GND                                  |

### 6.2 Card Initialize

To initialize the SD NAND, follow the following procedure is recommended example.

**1) Supply Voltage for initialization**

Host System can apply the Operating Voltage from initialization to the card. Apply more than 74 cycles of Dummy-clock to the SD card.

**2) Select operation mode (SD mode or SPI mode)**

In case of SPI mode operation, host should drive 1 pin (CD/DAT3) of SD Card I/F to "Low" level. Then, issue CMD0. In case of SD mode operation, host should drive or detect 1 pin of SD Card I/F (Pull up register of 1 pin is pull up to "High" normally).

Card maintain selected operation mode except re-issue of CMD0 or power on below is SD mode initialization procedure.

**3)** Send the ACMD41 with Arg = 0 and identify the operating voltage range of the Card.

**4)** Apply the indicated operating voltage to the card.

Reissue ACMD41 with apply voltage storing and repeat ACMD41 until the busy bit is cleared. (Bit 31 Busy = 1) If response timeout occurred, host can recognize not SD Card.

**5)** Issue the CMD2 and get the Card ID (CID).

**6)** Issue the CMD3 and get the RCA. (RCA value is randomly changed by access, not equal zero)

**7)** Issue the CMD7 and move to the transfer state.

If necessary, Host may issue the ACMD42 and disabled the pull up resistor for Card detect.

**8)** Issue the ACMD13 and poll the Card status as SD Memory Card. Check SD_CARD_TYPE value. If significant 8 bits are "all zero", that means SD Card. If it is not, stop initialization.

**9)** Issue CMD7 and move to standby state. Issue CMD9 and get CSD. Issue CMD10 and get CID.

**10)** Back to the Transfer state with CMD7.

**11)** Issue ACMD6 and choose the appropriate bus-width.

Then the Host can access the Data between the SD card as a storage device.

**SPI Mode Initialization Flow**

*(Normal SD initial flow diagram)*

**SD card Initialize Procedure**

*(SD card initialization flow diagram)*

### 6.3 Electrical Characteristics

**Absolute Maximum Rating**

| Item                | Symbol | CONDITION     | MIN. | Typ. | MAX. | Unit |
|---------------------|--------|---------------|------|------|------|------|
| Supply Voltage      | VDD    | —             | 2.7  | 3.3  | 3.6  | V    |
| Input Leakage Current | IIL   | VIN = 0V to VDD | -10 | —   | 10  | μA   |
| Output Leakage Current | ILO  | VOUT = 0V to VDD | -10 | —   | 10  | μA   |
| Input Voltage Setup Time | Ts | VIN = 0V to VDD | 0   | —   | 250 | ms   |

**Current Consumption**

| Item           | Symbol | CONDITION  | MIN. | Typ. | MAX. | Unit |
|----------------|--------|------------|------|------|------|------|
| Read Current   | ICC01  | CLK = 50MHz | —   | —   | 25   | mA   |
| Write Current  | ICC02  | CLK = 50MHz | —   | —   | 35   | mA   |
| Standby Current | ICCS  | —           | —   | —   | 80   | μA   |

**Note:**
1. Full speed test
2. The SD NAND mode was four-wire during the test
3. The test ambient temperature was room temperature 25°C

---

## 7. Internal Information

### 7.1 Registers

The SD NAND has six registers and SD Status information: OCR, CID, CSD, RCA, DSR, SCR and SD Status.

DSR IS NOT SUPPORTED in this card.

There are two types of register groups.
- MMC compatible registers: OCR, CID, CSD, RCA, DSR, and SCR
- SD card Specific: SD Status

**SD card Registers**

| Register Name | Bit Width | Description |
|---------------|-----------|-------------|
| OCR           | 32        | Operation Conditions (VDU Voltage Profile and Busy Status) |
| CID           | 128       | Card Identification information |
| CSD           | 128       | Card specific information |
| RCA           | 16        | Relative Card Address |
| DSR           | 16        | Not Implemented (Programmable Card Driver): Driver Stage Register |
| SCR           | 64        | SD Memory Card's special features |
| SD Status     | 512       | Status bits and Card features |

#### 7.1.1 OCR Register

This 32-bit register describes operating voltage range and status bit in the power supply.

**OCR register definition**

| OCR bit | VDD voltage window | Initial |
|---------|-------------------|---------|
| 31      | Card power up status bit (busy) | "0" = busy |
| 30      | Card Capacity Status | "0" = SD Memory Card |
| 29-25   | reserved | All 0 |
| 24      | Switching to 1.8V Accepted (S18A) | 0 |
| 23      | 3.6 - 3.5 | 1 |
| 22      | 3.5 - 3.4 | 1 |
| 21      | 3.4 - 3.3 | 1 |
| 20      | 3.3 - 3.2 | 1 |
| 19      | 3.2 - 3.1 | 1 |
| 18      | 3.1 - 3.0 | 1 |
| 17      | 3.0 - 2.9 | 1 |
| 16      | 2.9 - 2.8 | 1 |
| 15      | 2.8 - 2.7 | 1 |
| 14      | Reserved | 0 |
| 13      | Reserved | 0 |
| 12      | Reserved | 0 |
| 11      | Reserved | 0 |
| 10      | Reserved | 0 |
| 9       | Reserved | 0 |
| 8       | Reserved | 0 |
| 7       | Reserved for Low Voltage Range | 0 |
| 6       | Reserved | 0 |
| 5       | Reserved | 0 |
| 4       | Reserved | 0 |
| 3-0     | reserved | All 0 |

bit 23-4: Describes the SD Card Voltage

bit 31 indicates the card power up status. Value "1" is set after power up and initialization procedure has been completed.

#### 7.1.2 CID Register

The CID (Card Identification) register is 128-bit width. It contains the card identification information. (Refer Appendix 3. for the detail)

The Value of CID Register is vendor specific.

**CID Register**

| Field    | Width | CID-slice  | Initial Value |
|----------|-------|------------|---------------|
| MID      | 8     | [127:120]  | 0xF2          |
| OID      | 16    | [119:104]  | 0x2345        |
| PNM      | 40    | [103:64]   | MK            |
| PRV      | 8     | [63:56]    | 0x06          |
| PSN      | 32    | [55:24]    | 150C0415      |
| reserved | 4     | [23:20]    | 0x0           |
| MDT      | 12    | [19:8]     | 0x21C         |
| CRC      | 7     | [7:1]      | CRC7          |
| reserved | 1     | [0:0]      | 0x1           |

**Note:**
1. Depends on the SD Card. Controlled by Production Lot.
2. Depends on the CID Register

#### 7.1.3 CSD Register

CSD is Card-Specific Data register provides information on 128 bit width. Some field of this register can writable by PROGRAM_CSD (CMD27).

**CSD Register**

| Field              | Width | CellType | CSD Slice   | Initial Value  |
|--------------------|-------|----------|-------------|----------------|
| CSD_STRUCTURE      | 2     | R        | [127:126]   | 00b            |
| reserved           | 6     | R        | [125:120]   | 000000b        |
| TAAC               | 8     | R        | [119:112]   | 01111111b      |
| NSAC               | 8     | R        | [111:104]   | 00000000b      |
| TRAN_SPEED         | 8     | R        | [103:96]    | 00110010b      |
| CCC                | 12    | R        | [95:84]     | 010100110101b  |
| READ_BL_LEN        | 4     | R        | [83:80]     | 1010b          |
| READ_BL_PARTIAL    | 1     | R        | [79:79]     | 1b             |
| WRITE_BLK_MISALIGN | 1     | R        | [78:78]     | 0b             |
| READ_BLK_MISALIGN  | 1     | R        | [77:77]     | 0b             |
| DSR_IMP            | 1     | R        | [76:76]     | 0b             |
| reserved           | 6     | R        | [75:70]     | 000000b        |
| C_SIZE             | 22    | R        | [69:48]     | xxxxxxxb       |
| reserved           | 1     | R        | [47:47]     | 1b             |
| ERASE_BLK_EN       | 1     | R        | [46:46]     | 1b             |
| SECTOR_SIZE        | 7     | R        | [45:39]     | 1111111b       |
| WP_GRP_SIZE        | 7     | R        | [38:32]     | 0011111b       |
| WP_GRP_ENABLE      | 1     | R        | [31:31]     | 0b             |
| reserved           | 2     | R        | [30:29]     | 00b            |
| R2W_FACTOR         | 3     | R        | [28:26]     | 101b           |
| WRITE_BL_LEN       | 4     | R        | [25:22]     | 1010b          |
| WRITE_BL_PARTIAL   | 1     | R        | [21:21]     | 0b             |
| reserved           | 5     | R        | [20:16]     | 00000b         |
| FILE_FORMAT_GRP    | 1     | R/W(1)   | [15:15]     | 0b             |
| COPY               | 1     | R/W(1)   | [14:14]     | 0b             |
| PERM_WRITE_PROTE   | 1     | R/W(1)   | [13:13]     | 0b             |
| TMP_WRITE_PROTEC   | 1     | R/W      | [12:12]     | 0b             |
| FILE_FORMAT        | 2     | R        | [11:10]     | 00b            |
| reserved           | 2     | R        | [9:8]       | 00b            |
| CRC                | 7     | R/W      | [7:1]       | 1011111b       |
| not used, always '1' | 1   | —        | [0:0]       | 1b             |

CellType: R: Read Only, R/W: Writable and Readable, R/W(1): One-time Writable/Readable

**Note:** Erase of one data block is not allowed in this card. This information is indicated by "ERASE_BLK_EN". Host System should refer this value before one data block size erase.

#### 7.1.4 RCA Register

The writable 16 bit relative card address register carries the card address in SD Card mode.

#### 7.1.5 DSR Register

This register is not implemented on this card.

---

## 8. Power Scheme

### 8.1 Power Up

"Power up time" is defined as voltage rising time from 0 volt to VDD min.

"Supply ramp up time" provides the time that the power is built up to the operating level (Host Supply Voltage) and the time to wait until the SD NAND can accept the first command.

The host shall supply power to the card so that the voltage is reached to Vdd_min within 250 ms and start to supply at least 74 SD clocks to the SD NAND with keeping CMD line to high.

### 8.2 Power Up Time

Host needs to keep power line level less than 0.5V and more than 1ms before power ramp up.

#### 8.2.1 Power On or Power Cycle

Followings are requirements for Power on and Power cycle to assure a reliable SD NAND hard reset.

1. Voltage level shall be below 0.5V
2. Duration shall be at least 1ms

#### 8.2.2 Power Supply Ramp Up

The power ramp up time is defined from 0.5V threshold level up to the operating supply voltage which is stable between VDD(min.) and VDD(max.) and host can supply SDCLK.

Followings are recommendation of Power ramp up:
1. Voltage of power ramp up should be monotonic as much as possible.
2. The minimum ramp up time should be 0.1 ms.
3. The maximum ramp up time should be 35 ms for 2.7-3.6V power supply.

#### 8.2.3 Power Supply Ramp Up

When the host shuts down the power, the VDD shall be lowered to less than 0.5 Volt for a minimum period of 1 ms. During power down, DAT, CMD, and CLK should be disconnected or driven to logical 0 by the host to avoid a situation that the operating current is drawn through the signal lines.

If the host needs to change the operating voltage, a power cycle is required. Power cycle means the power is turned off and supplied again. Power cycle is also needed for accessing cards that are already in Inactive State. To create a power cycle the host shall follow the power down description before power up the card (i.e. the VDD shall be once lowered to less than 0.5 Volt for a minimum period of 1 ms).

---

## 9. Part Numbering

```
M K D V X G C L - A B F
| | | | | | | |   | | |
| | | | | | | |   | | └── Version
| | | | | | | |   | └──── Version
| | | | | | | |   └────── Version
| | | | | | | └────────── Product Package: L = LGA Package
| | | | | | └──────────── Temperature Range: C = Consumption Grade (-25°C to +85°C)
| | | | | └────────────── Product Density: 1G/2G/4G/8G = 1Gbit/2Gbit/4Gbit/8Gbit
| | | | └──────────────── Operating Voltage: V = 2.7~3.6V
| | | └────────────────── Product Family: D = SD NAND Series
| | └──────────────────── Company Prefix: MK = MK company
| └────────────────────── Company Prefix
└──────────────────────── Company Prefix
```

| Field               | Description |
|---------------------|-------------|
| MK                  | MK company |
| D                   | SD NAND Series |
| E                   | eMMC Series |
| V                   | Operating Voltage: 2.7~3.6V (6.x * 8.0) |
| 1G/2G/4G/8G         | Product Density: 1Gbit/2Gbit/4Gbit/8Gbit |
| C                   | Temperature Range: Consumption Grade (-25°C to +85°C) |
| L                   | Product Package: LGA Package |

---

## 10. Package Dimensions

**LGA-8 Package**

*(Package dimensions diagram - SD NAND)*

---

## 11. Reference Design

*(Reference design circuit diagram for SD NAND)*

**NOTE:**
1. RDAT and RCMD (10K~100 kΩ) are pull-up resistors protecting the CMD and the DAT lines against bus floating when SD NAND is in a high-impedance mode.
2. The host shall pull-up all DAT0-3 lines by RDAT, even if the host uses the SD NAND as 1bit mode-only in SD mode.
3. It is recommended to have 2.2 μF capacitance on VDD.
4. RCLK reference 0~120Ω, recommend 22 Ω.
