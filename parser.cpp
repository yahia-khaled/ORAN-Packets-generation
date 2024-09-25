#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include "parser.h"

using namespace std;

int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

int * readHexFromPosition(string filename,long long  int & size) {

    ifstream myfile(filename);

    string myline;

    int * hexa_array = (int *) malloc(sizeof(int) * 8);

    int count = 0;

    if (myfile.is_open())
    {
        while (myfile)
        {
            std::getline(myfile, myline);
            for (int i = 0; i < 8; i++)
            {
                hexa_array[count + i] = hex2int(myline[i]);
            }
            count = count + 8;
            size = size + 8;
            hexa_array = (int *)realloc(hexa_array, (sizeof(int) * 8 * (count/8 + 1)));
        }
    }

    return hexa_array;
}

long long int getHexFromPosition(int * arr , int & start , int & end,int size)
{
    start = end;
    end = end + size;
    long long int value = arr[start];
    for (int i = start+1; i < end; i++)
    {
        value = (value<<4) | arr[i];
    }
    return value;   
}

int searchForSequence(int *array, int arraySize, int startIndex)
{
    // The sequence to search for: "fb555555555555"
    string target = "fb555555555555d5";
    int targetLength = target.length();
    int target_array[16];
    for (int i = 0; i < 16; i++)
    {
        target_array[i] = hex2int(target[i]);
    }

    // Ensure the array is large enough to contain the target sequence after the start index
    if (startIndex + targetLength > arraySize)
    {
        return -1; // Not enough space for the target sequence
    }

    // Search through the array starting from startIndex
    for (int i = startIndex; i <= arraySize - targetLength; ++i)
    {
        bool found = true;
        for (int j = 0; j < targetLength; ++j)
        {
            if (array[i + j] != target_array[j])
            {
                found = false;
                break;
            }
        }
        if (found)
        {
            return i; // Return the starting index of the sequence
        }
    }

    return -1; // Sequence not found
}

void parse_file(string filename)
{
    ofstream my_output_file;
    my_output_file.open("output_of_parser.txt", ios::trunc);

    long long int size;
    long long int destinationMacAddress;
    long long int sourceMacAddress;
    int SFD;
    int ecpri_reserved;
    int ecpri_concatination;
    int ecpri_message;
    int ecpri_payload;
    int ecpri_RTC;
    int ecpri_Seqid;
    int data_direction;
    int payload_Version;
    int filterindex;
    int frame_id;
    int subframe_id;
    int slot_id;
    int symbol_id;
    int section_id;
    int rb;
    int synInc;
    int startPrbu;
    int numPrbu;
    long long int Preamble;
    int Ether_type;
    int ecpri_version;

    int num_of_frame = 0;
    int * hexa_array = readHexFromPosition(filename,size);

    int start = 0;
    int end = 0;

    while (start < size)
    {
        num_of_frame = num_of_frame + 1;
        int location  =  searchForSequence(hexa_array, size, start);
        if (location == -1)
        {
            break;
        }
        
        start = location-7*2;
        end = start + 7*2;
        Preamble = getHexFromPosition(hexa_array, start, end, 7 * 2);
        SFD = getHexFromPosition(hexa_array, start, end, 1 * 2);
        destinationMacAddress = getHexFromPosition(hexa_array, start, end, 6 * 2);
        sourceMacAddress = getHexFromPosition(hexa_array, start, end, 6 * 2);
        Ether_type = getHexFromPosition(hexa_array, start, end, 2 * 2);

        // parsing ecpri header
        ecpri_version = getHexFromPosition(hexa_array, start, end, 1);
        long long int val = getHexFromPosition(hexa_array, start, end, 1);
        ecpri_reserved = (val >> 1) & 3;
        ecpri_concatination = val & 1;
        ecpri_message = getHexFromPosition(hexa_array, start, end, 1 * 2);
        ecpri_payload = getHexFromPosition(hexa_array, start, end, 2 * 2);
        ecpri_RTC = getHexFromPosition(hexa_array, start, end, 2 * 2);
        ecpri_Seqid = getHexFromPosition(hexa_array, start, end, 2 * 2);

        // ORAN common header
        val = getHexFromPosition(hexa_array, start, end, 1 * 2);
        data_direction = (val >> 7) & 1;
        payload_Version = val & 0b01110000;
        filterindex = val & 0b00001111;
        frame_id = getHexFromPosition(hexa_array, start, end, 1 * 2);
        val = getHexFromPosition(hexa_array, start, end, 1 * 2);
        subframe_id = (val>>4) & 0b1111;
        slot_id = val & 0b00001111;
        val = getHexFromPosition(hexa_array, start, end, 1 * 2);

        slot_id = (slot_id << 2) | ((val>>6) & 0b11);
        symbol_id = val & 0b00111111;

        // ORAN section header
        section_id = getHexFromPosition(hexa_array, start, end, 1 * 2);
        val = getHexFromPosition(hexa_array, start, end, 1 * 2);
        section_id = (section_id << 4) | (val & 0b11110000);
        rb = val & 0b00001000;
        synInc = val & 0b00000100;
        startPrbu = val & 0b00000011;
        val = getHexFromPosition(hexa_array, start, end, 1 * 2);
        startPrbu = (startPrbu << 8) | val;
        numPrbu = getHexFromPosition(hexa_array, start, end, 1 * 2);

        my_output_file << "Frame numebr : " << num_of_frame << "\n";
        my_output_file << "Preamble is : " << hex << Preamble << "\n";
        my_output_file << "SFD is : " << hex << SFD << "\n";
        my_output_file << "source MAC address is : " << hex << sourceMacAddress << "\n";
        my_output_file << "destination MAC address is : " << hex << destinationMacAddress << "\n";
        my_output_file << "Etherne type is : " << hex << Ether_type << "\n";

        // ecpri header print
        my_output_file << "ecpri version is : " << hex << ecpri_version << "\n";
        my_output_file << "ecpri concatination is : " << hex << ecpri_concatination << "\n";
        my_output_file << "ecpri message is : " << hex << ecpri_message << "\n";
        my_output_file << "ecpri payload is : " << hex << ecpri_payload << "\n";
        my_output_file << "ecpriRtcid/ecpriPcid  is : " << hex << ecpri_RTC << "\n";
        my_output_file << "ecpriSeqid is : " << hex << ecpri_Seqid << "\n";
        // ORAN header print
        my_output_file << "data direction is : " << hex << data_direction << "\n";
        my_output_file << "payload version is : " << hex << payload_Version << "\n";
        my_output_file << "filter index is : " << hex << filterindex << "\n";
        my_output_file << "frame id is : " << hex << frame_id << "\n";
        my_output_file << "subframe id is : " << hex << subframe_id << "\n";
        my_output_file << "slot id is : " << hex << slot_id << "\n";
        my_output_file << "symbol id is : " << hex << symbol_id << "\n";
        // ORAN section header
        my_output_file << "section id is : " << hex << section_id << "\n";
        my_output_file << "rb is : " << hex << rb << "\n";
        my_output_file << "symInc is : " << hex << synInc << "\n";
        my_output_file << "startPrbu is : " << hex << startPrbu << "\n";
        my_output_file << "numPrbu is : " << hex << numPrbu << "\n";
    }
      
}


