//
//  ObjectiveCInterface.h
//  TunnelFever
//
//  Created by admin on 6/30/23.
//
#ifndef ObjectiveCInterface_h
#define ObjectiveCInterface_h

#include <vector>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

void readFileData(const char* filePath, uint8_t** data, size_t* dataSize);

#ifdef __cplusplus
}
#endif

#endif /* ObjectiveCInterface_h */
