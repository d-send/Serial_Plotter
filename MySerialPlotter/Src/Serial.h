#pragma once

#include <vector>
#include <array>
#include <string>

#define NO_OF_CHANNELS 2

void SerialInit(const std::string& COM, unsigned long Baudrate);

std::array<std::vector<double>, NO_OF_CHANNELS>& SerialParseData();
void ParseFloat(const std::string& buffer);

void SerialClose();