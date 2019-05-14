#include <control.h>
#include <time.h>


char * inttohex(int aa)
{
	char buffer[1]; //用于存放转换好的十六进制字符串，可根据需要定义长度
    sprintf(buffer, "%x", aa);
    return (buffer);
}

char * GetVersion(uint8_t devices)
{
	LOGD("GetStationVersion begin");
    int rc;
    int i = 0;
    unsigned char buf_tx[DEFAULT_BUF_SIZE] = {0,};
    unsigned char buf_rx[DEFAULT_BUF_SIZE] = {0,};

    memset(buf_tx, 0, DEFAULT_BUF_SIZE);
    memset(buf_rx, 0, DEFAULT_BUF_SIZE);

    buf_tx[0] = 0xBB;
    buf_tx[1] = 0x01;

    buf_tx[2] = devices;
	buf_tx[4] = devices;

    buf_tx[3] = 0x04;

    buf_tx[7] = crc8(buf_tx, 7);
	int isGetSucess = 0;
	if (openSPIDevice() == 1)
	{
		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		usleep(500);
		LOGD("GetStationVersion write rc = %d, buf_tx[7] = 0x%x", rc, buf_tx[7]);

		int reSendIndex = 0;

		for (i = 0; i < 800; i++) {
			LOGD("read i = %d", i);
			memset(buf_rx, 0, DEFAULT_BUF_SIZE);
			rc = readDataFromSPI(buf_rx, DEFAULT_BUF_SIZE);

			if (rc > 0) {
				if (buf_rx[0] == 0xBB) {
					if ((buf_rx[1] == 0x01) && (buf_rx[5] == 0x01)) {
						isGetSucess = 1;
						break;
					}
				} else {
					reSendIndex++;
					LOGD(" buf_rx[0] = %x,%x,%x,%x,%x,%x", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3], buf_rx[4], buf_rx[5]);
					if (reSendIndex >= 80) {
						usleep(1000);
						writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						usleep(500);
						LOGD("send buf_tx[0] = %x,%x,%x,%x,%x,%x", buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4], buf_tx[5]);
						reSendIndex = 0;
					}
				}
			} else {
				LOGD("No version data\n");
			}
		}
	} else {
		LOGD("open devices fail");
	}
    closeSPIDevice();
    //LOGD("GetStationVersion begin buf_rx=%s",buf_rx);
    LOGD("GetStationVersion end");
    if( isGetSucess == 1)
    {
    	char buff[60] = {0};
    	strncpy(buff, buf_rx+6, 60);
        return buff;
    }else
    {
    	LOGD("Get Version Fail");
    	return "Get Version Fail";
    }
}

int isEnterPairMode()
{
	unsigned char buf_rx[DEFAULT_BUF_SIZE] = { 0, };
	memset(buf_rx, 0, DEFAULT_BUF_SIZE);

	if (openSPIDevice() == 1)
	{
		int i;
		int rc;
		for (i = 0; i < 400; i++) {
			LOGD("isEnterPairMode read i = %d", i);
			rc = readDataFromSPI(buf_rx, DEFAULT_BUF_SIZE);

			LOGD(
					"isEnterPairMode rc = %d , buf_rx[129] = 0x%x", rc, buf_rx[129]);
			if (rc > 0) {
				if (buf_rx[129] & 0x04) {
					LOGD("EnterPairMode true");
					closeSPIDevice();
					return 1;
				}
				if ((buf_rx[129] & 0x01) && !(buf_rx[129] & 0x02)) {
					LOGD(
							"已成功连接  controller1 = %d,controller2 = %d", (buf_rx[129] & 0x01), (buf_rx[129] & 0x02));
					return 2;
				} else if (!(buf_rx[129] & 0x01) && (buf_rx[129] & 0x02)) {
					LOGD(
							"已成功连接  controller1 = %d,controller2 = %d", (buf_rx[129] & 0x01), (buf_rx[129] & 0x02));
					return 3;
				} else if ((buf_rx[129] & 0x01) && (buf_rx[129] & 0x02)) {
					return 4;
				}
			}
			usleep(500);
		}
	} else {
		LOGD("open devices fail");
	}
	closeSPIDevice();
	return 0;
}


void enterPairMode(uint8_t whichController)
{
	LOGD("enterPairMode begin whichController = %d",whichController);
    int rc;
    unsigned char buf_tx[DEFAULT_BUF_SIZE] = {0,};

    memset(buf_tx, 0, DEFAULT_BUF_SIZE);

    buf_tx[0] = 0xBB;
    buf_tx[1] = 0x01;
    buf_tx[2] = 0x00;
    buf_tx[3] = 0x01;
//	buf_tx[4] = inttohex(whichController)[0];
//	LOGD(" buf_tx[4] = 0x%x",whichController);
	buf_tx[4] = whichController;

    buf_tx[7] = crc8(buf_tx, 7);

    if(openSPIDevice()==1)
    {
		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		LOGD("enterPairMode write rc = %d, buf_tx[7] = 0x%x", rc, buf_tx[7]);

		closeSPIDevice();
		LOGD("enterPairMode end whichController = %d", whichController);
	}else
	{
		LOGD("open devices fail");
	}
}

