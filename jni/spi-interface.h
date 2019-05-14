#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include <string.h>
#include <jni.h>
#include <android/log.h>

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <pthread.h>

#ifndef  LOG_TAG
#define LOG_TAG "CVSPI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif

#define DEFAULT_BUF_SIZE					131

long currentTimeMillis();
ssize_t openSPIDevice();
ssize_t writeDataToSPI(const void *buf, size_t count);
ssize_t readDataFromSPI(void *buf, size_t count);
ssize_t closeSPIDevice();
uint8_t * loadUpdateData(int file,long *flen);

uint8_t crc8(const uint8_t *p, int counter);
uint16_t crc16(uint8_t *packet,uint32_t size,uint16_t *p_crc);
uint32_t crc32(uint32_t *ptr, uint32_t len);

#endif
