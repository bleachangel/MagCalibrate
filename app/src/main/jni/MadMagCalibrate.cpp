#include "com_madgaze_madmagcalibrate_MainActivity.h"
#include <string.h>
#include "Calibration.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

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
    if(NULL == parr)
    {
        return NULL;
    }

	//if(Calc_Process(radius, parr))
	if(0 == ellipsoid_fitting(parr))
	{
	    LOGD("radius = %f ,calc param OK!", radius);
		env->ReleaseDoubleArrayElements(p, parr, 0);
        //env->DeleteLocalRef(p);
		return p;
	}
    env->ReleaseDoubleArrayElements(p, parr, 0);
    env->DeleteLocalRef(p);
	LOGD("fail to calc param!");
	return NULL;
}

jboolean JNICALL Java_com_madgaze_madmagcalibrate_MainActivity_setBias
        (JNIEnv * env, jobject obj, jdoubleArray bias)
{
    jdouble* pbias;
    pbias = env->GetDoubleArrayElements(bias, NULL);
    if(pbias == NULL)
    {
        LOGE("input parameters error!");
        return false;
    }

    char datapath[64]={"/sys/class/misc/m_mag_misc/magcalibrate"};
    int fd = open(datapath, O_RDWR);
    char buf[64];
    int len=0;

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d %d %d %d %d %d",(int)pbias[0],(int)pbias[1],(int)pbias[2],
             (int)pbias[3],(int)pbias[4],(int)pbias[5]);

    len = strlen(buf);
    int size = write(fd, buf, len);
    if(size != len){
        LOGE("size = %d, len = %d, write parameters error!",size, len);
    }
    close(fd);
    env->ReleaseDoubleArrayElements(bias, pbias,0);
    return true;
}
