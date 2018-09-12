#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <exception>
#include <typeinfo>
#include <float.h>
#include "Calibration.h"

#define MATRIX_SIZE 7
#define u8 unsigned char

class sample{
public:
	double m_x;
	double m_y;
	double m_z;
};

static std::vector<sample> s_samples;
static double m_matrix[MATRIX_SIZE][MATRIX_SIZE+1];
static int m = MATRIX_SIZE;	
static int n = MATRIX_SIZE+1;
static double m_result[MATRIX_SIZE];	

static void DispMatrix(void);

static double Abs(double a)
{
	return a<0 ? -a : a;
}

static u8 Equal(double a,double b)
{
	return Abs(a-b) < 1e-6;
}

static void ResetMatrix(void)
{
	int row , column;
	
	for(row = 0 ; row<m ; row++){
		for(column = 0 ; column<n ; column++)
			m_matrix[row][column] = 0.0f;
	}
}
	
static void CalcData_Input(double x , double y , double z)
{
	double V[MATRIX_SIZE];
	int row , column;
	
	V[0] = x*x;
    V[1] = y*y;
    V[2] = z*z;
    V[3] = x;
    V[4] = y;
    V[5] = z;
    V[6] = 1.0;
	
	//构建VxVt矩阵(Vt为V的转置)，并进行累加
	for(row = 0 ; row<MATRIX_SIZE ; row++){
		for(column = 0 ; column<MATRIX_SIZE ; column++){
			m_matrix[row][column] += V[row]*V[column];
		}
	}
}

static void SwapRow(int row1 , int row2)
{
	int column;
	double tmp;
	
	for(column = 0 ; column<n ; column++){
		tmp = m_matrix[row1][column];
		m_matrix[row1][column] = m_matrix[row2][column];
		m_matrix[row2][column] = tmp;
	}
}

static void MoveBiggestElement2Top(int s_row , int s_column)
{
	int row,column;
	
	for(row = s_row+1 ; row<m ; row++){
		if( Abs(m_matrix[s_row][s_column])<Abs(m_matrix[row][s_column])){
			SwapRow(s_row , row);
		}
	}
}

//高斯消元法，求行阶梯型矩阵
static u8 Matrix_GaussElimination(void)
{
	int row,column,i,j;
	double tmp;
	
	for(row = 0,column=0 ; row<m-1 && column<n-1 ; row++,column++){
		//将当前列最大的一行移上来
		MoveBiggestElement2Top(row , column);
		
		//整列都为0
		if(Equal(m_matrix[row][column],0.0f)){
			LOGD("qiyi matrix:%d %d\r\n" , row , column);
			//DispMatrix();
			//return 0;
			row--;
			continue;
		}
		
		//高斯消元
		for(i = row+1 ; i<m ; i++){	
			if(Equal(m_matrix[i][column],0.0f))
				continue;	//为0，无需处理
			
			tmp = m_matrix[i][column]/m_matrix[row][column];
			
			for(j = column ; j<n ; j++){
				m_matrix[i][j] -= m_matrix[row][j]*tmp;
			}
		}

		DispMatrix();
		LOGD("\r\n");
	}

	return 1;
}

//求行最简型矩阵
static int Matrix_RowSimplify(void)
{
    int c = n;//返回值，表示(解的任意常量数+1)；
    //
    int row,column,k,s,t;
    double tmp;
    //
    for(row=0,column=0;row<m && column<n;row++,column++)
    {
        if(Equal(m_matrix[row][column],0))//平移，找出本行第一个非零；
        {
            row--;
            continue;
        }
        //
        c--;//少一个常量；
        //
        //化a[i][j]为1；
        tmp = 1 / m_matrix[row][column];
        for(k=column;k<n;k++)//前面的"0"就不处理了；
            m_matrix[row][k] *= tmp;
        //
        //化a[s][j]为0
        for(s=0;s<row;s++)//下面的0也不用处理；
        {
            if(Equal(m_matrix[s][column],0))
                continue;//已经为0；
            //
            tmp = m_matrix[s][column] / m_matrix[row][column];
            for(t=column;t<n;t++)
                m_matrix[s][t] -= m_matrix[row][t]*tmp;
            //
        }
    }
    //
    return c;
}

static void Matrix_Solve(double* C , double* sol)
{
	int row,column,i;
	int any_sol[MATRIX_SIZE];

	//找出任意解的位置
	memset(any_sol , 0 , MATRIX_SIZE);
	for(row=0,column=0 ; row<m && column<n-1 ; row++,column++){
		if(Equal(m_matrix[row][column] , 0.0f)){
			any_sol[column] = 1;	//记录任意解的位置
			row--;	//右移1列
		}
	}

	//求解
	row = 0;
	for(column = 0 ; column<n-1 ; column++){
		if(any_sol[column] == 1){	//任意解
			sol[column] = C[column];
		}else{
			sol[column] = m_matrix[row][n-1];
			//加上任意解
			for(i = column+1 ; i<n-1 ; i++){
				if(any_sol[i]==1 && !Equal(m_matrix[row][i],0.0f)){
					sol[column] -= m_matrix[row][i]*C[i];
				}
			}	
			row++;
		}
	}
}

static void DispMatrix(void)
{
	int row,column;
	
	for(row = 0 ; row<m ; row++){
		for(column = 0 ; column<n ; column++){
			LOGD("%.3f	" , m_matrix[row][column]);
		}
		LOGD("\r\n");
	}
}

