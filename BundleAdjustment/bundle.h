#ifndef BUNDLE_H_INCLUDED
#define BUNDLE_H_INCLUDED

#include "stdafx.h"
#include "opencv.h"


//�Z���t�L�����u���[�V�������邩�ǂ���
enum selfParam{
	SELF_CALIB_ON = 0,
	SELF_CALIB_OFF = 1
};

//���݃Z���t�L�����u���[�V����
enum selfAlterParam{
	ALTERNATE_ON = 0,
	ALTERNATE_OFF = 1
};

//�ǂ̂悤�Ƀp�����[�^���Œ肷�邩
enum fixParam{
	//7�Œ�CR0,T0�Ƃ��Ƃ������
	FIX_7_AUTO = 0, //��������
	FIX_7_TX = 1, //tx���Œ�
	FIX_7_TY = 2, //tx���Œ�
	FIX_7_TZ = 3, //tx���Œ�
	//6�Œ�CR0,T0
	FIX_6 = 4,
	//�Œ肵�Ȃ��B�p�����[�^��S�ē�����
	FIX_OFF = 5
};

//�J�����̏œ_�����C��_�ʒu���X�V���邩�ǂ���
enum fcParam{
	FC_VARIABLE = 0,
	FC_FIX = 1
};

class Bundle
{
private:
	///private�ϐ�
	Mat& points; //�_�Q(X,Y,Z)
	Mat& imageX; //�摜���Wx
	Mat& imageY; //�摜���Wy
	Mat * R; //��]�s�� 3*3
	Mat * T; //���i�x�N�g�� 3*1
	Mat * K; //�J���������s�� 3*3

	//�_�̑����C�t���[�����C�摜�T�C�Y
	const int pointTotal, frameTotal, width, height;

	//��ԏ�̓_���J�����ɓ��e����ĂȂ��Ƃ��̉摜���W
	//�R���X�g���N�^��-999.�ɏ��������Ă��܂�
	float imageNaN;

	//���[�x���o�[�O�E�}�[�J�[�g�@��c
	//�R���X�g���N�^��0.00001F�ɏ��������Ă��܂�
	float c;

	//�I�������̃C�v�V����
	//�R���X�g���N�^��0.1�ɏ��������Ă��܂�
	float epsilon; 

	//�c�݌W���̐�
	//�R���X�g���N�^��5�ɏ��������Ă��܂�
	//4 : K1,K2,P1,P2
	//5 : K1,K2,K3,P1,P2
	int paramD;

public:
	///public�ϐ�
	selfParam selfFlag; //�Z���t�L�����u���[�V�������邩�ǂ���
	selfAlterParam selfAlterFlag; //���݃Z���t�L�����u���[�V����
	fixParam fixFlag; //�ǂ̂悤�Ƀp�����[�^���Œ肷�邩
	fcParam fcFlag; //�J�����̏œ_�����C��_�ʒu���X�V���邩�ǂ���
	float E, newE; //�ē��e�덷E
	int allSize; //�S�p�����[�^
	int useAllSize; //�X�V����p�����[�^
	int freedom; //���R�x
	int param; //�J�����̃p�����[�^�� 6or10
	int paramK; //�J�����̓����p�����[�^�� 3 or (3+paramD)
	bool cFlag; //�����v�Z��c�̃t���O
	int KPtotal; //�e�t���[���Ɏʂ��Ă�������_�̐������v��������
	Mat Distort; //�c�݌W��
	vector<float> vectorE; //�ē��e�덷E��ۑ�����vector


	///-----�R���X�g���N�^-----
	//private�ϐ��ɁC�Q�Ɠn�����ꂽ�������������܂�
	//�Ăяo�����̃f�[�^�����̂܂܎g���̂ŁC���̃N���X���Œl���ύX�����ƌĂяo�����̃f�[�^�̒l���ύX����܂��B
	Bundle(Mat& _points, Mat& _imageX, Mat& _imageY, Mat * _R, Mat * _T, Mat * _K, int _width, int _height) : points(_points), imageX(_imageX), imageY(_imageY), width(_width), height(_height), imageNaN(-999.), epsilon(0.1F), c(0.0001F), pointTotal(imageX.rows), frameTotal(imageX.cols), paramD(5)
	{
		R = _R;
		T = _T;
		K = _K;

		//�c�݌W���̏�����
		Distort = (Mat_<float>(paramD,frameTotal));
		Distort = Scalar::all(0);

		//������W���璆�S���W�ɕϊ�
		changeCoordToCenter();

		//�œ_������fx,fy����f�ɓ���
		fxfyAverage();

		//KPtotal�̌v�Z
		calcKPtotal();
		
		cout << "Bundle::Constructor" << endl;
		cout << "frameTotal = " << frameTotal << endl;
		cout << "pointTotal = " << pointTotal << endl;
		cout << "KPTotal = " << KPtotal << endl << endl;

	}

