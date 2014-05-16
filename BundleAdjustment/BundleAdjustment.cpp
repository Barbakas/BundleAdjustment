// BundleAdjustment.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"
#include "opencv.h" //OpenCV���C�u����
#include "file.h" //�t�@�C���������֐��Q
#include "bundle.h" //�o���h������


////////////////////////
// �O���[�o���ϐ��錾
//
selfParam selfFlag = SELF_CALIB_ON;
selfAlterParam selfAlterFlag = ALTERNATE_OFF;
fixParam fixFlag = FIX_7_AUTO;
fcParam fcFlag = FC_FIX;
float c = 0.0001F, epsilon = 0.01F;
const int FRAMETOTAL = 5; //�t���[����
const int BNDL = FRAMETOTAL - 1; //�o���h�����i�t���[���� - 1�j
const int WID = 128, HEI = 128; //�摜�T�C�Y
const char filedir[] = "data"; //�f�[�^��u���f�B���N�g����
const char currentdir[] = "sample"; //���̒��Ŏg�p����f�B���N�g���� - �ŏI�I�� ./filedir/currentdir/hoge.txt �̂悤�Ȍ`�ɂȂ�

const char corrfilename[] = "allbundlepoints.csv"; //�����_���̓f�[�^
const char rtfilename[] = "rt.xml"; //���i�E��]�x�N�g�����̓f�[�^
const char camerafilename[] = "camera.xml"; //�J���������s����̓f�[�^
const char corroutfilename[] = "result_xyz"; //�����_�o�̓f�[�^
const char rtoutfilename[] = "result_rt.xml"; //���i�E��]�x�N�g���o��
const char Routfilename[] = "result_R.xml"; //���i�E��]�x�N�g���o��
const char Toutfilename[] = "result_T.xml"; //���i�E��]�x�N�g���o��
const char cameraoutfilename[] = "result_camera.xml"; //�J���������s��o��
const char distortoutfilename[] = "result_distort.txt"; //�J�����c�݌n���o��
int KPtotal; //�����_�̎��ۂ̐��uKP = Key Point = �����_�v
int KP2Dtotal; //�����_���t���[�����Ɏʂ��Ă��鐔�����v��������
float fx, fy, cx, cy; //�L�����u���[�V�����ŋ��܂����J���������p�����[�^
//double PI = 3.14159265358979;
//int fps = 15; 

////�N�C�b�N�\�[�g��r�֐�
int comp( const void *c1, const void *c2 );


