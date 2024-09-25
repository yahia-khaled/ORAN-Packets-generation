#pragma once
#ifndef parser_H_
#define parser_H_

void parse_file(std::string filename);
int hex2int(char ch);
long long int getHexFromPosition(int * arr , int & start , int & end,int size);
int searchForSequence(int *array, int arraySize, int startIndex);
int *readHexFromPosition(std::string filename, int &size);

#endif