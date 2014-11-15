//
//  PHKNetworkIP.cpp
//  Workbench
//
//  Created by Wai Man Chan on 4/8/14.
//
//

#include "PHKNetworkIP.h"
#define PHKNetworkServiceType "_hap._tcp"

#include <iostream>
#include <fstream>
#include <errno.h>
#include <strings.h>


//Security
extern "C" {
    //ChaCha20-Poly1035
#include "poly1305-opt-master/poly1305.h"
#include "Chacha20/chacha20_simple.h"
    //HKDF-SHA512
#include "rfc6234-master/hkdf.h"
    //ED25519
#include "ed25519-donna/ed25519.h"
    //Curve25519
#include "curve25519/curve25519-donna.h"
    //SRP
#include "srp/srp.h"
}

//Store controller public key
#include "PHKControllerRecord.h"

//Handle Data
#include "PHKAccessory.h"

#include "Configuration.h"

#if MCU
#else
#include <pthread.h>
#endif

using namespace std;

#define portNumber 0

const unsigned char modulusStr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD, 0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45, 0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED, 0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D, 0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F, 0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D, 0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B, 0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2, 0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9, 0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7C, 0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10, 0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAA, 0xC4, 0x2D, 0xAD, 0x33, 0x17, 0x0D, 0x04, 0x50, 0x7A, 0x33, 0xA8, 0x55, 0x21, 0xAB, 0xDF, 0x1C, 0xBA, 0x64, 0xEC, 0xFB, 0x85, 0x04, 0x58, 0xDB, 0xEF, 0x0A, 0x8A, 0xEA, 0x71, 0x57, 0x5D, 0x06, 0x0C, 0x7D, 0xB3, 0x97, 0x0F, 0x85, 0xA6, 0xE1, 0xE4, 0xC7, 0xAB, 0xF5, 0xAE, 0x8C, 0xDB, 0x09, 0x33, 0xD7, 0x1E, 0x8C, 0x94, 0xE0, 0x4A, 0x25, 0x61, 0x9D, 0xCE, 0xE3, 0xD2, 0x26, 0x1A, 0xD2, 0xEE, 0x6B, 0xF1, 0x2F, 0xFA, 0x06, 0xD9, 0x8A, 0x08, 0x64, 0xD8, 0x76, 0x02, 0x73, 0x3E, 0xC8, 0x6A, 0x64, 0x52, 0x1F, 0x2B, 0x18, 0x17, 0x7B, 0x20, 0x0C, 0xBB, 0xE1, 0x17, 0x57, 0x7A, 0x61, 0x5D, 0x6C, 0x77, 0x09, 0x88, 0xC0, 0xBA, 0xD9, 0x46, 0xE2, 0x08, 0xE2, 0x4F, 0xA0, 0x74, 0xE5, 0xAB, 0x31, 0x43, 0xDB, 0x5B, 0xFC, 0xE0, 0xFD, 0x10, 0x8E, 0x4B, 0x82, 0xD1, 0x20, 0xA9, 0x3A, 0xD2, 0xCA, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const unsigned char curveBasePoint[] = { 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char generator[] = {0x05};
char tempStr[3073];

const unsigned char accessorySecretKey[32] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74};

#if MCU
#else
//pthread_t threads[numberOfClient];
#endif
//int connection[numberOfClient];
vector<PHKConnection> PHKNetworkIP::connections;

//int _socket_v4, _socket_v6;
//DNSServiceRef netServiceV4, netServiceV6;

//Network Setup
int setupSocketV4(unsigned int maximumConnection) {
    int _socket = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;   bzero(&addr, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   addr.sin_family = PF_INET;    addr.sin_port = htons(portNumber);
    bind(_socket, (const struct sockaddr *)&addr, sizeof(addr));
    listen(_socket, maximumConnection);
    return _socket;
}

int setupSocketV6(unsigned int maximumConnection) {
    int _socket = socket(PF_INET6, SOCK_STREAM, 0);
    sockaddr_in6 addr;   bzero(&addr, sizeof(addr));
    addr.sin6_addr = in6addr_any;   addr.sin6_family = PF_INET6;    addr.sin6_port = htons(portNumber);
    bind(_socket, (const struct sockaddr *)&addr, sizeof(addr));
    listen(_socket, maximumConnection);
    return _socket;
}

unsigned short getSocketPortNumberV4(int _socket) {
    sockaddr_in addr; socklen_t len = sizeof(addr);
    getsockname(_socket, (struct sockaddr *)&addr, &len);
    return ntohs(addr.sin_port);
}

unsigned short getSocketPortNumberV6(int _socket) {
    sockaddr_in6 addr; socklen_t len = sizeof(addr);
    getsockname(_socket, (struct sockaddr *)&addr, &len);
    return ntohs(addr.sin6_port);
}

void registerFail(DNSServiceRef sdRef, DNSRecordRef RecordRef, DNSServiceFlags flags, DNSServiceErrorType errorCode, void *context ) {
    printf("Service can't register\n");
    exit(0);
}

TXTRecordRef PHKNetworkIP::buildTXTRecord(DeviceStruct* device) {
    printf("buildTXTRecord for: %s\n", device->nameAsChar());
    TXTRecordRef txtRecord;
    TXTRecordCreate(&txtRecord, 0, NULL);
    TXTRecordSetValue(&txtRecord, "pv", 3, "1.0");  //Version
    TXTRecordSetValue(&txtRecord, "id", 17, device->identityAsChar());    //Device id
    TXTRecordSetValue(&txtRecord, "c#", 1, "1");    //Number of User?
    TXTRecordSetValue(&txtRecord, "s#", 1, "2");    //Number of service
    TXTRecordSetValue(&txtRecord, "sf", 1, "1");    //No idea what it is
    TXTRecordSetValue(&txtRecord, "ff", 1, "0");    //1 for MFI product
    TXTRecordSetValue(&txtRecord, "md", device->name.length(), device->nameAsChar());    //Model Name
    return txtRecord;
}

void PHKNetworkIP::setupSocket(DeviceStruct* device) {
    TXTRecordRef txtRecord = buildTXTRecord(device);
    device->_socket_v4 = setupSocketV4(5);
    DNSServiceRegister(&device->netServiceV4, 0, 0, device->nameAsChar(), PHKNetworkServiceType, "", NULL, htons(getSocketPortNumberV4(device->_socket_v4)), TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord), NULL, NULL);
    TXTRecordDeallocate(&txtRecord);
}

