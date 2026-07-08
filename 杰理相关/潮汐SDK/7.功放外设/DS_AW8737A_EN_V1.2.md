# AW8737A

July 2022 V1.2

## High Efficiency, Low Noise, Ultra-Low Distortion, Constant Large Volume, Upgrade 7th-Generation Class K Audio Amplifier

---

## FEATURES

- Low noise: 53μV
- Power Amplifier's Overall Efficiency: 80%
- Within Lithium Battery Voltage Range, Outputs Constant Large Volume
- Selectable speaker-guard power level: 0.6W, 0.8W, 1W, 1.2W
- No-Crack-Noise (NCN) Technology
- Super TDD-Noise Suppression
- Excellent Pop-Click Suppression
- One-Wire Pulse Control
- High PSRR: -68dB (217Hz)
- ESD Protection: ±1kV (CDM)
- FCQFN 1.60mm×1.60mm×0.55mm-16L

---

## DESCRIPTION

AW8737A is specifically designed to enhance overall sound quality. It is an upgrading 7th-generation class K audio amplifier with high efficiency, low noise, ultra-low distortion and capability of outputting constant large volume.

With integrated AWINIC proprietary NCN output AGC audio algorithm, AW8737A can eliminate noise in playback and improve sound quality and effect. Using a novel K-Chargepump technology, its integrated charge pump efficiency can reach 93%, and power amplifier's overall efficiency can reach 80%. With high efficiency, AW8737A can greatly prolong smart phone usage time.

AW8737A noise floor is as low as to 53μV, with 97dB high signal-to-noise-ratio (SNR). The ultra-low distortion 0.008% brings high-quality musical enjoyment.

AW8737A has a setting of 4-step selectable speaker-guard output power level from 0.6W to 1.2W, suitable for different rated power speakers. Within lithium battery voltage range, it keeps output power constant, preventing voice from degrading.

The AW8737A uses Awinic proprietary TDD-Noise suppression technology and EMI suppression technology, effectively restrain TDD-Noise and EMI interference.

AW8737A has built-in over-current protection, over-temperature protection and short-circuit protection.

AW8737A is available in a FCQFN 1.60mm×1.60mm×0.55mm-16L.

---

## APPLICATIONS

- Smart Phones

---

## APPLICATION DIAGRAM

### Class K Speaker Mode

```
VBAT
  |
  |        Cin    Rin    Av    Fin
  |       4.7uF  0.1uF   47   3  16.3  173
  |        nF     kΩ     V/V   Hz
  |         |      |      |     |
  |        Cin    Rin           |
  |        33    10     12     181
  |         |      |            |
  |   HPL/HPR                   |          One-Wire Pulse Control
  |   AUDIO                     |
  |   DAC     Ground-Shielding  |
 |           Pseudo-Differential Routing
  |         |             |     |
  |   HPREF |            Cin   Rin
  |   C2    |             |     |
  |  0.1nF  |            220pF  |
  |         |     |            Cd
  |        GND   C1            |
  |                    C2,(C3),C4
  |
  +--[CF1 2.2uF]--+--[CF2 2.2uF]--+
  |                |                |
  |   A3,B3  D2 C1   D1 B1,(B2)
  |    VDD   C1P C1N  C2P C2N
  |    A4                D3
  |   SHDN              PVDD COUT
  |    1   2  3  4         4.7uF
  |                        10V
  |
         AW8737A
           |
           +--[SPK]--+
```

**Figure 1** AW8737A Application Diagram

---

## PIN DIAGRAM AND DEVICE MARKING

### TOP VIEW

```
      A       B       C       D
   +-------+-------+-------+-------+
1  |  INP  |  C2N  |  C1N  |  C2P  |
   |   I   |       |       |   l   |
2  |  INN  |  C2N  |  GND  |  C1P  |
   |   X   |       |       |       |
3  |  VDD  |  VDD  |  GND  |  PVDD |
   |       |       |       |   n   |
4  |  SHDN |  VOP  |  GND  |  VON  |
   |       |       |   d   |       |
   +-------+-------+-------+-------+

            37A
           XXX
     AW8737AFCR MARKING
```

**Figure 2** AW8737A Pin Diagram Top View and Device Marking

---

## PIN DESCRIPTION

### AW8737AFCR

| Number | Symbol | Description |
|--------|--------|-------------|
| A1 | INP | Positive audio input |
| A2 | INN | Negative audio input |
| A3, B3 | VDD | Power supply |
| A4 | SHDN | Chip power down pin, active low: one wire pulse control |
| B1, B2 | C2N | Negative terminal of the charge pump flying capacitor C~F2~ |
| B4 | VOP | Positive audio output |
| C1 | C1N | Negative terminal of the charge pump flying capacitor C~F1~ |
| C2, C3, C4 | GND | Ground |
| D1 | C2P | Positive terminal of the charge pump flying capacitor C~F2~ |
| D2 | C1P | Positive terminal of the charge pump flying capacitor C~F1~ |
| D3 | PVDD | Charge pump output |
| D4 | VON | Negative audio output |

---

## AWINIC CLASS K FAMILY

| ITEM | TEST CONDITION | AW8736 | AW8737A | AW87317A | AW87318 |
|------|---------------|--------|---------|----------|---------|
| PVDD (V) | VDD=4.2V | 5.8 | 6.05 | 6.05 | 6.05 |
| Output Noise (μV) | VDD=4.2V, f=20Hz to 20kHz<br>Input ac grounded, 8V/V, A-weighting | 125 | 53 | 53 | 40 |
| Efficiency (%) | VDD=3.6V, Po=1.0W, R~L~=8Ω+33μH | 75 | 80 | 80 | 83 |

