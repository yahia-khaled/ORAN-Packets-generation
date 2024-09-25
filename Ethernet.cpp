#include "Ethernet.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <unordered_map>

// CRC polynomial and table
const int CRC32_POLY = 0x04C11DB7;
const int CRC32_INIT = 0xFFFFFFFF;
const int CRC32_XOR_OUT = 0xFFFFFFFF;

static int last_byte_position;
#define IFG 0x07

using namespace std;

ofstream outFile;

Ethernet_ORAN::Ethernet_ORAN()
{
    cout << "Default Constructor called!" << endl;
}

void Ethernet_ORAN::ReadConfig(string filename)
{

    // set configuration of Etherent
    std::ifstream file(filename);
    std::unordered_map<std::string, std::string> configMap;
    std::string line;

    // Read the file line by line
    while (std::getline(file, line))
    {
        // Ignore comments (starting with '//')
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos)
        {
            line = line.substr(0, commentPos); // Remove comment part
        }

        // Trim any leading/trailing spaces
        line.erase(0, line.find_first_not_of(" \t")); // Left trim
        line.erase(line.find_last_not_of(" \t") + 1); // Right trim

        // Find the position of the '=' character
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos)
        {
            std::string key = line.substr(0, delimiterPos);    // Extract key
            std::string value = line.substr(delimiterPos + 1); // Extract value

            // Trim spaces around key and value
            key.erase(0, key.find_first_not_of(" \t"));     // Left trim key
            key.erase(key.find_last_not_of(" \t") + 1);     // Right trim key
            value.erase(0, value.find_first_not_of(" \t")); // Left trim value
            value.erase(value.find_last_not_of(" \t") + 1); // Right trim value

            // Store key-value pair in the map
            configMap[key] = value;
        }
    }

    // set valuse in member of class
    LineRate = stoi(configMap["Eth.LineRate"]);
    CaptureSizeMs = stoi(configMap["Eth.CaptureSizeMs"]);
    MinNumOfIFGsPerPacket = stoi(configMap["Eth.MinNumOfIFGsPerPacket"]);
    DestAddress = stoull(configMap["Eth.DestAddress"], nullptr, 16);
    SourceAddress = stoull(configMap["Eth.SourceAddress"], nullptr, 16);
    MaxPacketSize = stoi(configMap["Eth.MaxPacketSize"]);
    SCS = stoi(configMap["Oran.SCS"]);
    MaxNrb = stoi(configMap["Oran.MaxNrb"]);
    NrbPerPacket = stoi(configMap["Oran.NrbPerPacket"]);
    PayloadType = configMap["Oran.PayloadType"];
    Payload = configMap["Oran.Payload"];

    // check inputs validity of the file

    if (MaxNrb == 0)
    {
        MaxNrb = 273;
    }

    //check Max packet size if is valid or not for given NRB
    while (MaxPacketSize < (26+8+5+NrbPerPacket*12*2))
    {
        cout<<"enter valid MAX packet size :";
        cin>>MaxPacketSize;
    }
    

    // determine number of slots per frame

    switch (SCS)
    {
    case 15:
        number_of_slot_per_frame = 1;
        break;
    case 30:
        number_of_slot_per_frame = 2;
        break;
    case 60:
        number_of_slot_per_frame = 4;
        break;
    case 120:
        number_of_slot_per_frame = 8;
        break;
    case 240:
        number_of_slot_per_frame = 16;
        break;
    default:
        cout << "invalid SCS please try again with valid values" << endl;
        break;
    }

    ifstream infile(Payload); // Replace with your actual file name
    int i_sample, q_sample;
    int sampleCount = 0;

    // First, count the number of I-Q sample pairs
    while (infile >> i_sample >> q_sample)
    {
        sampleCount++;
    }

    infile.clear();                 // Clear the EOF flag
    infile.seekg(0, std::ios::beg); // Go back to the beginning of the file

    // Create a dynamic array to store the payload (each sample has 2 bytes: I and Q)
    ecpri_payload = new int[sampleCount * 2];

    int index = 0;
    while (infile >> i_sample >> q_sample)
    {
        // Clamp values to fit in 8-bit signed integers
        if (i_sample < -128)
            i_sample = -128;
        if (i_sample > 127)
            i_sample = 127;
        if (q_sample < -128)
            q_sample = -128;
        if (q_sample > 127)
            q_sample = 127;

        // Convert to unsigned 8-bit format
        uint8_t i_byte = static_cast<uint8_t>(static_cast<int8_t>(i_sample));
        uint8_t q_byte = static_cast<uint8_t>(static_cast<int8_t>(q_sample));

        // Store the I and Q samples in the dynamic array
        ecpri_payload[index++] = i_byte;
        ecpri_payload[index++] = q_byte;
    }

    infile.close();
}