PHKNetworkIP::PHKNetworkIP(Devices* devices_) : devices(devices_) {
    SRP_initialize_library();
    srand((unsigned int)time(NULL));
    for (int i = 0; i < numberOfClient; i++) {
        connections.push_back(PHKConnection(devices));
    }
    for(unsigned int i=0; i<devices->size(); i++) {
        DeviceStruct* device = &devices->at(i);
        setupSocket(device);
    }
}

//Connection Logic
void broadcastMessage(char *buffer, size_t len) {
    for (int i = 0; i < PHKNetworkIP::connections.size(); i++) {
        int socketNumber = PHKNetworkIP::connections.at(i).socketNumber;
        if (socketNumber >= 0) {
            write(socketNumber, buffer, len);
        }
    }
}

void* PHKConnection::connectionLoop() const {
    int subSocket = socketNumber;ssize_t len;
    if (subSocket >= 0) {
        printf("Start Connect: %d\n", subSocket);
        
        do {
            char *buffer = new char[4096];
            len = read(subSocket, buffer, 4096);
            PHKNetworkMessage msg(buffer);
            if (len > 0) {
                if (!strcmp(msg.directory, "pair-setup")){
                    
                    /*
                     * The processo f pair-setup
                     */
                    
                    handlePairSetup(subSocket, buffer);
                    
                }
                else if (!strcmp(msg.directory, "pair-verify")){
                    handlePairVerify(subSocket, buffer);
                }
            }
            
            delete [] buffer;
            
        } while (len > 0);
        
        close(subSocket);
        printf("Stop Connect: %d\n", subSocket);
        
        subSocket = -1;
        
    }
    return NULL;
}

void PHKNetworkIP::handleConnection() {
    for(unsigned int deviceNr=0; deviceNr<devices->size(); deviceNr++)
    {
        DeviceStruct device = devices->at(deviceNr);
        
        printf("device._socket_v4 = %d\n",device._socket_v4);
        
        int subSocket = accept(device._socket_v4, 0, NULL);
        
        int index = -1;
        for (int i = 0; i < connections.size(); i++) {
            PHKConnection* connection = &connections.at(i);
            if (connection->socketNumber == -1) {
                index = i;
                connection->socketNumber = subSocket;
                pthread_create(&connection->thread, NULL, PHKConnection::connectionLoopFunc, connection);
                break;
            }
        }
        
        if (index < 0) close(subSocket);
    }

}