---

## FUNCTIONAL DIAGRAM

```
          VDD    C1P    C1N    C2P    C2N
           |      |      |      |      |
          SHDN ---+--->| SYSCTRL  OVP |--- PVDD
           |           |      |      |    |
           |      +--->| K-CHARGEPUMP  |   |
           |      |    |      |      |    |
          INP -->| ULTRA  CLASS K  LOW EMI  |--> VOP
           |    | INPUT   MODULATOR  OUTPUT  |
          INN -->| BUFFER  STAGE              |--> VON
           |      |      |      |      |
           +--->| NCN    OTP   OSC   OCP  |
           |    GND
```

**Figure 3** AW8737A Functional Diagram

---

## APPLICATION DIAGRAM (SPEAKER MODE)

**Figure 4** AW8737A Speaker Mode Application Diagram (Note 1)

> **Note 1:** When single-ended input, audio signal line from audio DAC (HPL or HPR) can arbitrarily connected to either of INN or INP input terminal. The other terminal must be connected to reference ground (HPREF) through input capacitor and resistor.

---

## ORDERING INFORMATION

| Product | Device Marking | Environmental Information | Operation Temperature Range | Moisture Sensitivity Level (MSL) | Package | Delivery Form |
|---------|---------------|---------------------------|----------------------------|----------------------------------|---------|---------------|
| AW8737AFCR | 37A | ROHS+HF | -40°C to 85°C | MSL1 | FCQFN 1.60mm×1.60mm×0.55mm-16L | Tape & Reel 3000 pcs |

**AW8737A**

| | |
|---|---|
| Shipment | R: Tape & Reel |
| Package Type | FC: FCQFN-16 |

---

## ABSOLUTE MAXIMUM RATINGS (NOTE 2)

| Parameter | Range |
|-----------|-------|
| Power Supply VDD Voltage | -0.3V to 6V |
| Charge Pump Output PVDD Voltage | -0.3V to 7V |
| VOP, VON, C1P, C2P | -0.3V to PVDD+0.3V |
| C1N, C2N | -0.3V to VDD+0.3V |
| Input Pin INP, INN Voltage | -0.3V to VDD+0.3V |
| Junction-to-Ambient Thermal Resistance θ~JA~ | 76.5°C/W |
| Operating Free-Air Temperature T~A~ | -40°C to 85°C |
| Maximum Junction Temperature T~JMAX~ | 165°C |
| Storage Temperature T~STG~ | -40°C to 150°C |
| Lead Temperature (Soldering 10 Seconds) | 260°C |

### ESD Rating

| Charged Device Model (CDM) (Note 4) | ±1kV |

### Latch-up

+IT: 450mA
-IT: -450mA

Test Condition: JEDEC STANDARD NO.78D SEPTEMBER 2010

> **Note 2:** Stresses beyond those listed under "absolute maximum ratings" may cause permanent damage to the device. These are stress ratings only and functional operation of the device at these or any other conditions beyond those indicated under "recommended operating conditions" is not implied. Exposure to absolute-maximum-rated conditions for extended periods may affect device reliability.

> **Note 3:** The human body model is a 100pF capacitor discharged through a 1.5kΩ resistor into each pin. Test method: MIL-STD-883J Method 3015.9.

> **Note 4:** Test Method: ESDA/JEDEC JS-002-2014.

---

## MODE DESCRIPTIONS

(T~A~=25°C, VDD=4.2V)

Under Speaker Mode (Mode 1 to Mode 4), audio signal is inserted through INP & INN pins. The AW8737A external input capacitor is Cin and the external input resistor is Rin.

1) Under Speaker Mode, the internal input resister is 16.6kΩ. The gain of AW8737A (Av) can be calculated by 319.5k/(Rin+16.6k) (Rin unit: Ω). Recommended operating external setting is: Cin=47nF, Rin=3kΩ, Av=16.3V/V or Rin=10kΩ, Av=12V/V.

The operating modes of AW8737A are listed below:

| MODE | Enable Signal (SHDN) | Gain (V/V) | | NCN Power Level (W) | | |
|------|----------------------|------------|---|---------------------|---|---|
| | | Rin=3kΩ | Rin=10kΩ | R~L~=8Ω+33μH | R~L~=6Ω+33μH | NCN Function |
| 1 | | 16.3 | 12 | 1.2 | 1.6 | |
| 2 | | 16.3 | 12 | 1.0 | 1.3 | |
| 3 | | 16.3 | 12 | 0.8 | 1.0 | |
| 4 | | 16.3 | 12 | 0.6 | 0.8 | |

---

## ELECTRICAL CHARACTERISTICS

Test condition: T~A~=25°C, VDD=3.6V, R~L~=8Ω+33μH, f=1kHz (unless otherwise noted)

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| V~DD~ | Power supply voltage | | | 3.0~5.5 | V |
| V~IH~ | SHDN high input voltage | | 1.3 | V~DD~ | V |
| V~IL~ | SHDN low input voltage | 0 | | 0.45 | V |
| \|V~OS~\| | Output offset voltage | V~IN~=0V, V~DD~=3.0V to 5.5V | -30 | 0 | 30 | mV |
| I~SD~ | Shutdown current | V~DD~=3.6V, SHDN=0V | | | 2 | μA |
| T~TG~ | Thermal AGC start temperature threshold | | | 150 | | °C |
| T~TGR~ | Thermal AGC exit temperature threshold | | | 130 | | °C |
| T~SD~ | Over temperature protection threshold | | | 160 | | °C |
| T~SDR~ | Over temperature protection recovery threshold | | | 120 | | °C |
| T~ON~ | Start-up time | | | 40 | | ms |

