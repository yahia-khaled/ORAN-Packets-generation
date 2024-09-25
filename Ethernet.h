#pragma once
#ifndef ETHERNET_H_
#define ETHERNET_H_

#include <string>
class Ethernet_ORAN
{
private:
    int LineRate;
    int CaptureSizeMs;
    int MinNumOfIFGsPerPacket;
    unsigned long long int DestAddress;
    unsigned long long int SourceAddress;
    long long int Preamble_SFD_header;
    float time_now;
    int MaxPacketSize;
    int SCS;
    int MaxNrb;
    int *ecpri_payload;
    std::string PayloadType;
    std::string Payload;
    int NrbPerPacket;
    int number_of_burst;
    int PacketSize;
    int number_of_slot_per_frame;
    int number_of_symbols_per_frame;
    long long int current_byte;
    void printByte(long long int number, int number_of_bytes);
    int *CalculateCRC(int start_index, int end_index);
    void SendIFGs(int);
    int concatenateBits(int a, int b, int numBitsB);
public:
    Ethernet_ORAN();
    void ReadConfig(std::string filename);
    void CreateOutput(std::string output_file);
    ~Ethernet_ORAN();
};

#endif