	///-----�o���h�������J�n-----
	void start(selfParam _selfFlag=SELF_CALIB_ON, fixParam _fixFlag=FIX_7_AUTO, fcParam _fcFlag=FC_VARIABLE, selfAlterParam _selfAlterFlag=ALTERNATE_ON);

	///-----�ē��e�덷E�̌v�Z-----
	float calcE();

	///-----H,g�̌v�Z-----
	void calcHg(Mat& HE,Mat& HF,Mat& HG,Mat& Gp,Mat& Gf);

	///-----�A���ꎟ�������̌v�Z-----
	void solveHg(Mat& HE,Mat& HF,Mat& HG,Mat& Gp,Mat& Gf,Mat& Dp,Mat& Df);

	///-----�A���ꎟ�������̌v�Z-----
	void solveHg_k(Mat& HG,Mat& Gf,Mat& Dk);

	///-----���̍X�V-----
	void update(Mat& Dp,Mat& Df);

	///-----���̍X�V-----
	void update_6(Mat& Dp,Mat& Df);

	///-----���̍X�V-----
	void update_k(Mat& Dk);

	///-----����߂�-----
	void unupdate(Mat& Dp,Mat& Df);

	///-----����߂�-----
	void unupdate_6(Mat& Dp,Mat& Df);

	///-----����߂�-----
	void unupdate_k(Mat& Dk);

	///-----����̍s�Ɨ���ړ�������-----
	void moveLine(Mat& A,int i_=-1,int i=-1,int j_=-1,int j=-1);

	///-----������W���璆�S���W�ɕϊ�-----
	void changeCoordToCenter();

	///-----���S���W���獶����W�ɕϊ�-----
	void changeCoordToLeftUpper();

	///-----�œ_������fx,fy����f�ɓ���-----
	void fxfyAverage();

	///-----KPtotal�̌v�Z-----
	void calcKPtotal();

	///-----imageNaN�ύX-----
	//imageNaN�̏����l��-999.�ł�
	void set_imageNaN(float _imageNaN)
	{
		cout << "Set imageNaN " << imageNaN;
		imageNaN = _imageNaN;
		cout << " to " << imageNaN << endl << endl;
	}

	///-----c�ύX-----
	//c�̏����l��0.00001F�ł�
	void set_c(float _c)
	{
		cout << "Set c " << c;
		c = _c;
		cout << " to " << c << endl << endl;
	}

	///-----epsilon�ύX-----
	//epsilon�̏����l��0.1�ł�
	void set_epsilon(float _epsilon)
	{
		cout << "Set epsilon " << epsilon;
		epsilon = _epsilon;
		cout << " to " << epsilon << endl << endl;
	}

	///-----paramD�ύX-----
	//paramD�̏����l��5�ł�
	void set_paramD(int _paramD)
	{
		cout << "Set paramD " << paramD;
		paramD = _paramD;
		cout << " to " << paramD << endl << endl;

		//�c�݌W���̏�����
		Distort = (Mat_<float>(paramD,frameTotal));
		Distort = Scalar::all(0);
	}

	///-----Distort�f�[�^����-----
	void set_Distort(Mat& _Distort)
	{
		cout << "Set Distort " << endl;

		Distort = _Distort;
	}

