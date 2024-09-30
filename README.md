There are three CPP files,<br />
Ethernet.cpp<br />
have two main functions:<br />
first,read_configuration:<br />
that read configuration from second_milestone.txt and read IQ samples from iq_sample.txt, then it checks output validity and consistency.
second, generate an Ethernet frame in the following steps:
1. calculation of variables and initialization<br />
Calculate the number of ORAN packets, the number of Ethernet frames, and initialize variables used in the generation process, such as last_byte_position, which tracks the location where the last octet has been written, and different IDs for frame headers.
2. Generate headers and content of packets<br />
We start generating packets by generating different headers for 
Ethernet, ecpri, and ORAN headers, then we output IQ samples such that each sample is represented in 8 bits.
3. generate CRC bits<br />
We start to calculate CRC bits by passing the payload of an Ethernet packet to the function CRC, which returns an array that contains 32-bit CRC.
4. IFGs aligning and sending minimum IFGs<br />
Send IFGs such that packets are 4-byte aligned, then send 12 IFGs that are minimum required between packets.
5. Repeat the above steps until the frame content is finished, then wait for frame duration to send the next frame content.

parser.cpp
this code start to take output file from generation and extract the header informations from it in the following steps:<br />
1.reading output file form generation of packets<br />
start to read each hexadecimal digit and put as seperate element in array then retuen this array for header extractions.<br />
2.loop over file and search for preamble and SFD of frames<br />
there is function that search for the preamble sequence and start to decode the hexadeciaml digits to recognize the header of packets<br />.


