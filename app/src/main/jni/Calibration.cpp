#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <exception>
#include <typeinfo>
#include <float.h>
#include <malloc.h>
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

	LOGD("addSample(%f %f %f) \n",x, y, z);
	return true;
}

extern "C" void Matrix_x_Its_Transpose(double *, double *, int, int);
extern "C" void Get_Submatrix(double *, int, int, double *, int, int, int);
extern "C" int Choleski_LU_Decomposition(double *, int);
extern "C" int Choleski_LU_Inverse(double *, int);
extern "C" void Multiply_Matrices(double *, double *, int, int, double *, int);
extern "C" int Hessenberg_Form_Elementary(double *, double *, int);
extern "C" int QR_Hessenberg_Matrix(double *, double *, double[], double[], int, int);
extern "C" void Transpose_Square_Matrix(double *, int);

int ellipsoid_fitting(double params[6])
{
    int 			nlines = 0;
    char			buf[120];
    double *		D, *S, *C, *S11, *S12, *S12t, *S22, *S22_1, *S22a, *S22b, *SS, *E, *d, *U, *SSS;
    double *		eigen_real, *eigen_imag, *v1, *v2, *v, *Q, *Q_1, *B, *QB, J, hmb, *SSSS;
    int *			p;
    int 			i, index;
    double			maxval, norm, btqb, *eigen_real3, *eigen_imag3, *Dz, *vdz, *SQ, *A_1, hm, norm1, norm2, norm3;
    double			x, y, z;

    nlines              = s_samples.size();
    D					= (double *)malloc(10 * nlines * sizeof(double));

    for (i = 0; i < nlines; i++)
    {
        x = s_samples[i].m_x;
        y = s_samples[i].m_y;
        z = s_samples[i].m_z;
        D[i]				= x * x;
        D[nlines + i]		= y * y;
        D[nlines * 2 + i]	= z * z;
        D[nlines * 3 + i]	= 2.0 * y * z;
        D[nlines * 4 + i]	= 2.0 * x * z;
        D[nlines * 5 + i]	= 2.0 * x * y;
        D[nlines * 6 + i]	= 2.0 * x;
        D[nlines * 7 + i]	= 2.0 * y;
        D[nlines * 8 + i]	= 2.0 * z;
        D[nlines * 9 + i]	= 1.0;
    }

    // allocate memory for matrix S
    S					= (double *)malloc(10 * 10 * sizeof(double));
    Matrix_x_Its_Transpose(S, D, 10, nlines);

    // Create pre-inverted constraint matrix C
    C					= (double *)malloc(6 * 6 * sizeof(double));
    C[0]				= 0.0;
    C[1]				= 0.5;
    C[2]				= 0.5;
    C[3]				= 0.0;
    C[4]				= 0.0;
    C[5]				= 0.0;
    C[6]				= 0.5;
    C[7]				= 0.0;
    C[8]				= 0.5;
    C[9]				= 0.0;
    C[10]				= 0.0;
    C[11]				= 0.0;
    C[12]				= 0.5;
    C[13]				= 0.5;
    C[14]				= 0.0;
    C[15]				= 0.0;
    C[16]				= 0.0;
    C[17]				= 0.0;
    C[18]				= 0.0;
    C[19]				= 0.0;
    C[20]				= 0.0;
    C[21]				= -0.25;
    C[22]				= 0.0;
    C[23]				= 0.0;
    C[24]				= 0.0;
    C[25]				= 0.0;
    C[26]				= 0.0;
    C[27]				= 0.0;
    C[28]				= -0.25;
    C[29]				= 0.0;
    C[30]				= 0.0;
    C[31]				= 0.0;
    C[32]				= 0.0;
    C[33]				= 0.0;
    C[34]				= 0.0;
    C[35]				= -0.25;

    S11 				= (double *)malloc(6 * 6 * sizeof(double));
    Get_Submatrix(S11, 6, 6, S, 10, 0, 0);
    S12 				= (double *)malloc(6 * 4 * sizeof(double));
    Get_Submatrix(S12, 6, 4, S, 10, 0, 6);
    S12t				= (double *)malloc(4 * 6 * sizeof(double));
    Get_Submatrix(S12t, 4, 6, S, 10, 6, 0);
    S22 				= (double *)malloc(4 * 4 * sizeof(double));
    Get_Submatrix(S22, 4, 4, S, 10, 6, 6);

    S22_1				= (double *)malloc(4 * 4 * sizeof(double));

    for (i = 0; i < 16; i++)
        S22_1[i] = S22[i];

    Choleski_LU_Decomposition(S22_1, 4);
    Choleski_LU_Inverse(S22_1, 4);

    // Calculate S22a = S22_1 * S12t   4*6 = 4x4 * 4x6	 C = AB
    S22a				= (double *)malloc(4 * 6 * sizeof(double));
    Multiply_Matrices(S22a, S22_1, 4, 4, S12t, 6);

    // Then calculate S22b = S12 * S22a 	 ( 6x6 = 6x4 * 4x6)
    S22b				= (double *)malloc(6 * 6 * sizeof(double));
    Multiply_Matrices(S22b, S12, 6, 4, S22a, 6);

    // Calculate SS = S11 - S22b
    SS					= (double *)malloc(6 * 6 * sizeof(double));

    for (i = 0; i < 36; i++)
        SS[i] = S11[i] -S22b[i];

    E					= (double *)malloc(6 * 6 * sizeof(double));
    Multiply_Matrices(E, C, 6, 6, SS, 6);
    SSS 				= (double *)malloc(6 * 6 * sizeof(double));
    Hessenberg_Form_Elementary(E, SSS, 6);

    eigen_real			= (double *)malloc(6 * sizeof(double));
    eigen_imag			= (double *)malloc(6 * sizeof(double));

    QR_Hessenberg_Matrix(E, SSS, eigen_real, eigen_imag, 6, 100);

    index				= 0;
    maxval				= eigen_real[0];

    for (i = 1; i < 6; i++)
    {
        if (eigen_real[i] > maxval)
        {
            maxval				= eigen_real[i];
            index				= i;
        }
    }

    v1					= (double *)malloc(6 * sizeof(double));

    v1[0]				= SSS[index];
    v1[1]				= SSS[index + 6];
    v1[2]				= SSS[index + 12];
    v1[3]				= SSS[index + 18];
    v1[4]				= SSS[index + 24];
    v1[5]				= SSS[index + 30];

    // normalize v1
    norm				= sqrt(v1[0] *v1[0] +v1[1] *v1[1] +v1[2] *v1[2] +v1[3] *v1[3] +v1[4] *v1[4] +v1[5] *v1[5]);
    v1[0]				/= norm;
    v1[1]				/= norm;
    v1[2]				/= norm;
    v1[3]				/= norm;
    v1[4]				/= norm;
    v1[5]				/= norm;

    if (v1[0] < 0.0)
    {
        v1[0]				= -v1[0];
        v1[1]				= -v1[1];
        v1[2]				= -v1[2];
        v1[3]				= -v1[3];
        v1[4]				= -v1[4];
        v1[5]				= -v1[5];
    }

    // Calculate v2 = S22a * v1 	 ( 4x1 = 4x6 * 6x1)
    v2					= (double *)malloc(4 * sizeof(double));
    Multiply_Matrices(v2, S22a, 4, 6, v1, 1);

    v					= (double *)malloc(10 * sizeof(double));

    v[0]				= v1[0];
    v[1]				= v1[1];
    v[2]				= v1[2];
    v[3]				= v1[3];
    v[4]				= v1[4];
    v[5]				= v1[5];
    v[6]				= -v2[0];
    v[7]				= -v2[1];
    v[8]				= -v2[2];
    v[9]				= -v2[3];

    Q					= (double *)malloc(3 * 3 * sizeof(double));

    Q[0]				= v[0];
    Q[1]				= v[5];
    Q[2]				= v[4];
    Q[3]				= v[5];
    Q[4]				= v[1];
    Q[5]				= v[3];
    Q[6]				= v[4];
    Q[7]				= v[3];
    Q[8]				= v[2];

    U					= (double *)malloc(3 * sizeof(double));

    U[0]				= v[6];
    U[1]				= v[7];
    U[2]				= v[8];

    Q_1 				= (double *)malloc(3 * 3 * sizeof(double));

    for (i = 0; i < 9; i++)
        Q_1[i] = Q[i];

    Choleski_LU_Decomposition(Q_1, 3);
    Choleski_LU_Inverse(Q_1, 3);

    // Calculate B = Q-1 * U   ( 3x1 = 3x3 * 3x1)
    B					= (double *)malloc(3 * sizeof(double));
    Multiply_Matrices(B, Q_1, 3, 3, U, 1);

    B[0]				= -B[0];					// x-axis combined bias
    B[1]				= -B[1];					// y-axis combined bias
    B[2]				= -B[2];					// z-axis combined bias

    for (i = 0; i < 3; i++){
        params[i] = B[i];
        LOGD("%lf\r\n", B[i]);
    }

    params[3] = params[4] = params[5] = 1;

    // First calculate QB = Q * B	( 3x1 = 3x3 * 3x1)
    QB					= (double *)malloc(3 * sizeof(double));
    Multiply_Matrices(QB, Q, 3, 3, B, 1);

    // Then calculate btqb = BT * QB	( 1x1 = 1x3 * 3x1)
    Multiply_Matrices(&btqb, B, 1, 3, QB, 1);

    // Calculate hmb = sqrt(btqb - J).
    J					= v[9];
    hmb 				= sqrt(btqb - J);

    // Calculate SQ, the square root of matrix Q
    SSSS				= (double *)malloc(3 * 3 * sizeof(double));
    Hessenberg_Form_Elementary(Q, SSSS, 3);

    eigen_real3 		= (double *)malloc(3 * sizeof(double));
    eigen_imag3 		= (double *)malloc(3 * sizeof(double));
    QR_Hessenberg_Matrix(Q, SSSS, eigen_real3, eigen_imag3, 3, 100);

    // normalize eigenvectors
    norm1				= sqrt(SSSS[0] *SSSS[0] +SSSS[3] *SSSS[3] +SSSS[6] *SSSS[6]);
    SSSS[0] 			/= norm1;
    SSSS[3] 			/= norm1;
    SSSS[6] 			/= norm1;
    norm2				= sqrt(SSSS[1] *SSSS[1] +SSSS[4] *SSSS[4] +SSSS[7] *SSSS[7]);
    SSSS[1] 			/= norm2;
    SSSS[4] 			/= norm2;
    SSSS[7] 			/= norm2;
    norm3				= sqrt(SSSS[2] *SSSS[2] +SSSS[5] *SSSS[5] +SSSS[8] *SSSS[8]);
    SSSS[2] 			/= norm3;
    SSSS[5] 			/= norm3;
    SSSS[8] 			/= norm3;

    Dz					= (double *)malloc(3 * 3 * sizeof(double));

    for (i = 0; i < 9; i++)
        Dz[i] = 0.0;

    Dz[0]				= sqrt(eigen_real3[0]);
    Dz[4]				= sqrt(eigen_real3[1]);
    Dz[8]				= sqrt(eigen_real3[2]);

    vdz 				= (double *)malloc(3 * 3 * sizeof(double));
    Multiply_Matrices(vdz, SSSS, 3, 3, Dz, 3);

    Transpose_Square_Matrix(SSSS, 3);

    SQ					= (double *)malloc(3 * 3 * sizeof(double));
    Multiply_Matrices(SQ, vdz, 3, 3, SSSS, 3);

    hm					= 0.569;
    A_1 				= (double *)malloc(3 * 3 * sizeof(double));

    for (i = 0; i < 9; i++)
        A_1[i] = SQ[i] *hm / hmb;

    for (i = 0; i < 3; i++)
        LOGD("%lf %lf %lf\r\n", A_1[i * 3], A_1[i * 3 + 1], A_1[i * 3 + 2]);

    free(D);
    free(S);
    free(C);
    free(S11);
    free(S12);
    free(S12t);
    free(S22);
    free(S22_1);
    free(S22a);
    free(S22b);
    free(SS);
    free(E);
    free(U);
    free(SSS);
    free(eigen_real);
    free(eigen_imag);
    free(v1);
    free(v2);
    free(v);
    free(Q);
    free(Q_1);
    free(B);
    free(QB);
    free(SSSS);
    free(eigen_real3);
    free(eigen_imag3);
    free(Dz);
    free(vdz);
    free(SQ);
    free(A_1);
    return 0;
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
