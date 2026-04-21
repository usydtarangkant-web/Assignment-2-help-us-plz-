##1) The details about the project (group members, roles, responsibilities) 

###Description of the project
This project develops a modular embedded software system for the STM32F3 Discovery board. 
The project includes digital I/O, timer control, UART communication, and I2C sensor interfacing. 
Each part was developed as a separate module to improve code structure and reusability. 
The final goal of the project is to integrate these modules into a complete system. 
In the integrated system, one STM32 board reads heading data from the magnetometer and sends it to a second board through UART. 
The second board then uses the received data to control a servo motor or display the heading using LEDs.

###Software and Hardware
**Software**
- STM32CubeIDE
- C programming language
- GitHub repository

**Hardware**
- STM32F3 Discovery board
- Onboard LEDs
- User button
- Magnetometer sensor
- UART connection between two STM32 boards
- Hobby servo motor
- Jumper wires
- Breadboard
- Power supply

###Team Administration
Anu Prasanna (SID: 540724534):
Completed task 7.2 of the project
Wrote README for: Team Administration and task 7.2
Wrote some of the minutes for team meetings
Jennifer Zhang (SID: 520430295):
Completed task 7.4 of the project, helped debug 7.5
Wrote README for: task 7.4, and task 7.5
Wrote some of the minutes for team meetings
Harry Wang (SID: 540047592):
Completed task 7.1 of the project
Wrote README for: Description of the Project, Software and Hardware, task 7.1
Tarang Kant (SID: 550566676):
Completed task 7.3 and task 7.5 of the project
Wrote README for: task 7.3, and task 7.5

##2) Exercise 7.1: Digital I/O

###Summary
This part of the project implements the digital I/O functions for the STM32F3 Discovery board.  
It includes a generic GPIO module, an LED module, a button module, and a SysTick timing module.  
The design is modular so that each module performs one task and can be reused in other parts of the project.  
The button module supports a callback function, and the LED module supports non-blocking updates with a minimum time interval between state changes.

###Usage
The GPIO module is used as the base interface for digital input and output pins.
The LED module is built on top of the GPIO module and is used to control the onboard LEDs.  
The button module is also built on top of the GPIO module and is used to read the user button.  
The SysTick module provides timing in milliseconds and is used by the LED module to limit how quickly the LED state can change.
A simple example is:
- press the user button
- the button callback is triggered
- the target state of LED4 is changed
- `led_update()` applies the real LED change when the minimum time interval condition is met
###Valid input
- GPIO port must be a valid STM32 GPIO port such as `GPIOA` or `GPIOE`
- GPIO pin must be a valid pin number for the selected port
- GPIO mode must be either input or output
- LED value must be a valid `led_t` value from `LED3` to `LED10`
- LED state must be `true` or `false`
- Button callback may be a valid function pointer or `0`
- The LED minimum interval must be given in milliseconds
###Functions and modularity
The implementation is divided into four modules:
- GPIO 
  Provides generic functions to initialise, read, write, and toggle GPIO pins.
- LED
  Controls the onboard LEDs.  
  It stores the current LED state and target LED state internally.  
  LED updates are handled in `led_update()` so that state changes are non-blocking.
- Button
  Reads the user button and supports a callback function when a new button press is detected.
- Systick
  Provides a millisecond counter used for timing-based updates.
###Testing
The following tests were used for this part:
- GPIO output test: configured LED pins as outputs and checked that LEDs could be turned on and off
- GPIO input test: configured the user button pin as input and checked that the button state could be read correctly
- Button callback test: confirmed that the callback function was called only on a new button press
- LED toggle test: confirmed that pressing the button changed the target state of LED4
- LED timing test: confirmed that the LED state only changed after the minimum interval had passed
- SysTick test: verified that the millisecond counter increased correctly over time
###Notes
The LED module was designed so that `led_set()` and `led_toggle()` only update the target state.  The real LED change is applied later in `led_update()` when the timing condition is satisfied. This avoids using a blocking delay inside the LED control functions and supports a cleaner modular design.

##3) Exercise 7.2: Timer Interface