bool IsFiniteNumber(double x)
{
	return (x <= DBL_MAX && x >= -DBL_MAX);
}

bool Calc_Process(double radius, double p[6])
{
	double C[MATRIX_SIZE];
	double Res[MATRIX_SIZE];
	int i;
	double k;
	bool bRet = true;

	ResetMatrix();

	//输入任意个数磁场测量点坐标，请尽量保证在椭球上分布均匀
	/*CalcData_Input(245.0,-815.4,164.7);
	CalcData_Input(239.1,-818.0,165.4);
	CalcData_Input(237.1,-822.9,165.5);
	CalcData_Input(235.6,-829.1,166.6);
	CalcData_Input(239.2,-834.0,167.7);
	CalcData_Input(245.3,-838.6,166.9);
	CalcData_Input(252.2,-838.8,168.3);
	CalcData_Input(258.8,-835.1,167.1);
	CalcData_Input(261.2,-831.9,167.6);
	CalcData_Input(260.9,-823.9,165.8);
	CalcData_Input(257.2,-816.4,165.5);
	CalcData_Input(251.2,-814.9,163.9);*/
	for (int i = 0; i < s_samples.size(); i++)
	{
		CalcData_Input(s_samples.at(i).m_x, s_samples.at(i).m_y, s_samples.at(i).m_z);
	}

	Matrix_GaussElimination();
	Matrix_RowSimplify();

    //赋值任意解参数值
	for(i = 0 ; i<MATRIX_SIZE ; i++){
		C[i] = 1.0f;
	}

	Matrix_Solve(C , Res);

	LOGD("a:%.2f b:%.2f c:%.2f d:%.2f e:%.2f f:%.2f g:%.2f\r\n" , Res[0],Res[1],Res[2],Res[3],Res[4],Res[5],Res[6]);

	k = (Res[3]*Res[3]/Res[0]+Res[4]*Res[4]/Res[1]+Res[5]*Res[5]/Res[2] - 4*Res[6])/(4*radius*radius);

	m_result[0] = sqrt(Res[0] / k);
    m_result[1] = sqrt(Res[1] / k);
    m_result[2] = sqrt(Res[2] / k);
    m_result[3] = Res[3] / (2 * Res[0]);
    m_result[4] = Res[4] / (2 * Res[1]);
    m_result[5] = Res[5] / (2 * Res[2]);

	for (int i = 0; i < sizeof(m_result); i++)
	{
		if(!IsFiniteNumber(m_result[i]))
		{
			bRet = false;
			break;
		}
	}
	
	if(bRet)
	{
		LOGD("Xo:%f Yo:%f Zo:%f Xg:%f Yg:%f Zg:%f k:%f\r\n" , m_result[3],m_result[4],m_result[5],m_result[0],m_result[1],m_result[2],k);
		/*p.m_ox = m_result[3];
		p.m_oy = m_result[4];
		p.m_oz = m_result[5];
		p.m_gx = m_result[0];
		p.m_gy = m_result[1];
		p.m_gz = m_result[2];*/
		p[0] = m_result[3];
		p[1] = m_result[4];
		p[2] = m_result[5];
		p[3] = m_result[0];
		p[4] = m_result[1];
		p[5] = m_result[2];
	}
	
	return bRet;
}

bool addSample(double x, double y, double z)
{
	sample tmp;
	tmp.m_x = x;
	tmp.m_y = y;
	tmp.m_z = z;
	s_samples.push_back(tmp);

	LOGD("addSample(%f, %f, %f) \n",x, y, z);
	return true;
}
#ifdef TEST_MAIN
int main(int argc, char* argv[])
{
	double samples[12][3] = 
	{{245.0,-815.4,164.7},
	{239.1,-818.0,165.4},
	{237.1,-822.9,165.5},
	{235.6,-829.1,166.6},
	{239.2,-834.0,167.7},
	{245.3,-838.6,166.9},
	{252.2,-838.8,168.3},
	{258.8,-835.1,167.1},
	{261.2,-831.9,167.6},
	{260.9,-823.9,165.8},
	{257.2,-816.4,165.5},
	{251.2,-814.9,163.9}
	};
	//params p;
	double p[6];
	
	LOGD("array size : %d \n" , sizeof(samples)/sizeof(samples[0]));
	for (int i = 0; i < sizeof(samples)/sizeof(samples[0]); i++)
	{
		sample tmp;
		tmp.m_x = samples[i][0];
		tmp.m_y = samples[i][1];
		tmp.m_z = samples[i][2];
		addSample(samples[i][0], samples[i][1], samples[i][2]);
		LOGD("push x: %f, y: %f, z:%f \n",tmp.m_x,tmp.m_y,tmp.m_z);
	}

	//if(!Calc_Process(13.0, p))
	if(!Calc_Process(13.0, p))
	{
		LOGD("There is not enough data for calc! \n" );
	}
	
	//LOGD("Xo:%f Yo:%f Zo:%f Xg:%f Yg:%f Zg:%f \r\n" , p.m_ox,p.m_oy, p.m_oz, p.m_gx, p.m_gy, p.m_gz);
	LOGD("Xo:%f Yo:%f Zo:%f Xg:%f Yg:%f Zg:%f \r\n" , p[3],p[4], p[5], p[0], p[1], p[2]);
	
	return 0;
}
#endif