### K-Chargepump

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| PVDD Output voltage | V~DD~=3.0V to 4V | | V~DD~ | | V |
| | V~DD~>4V | | 6.05 | | V |
| V~hys~ | OVP hysteresis voltage | V~DD~>4V | | 50 | | mV |
| F~CP~ | Charge pump frequency | V~DD~=3.0V to 5.5V | 0.79 | 1.06 | 1.33 | MHz |
| η~CP~ | Charge pump efficiency | V~DD~=3.6V, I~load~=200mA | | 93 | | % |
| T~ST~ | Soft-start time | No load, C~OUT~=4.7μF | 1.0 | 1.2 | 1.4 | ms |
| I~L~ | Current limit when PVDD short to ground | | 200 | 300 | 400 | mA |

### Class K power amplifier (Mode1 to Mode4, and Mode7)

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| I~q~ | Quiescent current | V~DD~=4.2V, Vin=0, no load | | 10 | 15 | mA |
| η | Efficiency | V~DD~=3.6V, Po=1.0W, R~L~=8Ω+33μH | | 80 | | % |
| Fosc | Modulation frequency | V~DD~=3.0V to 5.5V | 600 | 800 | 1000 | kHz |
| A~V~ | Gain | External input resistance=3kΩ | | 16.3 | | V/V |
| Vin | Recommended max input voltage | V~DD~=3.0V to 5.5V | | | 1 | Vp |
| R~ini~ | Internal input resistor | Mode1 to Mode4 | | 16.6 | | kΩ |
| F~hin~ | Input high pass filter corner frequency | Cin=47nF, external input resistor=3kΩ | | 173 | | Hz |

### NCN output AGC power P~AGC~

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| Mode1 NCN output AGC power | V~DD~=4.2V, R~L~=8Ω+33μH | 1.08 | 1.2 | 1.32 | W |
| | V~DD~=4.2V, R~L~=6Ω+33μH | 1.44 | 1.6 | 1.76 | W |
| | V~DD~=4.2V, R~L~=4Ω+15μH | 2.16 | 2.4 | 2.64 | W |
| | V~DD~=4.2V, R~L~=3Ω+15μH | 2.16 | 2.4 | 2.64 | W |
| Mode2 NCN output AGC power | V~DD~=4.2V, R~L~=8Ω+33μH | 0.9 | 1 | 1.1 | W |
| | V~DD~=4.2V, R~L~=6Ω+33μH | 1.17 | 1.3 | 1.43 | W |
| | V~DD~=4.2V, R~L~=4Ω+15μH | 1.8 | 2 | 2.2 | W |
| | V~DD~=4.2V, R~L~=3Ω+15μH | 2.16 | 2.4 | 2.64 | W |
| Mode3 NCN output AGC power | V~DD~=4.2V, R~L~=8Ω+33μH | 0.72 | 0.8 | 0.88 | W |
| | V~DD~=4.2V, R~L~=6Ω+33μH | 0.9 | 1 | 1.1 | W |
| | V~DD~=4.2V, R~L~=4Ω+15μH | 1.44 | 1.6 | 1.76 | W |
| | V~DD~=4.2V, R~L~=3Ω+15μH | 1.8 | 2.0 | 2.2 | W |
| Mode4 NCN output AGC power | V~DD~=4.2V, R~L~=8Ω+33μH | 0.54 | 0.6 | 0.66 | W |
| | V~DD~=4.2V, R~L~=6Ω+33μH | 0.72 | 0.8 | 0.88 | W |
| | V~DD~=4.2V, R~L~=4Ω+15μH | 1.08 | 1.2 | 1.32 | W |
| | V~DD~=4.2V, R~L~=3Ω+15μH | 1.44 | 1.6 | 1.76 | W |
| PSRR | Power supply rejection ratio | V~DD~=4.2V, Vp-p_sin=200mV, 217Hz | | -68 | | dB |
| | | 1kHz | | -68 | | dB |
| SNR | Signal-to-noise ratio | V~DD~=4.2V, Po=1.75W, THD+N=1%, R~L~=8Ω+33μH, Av=8V/V | | 97 | | dB |
| V~n~ | Output noise voltage | V~DD~=4.2V, f=20Hz to 20kHz, input ac grounded, A-weighting, Av=8V/V | | 53 | | μVrms |
| | | Av=12V/V | | 58 | | μVrms |
| | | Av=16V/V | | 68 | | μVrms |
| THD+N | Total harmonic distortion+noise | V~DD~=3.6V, Po=1W, R~L~=8Ω+33μH, f=1kHz, Mode1 | | 0.008 | | % |
| | | V~DD~=3.6V, Po=1W, R~L~=6Ω+33μH, f=1kHz, Mode1 | | 0.008 | | % |

### One Wire Pulse Control

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| T~H~ | SHDN high level duration time | V~DD~=3.0V to 5.5V | 0.75 | 2 | 10 | μs |
| T~L~ | SHDN low level duration time | V~DD~=3.0V to 5.5V | 0.75 | 2 | 10 | μs |
| T~LATCH~ | SHDN turn on delay time | V~DD~=3.0V to 5.5V | | 150 | 500 | μs |
| T~OFF~ | SHDN turn off delay time | V~DD~=3.0V to 5.5V | | 150 | 500 | μs |

