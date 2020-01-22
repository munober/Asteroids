# asteroids
This repository hosts a modern implementation of the classic Atari arcade game Asteroids I've written with a friend for a lab project.
It has been written in C with FreeRTOS and ugfx and will run on STM32F4 boards -- in this specific case, a Discovery model with a jumper-connected cheapo joystick module/shield were used. It also features a multiplayer mode, in which 2 boards can be connected and synced via UART, allowing 2 players to control different spacecraft while playing on the same screen. The code is pretty self-explanatory.