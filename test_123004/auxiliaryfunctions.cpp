#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <direct.h>
#include <time.h>

#include "structuredlight.h"
#include "auxiliaryfunctions.h"

using namespace std;
using namespace cv;
//use mv-ub500 sdk api function
#include <process.h>
#include "windows.h"
#pragma comment(lib,"..\\structuredLight\\MVCAMSDK.lib")
#include "CameraApi.h"

CameraSdkStatus camera_sdk_status;	//���״̬
INT  CameraNums = 1;				//����iCameraNums = 1����ʾ���ֻ��ȡ1���豸������ö�ٸ�����豸������sCameraList��С��iCameraNums��ֵ
tSdkCameraDevInfo CameraList[1];	//�豸�б�����ָ��
CameraHandle    m_hCamera;			//��������������ͬʱʹ��ʱ���������������	
tSdkCameraCapbility CameraInfo;		//������������ṹ��
tSdkFrameHead 	FrameInfo;			//ͼ���֡ͷ��Ϣָ��
BYTE*			PbyBuffer;			//ָ��ԭʼͼ�����ݵĻ�����ָ��
BYTE*           FrameBuffer;		//��ԭʼͼ������ת��ΪRGBͼ��Ļ�����

IplImage *iplImage = NULL;

//ע��������������һ�δ�������ָ����ͼ��
int GetImage(Mat &frame_grab)
{
	clock_t clock_begin;
	clock_begin = clock();
	if (CameraGetImageBuffer(m_hCamera, &FrameInfo, &PbyBuffer, 200) == CAMERA_STATUS_SUCCESS)
	{
		////����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
		camera_sdk_status = CameraImageProcess(m_hCamera, PbyBuffer, FrameBuffer, &FrameInfo);//����ģʽ
		if (camera_sdk_status == CAMERA_STATUS_SUCCESS)
		{
			//ת�����ݲ���ʾ
			iplImage = cvCreateImageHeader(cvSize(FrameInfo.iWidth, FrameInfo.iHeight), IPL_DEPTH_8U, 3);
			cvSetData(iplImage, FrameBuffer, FrameInfo.iWidth * 3);
			//cvShowImage("camera", iplImage);
			//Mat frame_temp = Mat(iplImage, true);
			Mat frame_temp = cvarrToMat(iplImage);
			frame_grab=frame_temp.clone();
		}
		//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
		CameraReleaseImageBuffer(m_hCamera, PbyBuffer);
		cvReleaseImageHeader(&iplImage);
		cout << clock() - clock_begin <<" ";
		return -1;
	}
	else
		return 0;
	
}

//Initialize camera
int CameraInitialize(SlParameter &sl_parameter)
{
//	cv::Mat frame_grab;
	//use usb webcam
//	VideoCapture videocapture(sl_parameter.camera_id);
//	if (!videocapture.isOpened())
//	{
//		cerr << "Failed to open camera" << endl;
//		return -1;
//	}
//	cout << "Initialize camera------------------------------------------" << endl;
//	//set properties of the camera
//	videocapture.set(CV_CAP_PROP_FRAME_WIDTH, sl_parameter.camera_width);
//	videocapture.set(CV_CAP_PROP_FRAME_HEIGHT, sl_parameter.camera_height);
//
//	namedWindow("Video in real-time", WINDOW_NORMAL);
//	while (1)
//	{
//		videocapture >> frame_grab;
//		imshow("Video in real-time", frame_grab);
//		waitKey(50);
//#ifdef DEBUG_PROJECT
//		cout << "Camera Properties:" << endl;
//		cout << "camera id: " << sl_parameter.camera_id << endl;
//		cout << "frame rate: " << videocapture.get(CV_CAP_PROP_FPS) << endl;
//		cout << "width: " << videocapture.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
//		cout << "height: " << videocapture.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
//		cout << "brightness: " << videocapture.get(CV_CAP_PROP_BRIGHTNESS) << endl;
//		cout << "contrast: " << videocapture.get(CV_CAP_PROP_CONTRAST) << endl;
//		cout << "saturation: " << videocapture.get(CV_CAP_PROP_SATURATION) << endl;
//		cout << "hue: " << videocapture.get(CV_CAP_PROP_HUE) << endl;
//		cout << "gain: " << videocapture.get(CV_CAP_PROP_GAIN) << endl;
//		cout << "exposure: " << videocapture.get(CV_CAP_PROP_EXPOSURE) << endl;
//#endif
//		cout << "-------------------------------------------------------" << endl << endl;
//	}

	//use industry camera mv ub500
	Mat frame_grab;
	//���SDK��ʼ��
	if ((camera_sdk_status= CameraSdkInit(1)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera sdk init failed: " << camera_sdk_status<<endl;
		return -1;
	}
	//ö���豸������豸�б�
	if ((camera_sdk_status=CameraEnumerateDevice(CameraList, &CameraNums)) != CAMERA_STATUS_SUCCESS || CameraNums == 0)
	{
		cout << "No camera was found: " << camera_sdk_status << endl;
		return -1;
	}
	//��ʼ���豸
	if ((camera_sdk_status  = CameraInit(&CameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		cout << "Camera  init failed: " << camera_sdk_status << endl;
		return -1;
	}
	//��ʼ���������������
	CameraGetCapability(m_hCamera, &CameraInfo);
	FrameBuffer = (BYTE *)malloc(CameraInfo.sResolutionRange.iWidthMax*CameraInfo.sResolutionRange.iWidthMax * 3);
	//����ͼ��ɼ�ģʽ
	CameraPlay(m_hCamera);
	waitKey(2000);			//wait for camera start
	//for (int i = 0; i < 5; i++)
	//{
	//	GetImage(frame_grab);
	//	if (!frame_grab.empty())
	//		imshow("camera initialize", frame_grab);
	//	waitKey(200);
	//}
	GetImage(frame_grab);
	if (!frame_grab.empty())
		cout << "Camera initialize successful......" <<endl<<endl;
	else
		cout << "Camera initializa failed......" << endl<<endl;
//	destroyWindow("camera initialize");
	
	return 0;
}

void CameraClear(void)
{
	CameraUnInit(m_hCamera);
}

//Initialize projector
int ProjectorInitialize(SlParameter &sl_parameter)
{
	//show full white projector background pattern
	sl_parameter.projector_background_pattern = Mat::zeros(sl_parameter.projector_height, sl_parameter.projector_width, CV_8U);
	sl_parameter.projector_background_pattern.setTo(255);
	imshow("projector window", sl_parameter.projector_background_pattern);
	waitKey(1000);
	cout << "projector initialize successful......" << endl << endl;
	return 0;
}






