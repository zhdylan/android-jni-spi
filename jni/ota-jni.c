#include <spi-interface.h>
#include <control.h>


static int ratio = 0;

void Java_com_picovr_cvcontrol_MainActivity_setRatio( JNIEnv* env,
                                                  jobject thiz ,jint num)
{
	ratio = num;
}

void Java_com_picovr_cvcontrol_MainActivity_enterPairMode( JNIEnv* env,
                                                  jobject thiz ,jint whichController)
{
	enterPairMode( whichController);
}

jint Java_com_picovr_cvcontrol_MainActivity_isenterPairMode( JNIEnv* env,
                                                  jobject thiz )
{
	int status = isEnterPairMode();
	LOGI("isEnterPairMode = %d",status);
	return status;
}

void Java_com_picovr_cvcontrol_MainActivity_controllervibrate( JNIEnv* env,
                                                  jobject thiz )
{
	int i=0;
	//for(i=60;i<255;i+=10)
	{
		//setControllervibrateTest();
	    setControllervibrate(1,150,5,1000,1000);
	}
}

void Java_com_picovr_cvcontrol_MainActivity_cancelcontrollervibrate( JNIEnv* env,
                                                  jobject thiz )
{
	stopControllervibrate();
}

jstring Java_com_picovr_cvcontrol_MainActivity_getProcess( JNIEnv* env,
                                                  jobject thiz )
{
	char c[8];
    sprintf(c, "%d", ratio);
    return (*env)->NewStringUTF(env, c);
}

jstring Java_com_picovr_cvcontrol_MainActivity_getDevicesVersion( JNIEnv* env,
                                                  jobject thiz ,jint device)
{
	char buff[60] = {1};
	char * ver = GetVersion(device);//0 :station ,1:controller1, 2:controller2
	strcpy(buff, ver);

    return (*env)->NewStringUTF(env, buff);
}

jstring Java_com_picovr_cvcontrol_MainActivity_getControllerSN( JNIEnv* env,
                                                  jobject thiz )
{
	char buff[60] = {0};
	char * ver = GetControllerSN(1);

    return (*env)->NewStringUTF(env, buff);
}

void Java_com_picovr_cvcontrol_MainActivity_startStationOTAUpdate( JNIEnv* env,
                                                  jobject thiz )
{
	LOGI("ota start_OTA_Update");

	long length = 0;
	int stype = 0;
	uint8_t * dataSouce = loadUpdateData(stype,&length);//0是load station 升级包，1是controller升级包，length是数据长度，返回值是数据
	StartOTAStation(dataSouce,length);

	LOGI("ota end_OTA_Update");
}

void Java_com_picovr_cvcontrol_MainActivity_startControllerOTAUpdate( JNIEnv* env,
                                                  jobject thiz ,jint mode,jint whichController)
{
	LOGI("ota start_Controller_Update");

	long length = 0;

	uint8_t * dataSouce = loadUpdateData(mode,&length);//0是load station 升级包，1是controller升级包，length是数据长度，返回值是数据， 2 stm32 升级包

	StartOTAController(dataSouce,length,mode,whichController);//0x00	null,0x01	升级手柄RF,0x02	升级手柄Stm32
	                                          //0x01	1号手柄,	0x02	2号手柄

	LOGI("ota end_Controller_Update");
}

void Java_com_picovr_cvcontrol_MainActivity_setControllerUnbinding( JNIEnv* env,
                                                  jobject thiz ,jint whichController)
{
	LOGI("start setControllerUnbinding");
	setControllerUnbinding(whichController);
	LOGI("end setControllerUnbinding");
}