void Ethernet_ORAN ::CreateOutput(string output_file)
{

    last_byte_position = 1;
    current_byte = 0;
    int frame_id = 0;
    int subframe_id  = 0;
    int slot_id = 0;
    int symbol_id = 0;
    int startPrbu = 0;

    outFile.open(output_file, std::ios::trunc);

    long int total_size_IQ_samples = sizeof(ecpri_payload) / sizeof(int);


    //calculate times for byte, slot and symbol

    // float byte_duration = 8.0 / (LineRate); // time in nanosecond
    // float slot_duration = 1.0 / (number_of_slot_per_frame);
    // float symbol_duration = slot_duration / 14.0;

    // int ORAN_packet_size_IQ_samples_in_byte = NrbPerPacket * 12 * 2;

    int total_number_of_ORAN_packets_in_10ms = ceil(MaxNrb/NrbPerPacket) * 14 * number_of_slot_per_frame * 10;
    int number_of_packet_per_symbol = ceil(MaxNrb/NrbPerPacket);

    // ethernet payload

    int size_of_IQ_samples_in_byte = 12 * NrbPerPacket * 2;

    int size_of_etherent_frame = 26 + 8 + 9 + size_of_IQ_samples_in_byte;

    PacketSize = 8 + 5 + size_of_IQ_samples_in_byte ;

    int sequence_ID = 0;

    for (int l = 0; l < (CaptureSizeMs / 10); l++)
    {    

    for (int i = 0; i < total_number_of_ORAN_packets_in_10ms; i++)
    {

        // Preamble and SFD
        printByte(0xFB555555, 32);
        current_byte = current_byte + 1;
        printByte(0x555555D5, 32);

        // send Src and Des Address
        printByte((DestAddress >> 16), 32);
        printByte((0x00000000FFFF & DestAddress), 16);
        printByte((SourceAddress >> 32), 16);
        printByte((0x0000FFFFFFFF & SourceAddress), 32);
        // send Ether Type
        printByte(PacketSize, 16);
        // header of ecpri
        printByte(0x00, 8);

        // insert message of ecpri
        printByte(0x00, 8);

        // insert payload size
        int payload_size = (size_of_IQ_samples_in_byte + 9);

        // outFile << hex << setw(4) << setfill('0') << payload_size;
        printByte(payload_size, 16);

        // insert RTC ecpriRTCId/PCId
        printByte(0x00, 16);

        // insert sequence ID
        printByte(sequence_ID, 16);

        sequence_ID = (sequence_ID + 1) % 256;
        // last_byte_position = 5;

        current_byte = current_byte + 16;

        int start_of_sample_index = (i * size_of_IQ_samples_in_byte) % total_size_IQ_samples;
        int end_of_sample_index = start_of_sample_index + size_of_IQ_samples_in_byte;

            // construct ORAN header
            printByte(0, 8);
            current_byte = current_byte + 1;
            // insert frame id
            // time_now = current_byte * byte_duration;

            frame_id = (i) / (number_of_packet_per_symbol * 14 * number_of_slot_per_frame * 10);

            // printByte(int((time_now * pow(10, -6))/10), 8);
            printByte(frame_id, 8);
            current_byte = current_byte + 1;

            // insert subframe id

            subframe_id = ((i)/(number_of_slot_per_frame*14*number_of_packet_per_symbol))%10;
            // printByte((int(time_now * pow(10, -6)) % 10), 4);
            printByte(subframe_id,4);

            // insert slot id
            // slot_id = ((int((time_now * pow(10, -6)) / slot_duration)) % number_of_slot_per_frame);
            // symbol_id = ((int((time_now * pow(10, -6)) / symbol_duration)) % 14);

            slot_id = ((i)/(14*number_of_packet_per_symbol))%number_of_slot_per_frame;

            symbol_id = ((i) / (number_of_packet_per_symbol)) % 14;

            printByte((slot_id>>2), 4);

            // insert symbol id with 2 LSB bits of slot id
            printByte(concatenateBits((slot_id & 3),symbol_id,6), 8);

            current_byte = current_byte + 2;

            // //insert section id
            int section_id = 0;

            printByte(section_id,12);

            //insert rb, syminc and first part of startprbu
            int rb = 0;                     //default value to use every RBs
            int symInc = 0;                 // 0b=use the current symbol number
            

            int result = concatenateBits(rb, symInc, 1);
            result = concatenateBits(result, (startPrbu>>8), 2);

            printByte(result,4);
            current_byte = current_byte + 2;

            // insert remaining part of startprbu
            printByte((startPrbu & 0b11111111),8);
            
            startPrbu = (startPrbu + NrbPerPacket) % MaxNrb;
            //inset numprbu
            printByte(NrbPerPacket, 8);

            current_byte = current_byte + 2;

            for (int k = start_of_sample_index; k < end_of_sample_index; k++)
            {
                printByte(ecpri_payload[k], 8);
                current_byte = current_byte + 1;
            }


        int *CRC = CalculateCRC(start_of_sample_index, end_of_sample_index);
        for (int l = 0; l < 4; l++)
        {
            printByte(CRC[l], 8);
        }

        current_byte = current_byte + 4;
        while (last_byte_position != 1)
        {
            printByte(IFG, 8);
            current_byte = current_byte + 1;
        }

        SendIFGs(MinNumOfIFGsPerPacket * 8);
        current_byte = current_byte + 12;
    }

    //number of bytes of IFGs when packets finish

    int total_number_in_bytes_in_10ms = (LineRate * 10 * pow(10,6))/8;

    int total_number_of_bytes_for_IFG = total_number_in_bytes_in_10ms - size_of_etherent_frame * total_number_of_ORAN_packets_in_10ms;

    current_byte = current_byte + total_number_of_bytes_for_IFG;

    SendIFGs(total_number_of_bytes_for_IFG*8);

    }
}

