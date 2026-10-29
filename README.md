# Autonomous Obstacle Avoidance Rover with 5-Sector Dynamic Ultrasonic Scanning

An autonomous mobile robotic platform engineered around the **Arduino Uno** and the **Adafruit L293D Motor Shield v1**. The rover utilizes an ultrasonic sonar transceiver mounted to a micro-servo for dynamic 5-sector environment mapping ($0^\circ$, $45^\circ$, $90^\circ$, $135^\circ$, $180^\circ$). It features proportional linear deceleration within proximity zones, intelligent clearance arbitration, and trapped-zone recovery via autonomous U-turns.

---

## Technical Specifications & Features

- **Proportional Velocity Deceleration:** Dynamically scales motor drive voltages using a linear transfer function between the warning boundary ($25\text{ cm}$) and critical emergency stop boundary ($12\text{ cm}$) to minimize chassis momentum before turning.
- **5-Sector Spatial Spatial Sweep:** Upon detecting an obstacle within the critical threshold ($\le 12\text{ cm}$), the rover stops locomotion and sweeps its front sensor array across five discrete heading vectors ($0^\circ, 45^\circ, 90^\circ, 135^\circ, 180^\circ$).
- **Dynamic Clearance Arbitration:**
  - **Right Turn Sector ($0^\circ, 45^\circ$):** Engages differential pivot right (`motorL: FORWARD`, `motorR: BACKWARD`).
  - **Left Turn Sector ($135^\circ, 180^\circ$):** Engages differential pivot left (`motorL: BACKWARD`, `motorR: FORWARD`).
  - **Dead-End / Trap Recovery ($< 15\text{ cm}$ Across All Sectors):** Executes a sustained $180^\circ$ escape U-turn maneuver.
- **Calibrated Motor Asymmetry Bias:** Compensates for gearbox and mechanical drag differences (`MAX_SPEED_L = 150`, `MAX_SPEED_R = 120`, `MIN_SPEED_L = 116`, `MIN_SPEED_R = 90`) to maintain balanced straight-line trajectories.
- **Isolated Inductive Power Rail:** High-current DC motors are supplied through the shield's `EXT_PWR` rail, preventing microcontroller resets caused by inductive back-EMF spikes.

---

## Mathematical Transfer Function

During forward navigation, the rover regulates wheel speed as a function of instantaneous frontal obstacle distance $d$:

$$
v(d) = 
\begin{cases} 
v_{\max}, & \text{if } d > d_{\text{slow}} \\
v_{\min} + (v_{\max} - v_{\min}) \cdot \left(\frac{d - d_{\text{stop}}}{d_{\text{slow}} - d_{\text{stop}}}\right), & \text{if } d_{\text{stop}} < d \le d_{\text{slow}} \\
0, & \text{if } d \le d_{\text{stop}} \quad (\text{Triggers Scan Routine})
\end{cases}
$$

*Where $d_{\text{stop}} = 12\text{ cm}$ and $d_{\text{slow}} = 25\text{ cm}$.*

---

## System Architecture & Circuit Schematic

### Circuit Connection Diagram
<p align="center">
  <img src="docs/Autonomous%20Obstacle%20Avoidance%20Rover%20Schematic%20Connection%20Diagram.png" alt="Autonomous Obstacle Avoidance Rover Schematic" width="100%">
</p>

> **Vector Document:** The high-resolution vector schematic is archived under [`docs/Autonomous Obstacle Avoidance Rover Schematic Connection Diagram.pdf`](docs/Autonomous%20Obstacle%20Avoidance%20Rover%20Schematic%20Connection%20Diagram.pdf).

---

## Hardware Pinout Matrix

| Subsystem / Peripheral | Module Pin | Arduino / Shield Pin | Signal Type | Electrical Spec | Operational Role |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Pan Servo Motor** | Signal Lead | **Pin 9 (SERVO_2 Header)** | PWM Output | `5VDC` Regulated | Sweep actuator ($0^\circ - 180^\circ$ panning) |
| | VCC / GND | `5V` / `GND` | Power Rail | `5VDC` Rail | Servo motor power |
| **HC-SR04 Sonar Sensor** | TRIG | **Analog Pin A3** | Digital Output | `5V` Logic | 10µs ultrasonic sonic burst trigger |
| | ECHO | **Analog Pin A2** | Digital Input | `5V` Logic | High pulse duration echo receiver |
| | VCC / GND | `5V` / `GND` | Power Rail | `5VDC` Rail | Transceiver power supply |
| **Left Drive Motor (BO)** | Terminal Leads | **Shield Terminal M3** | PWM / H-Bridge | Up to `12VDC` | Left wheel propulsion (L293D Channel) |
| **Right Drive Motor (BO)** | Terminal Leads | **Shield Terminal M2** | PWM / H-Bridge | Up to `12VDC` | Right wheel propulsion (L293D Channel) |
| **Power Management** | Battery (+/-) | **Shield EXT_PWR Block** | Power In | `7.4V - 11.1V` | Dedicated high-current motor supply rail |