### NCN (Note 5)

| Parameter | Test Conditions | Min | Typ | Max | Unit |
|-----------|----------------|-----|-----|-----|------|
| T~AT~ | Attack time | -13.5dB gain attenuation completed | | 40 | | ms |
| T~RL~ | Release time | 13.5dB gain release completed | | 1.2 | | s |
| A~MAX~ | Maximum attenuation | | | -13.5 | | dB |

> **Note 5:** Attack Time refers to the duration of gain attenuation by 13.5dB. Similarly, Release Time refers to the duration of gain recovery by 13.5dB.

---

## MEASUREMENT SETUP

AW8737A features switching digital output, as shown in Figure 6. It is crucial to connect a low pass filter after VOP/VON outputs, respectively, to filter out switch modulation frequency, then measure the differential output of filter to obtain audio analog output signal.

```
         INP    VOP
          |      |
   Cin   |      |      10nF
   Rin --+  AW8737A  +--[500Ω]--+
          |      |               |  32kHz Low-Pass
         INN    VON              |  Filter
          |      |      10nF    |
   Cin   |      |  +--[500Ω]--+
   Rin --+      |
```

**Figure 6** AW8737A Test Setup

The values of resistor and capacitor used by low pass filter are listed below:

| R~filter~ | C~filter~ | Low-pass cutoff frequency |
|-----------|-----------|---------------------------|
| 500Ω | 10nF | 32kHz |
| 1kΩ | 4.7nF | 34kHz |

### Output Power Calculation

According to the above test method, the differential audio analog output signal is obtained at the output of the low pass filter. The valid value Vo_rms of the differential signal is as shown below:

The power calculation of Speaker is as follows:

$$P_L = \frac{V_{O\_RMS}^2}{R_L}$$

(R~L~: Load Impedance of the speaker)

---

## TYPICAL CHARACTERISTICS

| | |
|---|---|
| **Efficiency vs Po** | **K-chargepump Efficiency** |
| VDD=3.6V<br>VDD=4.2V | VDD=3.6V<br>VDD=4.2V |
| COUT=4.7μF<br>R~L~=8Ω+33μH | CF1,CF2=2.2μF |
| (Efficiency % vs Po(W), 0-2W) | (Efficiency % vs Po(W), 0-2W) |

| | |
|---|---|
| **OUTPUT POWER vs VDD** | **GAIN vs FREQUENCY** |
| AW8737S Po keep constant<br>MODE1/2/3/4<br>R~L~=8Ω+33μH | MODE1~MODE4 Rin=3kΩ<br>Cin=1μF<br>R~L~=8Ω+33μH |
| (Output Power W vs VDD V, 3.3-4.3V) | (Gain V/V vs Frequency Hz, 20-20K) |

| | |
|---|---|
| **THD+N vs FREQUENCY** | **Po vs VIN** |
| Mode1 MODE1 0.8W<br>V~DD~=4.2V, Rin=3kΩ<br>R~L~=8Ω+33μH, Cin=1μF | V~DD~=4.2V, f=1kHz<br>R~L~=8Ω+33μH |
| (THD+N % vs Frequency Hz, 20-20K) | (Po W vs VIN Vp, 0.1-2) |

| | |
|---|---|
| **Po vs VIN (Mode2)** | **Po vs VIN (Mode3)** |
| V~DD~=4.2V, f=1kHz<br>R~L~=8Ω+33μH | V~DD~=4.2V, f=1kHz<br>R~L~=8Ω+33μH |

| | |
|---|---|
| **Po vs VIN (Mode4)** | **PSRR vs FREQUENCY** |
| V~DD~=4.2V, f=1kHz<br>R~L~=8Ω+33μH | Mode1, Rin=3kΩ, fCin=1μF<br>R~L~=8Ω+33μH |
| | V~DD~=4.2V, V~DD~=3.6V |

| | |
|---|---|
| **STARTUP SEQUENCE** | **SHUTDOWN SEQUENCE** |
| SHDN/VOP&VON, 10ms/div | SHDN/VOP&VON, 100μs/div |

---

## DETAILED FUNCTIONAL DESCRIPTION

AW8737A is specifically designed to enhance overall sound quality. It is an upgrading 7th-generation class K audio amplifier with high efficiency, low noise, ultra-low distortion and capability of outputting constant large volume.

With integrated AWINIC proprietary NCN output AGC audio algorithm, AW8737A can eliminate noise in playback and improve sound quality and effect. Using a novel K-Chargepump technology, its integrated charge pump efficiency can reach 93%, and power amplifier's overall efficiency can reach 80%. With high efficiency, AW8737A can greatly prolong smart phone usage time.

AW8737A noise floor is as low as to 53μV, with 97dB high signal-to-noise-ratio (SNR). The ultra-low distortion 0.008% brings high-quality musical enjoyment.

AW8737A has a setting of 4-step selectable speaker-guard output power level from 0.6W to 1.2W, suitable for different rated power speakers. Within lithium battery voltage range, it keeps output power constant, preventing voice from degrading.

THE AW8737A uses awinic proprietary TDD-Noise suppression technology and EMI suppression technology, effectively restrain TDD-Noise and EMI interference.

AW8737A has built-in over-current protection, over-temperature protection and short-circuit protection.

AW8737A is available in a 1.60mm×1.60mm, 0.4mm pitch FC-16 package.

---

### Constant Output Power