//////////////////
// main�֐�
//
int _tmain(int argc, _TCHAR* argv[])
{
	cout << "-------------------------" << endl;
	cout << "    Bundle Adjustment    " << endl;
	cout << "-------------------------" << endl;
	clock_t start_time_total,end_time_total;
	start_time_total = clock(); //���s���Ԍv��


	///---�����_���̓f�[�^�ǂݍ���---
	char filename[255];
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,corrfilename);
	//cout << "�����_���̓f�[�^" << filename << " �ǂݍ��݂܂�" << endl;
	//�t�@�C���̍s��(=�����_�̐�)�擾����KPtotal�ɑ��
	KPtotal = readfileLine(filename); //readfileLine�֐� - file.cpp�Q��
	//cout << "�����@�����_�̐� = " << KPtotal << "��" << endl << endl;
	//�����_�����Mat �摜���W�����Mat
	Mat points = Mat::zeros(KPtotal,3,CV_32F);
	Mat imageX = Mat::zeros(KPtotal,FRAMETOTAL,CV_32F);
	Mat imageY = Mat::zeros(KPtotal,FRAMETOTAL,CV_32F);
	//�t�@�C���ǂݍ����Mat�ɑ��
	readfilePoints(filename, points, imageX, imageY);
	
	//�\����CorrPoint�C���X�^���X�� - �\���̂̐錾��file.h�ɏ����Ă���܂�
	CorrPoint * corrpoint;
	corrpoint = (CorrPoint *)malloc(sizeof(CorrPoint) * KPtotal);
	initCorrPoint(corrpoint); //corrpoint�̏����� - file.cpp�Q��
	//�t�@�C���ǂݍ����corrpoint�ɑ��
	readfileCorr(filename, corrpoint); //readfileCorr�֐� - file.cpp�Q��
	


	///---�J�������i�E��]�x�N�g���ǂݍ���---
	/////ICPnormalMultiData��rt.xml���o�͂����ꍇ�CRT�s��̌`�ɒ��ӂ��邱�ƁI�ʏ��4*4RT�s��̌`�Ƃ͈Ⴄ�B
	/////��ԉ��̍s��[0,0,0,1]�ł͂Ȃ�[tx,ty,tz,1]�ɂȂ��Ă���B
	Mat rt[FRAMETOTAL]; //RT4*4�s������Mat�쐬
	for(int i=0;i<FRAMETOTAL;i++) rt[i] = Mat_<float>(4,4); //4*4�̃������m��
	//rt.xml�t�@�C���ǂݍ����Mat�ɑ��
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,rtfilename);
	//cout << "���i�E��]�f�[�^" << filename << " �ǂݍ��݂܂�" << endl;
	readfileRT(filename, rt); //readfileRT�֐� - file.cpp�Q��
	//cout << "����" << endl << endl;


	///---�J���������s��(fx,fy,cx,cy)�ǂݍ���---
	////OpenCV�̃J�����L�����u���[�V�����ŋ��܂�(cx,cy)�͉摜���㒆�S�ł����C
	////���̃v���O�����ł́C(cx,cy)�͉摜���S����̃Y����\���܂��B
	////������ŁC���S����̃Y���ɕϊ����Ă��܂�
	////�����ł̓f�[�^��ǂݍ��ނ����ł�
	Mat cameraA(3,3,CV_32F); //�J���������s�� 3*3�s�� [fx, 0, cx; 0, fy, cy; 0, 0, 1]
	//camera.xml�t�@�C���ǂݍ����Mat�ɑ��
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,camerafilename);
	//cout << "�J���������s��f�[�^" << filename << " �ǂݍ��݂܂�" << endl;
	readfileCamera(filename, &cameraA); //readfileCamera�֐� - file.cpp�Q��
	//cout << "����" << endl << endl;


	///---Bundle�ɓn�����߂Ƀf�[�^���`---
	Mat R[FRAMETOTAL]; //R3*3�s������Mat�쐬
	Mat T[FRAMETOTAL]; //R3*1�s������Mat�쐬
	Mat K[FRAMETOTAL]; //K3*3�s������Mat�쐬
	for(int i=0;i<FRAMETOTAL;i++){
		R[i] = Mat_<float>(3,3); //3*3�̃������m��
		R[i] = rt[i](Range(0,3),Range(0,3));
		T[i] = Mat_<float>(3,1); //3*1�̃������m��
		T[i] = rt[i](Range(3,4),Range(0,3)).t();
		K[i] = Mat_<float>(3,3); //3*3�̃������m��
		cameraA.copyTo(K[i]); //�[���R�s�[
	}


	///---�o���h������---
	//cout << endl;
	//cout << "------------------------------" << endl;
	//cout << "   �o���h���������J�n���܂�   " << endl;
	//cout << "------------------------------" << endl;
	clock_t start_time_bundle,end_time_bundle;
	start_time_bundle = clock(); //���s���Ԍv��


	///---�N���XBundle�̃C���X�^���X��---
	Bundle bundle(points, imageX, imageY, R, T, K, WID, HEI);
	///---�����ݒ�---
	//bundle.set_imageNaN(-1.); //�J�����ɓ��e����ĂȂ��Ƃ��̉摜���W
	bundle.set_c(c);     //���[�x���o�[�O�E�}�[�J�[�g�@��c
	bundle.set_epsilon(epsilon);    //�I�������̃C�v�V����
	///---�o���h������start---
	bundle.start(selfFlag, fixFlag, fcFlag, selfAlterFlag);
	

	end_time_bundle = clock(); //���s���Ԍv���I��
	cout << "�o���h���������s���� = " << (float)(end_time_bundle - start_time_bundle)/CLOCKS_PER_SEC << "�b" << endl << endl;

	
	///---�_�Q(x,y,z)�̏o��---
	///(x,y,z)��csv�t�@�C��
	sprintf_s(filename, _countof(filename), "./%s/%s/%s.csv", filedir, currentdir,corroutfilename);
	writefilePoints(filename, points, imageX.rows); //writefile�֐� - file.cpp�Q��
	///PointCloudLibrary�̓_�Q�f�[�^�`��.pcd
	//z�̒l�ɂ���ā@�߁̉��@�΁̐ԁ@�ƐF�w�肷��
	//z�̍ő�l�ƍŏ��l�����߂�
	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(points(Range(0,points.rows),Range(2,3)), &minVal, &maxVal, &minLoc, &maxLoc);
	//�t�@�C����������
	sprintf_s(filename, _countof(filename), "./%s/%s/%s.pcd", filedir, currentdir,corroutfilename);
	writefilePoints(filename, points, imageX.rows, imageX.cols, (float)minVal, (float)maxVal, R, T); //writefile�֐� - file.cpp�Q��
	

	///---��]R�̏o��---
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,Routfilename);
	writefileMat(filename, R); //writefileMat�֐� - file.cpp�Q��
	

	///---���iT�̏o��---
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,Toutfilename);
	writefileMat(filename, T); //writefileMat�֐� - file.cpp�Q��

	
	///---�J���������s��(fx,fy,cx,cy)�̏o��---
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,cameraoutfilename);
	writefileMat(filename, K); //writefileMat�֐� - file.cpp�Q��

	
	///---�J�����c�݌W��(K1,K2,P1,P2,K3)�̏o��---
	sprintf_s(filename, _countof(filename), "./%s/%s/%s", filedir, currentdir,distortoutfilename);
	writefileDistort(filename, bundle.Distort); //writefileMat�֐� - file.cpp�Q��

	
	///---�ē��e�덷vectorE��������---
	FILE * fp;
	sprintf_s(filename, _countof(filename), "%s/%s/result_E.csv",filedir,currentdir);
	errno_t error;
	error = fopen_s(&fp, filename, "w");
	if(error != 0){
		cout << "�t�@�C�����J���܂��� " << filename << endl;
		exit(1);
	}
	for(int i=0;i<bundle.vectorE.size();i++){
		fprintf(fp,"%f\n",bundle.vectorE[i]);
	}
	fclose(fp);
	
	
	///---�v���O�����I������---
	cout << endl;
	cout << "--------------" << endl;
	cout << "    Finish    " << endl;
	cout << "--------------" << endl;
	end_time_total = clock(); //���s���Ԍv���I��
	cout << "�v���O�������s���� = " << (float)(end_time_total - start_time_total)/CLOCKS_PER_SEC << "�b" << endl << endl;


	return 0;
}



///////////////////////////////
// �N�C�b�N�\�[�g�p��r�֐�
//
int comp( const void *c1, const void *c2 )
{
	CorrPoint point1 = *(CorrPoint *)c1;
	CorrPoint point2 = *(CorrPoint *)c2;

	float tmp1 = point1.z;   /* z ����Ƃ��� */
	float tmp2 = point2.z;

	if(tmp1 == tmp2)       return 0;
	else if(tmp1 > tmp2)   return 1;
	else					 return -1;
}