void setControllervibrateTest()
{
	unsigned char buf_tx[DEFAULT_BUF_SIZE] = { 0, };

	memset(buf_tx, 0, DEFAULT_BUF_SIZE);

	buf_tx[0] = 0xBB;
	buf_tx[1] = 0x01;
	buf_tx[2] = 0x01;
	buf_tx[3] = 0x02;
	buf_tx[4] = 100;
	buf_tx[7] = crc8(buf_tx, 7);

	if (openSPIDevice() == 1) {
		LOGI("setControllervibrateTest");
		readDataFromSPI(buf_tx, DEFAULT_BUF_SIZE);
	}
}
int stopVibrateFlag = 0;
void stopControllervibrate()
{
	stopVibrateFlag = 1;
	LOGI("stopVibrateFlag = %d",stopVibrateFlag);
}

void setControllervibrate(uint8_t whichController,uint8_t strength,int repeatTimes ,int vibrateDuration,int silenceDuration)
{
	stopVibrateFlag = 0;
	silenceDuration *=1000;
	LOGD("setControllervibrate begin");
	int rc;
	unsigned char buf_tx[DEFAULT_BUF_SIZE] = { 0, };

	memset(buf_tx, 0, DEFAULT_BUF_SIZE);

	buf_tx[0] = 0xBB;
	buf_tx[1] = 0x01;
	buf_tx[2] = whichController;
	buf_tx[3] = 0x02;
	buf_tx[4] = strength;
	buf_tx[7] = crc8(buf_tx, 7);

	if (openSPIDevice() == 1) {
		if (repeatTimes > 0) {
			int i = 0;
			for (i = 0; i < repeatTimes; i++) {
				//LOGD("repeatTimes  = %d",i);
				//time_t 获得时间只能精确到秒,clock_t 获得时间能够精确到毫秒
				long start, ends;
				start = currentTimeMillis();
				ends = currentTimeMillis();
				//long timer1 = time(NULL);
				//long timer2 = time(NULL);

				while ((ends - start) < vibrateDuration) {
					//LOGD("stopVibrateFlag 1 = %d",stopVibrateFlag);
					if (stopVibrateFlag == 0) {
						rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						//LOGD("setControllervibrate ends = %d,start=%d,diff = %d",ends,start,ends - start);
						usleep(20000);
						ends = currentTimeMillis();
					} else {
						break;
					}
				}

				if (stopVibrateFlag == 0) {
					usleep(silenceDuration);
				} else {
					LOGI("stopVibrateFlag break");
					break;
				}
			}

		}
	} else {
		LOGD("open devices fail");
	}
	if (stopVibrateFlag != 0) {
		stopVibrateFlag = 0;
		LOGI("stopVibrateFlag = %d", stopVibrateFlag);
	}
	closeSPIDevice();
	LOGD("setControllervibrate end");
}

int getRadioStatus()
{
	LOGD("getRadioStatus begin");
	int rc;
	unsigned char buf_tx[DEFAULT_BUF_SIZE] = { 0, };
	unsigned char buf_rx[DEFAULT_BUF_SIZE] = { 0, };

	memset(buf_tx, 0, DEFAULT_BUF_SIZE);

	buf_tx[0] = 0xBB;
	buf_tx[1] = 0x01;
	buf_tx[2] = 0x00;
	buf_tx[3] = 0x03;
	buf_tx[4] = 0x03;
	buf_tx[7] = crc8(buf_tx, 7);
	int isGetSucess = 0;
	if (openSPIDevice() == 1) {

		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		LOGD("getRadioStatus write rc = %d, buf_tx[7] = 0x%x", rc, buf_tx[7]);

		int i = 0;
		int reSendIndex = 0;

		for (i = 0; i < 200; i++) {
			LOGD("read i = %d", i);
			rc = readDataFromSPI(buf_rx, DEFAULT_BUF_SIZE);

			if (rc > 0) {
				if (buf_rx[0] == 0xBB) {
					if ((buf_rx[1] == 0x01) && (buf_rx[5] == 0x01)) {
						isGetSucess = 1;
						break;
					}
				} else {
					reSendIndex++;
					if (reSendIndex >= 80) {
						writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						LOGD(
								"send buf_tx[0] = %x,%x,%x,%x,%x,%x", buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4], buf_tx[5]);
						reSendIndex = 0;
					}
				}
			} else {
				LOGD("No version data\n");
			}
		}
	} else {
		LOGD("open devices fail");
	}
	closeSPIDevice();
	LOGD("getRadioStatus end");

	 if (isGetSucess == 1) {
		return buf_rx[6];
	} else {
		return -1;
	}
}