In the smart phone audio applications, the AGC function which can promote music volume and audio quality is very attractive, but as the lithium battery voltage drops, the driver capability of ordinary audio power amplifiers will reduce gradually, leading to degrading audio effect. Therefore, it is hard to provide high-quality music within the battery voltage range.

With integrated AWINIC proprietary NCN output AGC audio algorithm and within lithium battery voltage range (3.3V to 4.35V), AW8737A can keep output power constant and never decreasing during lithium battery voltage dropping down. As a result, even if the battery voltage drops, AW8737A can still provide high-quality large-volume music enjoyment.

AW8737A has 4 operating modes. They have NCN output AGC function and their output AGC power levels are 1.2W, 1W, 0.8W, 0.6W, respectively.

---

### 2nd Generation NCN Technology

In audio application, there is undesirable distortion in a clipping output signal, because of a too large input signal along with a drop of supply voltage powered by lithium battery. To prevent a speaker load from permanent damage by a clipping output signal, adoption of traditional NCN technology can adjust gain of power amplifier automatically by detecting "Crack" distortion in a output signal, and keep the output signal smooth without clipping. NCN function can effectively prevent a power amplifier overloading, protect a speaker load, and bring high quality music enjoyment at the same time. A traditional NCN function is shown in Figure 7 below.

By adopting AWINIC unique 2nd generation NCN technology, AW8737A's output signal is not limited by lithium battery voltage. When battery voltage drops, output signal keeps unchanged and free from distortion, realizing constant output power as shown in figure 8. Therefore, even if battery voltage drops, AW8737A can still provide high quality large volume music enjoyment.

**Figure 7** Traditional NCN Operating Principle

**Figure 8** 2nd Generation NCN Operating Principle

---

### Attack Time

Attack time is the time which NCN output AGC takes for the gain to be attenuated by 13.5dB when audio signal exceeds the constant output power threshold level. Short attack time (fast attack) allows NCN function to react quickly and suppress harmful transients. However, it can lead to volume pumped and make process of gain reduction noticeable. While long attack time (slow attack) makes NCN function ignore fast transients and act upon longer passages instead, resulting in an increase of distortion. According to audio features in portable equipment, attack time in AW8737A is set to be 40ms, improving the music rhythm, eliminating crack distortion, and protecting the speaker at the same time.

---

### Release time

Release time is the time which NCN output AGC takes for the gain to return to its setting value when audio signal is smaller than clipping level or constant output power threshold level. According to features of music noise in smart phone application and demands for better music quality and volume, release time of AW8737A is designed to be 1.2s, which can effectively eliminate the noise, and make sound smoother.

---

### K-Chargepump

AW8737A adopts a new generation of charge pump technology: K-Chargepump structure. It has higher efficiency and larger driving capability. Its operating frequency is 1.06MHz. With built-in soft-start circuit, current-limit control loop and over-voltage-protection (OVP) loop, charge pump of this configuration can provide more stable and reliable power supply.

---

### High Efficiency

The output voltage PVDD is 1.5 times of supply voltage VDD in K-Chargepump, of which the ideal efficiency can reach 100%. Actually, the K-Chargepump efficiency can be calculated as the ratio of output power to input power, that is

$$\eta = \frac{P_{OUT}}{P_{IN}} \times 100\%$$

For example, in an ideal M-times charge pump, the input current I~IN~ is M times of the output current I~OUT~, the efficiency formula can be written as:

$$\eta = \frac{P_{OUT}}{P_{IN}} \times 100\% = \frac{V_{OUT} \cdot I_{OUT}}{V_{IN} \cdot M \cdot I_{OUT}} \times 100\% = \frac{V_{OUT}}{M \cdot V_{IN}} \times 100\%$$

Also, M is a parameter depending on the operating mode of a charge pump; V~OUT~ is the output voltage of a charge pump; V~IN~ is the input voltage (generally is also the power supply voltage) of a charge pump; I~OUT~ is also the load current. For K-Chargepump structure, the output voltage is 1.5 times of the input voltage. Due to the switch loss and quiescent current loss inside the charge pump, the actual efficiency can still be up to 93%. As a result, the power booster technology of K-Chargepump can greatly improve the power efficiency.

---

### K-Chargepump Structure

As shown in Figure 9 is a K-Chargepump fundamental functional diagram: K-Chargepump integrated in AW8737A has seven switches, of which the output voltage PVDD is boosted to 1.5 times as input voltage VDD through seven switches operating timing.

**Figure 9** K-Chargepump Functional Diagram

The operation of the charge pump has two phases. In Φ~1~, as shown in Figure 10, when switches S1, S2 and S3 are closed, VDD charges to the flying capacitor C~F1~ and C~F2~.

**Figure 10** Φ1: Charge Flying Capacitors C~F1~ and C~F2~ (Charging Phase)

In Φ~2~, as shown in Figure 11, switches S1, S2 and S3 are opened, and switches S4, S5, S6 and S7 are closed. Because the voltage across the capacitor can't change instantaneously, so either the voltage on flying capacitors C~F1~ or C~F2~, is added to the VDD, realizing a PVDD boosted to a higher voltage.

**Figure 11** Φ2: Flying Capacitor Charges Transfer to the Output Capacitor C~OUT~ (Discharging Phase)

---

### Soft Start

K-chargepump has integrated soft start function in order to limit inrush current from power supply during start-up. The current from power supply can be limited to 300mA, and the start-up time is about 1.2ms.

---

### Peak Current Control