void PHKConnection::handlePairSetup(int subSocket, char *buffer) const {
    PHKNetworkMessageDataRecord stateRecord;
    stateRecord.activate = true;    stateRecord.data = new char[1]; stateRecord.length = 1;    stateRecord.index = 6;
    int state = 1;
    SRP *srp;
    srp = SRP_new(SRP6a_server_method());
    cstr *secretKey = NULL, *publicKey = NULL, *response = NULL;
    char sessionKey[64];
    PHKNetworkMessage msg = PHKNetworkMessage(buffer);
    char *responseBuffer = 0; int responseLen = 0;
    
    do {
        msg = PHKNetworkMessage(buffer);
        PHKNetworkResponse mResponse(200);
        
        state = *msg.data.dataPtrForIndex(6);
        
        printf("Start pairing state: %d\n", state);
        
        *stateRecord.data = state+1;
        switch (state) {
            case 1: {
                PHKNetworkMessageDataRecord saltRec;
                PHKNetworkMessageDataRecord publicKeyRec;
                unsigned char saltChar[16];
                
                for (int i = 0; i < 16; i++) {
                    saltChar[i] = rand();
                }
                
                SRP_RESULT result = SRP_set_username(srp, "Pair-Setup");
                result = SRP_set_params(srp, (const unsigned char *)modulusStr, 384, (const unsigned char *)generator, 1, saltChar, 16);
                result = SRP_set_auth_password(srp, devicePassword);
                result = SRP_gen_pub(srp, &publicKey);
                
                saltRec.index = 2;
                saltRec.activate = true;
                saltRec.length = 16;
                saltRec.data = new char[saltRec.length];
                bcopy(saltChar, saltRec.data, saltRec.length);
                publicKeyRec.index = 3;
                publicKeyRec.activate = true;
                publicKeyRec.length = publicKey->length;
                publicKeyRec.data = new char[publicKeyRec.length];
                bcopy(publicKey->data, publicKeyRec.data, publicKeyRec.length);
                
                mResponse.data.addRecord(stateRecord);
                mResponse.data.addRecord(publicKeyRec);
                mResponse.data.addRecord(saltRec);
            }
                break;
            case 3: {
                const char *keyStr = 0;
                int keyLen = 0;
                const char *proofStr;
                int proofLen;
                keyStr = msg.data.dataPtrForIndex(3);
                keyLen = msg.data.lengthForIndex(3);
                char *temp = msg.data.dataPtrForIndex(4);
                if (temp != NULL) {
                    proofStr = temp;
                    proofLen = msg.data.lengthForIndex(4);
                }
                
                
                SRP_RESULT result = SRP_compute_key(srp, &secretKey, (const unsigned char*)keyStr, keyLen);
                result = SRP_verify(srp, (const unsigned char*)proofStr, proofLen);
                
                if (!SRP_OK(result)) {
                    PHKNetworkMessageDataRecord responseRecord;
                    responseRecord.activate = true; responseRecord.data = new char[1]; responseRecord.data[0] = 2; responseRecord.index = 7;   responseRecord.length = 1;
                    mResponse.data.addRecord(responseRecord);
#if HomeKitLog == 1
                    printf("Oops at M3\n");
#endif
                } else {
                    SRP_respond(srp, &response);
                    PHKNetworkMessageDataRecord responseRecord;
                    responseRecord.activate = true; responseRecord.index = 4;   responseRecord.length = response->length; responseRecord.data = new char[responseRecord.length];    bcopy(response->data, responseRecord.data, responseRecord.length);
                    mResponse.data.addRecord(responseRecord);
#if HomeKitLog == 1
                    printf("Password Correct\n");
#endif
                }
                
                const char salt[] = "Pair-Setup-Encrypt-Salt";
                const char info[] = "Pair-Setup-Encrypt-Info";
                int i = hkdf((const unsigned char*)salt, strlen(salt), (const unsigned char*)secretKey->data, secretKey->length, (const unsigned char*)info, strlen(info), (uint8_t*)sessionKey, 32);
                if (i != 0) return;
            }
                break;
            case 5: {
                
                const char *encryptedPackage = NULL;int packageLen = 0;
                encryptedPackage = msg.data.dataPtrForIndex(5);
                packageLen = msg.data.lengthForIndex(5);
                char *encryptedData = new char[packageLen];
                bcopy(encryptedPackage, encryptedData, packageLen-16);
                char *mac = new char[16];
                bcopy(&encryptedPackage[packageLen-16], mac, 16);
                
                chacha20_ctx chacha20;    bzero(&chacha20, sizeof(chacha20));
                chacha20_setup(&chacha20, (const uint8_t *)sessionKey, 32, (uint8_t *)"PS-Msg05");
                
                //Ploy1305 key
                char temp[64];  bzero(temp, 64); char temp2[64];  bzero(temp2, 64);
                chacha20_encrypt(&chacha20, (const uint8_t*)temp, (uint8_t *)temp2, 64);
                
                char verify[16];    bzero(verify, 16);
                poly1305_context verifyContext; bzero(&verifyContext, sizeof(verifyContext));
                poly1305_init(&verifyContext, (const unsigned char*)temp2);
                poly1305_update(&verifyContext, (const unsigned char *)encryptedData, packageLen-16);
                {
                    char *waste = new char[16];
                    bzero(waste, 16);
                    poly1305_update(&verifyContext, (const unsigned char *)waste, 16-packageLen%16);
                    unsigned long long _len;
                    _len = 0;
                    poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                    _len = packageLen - 16;
                    poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                }
                poly1305_finish(&verifyContext, (unsigned char *)verify);
                
                if (bcmp(verify, mac, 16)) {
                    PHKNetworkMessageDataRecord responseRecord;
                    responseRecord.activate = true;
                    responseRecord.data = new char[1];
                    responseRecord.data[0] = 1;
                    responseRecord.index = 7;
                    responseRecord.length = 1;
                    mResponse.data.addRecord(responseRecord);
#if HomeKitLog == 1
                    printf("Corrupt TLv8 at M5\n");
#endif
                } else {
                    /*
                     * HAK Pair Setup M6
                     */
                    printf("Chacha20-Poly1305 on M5 packet check out\n");
                    char *decryptedData = new char[packageLen-16];
                    bzero(decryptedData, packageLen-16);
                    chacha20_decrypt(&chacha20, (const uint8_t *)encryptedData, (uint8_t *)decryptedData, packageLen-16);
                    PHKNetworkMessageData *subTLV8 = new PHKNetworkMessageData(decryptedData, packageLen-16);
                    char *controllerIdentifier = subTLV8->dataPtrForIndex(1);
                    char *controllerPublicKey = subTLV8->dataPtrForIndex(3);
                    char *controllerSignature = subTLV8->dataPtrForIndex(10);
                    char controllerHash[100];
                    
                    PHKKeyRecord newRecord;
                    bcopy(controllerIdentifier, newRecord.controllerID, 36);
                    bcopy(controllerPublicKey, newRecord.publicKey, 32);
                    addControllerKey(newRecord);
                    
                    const char salt[] = "Pair-Setup-Controller-Sign-Salt";
                    const char info[] = "Pair-Setup-Controller-Sign-Info";
                    int i = hkdf((const unsigned char*)salt, strlen(salt), (const unsigned char*)secretKey->data, secretKey->length, (const unsigned char*)info, strlen(info), (uint8_t*)controllerHash, 32);
                    if (i != 0) return;
                    
                    bcopy(controllerIdentifier, &controllerHash[32], 36);
                    bcopy(controllerPublicKey, &controllerHash[68], 32);
                    
                    int ed25519_err = ed25519_sign_open((const unsigned char*)controllerHash, 100, (const unsigned char*)controllerPublicKey, (const unsigned char*)controllerSignature);
                    delete subTLV8;
                    
                    if (ed25519_err) {
                        printf("Problem\n");
                        return;
                    }
                    else {
                        PHKNetworkMessageData *returnTLV8 = new PHKNetworkMessageData();
                        
                        for(unsigned int deviceNr=0; deviceNr<devices->size();deviceNr++)
                        {
                            DeviceStruct device = devices->at(deviceNr);
                            PHKNetworkMessageDataRecord usernameRecord;
                            usernameRecord.activate = true;
                            usernameRecord.index = 1;
                            usernameRecord.length = device.identity.length();
                            usernameRecord.data = new char[usernameRecord.length]; bcopy(device.identityAsChar(), usernameRecord.data, usernameRecord.length);
                            returnTLV8->addRecord(usernameRecord);
                        }
                        
                        for(unsigned int deviceNr=0; deviceNr<devices->size();deviceNr++)
                        {
                            DeviceStruct device = devices->at(deviceNr);
                        
                            /*
                             * Generate Signature
                             */
                            const char salt[] = "Pair-Setup-Accessory-Sign-Salt";
                            const char info[] = "Pair-Setup-Accessory-Sign-Info";
                            uint8_t output[150];
                            hkdf((const unsigned char*)salt, strlen(salt), (const unsigned char*)secretKey->data, secretKey->length, (const unsigned char*)info, strlen(info), output, 32);
                            
                            bcopy(device.identityAsChar(), &output[32], device.identity.length());
                            
                            char *signature = new char[64];
                            ed25519_secret_key edSecret;
                            bcopy(accessorySecretKey, edSecret, sizeof(edSecret));
                            ed25519_public_key edPubKey;
                            ed25519_publickey(edSecret, edPubKey);
                            
                            bcopy(edPubKey, &output[32+device.identity.length()], 32);
                            ed25519_sign(output, 64+device.identity.length(), (const unsigned char*)edSecret, (const unsigned char*)edPubKey, (unsigned char *)signature);
                            PHKNetworkMessageDataRecord signatureRecord;
                            signatureRecord.activate = true; signatureRecord.data = signature;   signatureRecord.index = 10;    signatureRecord.length = 64;
                            returnTLV8->addRecord(signatureRecord);
                            
                            PHKNetworkMessageDataRecord publicKeyRecord;
                            publicKeyRecord.activate = true;   publicKeyRecord.index = 3;    publicKeyRecord.length = 32; publicKeyRecord.data = new char[publicKeyRecord.length];    bcopy(edPubKey, publicKeyRecord.data, publicKeyRecord.length);
                            returnTLV8->addRecord(publicKeyRecord);
                        }
                        
                        const char *tlv8Data;unsigned short tlv8Len;
                        returnTLV8->rawData(&tlv8Data, &tlv8Len);
                        
                        PHKNetworkMessageDataRecord tlv8Record;
                        tlv8Record.data = new char[tlv8Len+16];tlv8Record.length = tlv8Len+16;
                        bzero(tlv8Record.data, tlv8Record.length);
                        {
                            
                            chacha20_ctx ctx;   bzero(&ctx, sizeof(ctx));
                            
                            chacha20_setup(&ctx, (const uint8_t *)sessionKey, 32, (uint8_t *)"PS-Msg06");
                            char buffer[64], key[64];   bzero(buffer, 64);
                            chacha20_encrypt(&ctx, (const uint8_t *)buffer, (uint8_t *)key, 64);
                            chacha20_encrypt(&ctx, (const uint8_t *)tlv8Data, (uint8_t *)tlv8Record.data, tlv8Len);
                            
                            poly1305_context polyCtx;   bzero(&polyCtx, sizeof(polyCtx));
                            
                            poly1305_init(&polyCtx, (const unsigned char *)key);
                            poly1305_update(&polyCtx, (const unsigned char*)tlv8Record.data, tlv8Len);
                            {
                                char *waste = new char[16];
                                bzero(waste, 16);
                                poly1305_update(&polyCtx, (const unsigned char *)waste, 16-tlv8Len%16);
                                unsigned long long _len;
                                _len = 0;
                                poly1305_update(&polyCtx, (const unsigned char *)&_len, 8);
                                _len = tlv8Len;
                                poly1305_update(&polyCtx, (const unsigned char *)&_len, 8);
                                delete [] waste;
                            }
                            
                            poly1305_finish(&polyCtx, (unsigned char *)&tlv8Record.data[tlv8Len]);
                        }
                        
                        tlv8Record.activate = true; tlv8Record.index = 5;
                        mResponse.data.addRecord(tlv8Record);
                        
                        delete returnTLV8;
                        printf("Send M6\n");
                    }
                    
                }
                
                mResponse.data.addRecord(stateRecord);
                mResponse.getBinaryPtr(&responseBuffer, &responseLen);
                if (responseBuffer) {
                    write(subSocket, (const void *)responseBuffer, (size_t)responseLen);
                    delete [] responseBuffer;
                }
                return;
            }
                break;
        }
        mResponse.data.addRecord(stateRecord);
        mResponse.getBinaryPtr(&responseBuffer, &responseLen);
        if (responseBuffer) {
            int len = write(subSocket, (const void *)responseBuffer, (size_t)responseLen);
            delete [] responseBuffer;
#if HomeKitLog == 1
            printf("Pair Setup Transfered length %d\n", len);
#endif
        } else {
#if HomeKitLog == 1
            printf("Why empty response\n");
#endif
        }
        
    } while (read(subSocket, (void *)buffer, 4096) > 0);
    SRP_free(srp);
}

