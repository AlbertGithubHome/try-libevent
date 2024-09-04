#ifndef AW_COMMON_H
#define AW_COMMON_H

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

using AWUINT64 = uint64_t;
using AWUINT32 = uint32_t;
using AWUINT16 = uint16_t;
using AWUINT8 = uint8_t;

using AWINT64 = int64_t;
using AWINT32 = int32_t;
using AWINT16 = int16_t;
using AWINT8 = int8_t;

using AWSOCK = int64_t;
using AWCONV = uint32_t;

#define AW_SAFE_DELETE(p) do { if (p) { delete (p); (p) = NULL; } } while (0) // AW_SAFE_DELETE

#endif