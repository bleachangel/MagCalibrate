
#ifndef _CALIBRATION_H
#define _CALIBRATION_H
#include<android/log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "MagCalibration" // 这个是自定义的LOG的标识

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

/**
测量值:xm,ym,zm;
校准后的值：xc,yc,zc;
平移参数：ox,oy,oz;
缩放参数：gx,gy,gz;
参数之间的关系如下：
xc=(xm+ox)*gx;
yc=(ym+oy)*gy;
zc=(zm+oz)*gz;
*/
class params{
public:
	double m_ox;
	double m_oy;
	double m_oz;
	double m_gx;
	double m_gy;
	double m_gz;
};
/**
* 功能：将8字校准过程中采集的数据传入给到算法模块
* 输入参数：
*		x--x轴方向地磁数值
*		y--y轴方向地磁数值
*		z--z轴方向地磁数值
* 返回值：
		成功添加返回true，否则返回false
*/
bool addSample(double x, double y, double z);

/**
* 功能：8字校准算法模块
* 输入参数：
*		radius--校准理论圆球半径，默认设置为13
* 输出参数：
*		p--算法根据采集的数据，计算出校准参数，参考params的说明
* 返回值：
		成功计算出校准参数返回true，否则返回false
*/
//bool Calc_Process(double radius, params &p);
bool Calc_Process(double radius, double p[6]);

/**
* 功能：椭圆拟合校准算法模块
* 输出参数：
*		p--算法根据采集的数据，计算出校准参数，参考params的说明
* 返回值：
		成功计算出校准参数返回true，否则返回false
*/
int ellipsoid_fitting(double p[6]);
#ifdef __cplusplus
}
#endif
#endif