	///-----�c�݌W���̔���-----
	inline float calcddx(float dp, float dq, float dr, float p, float q, float r, float cx, float cy, float K1, float K2, float K3, float P1, float P2, float xc, float yc, float r2, float r4, float r6, int cxFlag, int cyFlag)
	{
		float x_ = (r*dp-p*dr)/(r*r);
		float y_ = (r*dq-q*dr)/(r*r);
		float r2_; 
		if(cxFlag==0 && cyFlag==0){
			r2_ = 2*xc*x_ + 2*yc*y_;
		}else if(cxFlag!=0){
			r2_ = 2*xc*(x_-1) + 2*yc*y_;
		}else if(cyFlag!=0){
			r2_ = 2*xc*x_ + 2*yc*(y_-1);
		}else{
			cout << "Error at dE" << endl;
			exit(1);
		}
		float r4_ = 2*r2*r2_;
		float r6_ = 3*r4*r2_;

		float ddx;
		if(cxFlag==0 && cyFlag==0){
			ddx = (K1*r2+K2*r4+K3*r6)*x_ + (K1*r2_+K2*r4_+K3*r6_)*xc + P1*(r2_+4*xc*x_) + 2*P2*(x_*yc+xc*y_);
		}else if(cxFlag!=0){
			ddx = (K1*r2+K2*r4+K3*r6)*(x_-1) + (K1*r2_+K2*r4_+K3*r6_)*xc + P1*(r2_+4*xc*(x_-1)) + 2*P2*((x_-1)*yc+xc*y_);
		}else if(cyFlag!=0){
			ddx = (K1*r2+K2*r4+K3*r6)*x_ + (K1*r2_+K2*r4_+K3*r6_)*xc + P1*(r2_+4*xc*x_) + 2*P2*(x_*yc+xc*(y_-1));
		}else{
			cout << "Error at dE" << endl;
			exit(1);
		}

		return ddx;
	}

	///-----�c�݌W���̔���-----
	inline float calcddy(float dp, float dq, float dr, float p, float q, float r, float cx, float cy, float K1, float K2, float K3, float P1, float P2, float xc, float yc, float r2, float r4, float r6, int cxFlag, int cyFlag)
	{
		float x_ = (r*dp-p*dr)/(r*r);
		float y_ = (r*dq-q*dr)/(r*r);
		float r2_; 
		if(cxFlag==0 && cyFlag==0){
			r2_ = 2*xc*x_ + 2*yc*y_;
		}else if(cxFlag!=0){
			r2_ = 2*xc*(x_-1) + 2*yc*y_;
		}else if(cyFlag!=0){
			r2_ = 2*xc*x_ + 2*yc*(y_-1);
		}else{
			cout << "Error at dE" << endl;
			exit(1);
		}
		float r4_ = 2*r2*r2_;
		float r6_ = 3*r4*r2_;

		float ddy;
		if(cxFlag==0 && cyFlag==0){
			ddy = (K1*r2+K2*r4+K3*r6)*y_ + (K1*r2_+K2*r4_+K3*r6_)*yc + 2*P1*(x_*yc+xc*y_) + P2*(r2_+4*yc*y_);
		}else if(cxFlag!=0){
			ddy = (K1*r2+K2*r4+K3*r6)*y_ + (K1*r2_+K2*r4_+K3*r6_)*yc + 2*P1*((x_-1)*yc+xc*y_) + P2*(r2_+4*yc*y_);
		}else if(cyFlag!=0){
			ddy = (K1*r2+K2*r4+K3*r6)*(y_-1) + (K1*r2_+K2*r4_+K3*r6_)*yc + 2*P1*(x_*yc+xc*(y_-1)) + P2*(r2_+4*yc*(y_-1));
		}else{
			cout << "Error at dE" << endl;
			exit(1);
		}

		return ddy;
	}
	
	///-----�ꎟ�����̌v�Z-----
	inline float dE(float dp, float dq, float dr, float p, float q, float r, float x0, float y0, float dx=0, float dy=0, float ddx=0, float ddy=0)
	{
		return 2*((p/r+dx-x0)*((r*dp-p*dr)/(r*r)+ddx)+(q/r+dy-y0)*((r*dq-q*dr)/(r*r)+ddy));
	}
	
	///-----�񎟔����̌v�Z-----
	inline float ddE(float dpk, float dqk, float drk, float dpl, float dql, float drl, float p, float q, float r, float ddxk=0, float ddyk=0, float ddxl=0, float ddyl=0)
	{
		return 2*(((r*dpk-p*drk)/(r*r)+ddxk)*((r*dpl-p*drl)/(r*r)+ddxl)+((r*dqk-q*drk)/(r*r)+ddyk)*((r*dql-q*drl)/(r*r)+ddyl));
	}

};

#endif