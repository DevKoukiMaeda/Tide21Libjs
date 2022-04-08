#include <emscripten.h>
#include <stdio.h>
#include "Tide21Lib.h"
#include <string>
#include <string.h>
#include "picojson.h"

#ifdef __cplusplus
extern "C"
{
#endif

	char *HourToTimeString(double h)
	{
		static char buf[1024];

		int t = (int)(h * 60 + 0.5);
		sprintf(buf, "%02d:%02d", t / 60, t % 60);
		return buf;
	}

	//指定日時の天文潮位を取得
	void EMSCRIPTEN_KEEPALIVE cr_tide(int y, int m, int d, int h, int mn, char *td2_path, char *ptrRet)
	{
		// JSONのルートオブジェクト
		picojson::object root_obj;
		// JSONの子要素
		using v = picojson::value;
		//潮位計算クラス
		Tide21Lib lib;

		//調和定数表のパスが空白ならエラー
		if (strlen(td2_path) == 0)
		{
			root_obj.insert({"error", v("td2 path is empty")});
		}
		else
		{
			//調和定数表をロード
			if (lib.LoadTD2(td2_path))
			{
				//潮位を求める日付で初期化
				lib.TideCalcInit(y, m, d);
				int tmn = h * 60 + mn;

				double h = lib.CalcTideHight(tmn);
				double _h = lib.CalcTideHight(tmn + 5);
				//指定日時の潮位
				root_obj.insert({"height", v(h)});
				//指定日時の潮位の５分後の比高
				root_obj.insert({"r_height", v(_h - h)});

				//潮位を求める日付で月齢を取得
				lib.CalcMoon(y, m, d);
				double age = lib.m_Moon.m_lfMoonAge;
				//潮まわりを取得
				root_obj.insert({"tidename", v(lib.GetTideName(age))});
				//潮位を求める日付でを太陽の情報を取得
				lib.CalcSun(y, m, d);
				//日の出の時刻を取得
				root_obj.insert({"sunrise", v(HourToTimeString(lib.m_Sun.m_lfSunRise))});
				//日没の時刻を取得
				root_obj.insert({"sunset", v(HourToTimeString(lib.m_Sun.m_lfSunSet))});
				// JSONオブジェクトをシリアライズし確保されたメモリに格納する
			}
			else
			{
				//パスで指定した調和定数表をロードできない場合エラー
				root_obj.insert({"error", v("td2 not found")});
			}
		}

		// JSONオブジェクトをシリアライズし確保されたメモリに格納する
		std::string result_str = std::string(v(root_obj).serialize());
		const char *srcchr = result_str.c_str();
		for (int i = 0; i < strlen(srcchr); i++)
		{
			ptrRet[i] = srcchr[i];
		}
		return;
	}
	//グラフ用に一日分の潮位を取得
	void EMSCRIPTEN_KEEPALIVE calctide(int y, int m, int d, char *td2_path, char *ptrRet)
	{
		// JSONのルートオブジェクト
		picojson::object root_obj;
		// JSONの配列潮位用
		picojson::array tide_arr;
		// JSONの子要素
		using v = picojson::value;
		//潮位計算クラス
		Tide21Lib lib;

		//調和定数表のパスが空白ならエラー
		if (strlen(td2_path) == 0)
		{
			root_obj.insert({"error", v("td2 path is empty")});
		}
		else
		{
			//調和定数表をロード
			if (lib.LoadTD2(td2_path))
			{
				//潮位を求める日付で初期化
				lib.TideCalcInit(y, m, d);
				//２４時間分の潮位を20分間隔で取得
				for (int m = 0; m < 60 * 24; m += 20)
				{
					picojson::array _tidepair;
					double h = lib.CalcTideHight(m);
					_tidepair.push_back(v((double)m / 60.));
					_tidepair.push_back(v(h));
					tide_arr.push_back(v(_tidepair));
				}
				//２４時間分の潮位を格納
				root_obj.insert({"tidedata", v(tide_arr)});
				//潮位を求める日付で月齢を計算
				lib.CalcMoon(y, m, d);
				double age = lib.m_Moon.m_lfMoonAge;

				//潮まわりを取得
				root_obj.insert({"tidename", v(lib.GetTideName(age))});
				lib.CalcSun(y, m, d);
				//潮位を求める日付でを太陽の情報を取得
				root_obj.insert({"sunrise", v(HourToTimeString(lib.m_Sun.m_lfSunRise))});
				//日没の時刻を取得
				root_obj.insert({"sunset", v(HourToTimeString(lib.m_Sun.m_lfSunSet))});
			}
			else
			{
				//パスで指定した調和定数表をロードできない場合エラー
				root_obj.insert({"error", v("td2 not found")});
			}
		}

		// JSONオブジェクトをシリアライズし確保されたメモリに格納する
		std::string result_str = std::string(v(root_obj).serialize());
		const char *srcchr = result_str.c_str();
		for (int i = 0; i < strlen(srcchr); i++)
		{
			ptrRet[i] = srcchr[i];
		}
		return;
	}

#ifdef __cplusplus
}
#endif