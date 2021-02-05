# DMM-Project: Group5

QRCode-Generator
Members: Allaa El-Khodr, Bker Sawalha, Mayiz Zgheib, Lucas Kneffel Otal

This a port of the QR-Code-generator (Copyright (c) Project Nayuki) to the ATmega1284P-µC (for further details refer to files in "µ" folder) by Group 5.
UART input added and minor changes were done to the source code by Project Nayuki.

Basically, this programm encodes a message received via UART and displays the equivalent QR-Code. 
It auto-detects size and encoding level depending on the input message.
Message must fit the 10-H standards at most given by LCD size.