void PHKConnection::handlePairVerify(int subSocket, char *buffer) const {
    bool end = false;
    unsigned char state = 1;
    
    curved25519_key secretKey;
    curved25519_key publicKey;
    curved25519_key controllerPublicKey;
    curved25519_key sharedKey;
    
    uint8_t controllerToAccessoryKey[32], accessoryToControllerKey[32];
    
    unsigned long long numberOfMsgRec = 0, numberOfMsgSend = 0;
    
    uint8_t enKey[32];
#if HomeKitLog == 1
    printf("Start Pair Verify\n");
#endif
    
    do {
        PHKNetworkMessage msg(buffer);
        PHKNetworkResponse response = PHKNetworkResponse(200);
        bcopy(msg.data.dataPtrForIndex(6), &state, 1);
        printf("State = %d\n", state);
        switch (state) {
            case 1: {
#if HomeKitLog == 1
                printf("Pair Verify M1\n");
#endif
                for(unsigned int deviceNr=0; deviceNr<devices->size(); deviceNr++)
                {
                    DeviceStruct device = devices->at(deviceNr);
                    
                    bcopy(msg.data.dataPtrForIndex(3), controllerPublicKey, 32);
                    for (short i = 0; i < sizeof(secretKey); i++) {
                        secretKey[i] = rand();
                    }
                    curve25519_donna((u8*)publicKey, (const u8 *)secretKey, (const u8 *)curveBasePoint);
                    
                    curve25519_donna(sharedKey, secretKey, controllerPublicKey);
                    
                    char *temp = new char[100];
                    bcopy(publicKey, temp, 32);
                    bcopy(device.identityAsChar(), &temp[32], device.identity.length());
                    bcopy(controllerPublicKey, &temp[32+device.identity.length()], 32);
                    
                    PHKNetworkMessageDataRecord signRecord;
                    signRecord.activate = true; signRecord.data = new char[64]; signRecord.index = 10;  signRecord.length = 64;
                    
                    ed25519_secret_key edSecret;
                    bcopy(accessorySecretKey, edSecret, sizeof(edSecret));
                    ed25519_public_key edPubKey;
                    ed25519_publickey(edSecret, edPubKey);
                    
                    ed25519_sign((const unsigned char *)temp, 64+device.identity.length(), edSecret, edPubKey, (unsigned char *)signRecord.data);
                    delete [] temp;
                    
                    PHKNetworkMessageDataRecord idRecord;
                    idRecord.activate = true;   idRecord.data = new char[device.identity.length()]; strcpy(idRecord.data, device.identityAsChar()); idRecord.index = 1; idRecord.length = (unsigned int)device.identity.length();
                    PHKNetworkMessageDataRecord pubKeyRecord;
                    pubKeyRecord.activate = true;   pubKeyRecord.data = new char[32];bcopy(publicKey, pubKeyRecord.data, 32);  pubKeyRecord.index = 3; pubKeyRecord.length = 32;
                    
                    PHKNetworkMessageData data;
                    response.data.addRecord(pubKeyRecord);
                    data.addRecord(signRecord);
                    data.addRecord(idRecord);
                    
                    unsigned char salt[] = "Pair-Verify-Encrypt-Salt";
                    unsigned char info[] = "Pair-Verify-Encrypt-Info";
                    
                    int i = hkdf(salt, 24, sharedKey, 32, info, 24, enKey, 32);
                    const char *plainMsg = 0;   unsigned short msgLen = 0;
                    data.rawData(&plainMsg, &msgLen);
                    
                    char *encryptMsg = new char[msgLen+16];
                    char *polyKey = new char[64];   bzero(polyKey, 64);
                    
                    char zero[64];  bzero(zero, 64);
                    
                    chacha20_ctx chacha;
                    chacha20_setup(&chacha, enKey, 32, (uint8_t *)"PV-Msg02");
                    chacha20_encrypt(&chacha, (uint8_t *)zero, (uint8_t *)polyKey, 64);
                    chacha20_encrypt(&chacha, (uint8_t *)plainMsg, (uint8_t *)encryptMsg, msgLen);
                    
                    delete [] plainMsg;
                    
                    poly1305_context poly;
                    poly1305_init(&poly, (const unsigned char *)polyKey);
                    poly1305_update(&poly, (unsigned char *)encryptMsg, msgLen);
                    
                    {
                        char *waste = new char[16];
                        bzero(waste, 16);
                        poly1305_update(&poly, (const unsigned char *)waste, 16-msgLen%16);
                        unsigned long long _len;
                        _len = 0;
                        poly1305_update(&poly, (const unsigned char *)&_len, 8);
                        _len = msgLen;
                        poly1305_update(&poly, (const unsigned char *)&_len, 8);
                        delete [] waste;
                    }
                    poly1305_finish(&poly, (unsigned char *)&encryptMsg[msgLen]);
                    
                    PHKNetworkMessageDataRecord encryptRecord;
                    encryptRecord.activate = true;  encryptRecord.index = 5; encryptRecord.length = msgLen+16;  encryptRecord.data = new char[encryptRecord.length];    bcopy(encryptMsg, encryptRecord.data, encryptRecord.length);
                    response.data.addRecord(encryptRecord);
                    
                    delete [] encryptMsg;
                    delete [] polyKey;
                }
            }
                break;
            case 3: {
#if HomeKitLog == 1
                printf("Pair Verify M3\n");
#endif
                char *encryptedData = msg.data.dataPtrForIndex(5);
                short packageLen = msg.data.lengthForIndex(5);
                
                chacha20_ctx chacha20;    bzero(&chacha20, sizeof(chacha20));
                chacha20_setup(&chacha20, (const uint8_t *)enKey, 32, (uint8_t *)"PV-Msg03");
                
                //Ploy1305 key
                char temp[64];  bzero(temp, 64); char temp2[64];  bzero(temp2, 64);
                chacha20_encrypt(&chacha20, (const uint8_t*)temp, (uint8_t *)temp2, 64);
                
                char verify[16];    bzero(verify, 16);
                poly1305_context verifyContext; bzero(&verifyContext, sizeof(verifyContext));
                poly1305_init(&verifyContext, (const unsigned char*)temp2);
                poly1305_update(&verifyContext, (const unsigned char *)encryptedData, packageLen-16);
                {
                    char waste[16];
                    bzero(waste, 16);
                    poly1305_update(&verifyContext, (const unsigned char *)waste, 16-packageLen%16);
                    unsigned long long _len;
                    _len = 0;
                    poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                    _len = packageLen - 16;
                    poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                }
                poly1305_finish(&verifyContext, (unsigned char *)verify);
                
                if (!bcmp(verify, &encryptedData[packageLen-16], 16)) {
                    char *decryptData = new char[packageLen-16];
                    chacha20_decrypt(&chacha20, (const uint8_t *)encryptedData, (uint8_t *)decryptData, packageLen-16);
                    PHKNetworkMessageData data = PHKNetworkMessageData(decryptData, packageLen-16);
                    
                    PHKKeyRecord rec = getControllerKey(data.dataPtrForIndex(1));
                    
                    char tempMsg[100];
                    bcopy(controllerPublicKey, tempMsg, 32);
                    bcopy(data.dataPtrForIndex(1), &tempMsg[32], 36);
                    bcopy(publicKey, &tempMsg[68], 32);
                    
                    int err = ed25519_sign_open((const unsigned char *)tempMsg, 100, (const unsigned char *)rec.publicKey, (const unsigned char *)data.dataPtrForIndex(10));
                    
                    char *repBuffer = 0;  int repLen = 0;
                    if (err == 0) {
                        end = true;
                        
                        hkdf((uint8_t *)"Control-Salt", 12, sharedKey, 32, (uint8_t *)"Control-Read-Encryption-Key", 27, accessoryToControllerKey, 32);
                        hkdf((uint8_t *)"Control-Salt", 12, sharedKey, 32, (uint8_t *)"Control-Write-Encryption-Key", 28, controllerToAccessoryKey, 32);
                        
                        
                    } else {
                        PHKNetworkMessageDataRecord error;
                        error.activate = true;  error.data = new char[1];   error.data[0] = 2;  error.index = 7;    error.length = 1;
                        response.data.addRecord(error);
                    }
                    
                    delete [] decryptData;
                }
                
            }
        }
        
        PHKNetworkMessageDataRecord stage;
        stage.activate = true;   stage.data = new char; stage.data[0] = state+1; stage.index = 6; stage.length = 1;
        response.data.addRecord(stage);
        
        char *repBuffer = 0;  int repLen = 0;
        response.getBinaryPtr(&repBuffer, &repLen);
        write(subSocket, repBuffer, repLen);
        delete [] repBuffer;
    } while (!end && read(subSocket, buffer, 4096) > 0);
    
    char *decryptData = new char[2048];
    
    int len;
    
#if HomeKitLog == 1
    printf("Successfully Connect\n");
#endif
    
    do {
        bzero(buffer, 4096);
        len = read(subSocket, buffer, 4096);
        
        if (len > 0) {
            uint16_t msgLen = (uint8_t)buffer[1]*256+(uint8_t)*buffer;
            
            chacha20_ctx chacha20;    bzero(&chacha20, sizeof(chacha20));
            chacha20_setup(&chacha20, (const uint8_t *)controllerToAccessoryKey, 32, (uint8_t *)&numberOfMsgRec);
            numberOfMsgRec++;
            
            //Ploy1305 key
            char temp[64];  bzero(temp, 64); char temp2[64];  bzero(temp2, 64);
            chacha20_encrypt(&chacha20, (const uint8_t*)temp, (uint8_t *)temp2, 64);
            bzero(decryptData, 2048);
            chacha20_decrypt(&chacha20, (const uint8_t *)&buffer[2], (uint8_t *)decryptData, msgLen);
            
            char verify[16];    bzero(verify, 16);
            poly1305_context verifyContext; bzero(&verifyContext, sizeof(verifyContext));
            poly1305_init(&verifyContext, (const unsigned char*)temp2);
            {
                char waste[16];
                bzero(waste, 16);
                
                poly1305_update(&verifyContext, (const unsigned char *)buffer, 2);
                poly1305_update(&verifyContext, (const unsigned char *)waste, 14);
                
                poly1305_update(&verifyContext, (const unsigned char *)&buffer[2], msgLen);
                poly1305_update(&verifyContext, (const unsigned char *)waste, 16-msgLen%16);
                
                unsigned long long _len;
                _len = 2;
                poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                _len = msgLen;
                poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
            }
            poly1305_finish(&verifyContext, (unsigned char *)verify);
            
            //Output return
            char *resultData = 0; unsigned int resultLen = 0;
            handleAccessory(decryptData, msgLen, &resultData, &resultLen);
            
            
            char *reply = new char[resultLen+18];
            reply[0] = resultLen%256;
            reply[1] = (resultLen-(uint8_t)reply[0])/256;
            
            chacha20_setup(&chacha20, (const uint8_t *)accessoryToControllerKey, 32, (uint8_t *)&numberOfMsgSend);
            numberOfMsgSend++;
            chacha20_encrypt(&chacha20, (const uint8_t*)temp, (uint8_t *)temp2, 64);
            chacha20_encrypt(&chacha20, (const uint8_t*)resultData, (uint8_t*)&reply[2], resultLen);
            
            poly1305_init(&verifyContext, (const unsigned char*)temp2);
            {
                char waste[16];
                bzero(waste, 16);
                
                poly1305_update(&verifyContext, (const unsigned char *)reply, 2);
                poly1305_update(&verifyContext, (const unsigned char *)waste, 14);
                
                poly1305_update(&verifyContext, (const unsigned char *)&reply[2], resultLen);
                poly1305_update(&verifyContext, (const unsigned char *)waste, 16-resultLen%16);
                unsigned long long _len;
                _len = 2;
                poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
                _len = resultLen;
                poly1305_update(&verifyContext, (const unsigned char *)&_len, 8);
            }
            poly1305_finish(&verifyContext, (unsigned char*)&reply[resultLen+2]);
            
            write(subSocket, reply, resultLen+18);
            
            delete [] reply;
            delete [] resultData;
            
            
            
        }
    } while (len > 0);

    delete [] decryptData;
}

