#include "Serial.h"
#include <Windows.h>
#include <SetupAPI.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <array>
#include <sstream>

bool Connected = false;

HANDLE hSerial;
DCB dcbSerialParams = { 0 };
COMMTIMEOUTS timeouts = { 0 };
COMSTAT status = { 0 };
DWORD errors = 0;

std::array<std::vector<double>, NO_OF_CHANNELS> valarrays;

void SerialInit(const std::string& COM, unsigned long Baudrate)
{
	if (Connected)
	{
		std::cerr << "Already Connected to the Serial Port!\n";
		return;
	}

	hSerial = CreateFileA(COM.c_str(),GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

	if (hSerial == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error in opening Serial Port!\n";
		return;
	}

	std::cout << "Port opened successfully!\n";
	Connected = true;

	if (!GetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Error gettinf Serial port state!\n";
		return;
	}

	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	dcbSerialParams.BaudRate = Baudrate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;
	dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

	if (!SetCommState(hSerial, &dcbSerialParams))
	{
		std::cerr << "Error setting serial port state!\n";
		return;
	}

	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;

	if (!SetCommTimeouts(hSerial, &timeouts))
	{
		std::cerr << "Error setting timeouts!\n";
		return;
	}

	PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
	Sleep(2000);
}

std::array<std::vector<double>, NO_OF_CHANNELS>& SerialParseData()
{
	ClearCommError(hSerial,&errors,&status);

	int buffer_datasize = status.cbInQue;
	DWORD Bytes_red;

	std::string buf;
	buf.resize(buffer_datasize);

	if (ReadFile(hSerial, (void*)buf.c_str(), buffer_datasize, &Bytes_red, NULL))
	{
		ParseFloat(buf);
	}

	return valarrays;
}

inline std::stringstream ss("");

void ParseFloat(const std::string& buffer)
{
	ss << buffer;
	std::string vals;

	while (std::getline(ss, vals, '\n'))
	{
		std::string v;
		std::stringstream s(vals);

		int i = 0;
		if(!ss.eof())
		{

			while (std::getline(s, v, ','))
			{
				if (i < NO_OF_CHANNELS)
				{

					try
					{
						double vv = std::stod(v);
						valarrays[i].push_back(vv);
					}
					catch (...)
					{

					}
				}
				i++;

			}
		}
	}

	ss.str("");
	ss.clear();
	ss << vals;
}

void SerialClose()
{

	if (Connected)
	{
		CloseHandle(hSerial);
		Connected = false;
		hSerial = nullptr;
		dcbSerialParams = { 0 };
		timeouts = { 0 };
		status = { 0 };

		for (auto& arr : valarrays)
		{
			arr.clear();
		}

		errors = 0;
	}
}



