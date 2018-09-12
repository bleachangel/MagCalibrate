#include "com_madgaze_madmagcalibrate_MainActivity.h"
#include <string.h>
#include "Calibration.h"

jboolean JNICALL Java_com_madgaze_madmagcalibrate_MainActivity_addSample
  (JNIEnv * env, jobject obj, jdouble x, jdouble y, jdouble z)
{
	jboolean ret = true;
	if(!addSample(x, y, z))
	{
		ret = false;
	}
	return ret;
}
jdoubleArray JNICALL Java_com_madgaze_madmagcalibrate_MainActivity_getParams
  (JNIEnv * env, jobject obj, jdouble radius)
{
	jdoubleArray p = env->NewDoubleArray(6);
	jdouble *parr = env->GetDoubleArrayElements(p, NULL);

	if(Calc_Process(radius, parr))
	{
	    LOGD("radius = %f ,calc param OK!", radius);
		env->ReleaseDoubleArrayElements(p, parr, 0);
		return p;
	}

	LOGD("fail to calc param!");
	return NULL;
}