//Object Logic
PHKNetworkIP::~PHKNetworkIP() {
    for(unsigned int i=0; i<devices->size(); i++) {
        DNSServiceRefDeallocate(devices->at(i).netServiceV4);
    }
}

PHKNetworkMessage::PHKNetworkMessage(const char *rawData) {
    strcpy(method, "POST");
    
    const char *ptr = rawData;
    for (int i = 0; (*ptr)!=0 && (*ptr)!=' '; ptr++, i++) {
        method[i] = (*ptr);
        method[i+1] = 0;
    } ptr+=2;
    
    //Copy message directory
    for (int i = 0; (*ptr)!=0 && (*ptr)!=' '; ptr++, i++) {
        directory[i] = (*ptr);
        directory[i+1] = 0;
    }
    
    //Get the length of content
    //Skip to the content-length
    for (; (*ptr)!=0&&(*ptr)!=':'; ptr++);  ptr++;
    unsigned int dataSize = (unsigned int)strtol(ptr, (char **)&ptr, 10);
    
    //Get the type of content
    //Skip to the content-length
    for (; (*ptr)!=0&&(*ptr)!=':'; ptr++);  ptr+=2;
    for (int i = 0; (*ptr)!=0 && (*ptr)!='\r'; ptr++, i++) {
        type[i] = (*ptr);
        type[i+1] = 0;
    }
    ptr+=4;
    
    //Data
    data = PHKNetworkMessageData(ptr, dataSize);
    
}