---

## Navigation & Decision Matrix

| Environmental Condition | Front Distance ($d$) | Selected Scan Heading | Navigation State | Left Motor (M3) | Right Motor (M2) | Duration |
| :--- | :---: | :---: | :--- | :--- | :--- | :---: |
| **Clear Path** | $> 25\text{ cm}$ | $90^\circ$ (Forward) | Full Speed Forward | `FORWARD` (Speed: 150) | `FORWARD` (Speed: 120) | Continuous |
| **Approaching Barrier** | $12\text{ cm} < d \le 25\text{ cm}$ | $90^\circ$ (Forward) | Linear Deceleration | `FORWARD` (Speed: $116-150$) | `FORWARD` (Speed: $90-120$) | Continuous |
| **Critical Proximity** | $\le 12\text{ cm}$ | Dynamic Sweep | Halt & Environment Scan | `RELEASE` (Off) | `RELEASE` (Off) | $2000\text{ ms}$ |
| **Right Clearance Best** | Blocked Ahead | $0^\circ \text{ or } 45^\circ$ | Differential Pivot Right | `FORWARD` (Speed: 150) | `BACKWARD` (Speed: 120) | $350\text{ ms}$ |
| **Left Clearance Best** | Blocked Ahead | $135^\circ \text{ or } 180^\circ$ | Differential Pivot Left | `BACKWARD` (Speed: 150) | `FORWARD` (Speed: 120) | $450\text{ ms}$ |
| **Enclosed / Dead-End** | Blocked Ahead | $\text{All } < 15\text{ cm}$ | Escape U-Turn | `BACKWARD` (Speed: 150) | `FORWARD` (Speed: 120) | $800\text{ ms}$ |

---

## Software & Library Requirements

Libraries declared in `libraries.txt`:
- `AFMotor` (Adafruit Motor Shield v1 Library) — `>= 1.0.1`
- `Servo` (Built-in Arduino AVR Core Library)

```bash
# Search and install via Arduino IDE Library Manager:
Adafruit Motor Shield library
```

---

## Project Structure

```text
Obstacle-Avoidance-Rover/
├── .gitignore
├── LICENSE
├── libraries.txt
├── README.md
├── src/
│   └── Obstacle_Avoidance_Rover_AFM.ino
└── docs/
    ├── Autonomous Obstacle Avoidance Rover Schematic Connection Diagram.pdf
    └── Autonomous Obstacle Avoidance Rover Schematic Connection Diagram.png
```

---

## Getting Started & Calibration

### 1. Hardware Assembly
1. Mount the Adafruit Motor Shield onto the Arduino Uno headers.
2. Remove the **PWR Jumper** on the shield to isolate logic from the motors.
3. Wire your DC motors to terminals **M3** (Left) and **M2** (Right).
4. Connect the HC-SR04 trigger to **A3** and echo to **A2**.
5. Plug the micro servo into the **SERVO_2** header (Pin 9) and attach the ultrasonic sensor to the servo horn.
6. Supply $7.4\text{V} - 11.1\text{V}$ via the **EXT_PWR** screw terminal.

### 2. Firmware Upload
1. Open `src/Obstacle_Avoidance_Rover_AFM.ino` in the Arduino IDE.
2. Select **Arduino Uno** under `Tools > Board`.
3. Select the appropriate serial COM port and click **Upload**.

### 3. Drift & Turn Calibration
- **Straight Drive Balancing:** If the chassis veers left during forward driving, decrease `MAX_SPEED_L` or increase `MAX_SPEED_R`.
- **Threshold Tuning:** Modify `STOP_DIST` ($12\text{ cm}$) and `SLOW_DIST` ($25\text{ cm}$) to match the braking distance and weight of your chassis.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.