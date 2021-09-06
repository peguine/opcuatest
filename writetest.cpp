//
// Created by lrp on 2021/9/6.
//


#include "open62541.h"
#if _WIN32
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef _WIN32
#define UA_ARCHITECTURE_WIN32
#else
#define UA_ARCHITECTURE_POSIX
#endif

#include <string>
using namespace std;

static UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId *parent = (UA_NodeId *)handle;
    printf("%u, %u --- %u ---> NodeId %u, %u\n",
           parent->namespaceIndex, parent->identifier.numeric,
           referenceTypeId.identifier.numeric, childId.namespaceIndex,
           childId.identifier.numeric);
    return UA_STATUSCODE_GOOD;
}


#include <vector>

using namespace std;

void listTreeRecursive(UA_Client *client, UA_NodeId nodeId)
{
    size_t i, j;

    /* Browse some objects */
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;

    UA_NodeId_copy(&nodeId, &bReq.nodesToBrowse[0].nodeId);
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; /* return everything */

    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    UA_BrowseNextRequest bNextReq;
    UA_BrowseNextRequest_init(&bNextReq);
    // normally is set to 0, to get all the nodes, but we want to test browse next
    bNextReq.releaseContinuationPoints = UA_FALSE;
    bNextReq.continuationPoints = &bResp.results[0].continuationPoint;
    bNextReq.continuationPointsSize = 1;

    UA_BrowseNextResponse bNextResp = UA_Client_Service_browseNext(client, bNextReq);


    for ( i = 0; i < bResp.resultsSize; i++)
    {
        for ( j = 0; j < bResp.results[i].referencesSize; j++)
        {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE||ref->nodeClass == UA_NODECLASS_METHOD)) {

                if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
                    printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                           ref->nodeId.nodeId.identifier.numeric, (int) ref->browseName.name.length,
                           ref->browseName.name.data, (int) ref->displayName.text.length,
                           ref->displayName.text.data);

                    listTreeRecursive(client, UA_NODEID_NUMERIC(ref->nodeId.nodeId.namespaceIndex,
                                                                ref->nodeId.nodeId.identifier.numeric));

                } else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                    printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                           (int) ref->nodeId.nodeId.identifier.string.length,
                           ref->nodeId.nodeId.identifier.string.data,
                           (int) ref->browseName.name.length, ref->browseName.name.data,
                           (int) ref->displayName.text.length, ref->displayName.text.data);

                    listTreeRecursive(client, UA_NODEID_STRING(ref->nodeId.nodeId.namespaceIndex,
                                                               (char *) ref->nodeId.nodeId.identifier.string.data));
                }
            }
            /* TODO: distinguish further types */
        }
    }

    for ( i = 0; i < bNextResp.resultsSize; i++)
    {
        for ( j = 0; j < bNextResp.results[i].referencesSize; j++)
        {
            UA_ReferenceDescription *ref = &(bNextResp.results[i].references[j]);
            if ((ref->nodeClass == UA_NODECLASS_OBJECT || ref->nodeClass == UA_NODECLASS_VARIABLE||ref->nodeClass == UA_NODECLASS_METHOD)) {

                if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
                    printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                           ref->nodeId.nodeId.identifier.numeric, (int) ref->browseName.name.length,
                           ref->browseName.name.data, (int) ref->displayName.text.length,
                           ref->displayName.text.data);

                    listTreeRecursive(client, UA_NODEID_NUMERIC(ref->nodeId.nodeId.namespaceIndex,
                                                                ref->nodeId.nodeId.identifier.numeric));

                } else if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                    printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                           (int) ref->nodeId.nodeId.identifier.string.length,
                           ref->nodeId.nodeId.identifier.string.data,
                           (int) ref->browseName.name.length, ref->browseName.name.data,
                           (int) ref->displayName.text.length, ref->displayName.text.data);

                    listTreeRecursive(client, UA_NODEID_STRING(ref->nodeId.nodeId.namespaceIndex,
                                                               (char *) ref->nodeId.nodeId.identifier.string.data));
                }
            }
            /* TODO: distinguish further types */
        }
    }
    UA_BrowseRequest_deleteMembers(&bReq);
    UA_BrowseResponse_deleteMembers(&bResp);
}

int main() {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://118.24.36.220:62547/DataAccessServer");
    while(true) {
        if (retval != UA_STATUSCODE_GOOD) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                         "Not connected. Retrying to connect in 1 second");
            /* The connect may timeout after 1 second (see above) or it may fail immediately on network errors */
            /* E.g. name resolution errors or unreachable network. Thus there should be a small sleep here */
            UA_sleep_ms(1000);
            continue;
        }else{break;}
    }

    /* Browse some objects */
/*    printf("Browsing nodes in objects folder:\n");
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); *//* browse objects folder *//*
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; *//* return everything *//*
    UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
    printf("%-9s %-16s %-16s %-16s\n", "NAMESPACE", "NODEID", "BROWSE NAME", "DISPLAY NAME");
    for(size_t i = 0; i < bResp.resultsSize; ++i) {
        for(size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
            if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
                printf("%-9d %-16d %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       ref->nodeId.nodeId.identifier.numeric, (int)ref->browseName.name.length,
                       ref->browseName.name.data, (int)ref->displayName.text.length,
                       ref->displayName.text.data);
            } else if(ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING) {
                printf("%-9d %-16.*s %-16.*s %-16.*s\n", ref->nodeId.nodeId.namespaceIndex,
                       (int)ref->nodeId.nodeId.identifier.string.length,
                       ref->nodeId.nodeId.identifier.string.data,
                       (int)ref->browseName.name.length, ref->browseName.name.data,
                       (int)ref->displayName.text.length, ref->displayName.text.data);
            }
            *//* TODO: distinguish further types *//*
        }
    }
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);

    *//* Same thing, this time using the node iterator... *//*
    UA_NodeId *parent = UA_NodeId_new();
    *parent = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_Client_forEachChildNodeCall(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                                   nodeIter, (void *) parent);
    UA_NodeId_delete(parent);*/

   // listTreeRecursive(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER));
    listTreeRecursive(client,  UA_NODEID_STRING(2, "Machines"));
    /* Write node attribute (using the highlevel API) */
    UA_String value= UA_STRING("abc");
    UA_Variant *myVariant = UA_Variant_new();
    UA_Variant_setScalarCopy(myVariant, &value, &UA_TYPES[UA_TYPES_STRING]);
    retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(2, "Machines/Machine B/Name"), myVariant);
    if (retval != UA_STATUSCODE_GOOD) {
        printf("write error:%08x\n",retval);
    }
    UA_Variant_delete(myVariant);
    Sleep(1000);
    /* Read attribute */
    printf("\nReading the value of node (2, \"Machines/Machine B/Name\"):\n");
    UA_Variant *val = UA_Variant_new();
    retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(2, "Machines/Machine B/Name"), val);
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(val) &&
       val->type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String *text = (UA_String*)(val->data);
        string temp = string((char *)text->data,(char *)text->data + text->length);
        printf("the value is: %s\n", temp.c_str());
    }else{
        printf("read error:%08x\n",retval);
    }
    UA_Variant_delete(val);




    return 0;
}