void PHKNetworkMessage::getBinaryPtr(char **buffer, int *contentLength) {
    const char *_data; unsigned short dataSize;
    data.rawData(&_data, &dataSize);
    (*buffer) = new char[1024];
    (*contentLength) = snprintf((*buffer), 1024, "%s /%s HTTP/1.1\r\nContent-Length: %hu\r\nContent-Type: %s\r\n\r\n", method, directory, dataSize, type);
    for (int i = 0; i < dataSize; i++) {
        (*buffer)[*contentLength+i] = _data[i];
    }
    (*contentLength)+=dataSize;
    (*buffer)[*contentLength] = 0;
}



PHKNetworkMessageData & PHKNetworkMessageData::operator=(const PHKNetworkMessageData &data) {
    count = data.count;
    for (int i = 0; i < 10; i++) {
        if (data.records[i].length) {
            records[i] = data.records[i];
            records[i].data = new char[records[i].length];
            bcopy(data.records[i].data, records[i].data, data.records[i].length);
        }
    }
    return *this;
}

PHKNetworkMessageData::PHKNetworkMessageData(const char *rawData, unsigned short len) {
    unsigned short delta = 0;
    while (delta < len) {
        int index = recordIndex(rawData[delta+0]);
        if (index < 0) {
            records[count].index = (rawData)[delta+0];
            records[count].length = (unsigned char)(rawData)[delta+1];
            records[count].data = new char[records[count].length];
            records[count].activate = true;
            bcopy(&rawData[delta+2], records[count].data, records[count].length);
            delta += (records[count].length+2);
            count++;
        } else {
            int newLen = ((unsigned char*)(rawData))[delta+1];
            newLen += records[index].length;
            char *ptr = new char[newLen];
            bcopy(records[index].data, ptr, records[index].length);
            bcopy(&rawData[delta+2], &ptr[records[index].length], newLen-records[index].length);
            delete [] records[index].data;
            records[index].data = ptr;
            
            delta += (newLen-records[index].length+2);
            records[index].length = newLen;
            
        }
    }
}