void Java_com_picovr_cvcontrol_MainActivity_setControllerShutdown( JNIEnv* env,
                                                  jobject thiz ,jint whichController)
{
	LOGI("start setControllerShutdown");
	setControllerShutdown(whichController);
	LOGI("end setControllerShutdown");
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define BUF_SIZE					131
#define ota_buf_size				32

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static uint32_t mode;
static uint8_t bits = 8;
static uint32_t speed = 4900000;
static const uint16_t delay1 = 1;
static const uint16_t delay2 = 500;

static const uint16_t delay_ota_controllerstart = 2000;
static const uint16_t delay_ota_controller = 6000;

int StartOTAStation(uint8_t * dataSouce,long length)
{
	int i = 0;
	int num = 0;
	int w_count = 1;
	int state = 0;

	uint32_t crc32_result = crc32((uint32_t*)dataSouce,length/4);
	uint16_t rf_code_bytes = length;

	unsigned char update_tx[BUF_SIZE] = {0, };
	unsigned char default_rx[BUF_SIZE] = {0, };

	int Total = ceil((rf_code_bytes*1.0f) /124);

	if (openSPIDevice() == 1) {

		memset(update_tx, 0, BUF_SIZE);
		memset(default_rx, 0, BUF_SIZE);
		update_tx[0] = 0xBB;
		update_tx[1] = 0x02;
		update_tx[2] = 0x01;
		update_tx[3] = 0x00;

		update_tx[4] = (uint8_t)(crc32_result >> 24);
		update_tx[5] = (uint8_t)(crc32_result >> 16);
		update_tx[6] = (uint8_t)(crc32_result >> 8);
		update_tx[7] = (uint8_t) crc32_result;
		update_tx[8] = (uint8_t)(rf_code_bytes >> 8);
		update_tx[9] = (uint8_t)(rf_code_bytes);
		update_tx[128] = crc8(update_tx, 128);

		int result = writeDataToSPI(update_tx, BUF_SIZE);
		LOGD("writeDataToSPI sucess result = %d", result);

		int lastLineIndex = (rf_code_bytes / 124) * 124;
		int reSendIndex = 0;
		int flagFailSend = 0;
		int index = 0;
		while (1) {
			usleep(delay1);

			int result = readDataFromSPI(default_rx, BUF_SIZE);
			//LOGD("readDataFromSPI result = %d",result);
			//LOGI("index =%d,default_rx[0] = %x,%x,%x,%x,%x,%x",index,default_rx[0],default_rx[1],default_rx[2],default_rx[3],default_rx[4],default_rx[5] );

			if ((default_rx[0] == 0xCC) || (flagFailSend == 1)) {
				if ((default_rx[1] == 0x02) || (flagFailSend == 1)) {
					index++;
					reSendIndex = 0;
					if (default_rx[3] == 0x02) {
						usleep(delay2);

						if (writeDataToSPI(update_tx, BUF_SIZE) != -1) {
							LOGD("writeDataToSPI 1 sucess %d", w_count);
							continue;
						} else {
							usleep(delay2);
							writeDataToSPI(update_tx, BUF_SIZE);
						}

					}

					if (state == 1)
						break;
					memset(update_tx, 0, BUF_SIZE);
					update_tx[0] = 0xBB;
					update_tx[1] = 0x02;
					update_tx[2] = 0x02;
					//LOGI("default_rx[0] = %x", default_rx[0]);
					if (num == lastLineIndex) {
						update_tx[3] = inttohex(rf_code_bytes % 124)[0];
					} else {
						update_tx[3] = 0x7C;
					}

					for (i = 4; i < 128 && num < rf_code_bytes; i++, num++) {
						update_tx[i] = dataSouce[num];
					}

					update_tx[128] = crc8(update_tx, 128);
					usleep(delay2);
					int result = writeDataToSPI(update_tx, BUF_SIZE);
					//LOGD("writeDataToSPI 2 result = %d",result);
					w_count++;
					ratio = (w_count * 99) / Total;

					if (num == rf_code_bytes) {
						state = 1;
					}

					LOGI("ratio = %d", ratio);
				}
				memset(default_rx, 0, BUF_SIZE);
				flagFailSend = 0;
			} else {
				reSendIndex++;
				if (reSendIndex > 10) {
					//flagFailSend = 1;
					LOGI("erro default_rx[0] = %x,%x,%x,%x,%x,%x", default_rx[0], default_rx[1], default_rx[2], default_rx[3], default_rx[4], default_rx[5]);
					usleep(delay2);
					int result = writeDataToSPI(update_tx, BUF_SIZE);
					LOGD("writeDataToSPI 3 result = %d",result);
					reSendIndex = 0;
				}

			}
		}

		LOGI("Send LAST CMD");
		memset(update_tx, 0, BUF_SIZE);
		update_tx[0] = 0xBB;
		update_tx[1] = 0x02;
		update_tx[2] = 0x03;
		update_tx[128] = crc8(update_tx, 128);

		writeDataToSPI(update_tx, BUF_SIZE);
		usleep(delay1);
		memset(default_rx, 0, BUF_SIZE);
		reSendIndex = 0;
		while (1) {
			readDataFromSPI(default_rx, BUF_SIZE);

			if (default_rx[0] == 0xCC) {
				if (default_rx[3] == 0x01) {
					LOGI("default_rx[4] = 0x%x,0x%x, 0x%x,0x%x,0x%x", default_rx[0],default_rx[1],default_rx[2],default_rx[3],default_rx[4]);
					if (default_rx[4] == 0x01) {
						LOGI("OTA升级完成");
						ratio = 100;
						break;
					} else {
						LOGI("OTA升级失败,请重新升级");
						ratio = 101;
						break;
					}
				} else {
					usleep(delay2);
					writeDataToSPI(update_tx, BUF_SIZE);
					LOGI("重新发送最后一包");
				}
			} else {
				reSendIndex++;
				if (reSendIndex > 10) {
					usleep(delay2);
					writeDataToSPI(update_tx, BUF_SIZE);
					reSendIndex = 0;
				}
				memset(default_rx, 0, BUF_SIZE);
			}
		}
	} else {
		LOGD("open devices fail");
	}
	free(dataSouce);
	closeSPIDevice();

	return 0;
}

void StartOTAController(uint8_t * dataSouce,long length,uint8_t mode ,uint8_t whichController )
{
	int state = 0;
	uint16_t indexNum = 0;
	int num = 0;
	int i = 0;

	unsigned char update_tx[ota_buf_size] = {0, };
	unsigned char default_rx[ota_buf_size] = {0, };

	uint32_t rf_code_bytes = length;

	int Toatal = ceil((rf_code_bytes*1.0f) /16)+2;
    LOGI("Toatal = %d",Toatal);
	if (openSPIDevice() == 1)
	{
		memset(update_tx, 0, ota_buf_size);
		update_tx[0] = 0x55;
		update_tx[1] = 0x11;
		update_tx[2] = mode;
		update_tx[3] = whichController;

		update_tx[31] = crc8(update_tx, 31);

		LOGI("update_tx start 0 = %x,%x,%x,%x,%x", update_tx[0], update_tx[1], update_tx[2], update_tx[3],update_tx[31]);
		writeDataToSPI(update_tx, ota_buf_size);
		usleep(delay_ota_controllerstart);

	    int isControllerReady = 0;
	    for( i=0 ; i < 1600 ; i++)
	    {
			memset(default_rx, 0, ota_buf_size);
			readDataFromSPI(default_rx, ota_buf_size);

			LOGI("start readDataFromSPI   1 = %x,%x,%x,%x,%x", default_rx[0], default_rx[1], default_rx[2], default_rx[3],default_rx[4]);
			if ((default_rx[0] == 0x55) && (default_rx[1] == 0x11)&& (default_rx[2] == mode))
			{
				LOGI("isController  enter");
				if ((default_rx[3] == 0x01) && (default_rx[4] == whichController))//加座位号
				{
					ratio = 1;
					isControllerReady = 1;
					LOGI("isController %d Ready= %d", whichController,isControllerReady);
					break;

				} else {
					LOGI("update_tx start 2 = %x,%x,%x,%x,%x", update_tx[0], update_tx[1], update_tx[2], update_tx[3],update_tx[31]);
					writeDataToSPI(update_tx, ota_buf_size);
					usleep(delay_ota_controllerstart);
				}
			}else
			{
				LOGI("update_tx start 1 = %x,%x,%x,%x,%x", update_tx[0], update_tx[1], update_tx[2], update_tx[3],update_tx[31]);
				writeDataToSPI(update_tx, ota_buf_size);
				usleep(delay_ota_controllerstart);
			}
	    }
		if (isControllerReady == 1)
		{
			memset(update_tx, 0, ota_buf_size);
			update_tx[0] = 0x56;
			update_tx[1] = (uint8_t)(indexNum >> 8);
			update_tx[2] = (uint8_t)(indexNum);

			for (i = 3; i < 19 && num < rf_code_bytes; i++, num++) {
				update_tx[i] = dataSouce[num];
				//LOGI("DATA %d = 0x%x",num,dataSouce[num]);
			}
			update_tx[31] = crc8(update_tx, 31);
			writeDataToSPI(update_tx, ota_buf_size);
			LOGI("update_tx = %x,%x,%x,%x",update_tx[0],update_tx[1],update_tx[2],update_tx[31]);

			while (1)
			{
				usleep(delay_ota_controller);
				memset(default_rx, 0, ota_buf_size);
				readDataFromSPI(default_rx, ota_buf_size);
				LOGI("default_rx2 = %x,%x,%x,%x",default_rx[0],default_rx[1],default_rx[2],default_rx[3]);
				if ((default_rx[0] == 0x56)
						&& (default_rx[1] == (uint8_t)(indexNum >> 8))
						&& (default_rx[2] == (uint8_t)(indexNum))) {
					if (default_rx[3] == 0x01) {
						indexNum++;
						if (num >= rf_code_bytes) {
							break;
						}
						ratio = (indexNum * 1.0f) * 99 / Toatal;
						memset(update_tx, 0, ota_buf_size);
						update_tx[0] = 0x56;
						update_tx[1] = (uint8_t)(indexNum >> 8);
						update_tx[2] = (uint8_t)(indexNum);

						for (i = 3; i < 19 && num < rf_code_bytes; i++, num++) {
							update_tx[i] = dataSouce[num];
							//LOGI("DATA %d = 0x%x", num, dataSouce[num]);
						}
						LOGI("num = %d", num);
						update_tx[31] = crc8(update_tx, 31);

						writeDataToSPI(update_tx, ota_buf_size);
						LOGI("update_tx = %x,%x,%x,%x,indexNum=%x,%d,Toatal = %d", update_tx[0], update_tx[1], update_tx[2], update_tx[31], indexNum,indexNum,Toatal);
					} else {
						usleep(delay_ota_controller);
						writeDataToSPI(update_tx, ota_buf_size);
						LOGI("update_tx  1= %x,%x,%x,%x", update_tx[0], update_tx[1], update_tx[2], update_tx[31]);
					}

				} else {

					if (default_rx[31] == crc8(default_rx, 31)&&default_rx[0] == 0x56)
					{
						num = ((default_rx[1] << 8) | (default_rx[2])) * 16;
						memset(update_tx, 0, ota_buf_size);
						update_tx[0] = 0x56;
						update_tx[1] = default_rx[1];
						update_tx[2] = default_rx[2];

						for (i = 3; i < 19 && num < rf_code_bytes; i++, num++) {
							update_tx[i] = dataSouce[num];
							//LOGI("DATA %d = 0x%x", num, dataSouce[num]);
						}
						update_tx[31] = crc8(update_tx, 31);
						LOGI("num = %d", num);
						indexNum = (default_rx[1] << 8) | (default_rx[2]);
						writeDataToSPI(update_tx, ota_buf_size);
						LOGI("update_tx 2 = %x,%x,%x,%x,indexNum=%d", update_tx[0], update_tx[1], update_tx[2], update_tx[31], indexNum);
					}else
					{
						writeDataToSPI(update_tx, ota_buf_size);
					}

				}
			}

			uint16_t crc16_result = crc16(dataSouce,length,NULL);
			memset(update_tx, 0, ota_buf_size);
			update_tx[0] = 0x55;
			update_tx[1] = 0x22;
			update_tx[2] = (uint8_t)(crc16_result >> 8);
			update_tx[3] = (uint8_t) crc16_result;
			update_tx[4] = (uint8_t)(rf_code_bytes >> 24);
			update_tx[5] = (uint8_t)(rf_code_bytes >> 16);
			update_tx[6] = (uint8_t)(rf_code_bytes >> 8);
			update_tx[7] = (uint8_t)(rf_code_bytes);
			update_tx[31] = crc8(update_tx, 31);
			//LOGI("end writeDataToSPI = %x,%x,%x,%x,%x", default_rx[0], default_rx[1], default_rx[2], default_rx[3], default_rx[4]);
			writeDataToSPI(update_tx, ota_buf_size);
			usleep(delay_ota_controller);

			for (i = 0; i < 375; i++) {
				memset(default_rx, 0, ota_buf_size);
				readDataFromSPI(default_rx, ota_buf_size);

				LOGI("end readDataFromSPI = %x,%x,%x,%x,%x", default_rx[0], default_rx[1], default_rx[2], default_rx[3], default_rx[4]);
				if ((default_rx[0] == 0x55) && (default_rx[1] == 0x22)
						&& (default_rx[2] == 0x01)) {
					    ratio = 100;
						break;
				} else {
					LOGI("update_tx start 1 = %x,%x,%x,%x,%x", update_tx[0], update_tx[1], update_tx[2], update_tx[3], update_tx[31]);
					writeDataToSPI(update_tx, ota_buf_size);
					usleep(delay_ota_controllerstart);
				}
			}
			if(ratio!=100)
			{
				ratio = 101;
			}
		}else
		{
			ratio = 102;
		}
	} else {
		LOGD("open devices fail");
	}

	free(dataSouce);
	closeSPIDevice();

}