###Summary
This module interfaces with the hardware timers on the STM32 Discovery board to generate a pulse width modulation (PWM) signal that drives a hobby servo–with the ability to change the position between clockwise, centre and counterclockwise; and a one-shot event is also triggered after a delay.
###Usage
First, this module can be utilised to set the servo position in either:
Clockwise (SERVO_PULSE_CW_US) which sets the servo at 1ms
Centre (SERVO_PULSE_CENTRE_US) which sets the servo at 1.5ms
Counterclockwise (SERVO_PULSE_CCW_US) which sets the servo at 2ms
When the user changes the position on the main.c module (Servo_SetPulse(USER INPUT), and runs the program: 
The servo receives the required pwm signal and holds the required position
The Systick interrupt runs every 100us until the timer counts down from the set period to 0 (this period can be changed from initial period of 20ms to another period in Timer_SetPeriod(&g_ledTimer, USER INPUT))
This timer can be seen in PE8
Then a callback is triggered to reload the timer
The one-shot occurs after a specified delay (i.e 12000ms) after the user runs the program:
The timer is reset and counts down for 12000ms (12 seconds) or any other specified value
When the callback occurs, the timer is disabled completely (showcased in PE9)
###Valid input
As stated earlier, the user needs to input three values: the position and the period after the initial period and this can be done by changing:
Servo_SetPulse(USER INPUT) in main.c to SERVO_PULSE_CW_US, SERVO_PULSE_CENTRE_US, or SERVO_PULSE_CCW_US
Timer_SetPeriod(&g_ledTimer, USER INPUT) in main.c to any specified period
Timer_OnceMs(&g_shotTimer, USER INPUT, OneShotCallback) in main.c to any specified delay value
###Functions and modularity
To successfully implement this task, it was divided into three modules:
main.c: which initialises the board, enables the clocks, toggles required LEDs, sets the interrupt, and ties together all other modules in a loop
timer.c: which deals with part a, b and d of the task. This module allows 8 different software timers to be stored, changes between ms to us to timer ticks for the servo, stores a pointer for the callback, starts and stops the periodic timer, allows a change in the period, and has functions for the one-shot (one for the servo in ticks)
pwm.c: which deals with part c; and so sets the PWM signal for high and low, initialises the servo, defines the different servo positions, defines the callback functions for the periodic and one-shot functions, and defines the timer ticks to the Systick interrupt
The corresponding timer.h and pwm.h modules state the functions that are present in each module.
###Testing
To test whether the program is working, in debug mode set breakpoints at:
int main(void) to ensure main is reached
void Systick_Handler(void) to ensure the interrupt works
static void LED_ToggleCallback (void), if this function is reached multiple times then the periodic timer works (this is also verified by viewing PE8 on the board)
static void LED_OneShotCallback (void), if this function is only stopped at once then the oneshot timer works (also verified by viewing PE9 on the board)
Change the positions in main.c, the period in main.c, and the delay in main.c to verify these functions work by observing how this affects the servo, PE8 and PE9
###Notes
The units were converted from ms to us (where 1 tick is 100us) to allow a more accurate representation of the centre position–and these were represented in timer ticks for the pwm signal and the servo.

##4) Exercise 7.3: Serial Interface

###Summary
This module provides UART communication for the STM32F3 Discovery Board. It was designed to send and receive structured data packets between boards, while also providing a debug serial output to a computer through USART1. The module supports packet framing, checksums, message identifiers, and interrupt-based receiving.
###Usage
Program the transmitter firmware onto the first STM32 board.
Program the receiver firmware onto the second STM32 board.
Connect the UART transmit and receive pins between the boards, with a shared ground.
Power both boards.
The transmitter board automatically sends data packets.
The receiver board waits for packets and updates LEDs based on received data.
Debug text can be viewed on a serial console through USART1.
###Valid input
Structures or arrays of bytes prepared on the transmitter board.
Text strings for debug output.
UART packets containing start byte, message type, payload length, payload data, checksum, and stop byte.
Incoming serial bytes received through UART interrupts.

###Functions and modularity
The serial interface task was divided across two STM32 boards, with each source file performing a separate role.

Transmitter board modules:

main.c controls the transmitter logic and prepares data packets for transmission.
serial.c handles UART communication, packet formatting, message sending, and debug output.
led.c controls onboard LEDs used to indicate successful transmitting or errors.

Receiver board modules:

main.c controls the receiver logic and processes received messages.
serial.c receives incoming UART data, checks packet validity, and triggers callbacks.
led.c controls onboard LEDs used to indicate successful reception or errors.

Each module has a specific responsibility, making the software easier to debug, maintain, and reuse in later exercises.
###Testing
The system was tested using two STM32 boards connected through UART. Packets were transmitted successfully, checksums were validated, receive callbacks operated correctly, and LEDs responded to valid or invalid packets (see 7.5 for specific LED mapping). Debug strings were also displayed through a serial terminal.
###Notes
Both boards must use the same baud rate.
TX connects to RX and ground must be shared.
Packet framing improves reliable communication.
Interrupt-based reception reduces CPU polling and improves responsiveness.

##5) Exercise 7.4: I2C Sensor Interfacing

###Summary
This module interfaces with the STM32F3 Discovery Board’s onboard magnetometer using I2C communication. It reads raw magnetic field data, calculates a compass heading, and stores the results in a structure with a timestamp.
###Usage
Include the module in the program with #include "compass.h".
Call compassInit() once at the start of the program to initialise the I2C interface and configure the sensor.
Create a CompassData structure to store the readings.
Call compassRead(&compass) repeatedly in the main loop to update the structure with the latest raw values, heading, and timestamp.
###Valid input
The module communicates directly with the onboard compass sensor through I2C1. No user input is required. Valid output data includes:
raw x, y, z values
heading in degrees
timestamp in milliseconds
###Functions and modularity
###Testing
The module was tested by rotating the board and observing changing heading values in the debugger with a breakpoint. An LED direction indication was also added to visualise the headings. Raw x, y, and z values were also confirmed to update.
###Notes
Uses I2C1 communication
Heading is calculated using atan2(y,x)
X and Y axis are offset to ensure correct headings
Board should remain level for best heading accuracy
Nearby metal objects may affect readings

##6) Exercise 7.5: Integration Task

###Summary
This task combines the modules developed in earlier exercises into a complete two-board embedded system. The transmitter STM32 board reads heading data from the onboard compass sensor, applies north-reference calibration, and sends the data over UART. A button interrupt is used to change the display mode. The receiver STM32 board receives the transmitted data and either moves a servo motor according to heading or displays the heading using onboard LEDs.
###Usage
Program the transmitter firmware onto the first STM32 board.
Program the receiver firmware onto the second STM32 board.
Connect the UART transmit and receive pins between the two boards, with a shared ground connection.
Power both boards.
Rotate the transmitter board to change the heading reading.
Press the onboard button on the transmitter board to switch between servo mode and LED display mode on the receiver board.
###Valid input
Raw magnetometer data from the onboard compass sensor on the transmitter board.
Button press interrupts from the transmitter board user button.
UART packets containing heading, timestamp, and display mode data.
Heading updates caused by rotating the transmitter board.
###Functions and modularity
The integration task was divided across two STM32 boards, with each source file handling a specific function.

Transmitter board modules:

main.c controls the overall transmitter operation, reads compass data, and sends packets over UART.
compass.c interfaces with the onboard magnetometer using I2C and calculates heading values.
button.c handles the interrupt-driven button input used to change display mode.
north_ref.c applies heading offset calibration so the displayed heading aligns with true north.
serial.c packages and transmits data to the receiver board using UART.

Receiver board modules:

main.c controls the receiver logic and processes incoming UART data.
serial.c receives and decodes UART packets.
pwm.c generates the PWM signal required to control the servo motor.
timer.c provides timing functions used for PWM generation.
led.c controls the onboard LEDs used to display heading direction.

Each module has a separate responsibility, improving readability, testing, maintenance, and reuse of code.
###Testing
The system was tested using two STM32 boards connected through UART. Compass data was successfully transmitted between boards. The servo responded to heading changes with LDs 4-7 updating as the board rotated. L10 would light up for checksum errors and serial errors, LD8 represents transmitting and receiving, and LD6 represents power to the board and running code. LD9 represents button functionality.

###Notes
Servo movement is scaled from 0–360° heading into the servo pulse range, using magnetometer readings.
UART baud rate must match on both boards.
TX connects to RX and ground must be common.
This design demonstrates modular embedded software integration using interrupts, communication, sensing, and actuation.


 
