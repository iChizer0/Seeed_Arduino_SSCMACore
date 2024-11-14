#pragma once

#ifndef _MA_CONFIG_H_
#define _MA_CONFIG_H_

#include <sdkconfig.h>

#if CONFIG_IDF_TARGET_ESP32S3
#define MA_PORTING_ESPRESSIF_ESP32S3 1
#endif

#if MA_PORTING_ESPRESSIF_ESP32S3
#include "sscma-micro-porting/porting/espressif/esp32s3/ma_config_board.h"
#else
#error "Unsupported platform"
#endif

#endif