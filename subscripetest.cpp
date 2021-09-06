//
// Created by lrp on 2021/7/15.
//

/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

/**
 * Client disconnect handling
 * --------------------------
 * This example shows you how to handle a client disconnect, e.g., if the server
 * is shut down while the client is connected. You just need to call connect
 * again and the client will automatically reconnect.
 *
 * This example is very similar to the tutorial_client_firststeps.c. */

#include "open62541.h"
#if _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef _WIN32
#define UA_ARCHITECTURE_WIN32
#else
#define UA_ARCHITECTURE_POSIX
#endif
#include <signal.h>
#include <stdlib.h>
#include <vector>
using namespace std;
UA_Boolean running = true;

static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Received Ctrl-C");
    running = 0;
}

static void
handler_currentTimeChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                           UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "currentTime has changed!");
    if(UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *) value->value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "date is: %02u-%02u-%04u %02u:%02u:%02u.%03u",
                    dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    }
}

static void
deleteSubscriptionCallback(UA_Client *client, UA_UInt32 subscriptionId, void *subscriptionContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                "Subscription Id %u was deleted", subscriptionId);
}

static void
subscriptionInactivityCallback (UA_Client *client, UA_UInt32 subId, void *subContext) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Inactivity for subscription %u", subId);
}

static void
dataChangeHandler(UA_Client *client, UA_UInt32 subId, void *subContext,
                  UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    //notificationReceived = true;
   // countNotificationReceived++;
    UA_String *text = (UA_String*)(value->value.data);
    char uabyte[100];
    memset(uabyte,'\0',100);
    memcpy(uabyte,text->data,text->length);

    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "datachanged handler monid %d value %s", monId,uabyte);
}


static void
stateCallback(UA_Client *client, UA_SecureChannelState channelState,
              UA_SessionState sessionState, UA_StatusCode recoveryStatus) {
    switch(channelState) {
    case UA_SECURECHANNELSTATE_FRESH:
    case UA_SECURECHANNELSTATE_CLOSED:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "The client is disconnected");
        break;
    case UA_SECURECHANNELSTATE_HEL_SENT:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Waiting for ack");
        break;
    case UA_SECURECHANNELSTATE_OPN_SENT:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Waiting for OPN Response");
        break;
    case UA_SECURECHANNELSTATE_OPEN:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A SecureChannel to the server is open");
        break;
    default:
        break;
    }

    switch(sessionState) {
    case UA_SESSIONSTATE_ACTIVATED: {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "A session with the server is activated");
        /* A new session was created. We need to create the subscription. */
        /* Create a subscription */
        UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
        UA_CreateSubscriptionResponse response =
            UA_Client_Subscriptions_create(client, request, NULL, NULL, deleteSubscriptionCallback);
            if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Create subscription succeeded, id %u",
                            response.subscriptionId);
            else
                return;

            /* Add a MonitoredItem */
/*            UA_NodeId currentTimeNode =
                UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
            UA_MonitoredItemCreateRequest monRequest =
                UA_MonitoredItemCreateRequest_default(currentTimeNode);

            UA_MonitoredItemCreateResult monResponse =
                UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
                                                          UA_TIMESTAMPSTORETURN_BOTH, monRequest,
                                                          NULL, handler_currentTimeChanged, NULL);
            if(monResponse.statusCode == UA_STATUSCODE_GOOD)
                UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                            "Monitoring UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME', id %u",
                            monResponse.monitoredItemId);*/

        UA_MonitoredItemCreateRequest items[4];
        UA_UInt32 newMonitoredItemIds[4];
        UA_Client_DataChangeNotificationCallback callbacks[4];
        UA_Client_DeleteMonitoredItemCallback deleteCallbacks[4];
        void *contexts[3];

        /* monitor the server state */
        items[0] = UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(2, "Machines/Machine A/SDFFF"));
        callbacks[0] = dataChangeHandler;
        contexts[0] = NULL;
        deleteCallbacks[0] = NULL;

        /* monitor invalid node */
        items[1] = UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(2, "Machines/Machine B/Name"));
        callbacks[1] = dataChangeHandler;
        contexts[1] = NULL;
        deleteCallbacks[1] = NULL;

        /* monitor current time */
        items[2] = UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(2, "Machines/Machine C/Name"));
        callbacks[2] = dataChangeHandler;
        contexts[2] = NULL;
        deleteCallbacks[2] = NULL;

        /* monitor the server state */
        items[3] = UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(2, "Machines/Machine A/fff"));
        callbacks[3] = dataChangeHandler;
        contexts[3] = NULL;
        deleteCallbacks[3] = NULL;

        UA_CreateMonitoredItemsRequest createRequest;
        UA_CreateMonitoredItemsRequest_init(&createRequest);
        createRequest.subscriptionId = response.subscriptionId;
        createRequest.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        createRequest.itemsToCreate = items;
        createRequest.itemsToCreateSize = 4;
        UA_CreateMonitoredItemsResponse createResponse =
                UA_Client_MonitoredItems_createDataChanges(client, createRequest, contexts,
                                                           callbacks, deleteCallbacks);

        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "createResponse resultsSize %d",createResponse.resultsSize );
        vector<UA_MonitoredItemCreateResult> results(createResponse.results, createResponse.results+ createResponse.resultsSize);
        for(auto it = results.begin();it != results.end();it++)
        {
            UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "createResponse result monid %d status %d", it-> monitoredItemId, it->statusCode );
        }



    }
        break;
    case UA_SESSIONSTATE_CLOSED:
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Session disconnected");
        break;
    default:
        break;
    }
}

int
main(void) {
    signal(SIGINT, stopHandler); /* catches ctrl-c */

    UA_Client *client = UA_Client_new();
    UA_ClientConfig *cc = UA_Client_getConfig(client);
    UA_ClientConfig_setDefault(cc);

    /* Set stateCallback */
    cc->stateCallback = stateCallback;
    cc->subscriptionInactivityCallback = subscriptionInactivityCallback;

    /* Endless loop runAsync */
    while(running) {
        /* if already connected, this will return GOOD and do nothing */
        /* if the connection is closed/errored, the connection will be reset and then reconnected */
        /* Alternatively you can also use UA_Client_getState to get the current state */
        UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://118.24.36.220:62547/DataAccessServer");
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                         "Not connected. Retrying to connect in 1 second");
            /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
            /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
            UA_sleep_ms(1000);
            continue;
        }

        UA_Client_run_iterate(client, 1000);
    };

    /* Clean up */
    UA_Client_delete(client); /* Disconnects the client internally */
    return EXIT_SUCCESS;
}
