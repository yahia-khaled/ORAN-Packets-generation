There are three CPP files,<br />
Ethernet.cpp<br />
have two main functions:read_configuration and generate output packets by following steps : <br />
1.read configuration<br />
from second_milestone.txt and read IQ samples from iq_sample.txt, then it checks output validity and consistency.
2. calculation of variables and initialization<br />
Calculate the number of ORAN packets, the number of Ethernet frames, and initialize variables used in the generation process, such as last_byte_position, which tracks the location where the last octet has been written, and different IDs for frame headers.
3. Generate headers and content of packets<br />
We start generating packets by generating different headers for 
Ethernet, ecpri, and ORAN headers, then we output IQ samples such that each sample is represented in 8 bits.
4. generate CRC bits<br />
We start to calculate CRC bits by passing the payload of an Ethernet packet to the function CRC, which returns an array that contains 32-bit CRC.
5. IFGs aligning and sending minimum IFGs<br />
Send IFGs such that packets are 4-byte aligned, then send 12 IFGs that are minimum required between packets.
6. Repeat 2-5 the above steps until the frame content is finished, then wait for frame duration to send the next frame content.

parser.cpp<br />
This code starts to take the output file from generation and extract the header information from it in the following steps:<br />
1. reading output file form generation of packets<br />
Start to read each hexadecimal digit and put it as a separate element in the array, then reuse this array for header extractions. <br />
2. Loop over file and search for preamble and SFD of frames<br />
There is a function that searches for the preamble sequence and starts to decode the hexadecimal digits to recognize the header of packets<br/>.
3. Start to recognize the packet header<br />
extract each header and assign it in an equivalent variable such that it will be used to create an output file that contains packet header information<br />
4. Generate an output file that contains the packet header.

main.cpp<br />
In this file, we just call functions to generate packets, then call functions for parsing the output file from generation.<br />

Makefile is used to automate the compilation and running of different CPP files.
