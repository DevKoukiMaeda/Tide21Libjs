// Tide21Lib.cpp
//                2006.7 Y.Senta

// Original C source (Tide21cv for DOS) by
//  NIFTY-Serve Stormy Petrel [MHB00073] / PC-VAN カツオドリ [PCA95337] 1992.10 - 1996.1 
//        (Now, You can get it on http://www.marubishi.co.jp/fish/download.htm)
// This code is translated for C++ Library by Y.Senta 2006.7

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tide21Lib.h"

#define  RAD   0.0174532925199433	// 【dr】
#define  DEG   57.29577951308232	// 【rd】

/////////////////////////////////////////////////////////////////
void TideLagrange::Init(){
	for (int i=0;i<4;i++){
		m_SG[i]=0;
		m_TM[i]=m_T[i]=m_FX[i]=m_DF[i]=0;
	}
}

bool TideLagrange::IsPeek(double T, double y, bool bK){
	m_TM[0]=m_TM[1]; m_TM[1]=m_TM[2]; m_TM[2]=m_TM[3]; m_TM[3]=T;
	m_FX[0]=m_FX[1]; m_FX[1]=m_FX[2]; m_FX[2]=m_FX[3]; m_FX[3]=y;

	m_DF[0]=m_DF[1]; m_DF[1]=m_DF[2]; m_DF[2]=m_FX[3]-m_FX[2];
	m_SG[0]=m_SG[1]; m_SG[1]=m_SG[2]; m_SG[2]=m_DF[2] >0. ? 1 : -1;

	if(bK && m_SG[0]!=m_SG[1])	{
		double itv=m_TM[3]-m_TM[2];
		m_T[0]=m_TM[0]+itv/2; m_T[1]=m_TM[1]+itv/2; m_T[2]=m_TM[2]+itv/2;
		double FNN  = -m_DF[0]/(m_DF[1]-m_DF[0]);
		m_lfT = (1-FNN)*(2-FNN)*m_T[0]/2 + FNN*(2-FNN)*m_T[1] - (1-FNN)*FNN*m_T[2]/2;
		if(m_lfT > 0) {
			FNN  = (m_lfT-m_TM[1])/(m_TM[2]-m_TM[1]);
			m_lfY = (1-FNN)*(2-FNN)*m_FX[1]/2 + FNN*(2-FNN)*m_FX[2] - (1-FNN)*FNN*m_FX[3]/2;
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////
double TideBaseLib::m_lfTimeZone=0;

void TideBaseLib::SetMoonDay(int yr){
	int days[13]={31,31,28,31,30,31,30,31,31,30,31,30,31};
	for (int i=0;i<13;i++)
		m_MoonDay[i]=days[i];

	if (!(yr%4)){
		if (!(yr%100) || yr%400)
			m_MoonDay[2] = 29;
	}
}

double TideBaseLib::Fix(double x){
	if (x>=0.0) 
		return floor(fabs(x));
       else        
		return -floor(fabs(x)); 
}

double TideBaseLib::Rnd(double x){ 
	return  x-floor(x/360.0)*360.0;
}

int TideBaseLib::Sgn(double x){ 
	return x>0. ? 1 : -1; 
}

double TideBaseLib::Rnd2(double x){ 
	return x-Fix((x+Sgn(x)*180.0)/360.0)*360.0;
}

double TideBaseLib::SerialZ(int yr, int mh, int dy) {
	SetMoonDay(yr);

	int   b, YK, XM;
	double a, c, z;

	if(mh > 2){
		YK = yr, XM = mh;
	}else{
		YK = yr - 1, XM = mh + 12;
	}

	a = Fix(YK / 100);
	b = (int)( 2.0 - a + Fix(a / 4) );
	c = Fix(365.25 * YK);
	z = Fix(30.6001 * (XM + 1)) + b + c + dy;

	z = z -730550.5; 
	return z;
}

void TideBaseLib::SetTimeZone(int tz){
	m_lfTimeZone=tz;
}

/////////////////////////////////////////////////////////////////

//! グリニッチ恒星時を求める
double PlanetLib::GetGrsidTime(double t, double tj){
	double tg;
	tg = 100.4604 + 36000.7695 * t + 360.0 * tj;
	tg = Rnd(tg);
	return tg;
}

double PlanetLib::LongSun(double t) {
	double g = Rnd(36000.77 * t) + 357.53;
	return Rnd(g - (77.06 - 1.91 * sin(g*RAD)));
}

double PlanetLib::GetRisesetHourAngle(double x, double la){
	double arg = (sin(x*RAD)-sin(la*RAD)*sin(m_DC*RAD))/cos(la*RAD)/cos(m_DC*RAD);

	if(arg<-1.0 || 1.0<arg){
		return 360.0;
	}else{
		return acos(arg)*DEG;
	}
}

/////////////////////////////////////////////////////////////////
void SunLib::GetSunPosition(double t) {
	double ls = LongSun(t);
	
	double p = 23.44;
	
	double u = cos(ls * RAD);
	double v = sin(ls * RAD) * cos(p * RAD);
	double w = sin(ls * RAD) * sin(p * RAD);
	m_RA = Rnd(atan2(v, u)*DEG);
	m_DC = atan(w/sqrt(u*u+v*v))*DEG;
}

void SunLib::GetCulminant(double lo){
	double tu = 180.0 - lo;

	for(int i=1;i<3;i++){
		double tj = tu / 360.0;
		double t = (m_Z + tj) / 36525.0;

		GetSunPosition(t);
		
		double tg = GetGrsidTime(t, tj);   
		double hg = Rnd(tg - m_RA);       
		double lha = hg + lo;           
		if(lha > 180.0)
			lha = lha - 360.0;
		if(lha < -180.0)
			lha = lha + 360.0;
		tu = tu - lha;
	}
	m_lfCulminant=tu;
}

void SunLib::GetSunRise(double la, double lo){
	m_TU = m_lfCulminant;
	m_lfAstroDawnTwilight = GetSunHightTime(-18., true, la, lo);
	m_lfDawnTwilight = GetSunHightTime(-6., true, la, lo);
	m_lfSunRise = GetSunHightTime(-54.2 / 60.0, true, la, lo);
	
	m_TU = m_lfCulminant;
	m_lfAstroEveningTwilight = GetSunHightTime(-18., false, la, lo);
	m_lfEveningTwilight = GetSunHightTime(-6., false, la, lo);
	m_lfSunSet = GetSunHightTime(-54.2 / 60.0, false, la, lo);
}

double SunLib::GetSunHightTime(double als, bool b, double la, double lo){
	double tu=m_TU;
	double sg = b ? -1 : 1;

	for(int i=1;i<3;i++){
		double tj = tu / 360.0;
		double t  = (m_Z + tj) / 36525.0;
	
		GetSunPosition(t);

		double lha = Rnd(GetGrsidTime(t, tj) - m_RA) + lo;

		if( lha >  180.0)
			lha -= 360.0;
		if( lha < -180.0)
			lha += 360.0;

		double hr = GetRisesetHourAngle(als, la);

		if(hr==360.0)
			return 360.0;

		tu += hr * sg - lha;
	}

	if (tu != 360)
		m_TU=tu;

	return tu;
}

void SunLib::Calc(int yr, int mh, int dy, double la, double lo){
	m_Z =  SerialZ(yr, mh, dy);
	
	GetCulminant(lo);
	GetSunRise(la, lo);
	
	if(m_lfDawnTwilight != 360.0 )
		m_lfDawnTwilight=(m_lfDawnTwilight+m_lfTimeZone)/15;
	if(m_lfEveningTwilight != 360.0 )
		m_lfEveningTwilight=(m_lfEveningTwilight+m_lfTimeZone)/15;
	if(m_lfAstroDawnTwilight != 360.0 )
		m_lfAstroDawnTwilight=(m_lfAstroDawnTwilight+m_lfTimeZone)/15;
	if(m_lfAstroEveningTwilight != 360.0 )
		m_lfAstroEveningTwilight=(m_lfAstroEveningTwilight+m_lfTimeZone)/15;
	if(m_lfSunRise != 360.0 )
		m_lfSunRise=(m_lfSunRise+m_lfTimeZone)/15;
	if(m_lfSunSet != 360.0 )
		m_lfSunSet=(m_lfSunSet+m_lfTimeZone)/15;
	if(m_lfCulminant != 360.0 )
		m_lfCulminant=(m_lfCulminant+m_lfTimeZone)/15;
}

/////////////////////////////////////////////////////////////////
double MoonLib::Fnc(double b, double t, double c){
	double arg = b * t + c ;
	arg = arg - floor(arg/360.0)*360.0;
	return cos(arg * RAD);
}

double MoonLib::LongMoon(double t){
	double lm = 218.316;
	double arg= 481267.8809 * t;

	arg -= floor(arg/360.0)*360.0;
	lm += arg - .00133 * t * t;

	lm += 6.2888 * Fnc( 477198.868, t, 44.963 );
	lm += 1.274  * Fnc( 413335.35 , t, 10.74 );
    lm += 0.6583 * Fnc( 890534.22 , t,145.7  );
    lm += 0.2136 * Fnc( 954397.74 , t,179.93 );
    lm += 0.1851 * Fnc(  35999.05 , t, 87.53 );
    lm += 0.1144 * Fnc( 966404.0  , t,276.5  );
    lm += 0.0588 * Fnc(  63863.5  , t,124.2  );
    lm += 0.0571 * Fnc( 377336.3  , t, 13.2  );
    lm += 0.0533 * Fnc(1367733.1  , t,280.7  );
    lm += 0.0458 * Fnc( 854535.2  , t,148.2  );
    lm += 0.0409 * Fnc( 441199.8  , t, 47.4  );
    lm += 0.0347 * Fnc( 445267.1  , t, 27.9  );
    lm += 0.0304 * Fnc( 513197.9  , t,222.5  );

    return Rnd(lm);
}

double MoonLib::LatMoon(double t){
	double bt = 5.1281 * Fnc(483202.019, t, 3.273);
	bt += .2806 * Fnc( 960400.89, t, 138.24);
	bt += .2777 * Fnc(   6003.15, t,  48.31);
	bt += .1733 * Fnc( 407332.2 , t,  52.43);
	return bt;
}

double MoonLib::SinePai(double t){
	return .950725 + .05182  * Fnc( 477198.868 , t,  134.963);
}

void MoonLib::GetMoonPosition(double t) {
	m_LM = LongMoon(t);
	double bt = LatMoon(t);
	m_HP = asin(SinePai(t) * RAD)*DEG*60;
	m_SD = .2725 * m_HP;
	
	double p = 23.43928 - 0.01300417*t;

	double c_bt=cos(bt*RAD);
	double s_bt=sin(bt*RAD);
	double s_p =sin(p*RAD);
	double c_p =cos(p*RAD);
	double s_lm=sin(m_LM*RAD);

	double u = c_bt * cos(m_LM*RAD);
	double v = c_bt * s_lm * c_p - s_bt * s_p;
	double w = c_bt * s_lm * s_p + s_bt * c_p;
	m_RA = Rnd(atan2(v, u)*DEG);
	m_DC = atan(w / sqrt(u*u+v*v))*DEG;
}


void MoonLib::GetCulminant(double lo){
	bool flg=true;
	double tu2 = 0;
	
	double tu = 165.0 - lo;
	double LST=0;

	for (int j=0;;) {
		double DT = 360.0;
	
		while( fabs(DT) > .1) {
			j++;
			double TJ = tu / 360.0;
			double t  = (m_Z + TJ) / 36525.0;
			double tg = GetGrsidTime(t, TJ);

			GetMoonPosition(t);

			double LHA = Rnd2(tg - m_RA + lo);
			tu -= LHA;
			LST = tu + m_lfTimeZone;
			DT = tu - tu2;
			tu2 = tu;
		}
	
		m_lfCulminant = tu;

		if (flg && LST < 0){
			tu += 360.0;
			flg = false;
		} else {
			break;
		}
	}
}

void MoonLib::GetMoonRise(double la, double lo) {
	double TU = m_lfCulminant;
	for(int j=1; j<4; j++){
		double TJ = TU / 360.0;
		
		double t = (m_Z + TJ) / 36525.0;
		GetMoonPosition(t);
		
		double LHA = Rnd2(GetGrsidTime(t, TJ) - m_RA + lo);
		double HR = GetRisesetHourAngle((-34.0 - m_SD + m_HP) / 60.0, la);
		if(HR == 360.0){
			m_lfMoonRise=360.0;
			break;
		}else{
			TU = -HR + TU - LHA;
			m_lfMoonRise=TU;
		}
	}

	TU = m_lfCulminant; 
	for(int j=1; j<4; j++){
		double TJ = TU / 360.0;
		double t = (m_Z + TJ) / 36525.0;

		GetMoonPosition(t);
		double LHA = Rnd2(GetGrsidTime(t, TJ) - m_RA + lo);
		double HR = GetRisesetHourAngle((-34.0 - m_SD + m_HP) / 60.0, la);
	   
		if(HR == 360.0){
			m_lfMoonSet = 360.0;
			break;
		}else{
			TU =  HR + TU - LHA;
			m_lfMoonSet = TU;
		}
	}
}

double MoonLib::GetSubTime(int incl, double td, double smd){
	double t;
	for(int j=1; j<=incl; j++){
		t = td / 36525.0;
		td -= 29.5305 * Rnd2(LongMoon(t) - LongSun(t) - smd) / 360;
	}
	return td;
}

void MoonLib::GetMoonAge(double td12, double lm12, double ls12){
	double age = lm12 - ls12;
	
	if(age < 0) 
		age += 360;
	
	double td = td12 - 29.5305 * age / 360;
	td = GetSubTime(3, td, 0.);
	
	m_lfMoonAge = td12 - td;
}


void MoonLib::Calc(int yr, int mh, int dy, double la, double lo){
	m_Z =  SerialZ(yr, mh, dy);
	double tj = (180.0-m_lfTimeZone)/360.0;

	double td12=m_Z+tj;
	double t = td12/36525.0;

	double ls12 =LongSun(t);
	double lm12 =LongMoon(t);
	double bt12 =LatMoon(t);

	GetCulminant(lo);
	GetMoonRise(la,lo);
	GetMoonAge(td12, lm12, ls12);
	
	m_lfCulminant=(m_lfCulminant+m_lfTimeZone)/15.0;

	m_nMoonRiseDay=m_nMoonSetDay=m_nCulminantDay=dy;

	if(m_lfCulminant> 24){
		m_nCulminantDay++;
		m_lfCulminant -= 24;
		if(m_nCulminantDay > m_MoonDay[mh]) 
			m_nCulminantDay = 1;
	}

	if(m_lfMoonRise != 360.0 ){
		m_lfMoonRise =(m_lfMoonRise +m_lfTimeZone)/15.0;
		m_lfMoonSet =(m_lfMoonSet +m_lfTimeZone)/15.0;
	
		if(m_lfMoonRise< 0){
			m_nMoonRiseDay--;
			m_lfMoonRise += 24;
			if(m_nMoonRiseDay < 1) 
				m_nMoonRiseDay = m_MoonDay[mh-1];
		}
	
		if(m_lfMoonSet > 24){
			m_nMoonSetDay++;
			m_lfMoonSet -= 24;
			if(m_nMoonSetDay > m_MoonDay[mh]) 
				m_nMoonSetDay = 1;
		}
	}
	
	m_SMD12 = Rnd(lm12 - ls12);
	double smd = cos(m_SMD12 * RAD) * cos(bt12 * RAD);
	smd = acos(smd)*DEG;
	m_IOTA = 180 - smd - .1468 * sin(smd * RAD);
	m_lfIlluminated = (1 + cos(m_IOTA * RAD)) / 2 * 100;
}

/////////////////////////////////////////////////////////////////

const int Tide21Lib::m_cNC[40]={ // 【cycle_number()】
	0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 3, 3, 3, 3, 4, 4, 6, 6
};

const double Tide21Lib::m_cAgs[40]={ // 【angular_speed()】
	 0.0410686,  0.0821373,  0.5443747,  1.0158958,
	 1.0980331, 13.3986609, 13.4715145, 13.9430356,
	14.0251729, 14.4920521, 14.9178647, 14.9589314,
	15.0000000, 15.0410686, 15.0821353, 15.1232059,
	15.5854433, 16.0569644, 16.1391017, 27.8953548,
	27.9682084, 28.4397295, 28.5125831, 28.9019669,
	28.9841042, 29.4556253, 29.5284789, 29.9589333,
	30.0000000, 30.0410667, 30.0821373, 31.0158958,
	42.9271398, 43.4761563, 44.0251729, 45.0410686,
	57.9682084, 58.9841042, 86.9523127, 87.9682084
};

Tide21Lib::Tide21Lib(){

}

Tide21Lib::~Tide21Lib(){

}

//! 度分表示から度に
double Tide21Lib::Dg2Dc(double x){
	return PlanetLib::Fix(x)+(x-PlanetLib::Fix(x))/6.0*10.0;
}

//! TD2ファイルから潮汐調和定数を読み込む
bool Tide21Lib::LoadTD2(const char *szPath){
	char buf[1024];
	char *ptr;	

	FILE *fp=fopen(szPath, "r");
	if (!fp)
		return false;

	ptr= fgets(buf, sizeof(buf), fp);
	if (ptr == NULL || feof(fp) || ferror(fp)){
		fclose(fp);
		return false;
	}
	
	ptr=strtok(buf, ",");
	if (!ptr)
		return false;
	strcpy(m_szPortName, ptr);

	ptr=strtok(NULL, ",");
	if (!ptr)
		return false;
	m_lfPortLat=atof(ptr);

	ptr=strtok(NULL, ",");
	if (!ptr)
		return false;
	m_lfPortLng=atof(ptr);

	ptr=strtok(NULL, ",");
	if (!ptr)
		return false;
	m_lfPortLevel=atof(ptr);

	char token[6][sizeof(buf)];
	for (int i=0;i<40;i+=2) {
		ptr = fgets(buf, sizeof(buf), fp);
		if (ptr == NULL || feof(fp) || ferror(fp)){
			fclose(fp);
			return false;
		}
	
		int j=0;
		ptr=strtok(buf, ",");
		strcpy(token[j], ptr);
		while((ptr=strtok(NULL,","))!=NULL && j<5){
			strcpy(token[++j],ptr);
		}

		m_lfHR[i]  =atof(token[1]);
		m_lfPL[i]  =atof(token[2]);
		m_lfHR[i+1]=atof(token[4]);
		m_lfPL[i+1]=atof(token[5]);
	}
    fclose(fp);

	m_lfFlood=m_lfPortLevel + C_M2hr;
	m_lfEbb  =m_lfPortLevel - C_M2hr;

	SetTimeZone((int)(floor((Dg2Dc(m_lfPortLng)+7.5)/15.0)*15.0));

	return true;
}

bool Tide21Lib::DateCheck(int yr, int mh, int dy){
	if (mh < 1 || mh > 12)
		return false;

	int days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	if (!(yr%4)){
		if (!(yr%100) || yr%400)
			days[1] = 29;
	}
	if (dy > days[mh-1])
		return false;
	return true;
}

int Tide21Lib::SerialDay(int month,int day){
	int sday=0;
	
	for(int i=1;i<month;i++){
		sday += m_MoonDay[i] ;
	}
	return sday+day-1;
}

void Tide21Lib::GetMeanLongitudes(int year, int tz) {
	int ty = year - 2000 ;
	m_S = Rnd( 211.728 + Rnd( 129.38471 * ty ) + Rnd( 13.176396 * tz )) ;
	m_H = Rnd( 279.974 + Rnd(  -0.23871 * ty ) + Rnd(  0.985647 * tz )) ;
	m_P = Rnd(  83.298 + Rnd(  40.66229 * ty ) + Rnd(  0.111404 * tz )) ;
	m_N = Rnd( 125.071 + Rnd( -19.32812 * ty ) + Rnd( -0.052954 * tz )) ;
}

void Tide21Lib::ArgumentAndCoeffic(){
	double f0[10], u0[10], u[40];
	{ // argument_f0
		double cu, su, arg;

		double n1 = cos(    m_N*1.0 *RAD) ;
		double n2 = cos(Rnd(m_N*2.0)*RAD) ;
		double n3 = cos(Rnd(m_N*3.0)*RAD) ;

		f0[ 0] = 1.0000 -0.1300 * n1 + 0.0013 * n2  + 0.0000 * n3 ;
		f0[ 1] = 1.0429 +0.4135 * n1 - 0.0040 * n2  + 0.0000 * n3 ;
		f0[ 2] = 1.0089 +0.1871 * n1 - 0.0147 * n2  + 0.0014 * n3 ;
		f0[ 3] = 1.0060 +0.1150 * n1 - 0.0088 * n2  + 0.0006 * n3 ;
		f0[ 4] = 1.0129 +0.1676 * n1 - 0.0170 * n2  + 0.0016 * n3 ;
		f0[ 5] = 1.1027 +0.6504 * n1 + 0.0317 * n2  - 0.0014 * n3 ;
		f0[ 6] = 1.0004 -0.0373 * n1 + 0.0002 * n2  + 0.0000 * n3 ;
		f0[ 7] = 1.0241 +0.2863 * n1 + 0.0083 * n2  - 0.0015 * n3 ;

		cu =1.0 -0.2505*cos( m_P*2.0*RAD       ) -0.1102*cos((m_P*2.0-m_N)*RAD)
		        -0.0156*cos((m_P*2.0-m_N*2.0)*RAD) -0.0370*cos(m_N*RAD        ) ;

		su =    -0.2505*sin( m_P*2.0 * RAD)      -0.1102*sin((m_P*2.0-m_N)*RAD) 
	    	    -0.0156*sin((m_P*2.0-m_N*2.0)*RAD) -0.0370*sin(m_N * RAD    ) ;

		arg = atan2(su, cu) * DEG;
		f0[ 8] = su / sin( arg *RAD ) ;

		cu = 2.0*cos(m_P*RAD) + 0.4*cos((m_P-m_N)*RAD) ;
		su =     sin(m_P*RAD) + 0.2*cos((m_P-m_N)*RAD) ;
	
		arg = atan2(su, cu) * DEG;
		f0[ 9] = cu / cos( arg * RAD ) ;
	}

	{ // argument_u0
		double cu, su;
    
		double s1 = sin(    m_N * RAD   ) ;
		double s2 = sin(Rnd(m_N*2.0)*RAD) ;
		double s3 = sin(Rnd(m_N*3.0)*RAD) ;
									;
		u0[ 0] =    0.00 * s1  +  0.00 * s2  +  0.00 * s3 ;
		u0[ 1] =  -23.74 * s1  +  2.68 * s2  -  0.38 * s3 ;
		u0[ 2] =   10.80 * s1  -  1.34 * s2  +  0.19 * s3 ;
		u0[ 3] =   -8.86 * s1  +  0.68 * s2  -  0.07 * s3 ;
		u0[ 4] =  -12.94 * s1  +  1.34 * s2  -  0.19 * s3 ;
		u0[ 5] =  -36.68 * s1  +  4.02 * s2  -  0.57 * s3 ;
		u0[ 6] =   -2.14 * s1  +  0.00 * s2  +  0.00 * s3 ;
		u0[ 7] =  -17.74 * s1  +  0.68 * s2  -  0.04 * s3 ;

		cu =1.0 -0.2505*cos( m_P*2.0*RAD       )  -0.1102*cos((m_P*2.0-m_N)*RAD)
	    	    -0.0156*cos((m_P*2.0-m_N*2.0)*RAD)  -0.0370*cos(m_N*RAD        ) ;

		su =    -0.2505*sin( m_P*2.0 * RAD     )  -0.1102*sin((m_P*2.0-m_N)*RAD) 
	    	    -0.0156*sin((m_P*2.0-m_N*2.0)*RAD)  -0.0370*sin(m_N * RAD      ) ;

		u0[ 8] = atan2(su, cu) * DEG   ;

		cu = 2.0*cos(m_P*RAD) + 0.4*cos((m_P-m_N)*RAD) ;
		su =     sin(m_P*RAD) + 0.2*cos((m_P-m_N)*RAD) ;

		u0[ 9] = atan2(su, cu) * DEG     ;
	}

	{ // argument_v1
		m_V[ 0] = (  0.0*m_S +  1.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[ 1] = (  0.0*m_S +  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[ 2] = (  1.0*m_S +  0.0*m_H -  1.0*m_P +   0.0 ) ;
		m_V[ 3] = (  2.0*m_S -  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[ 4] = (  2.0*m_S +  0.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[ 5] = ( -3.0*m_S +  1.0*m_H +  1.0*m_P + 270.0 ) ;
		m_V[ 6] = ( -3.0*m_S +  3.0*m_H -  1.0*m_P + 270.0 ) ;
		m_V[ 7] = ( -2.0*m_S +  1.0*m_H +  0.0*m_P + 270.0 ) ;
		m_V[ 8] = ( -2.0*m_S +  3.0*m_H +  0.0*m_P - 270.0 ) ;
		m_V[ 9] = ( -1.0*m_S +  1.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[10] = (  0.0*m_S -  2.0*m_H +  0.0*m_P + 192.0 ) ;
		m_V[11] = (  0.0*m_S -  1.0*m_H +  0.0*m_P + 270.0 ) ;
		m_V[12] = (  0.0*m_S +  0.0*m_H +  0.0*m_P + 180.0 ) ;
		m_V[13] = (  0.0*m_S +  1.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[14] = (  0.0*m_S +  2.0*m_H +  0.0*m_P + 168.0 ) ;
		m_V[15] = (  0.0*m_S +  3.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[16] = (  1.0*m_S +  1.0*m_H -  1.0*m_P +  90.0 ) ;
		m_V[17] = (  2.0*m_S -  1.0*m_H +  0.0*m_P - 270.0 ) ;
		m_V[18] = (  2.0*m_S +  1.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[19] = ( -4.0*m_S +  2.0*m_H +  2.0*m_P +   0.0 ) ;
	}

	{ // argument_v2
		m_V[20] = ( -4.0*m_S +  4.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[21] = ( -3.0*m_S +  2.0*m_H +  1.0*m_P +   0.0 ) ;
		m_V[22] = ( -3.0*m_S +  4.0*m_H -  1.0*m_P +   0.0 ) ;
		m_V[23] = ( -2.0*m_S +  0.0*m_H +  0.0*m_P + 180.0 ) ;
		m_V[24] = ( -2.0*m_S +  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[25] = ( -1.0*m_S +  0.0*m_H +  1.0*m_P + 180.0 ) ;
		m_V[26] = ( -1.0*m_S +  2.0*m_H -  1.0*m_P + 180.0 ) ;
		m_V[27] = (  0.0*m_S -  1.0*m_H +  0.0*m_P + 282.0 ) ;
		m_V[28] = (  0.0*m_S +  0.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[29] = (  0.0*m_S +  1.0*m_H +  0.0*m_P + 258.0 ) ;
		m_V[30] = (  0.0*m_S +  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[31] = (  2.0*m_S -  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[32] = ( -4.0*m_S +  3.0*m_H +  0.0*m_P + 270.0 ) ;
		m_V[33] = ( -3.0*m_S +  3.0*m_H +  0.0*m_P + 180.0 ) ;
		m_V[34] = ( -2.0*m_S +  3.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[35] = (  0.0*m_S +  1.0*m_H +  0.0*m_P +  90.0 ) ;
		m_V[36] = ( -4.0*m_S +  4.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[37] = ( -2.0*m_S +  2.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[38] = ( -6.0*m_S +  6.0*m_H +  0.0*m_P +   0.0 ) ;
		m_V[39] = ( -4.0*m_S +  4.0*m_H +  0.0*m_P +   0.0 ) ;
	}

	{ // argument_u1
		u[ 0] = 0.0 ;
		u[ 1] = 0.0 ;
		u[ 2] = 0.0 ;
		u[ 3] = -u0[ 6] ;
		u[ 4] =  u0[ 1] ;
		u[ 5] =  u0[ 2] ;
		u[ 6] =  u0[ 2] ;
		u[ 7] =  u0[ 2] ;
		u[ 8] =  u0[ 6] ;
		u[ 9] =  u0[ 9] ;
		u[10] =  0.0 ;
		u[11] =  0.0 ;
		u[12] =  0.0 ;
		u[13] =  u0[ 3] ;
		u[14] =  0.0 ;
		u[15] =  0.0 ;
		u[16] =  u0[ 4] ;
		u[17] = -u0[ 2] ;
		u[18] =  u0[ 5] ;
		u[19] =  u0[ 6] ;
	}

	{ // argument_u2
		u[20] =  u0[ 6] ;
		u[21] =  u0[ 6] ;
		u[22] =  u0[ 6] ;
		u[23] =  u0[ 2] ;
		u[24] =  u0[ 6] ;
		u[25] =  u0[ 6] ;
		u[26] =  u0[ 8] ;
		u[27] =  0.0 ;
		u[28] =  0.0 ;
		u[29] =  0.0 ;
		u[30] =  u0[ 7] ;
		u[31] = -u0[ 6] ;
		u[32] =  u0[ 6]       + u0[ 2] ;
		u[33] =  u0[ 6] * 1.5          ;
		u[34] =  u0[ 6]       + u0[ 3] ;
		u[35] =  u0[ 3]                ;
		u[36] =  u0[ 6] * 2.0          ;
		u[37] =  u0[ 6]                ;
		u[38] =  u0[ 6] * 3.0          ;
		u[39] =  u0[ 6] * 2.0          ;
	}

	for (int i=0; i<40 ;i++){
		m_V[i]=Rnd(m_V[i]+u[i]);
	}

	{ // coeffic_f1
		m_F[ 0] = 1.0     ;
		m_F[ 1] = 1.0     ;
		m_F[ 2] = f0[ 0]  ;
		m_F[ 3] = f0[ 6]  ;
		m_F[ 4] = f0[ 1]  ;
		m_F[ 5] = f0[ 2]  ;
		m_F[ 6] = f0[ 2]  ;
		m_F[ 7] = f0[ 2]  ;
		m_F[ 8] = f0[ 6]  ;
		m_F[ 9] = f0[ 9]  ;
		m_F[10] = 1.0     ;
		m_F[11] = 1.0     ;
		m_F[12] = 1.0     ;
		m_F[13] = f0[ 3]  ;
		m_F[14] = 1.0     ;
		m_F[15] = 1.0     ;
		m_F[16] = f0[ 4]  ;
		m_F[17] = f0[ 2]  ;
		m_F[18] = f0[ 5]  ;
		m_F[19] = f0[ 6]  ;
	}

	{ // coeffic_f2
		m_F[20] = f0[ 6]  ;
		m_F[21] = f0[ 6]  ;
		m_F[22] = f0[ 6]  ;
		m_F[23] = f0[ 2]  ;
		m_F[24] = f0[ 6]  ;
		m_F[25] = f0[ 6]  ;
		m_F[26] = f0[ 8]  ;
		m_F[27] = 1.0     ;
		m_F[28] = 1.0     ;
		m_F[29] = 1.0     ;
		m_F[30] = f0[ 7]  ;
		m_F[31] = f0[ 6]  ;
		m_F[32] = f0[ 6]  *  f0[ 2]  ;
		m_F[33] = pow((double)f0[ 6], (double)1.5 ) ;
		m_F[34] = f0[ 6]  *  f0[ 3]  ;
		m_F[35] = f0[ 3]             ;
		m_F[36] = pow( (double)f0[ 6], (double)2.0 ) ;
		m_F[37] = f0[ 6]             ;
		m_F[38] = pow( (double)f0[ 6], (double)3.0 ) ;
		m_F[39] = pow( (double)f0[ 6], (double)2.0 ) ;
	}
}

void Tide21Lib::TideCalcInit(int yr, int mh, int dy){
	double z =  SerialZ(yr, mh, dy);
	double arg1 = z + 6.5;

	int tz = SerialDay(mh, dy) + (int)Fix((yr+3)/4)-500;
	GetMeanLongitudes(yr, tz);
	ArgumentAndCoeffic();

	double lng=Dg2Dc (m_lfPortLng);
	for (int i=0 ;i<40 ;i++ ){
		m_Vl[i] = m_V[i] - (-lng)*m_cNC[i] + m_cAgs[i]*( - m_lfTimeZone/15.0);
		m_Vl[i] = Rnd(m_Vl[i]);
	}
}

double Tide21Lib::CalcTideHight(int m){
	double tc = m_lfPortLevel ;
	for (int j=0 ; j<40 ; j++)
		tc += m_F[j]*m_lfHR[j]*cos((m_Vl[j]+m_cAgs[j]*(double)m/60.-m_lfPL[j])*RAD);
	return tc;
}

bool Tide21Lib::GetTidePeek(double *mh, double *hight, double startmh){
	if (startmh<0 || startmh > 24 * 60)
		return false;

	TideLagrange Lagrange;
	int st =20*((int)(startmh/20.));
	for (int m=st - 20 * 5;m<24 * 60;m+=20){
		bool bFind=m > st ? true: false;

		if (Lagrange.IsPeek((double)m, CalcTideHight(m), bFind)){
			if (Lagrange.GetT() > startmh){
				*mh=Lagrange.GetT();
				*hight=Lagrange.GetY();
				return true;
			}
		}
	}
    return false;
}

char *Tide21Lib::GetTideName(double moonage){
	struct MoonName{
		char *szName;
		double lfAge;
	} name[13]= {
		{"大潮",1.5},
		{"中潮",5.5},
		{"小潮",8.5},
		{"長潮",9.5},
		{"若潮",10.5},
		{"中潮",12.5},
		{"大潮",16.5},
		{"中潮",20.5},
		{"小潮",23.5},
		{"長潮",24.5},
		{"若潮",25.5},
		{"中潮",27.5},
		{"大潮",30.5}
	};

	for (int i=0;i<13;i++){
		if (moonage <= name[i].lfAge)
			return name[i].szName;
	}
	return "";
}
