#include <spi-interface.h>
static const char *device = "/dev/spidev1.0";

int fd = -1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#define CRC_SEED   0xFFFF   // 该位称为预置值，使用人工算法（长除法）时 需要将除数多项式先与该与职位 异或 ，才能得到最后的除数多项式
#define POLY16 0x1021  // 该位为简式书写 实际为0x11021

long currentTimeMillis()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint8_t * loadUpdateData(int file,long *flen)
{
	FILE* pFile;
    if(file == 0)
    {
	    pFile = fopen("/mnt/sdcard/mybin/6dofstation.bin", "rb"); // 重新打开文件读操作
		if (pFile == NULL) {
			LOGI("fopen bin fail,please put 6dofstation.bin in sdcard/mybin/");
			abort();
		}
    }else if(file == 1)
    {
    	pFile = fopen("/mnt/sdcard/mybin/6dofcontroller.bin", "rb"); // 重新打开文件读操作
		if (pFile == NULL) {
			LOGI("fopen bin fail,please put 6dofcontroller.bin in sdcard/mybin/");
			abort();
		}
    }else if(file == 2)
    {
    	pFile = fopen("/mnt/sdcard/mybin/6dofSTM32.bin", "rb"); // 重新打开文件读操作
		if (pFile == NULL) {
			LOGI("fopen bin fail,please put 6dofSTM32.bin.bin in sdcard/mybin/");
			abort();
		}
    }

	fseek(pFile,0L,SEEK_END);

	*flen = ftell(pFile);

	fseek(pFile,0L,SEEK_SET);
	uint8_t * app_source_code = (uint8_t *)malloc(sizeof(uint8_t)*(*flen));
	memset(app_source_code,0,*flen);

	fread(app_source_code, 1, *flen, pFile); // 从文件中读数据
	fclose(pFile); // 关闭文件

	return app_source_code;
}
ssize_t openSPIDevice()
{
	LOGD("openSPIDevice");
	if(fd < 0)
	{
		fd = open(device, O_RDWR);
	}
	LOGI("openSPIDevice fd =%d",fd);
	if (fd < 0){
		//pabort("ota can't open device");
		LOGD("openSPIDevice ota can't open device");
		//pthread_mutex_init(&mutex,NULL);
		return 0;
	}else{
		LOGD("openSPIDevice success");
		return 1;
	}
}

ssize_t writeDataToSPI(const void *buf, size_t count)
{
	ssize_t result = -1;
	if(fd >= 0){
		pthread_mutex_lock(&mutex);
		result = write(fd,buf,count);
		pthread_mutex_unlock(&mutex);
	}else{
		LOGD("writeDataToSPI Error fd < 0");
	}
	return result;
}

ssize_t readDataFromSPI(void *buf, size_t count){
	ssize_t result = -1;
	if(fd >= 0){
		pthread_mutex_lock(&mutex);
		//LOGD("readDataFromSPI 1");
		result = read(fd,buf,count);
		//LOGD("readDataFromSPI 2");
		pthread_mutex_unlock(&mutex);
		//LOGD("readDataFromSPI 3");
	}else{
		LOGD("readDataFromSPI Error fd < 0");
	}
	return result;
}

ssize_t closeSPIDevice(){
	ssize_t result = -1;
	LOGD("closeSPIDevice fd=%d",fd);
	if(fd >= 0){
		result = close(fd);
		fd = -1;
	}
	return result;
}

const char CRC8Table[]={
  0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
  157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
  35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
  190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
  70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
  219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
  101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
  248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
  140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
  17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
  175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
  50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
  202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
  87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
  233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
  116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

uint8_t crc8(const uint8_t *p, int counter)
{
    unsigned char crc8 = 0;
    for( ; counter > 0; counter--){
        crc8 = CRC8Table[crc8^*p];
        p++;
    }
    return(crc8);
}
uint16_t crc16(uint8_t *packet,uint32_t size,uint16_t *p_crc)
{
    uint32_t i;
    uint16_t crc = (p_crc == NULL) ? 0xffff : *p_crc;
	 //uint16_t crc16_result = 0x00;
    for (i = 0; i < size; i++)
    {
        crc  = (unsigned char)(crc >> 8) | (crc << 8);
        crc ^= packet[i];
        crc ^= (unsigned char)(crc & 0xff) >> 4;
        crc ^= (crc << 8) << 4;
        crc ^= ((crc & 0xff) << 4) << 1;
    }
    //crc16_result = crc;
    return crc;
}
uint32_t crc32(uint32_t *ptr, uint32_t len)
{
    uint32_t        xbit;
    uint32_t        data;
    uint32_t        CRC32 = 0xFFFFFFFF;
    uint32_t  bits;
    const uint32_t dwPolynomial = 0x04c11db7;
    uint32_t        i;
    for(i = 0; i < len; i ++)
    {
        xbit = 1 << 31;
        data = ptr[i];
        for (bits = 0; bits < 32; bits++)
        {
            if (CRC32 & 0x80000000)
            {
                CRC32 <<= 1;
                CRC32 ^= dwPolynomial;
            }
            else
                CRC32 <<= 1;
            if (data & xbit)
                CRC32 ^= dwPolynomial;
            xbit >>= 1;
        }
    }
    return CRC32;
}