void PHKNetworkMessageData::rawData(const char **dataPtr, unsigned short *len) {
    string buffer = "";
    for (int i = 0; i < 10; i++) {
        if (records[i].activate) {
            for (int j = 0; j != records[i].length;) {
                unsigned char len = records[i].length-j>255?255:records[i].length-j;
                string temp(&records[i].data[j], len);
                temp = (char)records[i].index+((char)len+temp);
                buffer += temp;
                j += (unsigned int)len;
            }
        }
    }
    *len = buffer.length();
    *dataPtr = new char[*len];
    bcopy(buffer.c_str(), (void *)*dataPtr, *len);
}

int PHKNetworkMessageData::recordIndex(unsigned char index) {
    for (int i = 0; i < count; i++) {
        if (records[i].activate&&records[i].index==index) return i;
    }
    return -1;
}

void PHKNetworkMessageData::addRecord(PHKNetworkMessageDataRecord& record) {
    records[count] = record;
    count++;
}

PHKNetworkResponse::PHKNetworkResponse(unsigned short _responseCode) {
    responseCode = _responseCode;
}

string PHKNetworkResponse::responseType() {
    switch (responseCode) {
        case 200:
            return "OK";
        default:
            return "";
    }
}

inline void int2str(int i, char *s) { sprintf(s,"%d",i); }