int *Ethernet_ORAN ::CalculateCRC(int start_index, int end_index)
{
    // CRC32 table
    static int table[256];
    static bool tableComputed = false;

    if (!tableComputed)
    {
        for (int i = 0; i < 256; ++i)
        {
            int crc = i;
            for (int j = 8; j > 0; --j)
            {
                if (crc & 1)
                {
                    crc = (crc >> 1) ^ CRC32_POLY;
                }
                else
                {
                    crc = crc >> 1;
                }
            }
            table[i] = crc;
        }
        tableComputed = true;
    }

    int crc = CRC32_INIT;
    for (int i = start_index; i < end_index; ++i)
    {
        int byte = ecpri_payload[i];
        int tableIndex = (crc ^ byte) & 0xFF;
        crc = (crc >> 8) ^ table[tableIndex];
    }

    int output = crc ^ CRC32_XOR_OUT;

    int *CRCResult = new int[4];

    for (int i = 0; i < 4; i++)
    {
        int shift = i * 8;
        CRCResult[i] = (output >> shift) & 0xFF;
    }
    return CRCResult;
}

void Ethernet_ORAN ::SendIFGs(int total_IFGs_size)
{

    int number_of_IFG = total_IFGs_size / 8;

    for (int i = 0; i < (number_of_IFG / 4); i++)
    {
        for (int k = 0; k < 4; k++)
        {
            printByte(IFG, 8);
        }
    }
}

void Ethernet_ORAN ::printByte(long long int number, int number_of_bits)
{

    float byte_value = number_of_bits / 8.0;
    if (last_byte_position == 1)
    {
        if (current_byte != 0)
        {
            outFile << "\n";
        }
        outFile << setw(8) << setfill(' ');
        outFile << hex << setw(byte_value * 2) << setfill('0') << number;
        last_byte_position = int(last_byte_position + byte_value * 2) % 8;
    }
    else
    {
        outFile << hex << setw(byte_value * 2) << setfill('0') << number;
        last_byte_position = int(last_byte_position + byte_value * 2) % 8;
    }
}

int Ethernet_ORAN ::concatenateBits(int a, int b, int numBitsB)
{
    // Shift `a` left by the number of bits in `b`
    int result = (a << numBitsB) | b;
    return result;
}

Ethernet_ORAN::~Ethernet_ORAN()
{
    cout << "Default deConstructor called!" << endl;
}