K-chargepump has integrated a peak current control circuit. In normal operation, when a heavy load or a situation that makes the charge pump extracts very large current from power supply, the peak current control circuit can limit the maximum output load current, which is typically 2A.

---

### Over-Voltage Protection (OVP)

K-Chargepump keeps the output voltage PVDD a Singleple of the input voltage VDD. It provides a high voltage power rail for internal power amplifier circuits, allowing the amplifiers provide greater output dynamic range in the lithium battery voltage range, realizing much larger volume, higher audio quality. K-Chargepump has integrated a over-voltage protection circuit. When the input voltage VDD is greater than 4V, the output voltage PVDD is no longer a Singleple of VDD, but a controlled voltage by over-voltage protection (OVP) circuit and kept in 6.05V. The hysteresis voltage of OVP is about 50mV.

---

### One-Wire Pulse Control: Principle

One-wire pulse control technology only needs a single GPIO port to turn on the chip and select a variety of functions. It is very popular in an environment lack of GPIO ports, such as portable systems.

Considering the problems of signal integrity or RF interference, there is narrow glitch in signal line when the PCB routine is too long. AWINIC one-wire pulse control technology integrated a deglitch circuit along with the internal control pin. The deglitch-module can completely eliminate the harmful glitch interference, as shown in Figure 12.

**Figure 12** AWINIC Deglitch Working Principle

The traditional one-wire pulse control technology keeps working after the slave chip is powered up. Therefore, when the master chip (such as Baseband in a smart phone) sends other control signal through the same control port, the slave chip will probably enter into a wrong state. AW8737A uses one-wire pulse technology with a latch circuit, by which the right working state will be stored after the master chip sending order and AW8737A will no longer receive successive signals (except shutting down the chip firstly), as shown in Figure 13.

**Figure 13** Anti-Interference One-Wire Pulse Control Functional Diagram

---

### One-Wire Pulse Control: Working Mode

Each mode of AW8737A can be set by the on-wire pulse control circuit, which can detect the number of pulses sent by master chip through SHDN pin. When SHDN pulls to high level from shutdown state (low level), i.e. only a rising edge, AW8737A will enter into Mode1, and the constant output power level of NCN output AGC is 1.2W (with 8Ω speaker load). When SHDN shows a high-to-low-to-high logic signal, i.e. a rising edge after a pulse, or two rising edges, AW8737A will enter into Mode2, and the level is 1.0W. Similarly, N rising edges means Mode"N", as shown in Figure 14. After all, AW8737A has seven operating modes, more than four rising edges is forbidden.

**Figure 14** Working Mode Setting through One-Wire Pulse Control

To change the working mode of AW8737A, one needs to keep SHDN low longer than T~OFF~ firstly (1ms is recommended), to shut down the chip. Then, send pulses to bring the chip into a right mode, as shown in Figure 15.

**Figure 15** Mode Switch through One-Wire Pulse Control

---

### RNS (RF Noise Suppression)

GSM transmission adopts TDMA (Time Division Multiple Access) Technology which results in frame burst at frequency of 217Hz, also called TDD (Time Division Duplexing), leading to a strong RF interference (RF Noise) and the 217Hz energy along with its harmonics (TDD Noise) can be easily interacted with audio power amplifiers.

In applications, optimization of both layout and selection of peripheral components may decrease the AW8737A's susceptibility to RF noise and prevent TDD Noise from being demodulated into audible noise. Minimization of length of routings prevents them from functioning as antennas and coupling RF noise into an AW8737A. Further RF immunity can also be realized by using capacitors of which feature of frequency response is like a notch filter. Depending on manufacturers, self-resonance frequency of 10pF to 20pF capacitors typically located at RF band. Such capacitors placed in front of input pins of AW8737A can effectively suppress RF noise. Also, such capacitors must have a low-impedance, low-inductance path to the ground plane.

Even if part of RF energy is injected into AW8737A by traces connected to the chip, regardless of efforts of TDD Noise Reduction. AW8737A features a unique RNS technology, which effectively reduces RF energy and attenuates RF TDD-noise to an acceptable audible level for customers.

**Figure 16** AW8737A Rejection of RF Noise

---

### Filter-Free Pulse Width Modulation (Filter-Free PWM)

AW8737A features a filter-free PWM architecture which removes a LC filter behind the output stage of a traditional Class D power amplifier, resulting in improvement of overall efficiency, decrease of PCB area and reduction of system cost.

---

### Enhanced Emission Elimination (EoEE)

AW8737A features a unique Enhanced Emission Elimination (EEE) technology, which adjusts the speed of waveform transition of PWM output signal, and effectively reduces EMI over FM/AM bandwidth.

---

### Pop-Click Suppression

AW8737A integrates a unique timing-control circuit, which fundamentally suppresses pop-click noise, and eliminates audible crack at shut-down, wake-up, and power-up/down.

---

### Protection

When a short-circuit occurs among output pins (VOP, VON) and power pins (VDD, GND, PVDD) of AW8737A, an over-current protection (OCP) circuit will be trigged and shut down the chip immediately, preventing the device from being damaged. When abnormal condition is removed, AW8737A can restart automatically without wake-up.

When junction temperature in AW8737A is too high, an over-temperature protection (OTP) circuit will be triggered and shut down the chip immediately. The circuit will turn the device on once the temperature decrease into a safe scope.

---

## APPLICATION INFORMATION

### Gain Setting -- Selection of External Input Resistor (R~ine~)

AW8737A is a differential-input audio power amplifier. It integrates two internal input resistors (R~ini~), which are both 16.6kΩ. Take external input resistors R~ine~=3kΩ for instance, overall gain (A~V~) can be set as below:

| AW8737A Mode | Calculation of Overall Gain (V/V) |
|--------------|-----------------------------------|
| Class K Speaker Mode (Mode1~4) | $$A_V = \frac{319.5k\Omega}{R_{ine}+R_{ini}} = \frac{319.5k\Omega}{3k\Omega+16.6k\Omega} = 16.3V/V$$ |

---

### Input High-Pass Cutoff Frequency Setting -- Selection of Input Capacitor (C~in~)

Input capacitors in front of external input resistors can block DC component of input audio signals. An input capacitor (C~in~) along with input resistors (R~ine~+R~ini~) forms an input high-pass filter with a corner frequency (f~H~) calculated as below:

$$f_H(-3dB) = \frac{1}{2\pi(R_{ine}+R_{ini})C_{in}}$$

A higher f~H~ results in a better suppression of 217Hz GSM input noise. A better matching of input capacitors improves capability of blocking of common-mode interference of input stage in AW8737A and also helps to reduce pop-click noise.

Take typical application in Figure 1 for instance:

$$f_H(-3dB) = \frac{1}{2\pi(R_{ine}+R_{ini})C_{in}} = \frac{1}{2\pi \cdot 19.6k\Omega \cdot 47nF} = 173Hz$$

---

### Input Low-Pass Cutoff Frequency Setting – Selection of Differential Input Capacitor (C~d~)

A differential input capacitor behind external input resistors can block high-frequency component of input audio signals, such as screechy part in a song. A differential input capacitor (C~d~) along with input resistors (R~ine~+R~ini~) forms an input low-pass filter with a corner frequency (f~L~) calculated as below:

$$f_L(-3dB) = \frac{1}{4\pi(R_{ine}//R_{ini})C_d}$$

Take typical application in Figure 1 with C~d~=220pF and R~ine~=3kΩ for instance:

$$f_L(-3dB) = \frac{1}{4\pi \cdot 3k\Omega//16.6k\Omega \cdot 220pF} = 142.5kHz$$

---

### Selection of Power Supply Decoupling Capacitor (C~S~)

AW8737A is a high-performance audio power amplifier. It is essential to place a ceramic capacitor (C~S~) with low equivalent-series-resistance (ESR) (typical 0.1uF) for power supply decoupling. Optimized selection and placement of decoupling capacitors protect AW8737A from interference injection from power supply, such as high-frequency transients, spikes, or digital noise. Specifically, a layout of decoupling capacitor closer to AW8737A is preferred, since fewer parasitic resistance or inductance between power pin and the capacitor, less decoupling efficiency loss. In addition to a 0.1μF ceramic capacitor, another 10μF capacitor as a charge reservoir is required, providing transient power energy for AW8737A and preventing remarkable drop of the power supply voltage.

---

### Selection of Charge Pump Flying Capacitor (C~F~)

Value of charge pump flying capacitors (C~F~) affects load regulation and output impedance of the charge pump. Small capacitance may degrade driving capability of AW8737A. A 2.2μF/6.3V ceramic capacitor is usually recommended.

---

### Selection of Charge Pump Output Capacitor (C~OUT~)

Capacitance and ESR of charge pump output capacitors (C~OUT~) directly affect ripple magnitude of charge pump output voltage (PVDD). Increasing C~OUT~ Capacitance reduces variations of PVDD and decreasing C~OUT~ ESR also reduces both ripple and output resistance. A 4.7μF/10V ceramic capacitor is usually recommended.

---

### Usage of Ferrite Bead and Filter Capacitor

Without ferrite beads and filter capacitors, AW8737A can still pass the specifications of FCC and CE. If there is any EMI sensitive device near AW8737A and/or there are long traces routing from the amplifier to a speaker, use ferrite beads and filter capacitors and place beads and capacitors as close as possible to output pins (VOP&VON), as Figure 17 below.

In Class K Speaker Mode, outputs of AW8737A are square-wave PWM signals, which charge and discharge filter capacitors in each period, and result in additional static power consumption. Bigger filter capacitance, larger current consumption. Therefore, 0.1nF ceramic capacitor is usually recommended for low power application.

```
  VOP ----[Bead]----+     VON ----[Bead]----+
         |          |            |          |
         +---[0.1nF]---+         +---[0.1nF]---+
         |          |            |          |
```

**Figure 17** Ferrite Beads and Filter Capacitors

---

## PCB AND DEVICE LAYOUT CONSIDERATION

In order to exploit best performance of AW8737A, PCB layout must be carefully considered. Design consideration should be followed as below:

1. Isolated, short and wide power lines for both VDD pin and GND pin are required for better driving-capability of AW8737A. The copper width is recommended to be larger than 0.75mm (30mil). Power supply decoupling capacitors should be placed as close as possible to power supply pins.

2. Flying capacitors C~F1~, C~F2~ should be placed as close as possible to C1N, C1P pins and C2N, C2P pins. Likewise, capacitor C~OUT~ should be close to PVDD pin. The trace from C~OUT~ to both PVDD pin and GND pin should be short and wide.

3. Input capacitors and resistors should be close to INN and INP pins. Differential and ground-shielding input routing is required to suppress noise coupling.

4. Ferrite beads and filter capacitors should be close to VON and VOP pins. The trace from output pins to speaker should be short and wide. The copper width is recommended to be larger than 0.5mm (20mil).

---

## PACKAGE INFORMATION FOR AW8737AFCR

### TOP VIEW / BOTTOM VIEW / SIDE VIEW

```
         SYMM
   +---+---+---+---+
   | D |   |   |   |  4  3   2   1
D  |   |   |   |   |
   +---+---+---+---+
3e | 37A |         | C
   | XXX |         |
2e |     |         |
   +---+---+---+---+
   | E |   |   |   |
   +---+---+---+---+
         SYMM

   E: 1.6 ± 0.1
   D: 1.6 ± 0.1
   A: 0.55 ± 0.05
   A1: 0.02 -0.02/+0.03
   A2: 0.4 NA
   A3: 0.152 NA
   e1: 0.200 NA
   e2: 0.200 NA
   e3: 0.400 NA
```

### Dimensions

| Symbol | NOM | Tolerance |
|--------|-----|-----------|
| A | 0.55 | ±0.05 |
| A1 | 0.02 | -0.02/+0.03 |
| A2 | 0.4 | NA |
| A3 | 0.152 | NA |
| D | 1.6 | ±0.1 |
| E | 1.6 | ±0.1 |
| e1 | 0.200 | NA |
| e2 | 0.200 | NA |
| e3 | 0.400 | NA |

Unit: mm

### LAND PATTERN

```
  D1   D2  D3  D4
  C1   C2  C3  C4
  B1   B2  B3  B4
  A1   A2  A3  A4

  0.24mm
  0.4mm  0.4mm  0.4mm  0.4mm
```

---

## TAPE AND REEL INFORMATION FOR AW8737AFCR

### Dimensions and Pin 1 Orientation

| D1 (mm) | D0 (mm) | A0 (mm) | B0 (mm) | K0 (mm) | P0 (mm) | P1 (mm) | P2 (mm) | W (mm) | Pin 1 Quadrant |
|---------|---------|---------|---------|---------|---------|---------|---------|--------|----------------|
| 178.0±1.0 | 8.4 +2.0/-0.0 | 1.81±0.05 | 1.81±0.05 | 0.76±0.05 | 2.00±0.05 | 4.00±0.10 | 4.00±0.10 | 8.00 +0.30/-0.10 | Q1 |

### Notes

1. ALL DIMS IN MM
2. MATERIAL: BLACK CONDUCTIVE PC
3. 10 pocket hole pitch cumulative tolerance ±0.20mm
4. The meander of the tape is within 1mm in 250mm
5. Surface resistance 1×10E4 < Rs < 1×10E11 OHMS
6. Friction Voltage < 100V

---

## REFLOW SOLDERING CURVE

| Parameter | Specification |
|-----------|-------------|
| Average Ramp-up Rate (from 25°C to Peak) | Max. 3°C/sec |
| Preheat time to be Maintained | 60sec~120sec |
| Temperature within 5°C of Actual Peak Temperature (Tp) | 60sec~150sec |
| Peak Temperature (Reflow) | 250°C~260°C |
| Time within 5°C of Actual Peak Temperature (Tp) | 20sec~40sec |
| Ramp-down Rate | Max. 6°C/sec |
| Time from 25°C to Peak | Max. 8min |

---

## VERSION INFORMATION

| Version | Date | Description |
|---------|------|-------------|
| V1.0 | 2022-03-07 | AW8737AFCR datasheet V1.0 released |
| V1.1 | 2022-03-14 | Modify the application mode |
| V1.2 | 2022-07-20 | Update Anti-Interference One-Wire Pulse Control Functional Diagram |

---

## DISCLAIMER

Information in this document is believed to be accurate and reliable. However, Shanghai AWINIC Technology Co., Ltd (AWINIC Technology) does not give any representations or warranties, expressed or implied, as to the accuracy or completeness of such information and shall have no liability for the consequences of use of such information.

AWINIC Technology reserves the right to make changes to information published in this document, including without limitation specifications and product descriptions, at any time and without notice. Customers shall obtain the latest relevant information before placing orders and shall verify that such information is current and complete. This document supersedes and replaces all information supplied prior to the publication hereof.

AWINIC Technology products are not designed, authorized or warranted to be suitable for use in medical, military, aircraft, space or life support equipment, nor in applications where failure or malfunction of an AWINIC Technology product can reasonably be expected to result in personal injury, death or severe property or environmental damage. AWINIC Technology accepts no liability for inclusion and/or use of AWINIC Technology products in such equipment or applications and therefore such inclusion and/or use is at the customer's own risk.

Applications that are described herein for any of these products are for illustrative purposes only. AWINIC Technology makes no representation or warranty that such applications will be suitable for the specified use without further testing or modification.

All products are sold subject to the general terms and conditions of commercial sale supplied at the time of order acknowledgement.

Nothing in this document may be interpreted or construed as an offer to sell products that is open for acceptance or the grant, conveyance or implication of any license under any copyrights, patents or other industrial or intellectual property rights.

Reproduction of AWINIC information in AWINIC data books or data sheets is permissible only if reproduction is without alteration and is accompanied by all associated warranties, conditions, limitations, and notices. AWINIC is not responsible or liable for such altered documentation. Information of third parties may be subject to additional restrictions.

Resale of AWINIC components or services with statements different from or beyond the parameters stated by AWINIC for that component or service voids all express and any implied warranties for the associated AWINIC component or service and is an unfair and deceptive business practice. AWINIC is not responsible or liable for any such statements.

---

*All trademarks are the property of their respective owner*

*www.awinic.com*

*Copyright © 2022 SHANGHAI AWINIC TECHNOLOGY CO., LTD*