void PHKNetworkResponse::getBinaryPtr(char **buffer, int *contentLength) {
    char tempCstr[5];   int2str(responseCode, tempCstr);
    string temp = "HTTP/1.1 ";
    temp += tempCstr+(" "+responseType());
    temp += "\r\nContent-Type: application/pairing+tlv8\r\nContent-Length: ";
    const char *dataPtr;    unsigned short dataLen;
    data.rawData(&dataPtr, &dataLen);
    int2str(dataLen, tempCstr);
    temp += tempCstr;
    temp += "\r\n\r\n";
    *buffer = new char[temp.length()+dataLen];
    bcopy(temp.c_str(), *buffer, temp.length());
    bcopy(dataPtr, &((*buffer)[temp.length()]), dataLen);
    *contentLength = temp.length()+dataLen;
    delete [] dataPtr;
}

char *PHKNetworkMessageData::dataPtrForIndex(unsigned char index) {
    int _index = recordIndex(index);
    if (_index >= 0)
        return records[_index].data;
    return 0;
}

unsigned int PHKNetworkMessageData::lengthForIndex(unsigned char index) {
    int _index = recordIndex(index);
    if (_index >= 0)
        return records[_index].length;
    return 0;
}

PHKNetworkMessageDataRecord &PHKNetworkMessageDataRecord::operator=(const PHKNetworkMessageDataRecord& r) {
    index = r.index;
    activate = r.activate;
    length = r.length;
    if (data)
        delete [] data;
    data = new char[length];
    bcopy(r.data, data, length);
    return *this;
}

PHKNetworkMessageDataRecord::~PHKNetworkMessageDataRecord() {
    if (length) delete [] data;
}