char * GetControllerSN(uint8_t whichController)
{
	LOGD("GetControllerSN begin");
    int rc;
    int i = 0;
    unsigned char buf_tx[DEFAULT_BUF_SIZE] = {0,};
    unsigned char buf_rx[DEFAULT_BUF_SIZE] = {0,};

    memset(buf_tx, 0, DEFAULT_BUF_SIZE);
    memset(buf_rx, 0, DEFAULT_BUF_SIZE);

    buf_tx[0] = 0xBB;
    buf_tx[1] = 0x01;
    buf_tx[2] = whichController;
    buf_tx[3] = 0x05;
    buf_tx[4] = whichController;
    buf_tx[7] = crc8(buf_tx, 7);
    int isGetSucess = 0;
	if (openSPIDevice() == 1) {

		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		LOGD("GetControllerSN write rc = %d, buf_tx[7] = 0x%x", rc, buf_tx[7]);

		int reSendIndex = 0;
		for (i = 0; i < 300; i++) {
			LOGD("read i = %d", i);
			usleep(200);
			rc = readDataFromSPI(buf_rx, DEFAULT_BUF_SIZE);
			LOGD(
					"readDataFromSPI buf_tx[0] = %x,%x,%x,%x,%x,%x", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3], buf_rx[4], buf_rx[5]);
			if (rc > 0) {
				if (buf_rx[0] == 0xBB) {
					if ((buf_rx[1] == 0x01) && (buf_rx[5] == 0x01)) {
						isGetSucess = 1;
						break;
					}
				} else {
					reSendIndex++;
					if (reSendIndex >= 80) {
						writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						LOGD(
								"send buf_tx[0] = %x,%x,%x,%x,%x,%x", buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4], buf_tx[5]);
						reSendIndex = 0;
					}
				}
			} else {
				LOGD("No GetControllerSN data\n");
			}
		}
	} else {
		LOGD("open devices fail");
	}
    closeSPIDevice();
    //LOGD("GetStationVersion begin buf_rx=%s",buf_rx);
    LOGD("GetControllerSN end");
    if( isGetSucess == 1)
    {
    	char buff[30] = {0};
    	strncpy(buff, buf_rx+6, 30);
        return buff;
    }else
    {
    	return "Get Controller SN Fail";
    }
}

void setControllerShutdown(uint8_t whichController)
{
	LOGD("setControllerShutdown begin");
    int rc;
    unsigned char buf_tx[DEFAULT_BUF_SIZE] = {0,};

    memset(buf_tx, 0, DEFAULT_BUF_SIZE);

	buf_tx[0] = 0xBB;
	buf_tx[1] = 0x01;
	buf_tx[2] = 0x00;
	buf_tx[3] = 0x06;
	buf_tx[4] = whichController;
	buf_tx[7] = crc8(buf_tx, 7);

	if (openSPIDevice() == 1) {
		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		LOGD("setControllerShutdown write rc = %d, buf_tx[7] = 0x%x", rc, buf_tx[7]);
	} else {
		LOGD("open devices fail");
	}
    closeSPIDevice();
    LOGD("setControllerShutdown end");
}
void setControllerUnbinding(uint8_t whichController)
{
	LOGD("setControllerUnbinding begin");
    int rc;
    unsigned char buf_tx[DEFAULT_BUF_SIZE] = {0,};
    unsigned char buf_rx[DEFAULT_BUF_SIZE] = {0,};

    memset(buf_tx, 0, DEFAULT_BUF_SIZE);
    memset(buf_rx, 0, DEFAULT_BUF_SIZE);

    buf_tx[0] = 0xBB;
    buf_tx[1] = 0x01;
    buf_tx[2] = whichController;
    buf_tx[3] = 0x03;
    buf_tx[4] = whichController;
    buf_tx[7] = crc8(buf_tx, 7);

	if (openSPIDevice() == 1) {
		rc = writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
		LOGD("setControllerUnbinding write rc = %d,  %x,%x,%x,%x,%x", rc, buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4]);
		int erroNum = 0;
		int i=0;
		for (i=0;i<400;i++) {
			usleep(4000);
			rc = readDataFromSPI(buf_rx, DEFAULT_BUF_SIZE);
			LOGI("readDataFromSPI  = %x,%x,%x,%x,%x", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3],buf_rx[4]);
			if(rc > 0)
			{
				if(buf_rx[0] == 0xBB && buf_rx[1] == 0x01&&buf_rx[2] == whichController&&buf_rx[3] == 0x03)
				{
					LOGI("readDataFromSPI2  = %x,%x,%x,%x,%x", buf_rx[0], buf_rx[1], buf_rx[2], buf_rx[3],buf_rx[4]);
					erroNum = 0;
					if(buf_rx[4] == 0x00&&buf_rx[5] == 0x01)
					{
						LOGD("unbindController %d success",whichController);
						break;
					}
					else
					{
						writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						//LOGD("setControllerShutdown write rc = %d,  %x,%x,%x,%x,%x", rc, buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4]);
					}
				}else
				{
					erroNum++;
					if(erroNum > 100)
					{
						writeDataToSPI(buf_tx, DEFAULT_BUF_SIZE);
						//LOGD("setControllerShutdown write rc = %d,  %x,%x,%x,%x,%x", rc, buf_tx[0], buf_tx[1], buf_tx[2], buf_tx[3], buf_tx[4]);
						erroNum = 0;
					}
				}
			}

		}




	} else {
		LOGD("open devices fail");
	}
    closeSPIDevice();
    LOGD("setControllerShutdown end");
}
