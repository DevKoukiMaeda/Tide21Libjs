// Tide21Lib.h
//                2006.7 Y.Senta

// ���ĥ��ɥ���� Tide21CV �� C++ �饤�֥�경������Ρ�(�ܺ٤� cpp ��Ƭ������)
// �ʤ����������� �ڡۤǳ���Ƥ����Τϡ����ꥸ�ʥ���ѿ�̾

#ifndef _TIDE21LIB_H_
#define _TIDE21LIB_H_

#include <math.h>

class TideLagrange{	// ��Lagrange()��
	int m_K;
	int m_SG[4];		// ��sg��
	double m_TM[4];		// ��tm��
	double m_T[4];		// ��t��
	double m_FX[4];		// ��fx��
	double m_DF[4];		// ��df��

	double m_lfT;
	double m_lfY;
public:
	TideLagrange(){
		Init();
	}
	~TideLagrange(){}
	void Init();
	bool IsPeek(double, double, bool = true);
	double GetT(){
		return m_lfT;
	}
	double GetY(){
		return m_lfY;
	}
};

class SunLib;
class MoonLib;
class Tide21Lib;
class PlanetLib;

class TideBaseLib{
	friend class PlanetLib;
	friend class SunLib;
	friend class MoonLib;
	friend class Tide21Lib;

	double SerialZ(int, int, int);	// ��serial_z()��

	int m_MoonDay[13];				// ��M��
	void SetMoonDay(int);

	static double Fix(double);		// ��fix()��

	static double m_lfTimeZone;		// ��zt��

	static int Sgn(double);			// ��sgn()��
	static double Rnd(double);		// ��rnd()��
	static double Rnd2(double);		// ��rnd2()��
public:
	TideBaseLib(){}
	~TideBaseLib(){}

	static void SetTimeZone(int);
};

class PlanetLib : public TideBaseLib{
	friend class SunLib;
	friend class MoonLib;

	double m_DC;	// �ְ�?
	double m_RA;	// �ַ�?

	double m_TU;
	double m_Z;		// 1975ǯ1��1������1���Ȥ��������Τ褦��

	double LongSun(double);						// ��long_sun()��
	double GetGrsidTime(double, double);		// ��grsidtime()��
	double GetRisesetHourAngle(double, double); // ��riseset_hourangle()��


public:
	PlanetLib(){}
	virtual ~PlanetLib(){}
};

class SunLib: public PlanetLib {
	void GetCulminant(double);					// ��sun_meripass()��
	void GetSunPosition(double);				// ��sunposition()��
	void GetSunRise(double, double);			// ��sunrise()��
	double GetSunHightTime(double, bool, double, double); // ��subroutine()��

public:
	SunLib(){}
	~SunLib(){}

	double m_lfDawnTwilight;			// ��sun_event[1]�۾������� (ī)
	double m_lfEveningTwilight;			// ��sun_event[5]�۾������� (ͼ)
	double m_lfAstroDawnTwilight;		// ��sun_event[0]��ŷʸ���� (ī)
	double m_lfAstroEveningTwilight;	// ��sun_event[6]��ŷʸ���� (ͼ)
	double m_lfSunRise;					// ��sun_event[2]�����ν�
	double m_lfSunSet;					// ��sun_event[4]��������
	double m_lfCulminant;				// ��sun_event[3]������

	void Calc(int, int, int, double, double);	// ��sun()��
};

class MoonLib: public PlanetLib{
	double m_LM;	// ����?
	double m_HP;	// ��ʿ�뺹?
	double m_SD;	// ��Ⱦ��?

	double LongMoon(double);					// ��long_moon()��
	double LatMoon(double);						// ��lat_moon()��
	void GetCulminant(double);					// ��moon_meripass()��
	void GetMoonRise(double, double);			// ��moonrise()��
	double SinePai(double);						// ��sine_pai()��
	void GetMoonAge(double, double, double);	// ��moonage()��
	double GetSubTime(int, double, double);		// ��moonage_sub()��
	void GetMoonPosition(double);				// ��moon_position()��

	double Fnc(double, double, double);	// ��fnc()��
public:
	MoonLib(){}
	~MoonLib(){}

	double m_lfIlluminated;				// ���
	double m_lfMoonAge;					// ����
	double m_lfMoonRise;				// ���
	double m_lfMoonSet;					// ����
	double m_lfCulminant;				// ����

	int m_nMoonRiseDay;
	int m_nMoonSetDay;
	int m_nCulminantDay;

	double m_IOTA;	// ����
	double m_SMD12;	// ����

	void Calc(int, int, int, double, double);
};

#define C_O1hr (m_lfHR[7])
#define C_K1hr (m_lfHR[13])
#define C_M2hr (m_lfHR[24])
#define C_M2pl (m_lfPL[24])
#define C_S2hr (m_lfHR[28])
#define C_S2pl (m_lfPL[28])

class Tide21Lib : public TideBaseLib{
	char	m_szPortName[1024];	// ��na[0]��
	double	m_lfPortLat;		// ��lat0, na[1]��
	double	m_lfPortLng;		// ��lng0, na[2]��
	double	m_lfPortLevel;		// ��level,na[3]��
	double	m_lfFlood;			// ��flood��
	double	m_lfEbb;			// ��ebb��

	double	m_lfHR[40];			// ��hr��
	double	m_lfPL[40];			// ��pl��

	double m_S, m_H, m_P, m_N;	// ����, ���ۡ���ε�ƻ���ǤΤ褦��
	double m_V[40];				// ����
	double m_F[40];				// ŷʸ����?
	double m_Vl[40];			// ��vl��

	double Dg2Dc(double);
	bool DateCheck(int, int, int);
	int SerialDay(int, int);
	void GetMeanLongitudes(int, int);	// ��mean_longitudes()��

	void ArgumentAndCoeffic();	// ��argument_f0, u0, v1, v2, u1, u2, coeffic_f1, f2��

	static const int m_cNC[40];	// ��nc��
	static const double m_cAgs[40]; // ��ags��
public:
	MoonLib m_Moon;
	SunLib  m_Sun;

	Tide21Lib();
	~Tide21Lib();

	bool LoadTD2(const char *);

	void TideCalcInit(int, int, int);

	double CalcTideHight(int);
	double CalcTideHight(int h, int m){
		return CalcTideHight(h*60+m);
	}
	bool GetTidePeek(double *, double *, double);
	char *GetTideName(double);

	char *GetPortName(){
		return m_szPortName;
	}

	double GetLat(){
		return m_lfPortLat;
	}

	double GetLon(){
		return m_lfPortLng;
	}

	double GetSpringRange(){	// ��Ĭ��
		return 2*(C_M2hr+C_S2hr);
	}
	double GetNeapRange(){		// ��Ĭ��
		return 2*(C_M2hr-C_S2hr);
	}
	double GetSpringRise(){		// ��Ĭ��
		return C_M2hr+C_S2hr+m_lfPortLevel;
	}
	double GetNeapRise(){		// ��Ĭ��
		return C_M2hr-C_S2hr+m_lfPortLevel;
	}
	double GetTideAge(){		// Ĭ��
		return Rnd(C_S2pl-C_M2pl)/24.5;
	}
	double GetMeanRange(){		// ʿ��Ĭ��
		return C_M2hr*2;
	}
	double GetHighestWater(){	// ά�ǹ��Ĭ��
		return C_O1hr + C_K1hr + C_M2hr + C_S2hr + m_lfPortLevel;
	}
	double GetMeanInterval(){	// ʿ�ѹ�Ĭ�ֳ�
		return C_M2pl/29;
	}
	double GetAveHight(){		// ʿ�ѿ���
		return m_lfPortLevel;
	}

	void CalcMoon(int y, int m, int d){
		m_Moon.Calc(y, m, d, m_lfPortLat, m_lfPortLng);
	}
	void CalcSun(int y, int m, int d){
		m_Sun.Calc(y, m, d, m_lfPortLat, m_lfPortLng);
	}
};

#endif
