// OpcUaTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include <stdio.h>
#include <stddef.h>
#include <iostream>
#include "open62541.h"
#if _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef _WIN32
#define UA_ARCHITECTURE_WIN32
#else
#define UA_ARCHITECTURE_POSIX
#endif
int main()
{
	UA_Client *client = UA_Client_new();
	UA_ClientConfig_setDefault(UA_Client_getConfig(client));
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://118.24.36.220:62547/DataAccessServer");
	if (retval != UA_STATUSCODE_GOOD)
	{
		UA_Client_delete(client);
		//return (int)retval;
	}
	UA_Variant value; /* Variants can hold scalar values and arrays of any type */
	UA_Variant_init(&value);
	std::string str = "Machines/Machine A/Name";
	const UA_NodeId nodeId = UA_NODEID_STRING(2, &str[0]);
	retval = UA_Client_readValueAttribute(client, nodeId, &value);
	if (retval == UA_STATUSCODE_GOOD &&
		UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING]))
	{
		//std::string val;
		//val = *(std::string*)value.data;
		UA_String *text = (UA_String*)value.data;
		uint16_t size = text->length + 1;
		char uabyte[100];
		memset(uabyte,0,100);
		memcpy(uabyte,text->data,text->length);
		//strncpy_s(uabyte, (const char*)text->data, text->length);
		printf("the value is: %s\n", uabyte);
	}
	/* Clean up */
	//UA_Variant_deleteMembers(&value);
	UA_Client_delete(client); /* Disconnects the client internally */
    std::cout << retval; 
}
