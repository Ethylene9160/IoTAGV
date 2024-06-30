#include  "kalman.h"
#include  "stdlib.h"
#include  "math.h"


/*==============================================================================
1.预估计
   X(k|k-1) = A(k,k-1)*X(k-1|k-1)                 //控制量为0


2.计算预估计协方差矩阵
   P(k|k-1) = A(k,k-1)*P(k-1|k-1)*A(k,k-1)'+Q(k)
   Q(k) = U(k)×U(k)' 


3.计算卡尔曼增益矩阵
   K(k) = P(k|k-1)*C(k)' / (C(k)*P(k|k-1)*C(k)' + R(k))
   R(k) = N(k)×N(k)' 


4.更新估计
   X(k|k) = X(k|k-1)+K(k)*(Y(k)-C(k)*X(k|k-1))


5.计算更新后估计协防差矩阵
   P(k|k) =（I-K(k)*C(k)）*P(k|k-1)


6. 更新最优值


A(k,k-1):     状态转移矩阵
B(k,k-1):     状态的控制量
C(k):         观测矩阵转移矩阵
X(k|k-1):     根据k-1时刻的最优值估计k时刻的值
X(k-1|k-1):   k-1时刻的最优值
P(k|k-1):     X(k|k-1)对应的covariance
P(k-1|k-1):   X(k-1|k-1)对应的covariance
Q(k):         系统过程的covariance(对上一次测量估计值的信任程度)
R(k):         测量过程的协方差
Y(k):         k时刻的测量值
K(k):         卡尔曼增益
U(k):         k时刻动态噪声
N(k):         k时刻观测噪声


例如:     系统状态X-----------实际温度
          系统矩阵A-----------温度变化转移
          状态的控制量B-------(通常没有)
          观测值Y-------------温度计读数
          观测矩阵C-----------摄氏度-〉华氏度
          过程噪声Q-----------温度变化偏差
          测量噪声R-----------读数误差

基本思路: 首先根据上一次(如果是第一次则根据预赋值计算)的数据计算出本次的估计值,
          同理,根据上一次的数据计算出本次估计值的协方差;  接着,由本次估计值的协
          方差计算出卡尔曼增益;  最后,根据估测值和测量值计算当前最优值及其协方差
          另外,卡尔曼滤波只是时域上的处理,可以认为频率影响很小
==============================================================================*/



//================================================//
//==             最优值方差结构体               ==//
//================================================//
typedef struct  _tCovariance
{
  float PNowOpt[P_LENGTH];
  float PPreOpt[P_LENGTH];
}tCovariance;



//================================================//
//==               最优值结构体                 ==//
//================================================//
typedef struct  _tOptimal
{
  float XNowOpt[X_LENGTH];
  float XPreOpt[X_LENGTH];
}tOptimal;



tOptimal      tOpt;                                     //  在滤波函数中进行初始化零值
tCovariance   tCov;                                     //  在滤波函数中进行初始化零值
float         Y[Y_LENGTH]  = Y_VALUE;                   //  测量值(每次测量的数据需要存入该数组)
float         I[I_LENGTH]  = I_VALUE;                   //  单位矩阵
float         X[X_LENGTH]  = X_VALUE;                   //  当前状态的预测值
float         P[P_LENGTH]  = P_VALUE;                   //  当前状态的预测值的协方差
float         K[K_LENGTH]  = K_VALUE;                   //  卡尔曼增益
float         Temp1[1]     = {0};                       //  辅助变量
//============================================================================//
//==                    卡尔曼滤波需要配置的变量                            ==//
//============================================================================//
float         A[A_LENGTH]       = A_VALUE;              //  状态转移矩阵
float         B[B_LENGTH]       = B_VALUE;              //  系统参数
float         Q[Q_LENGTH]       = Q_VALUE;              //  系统过程的协方差
float         C[C_LENGTH]       = C_VALUE;              //  观测矩阵转移矩阵
float         R[R_LENGTH]       = R_VALUE;              //  测量过程的协方差
float         Temp2[X_LENGTH]   = X_VALUE;              //  辅助变量, 同时保存tOpt.XPreOpt[]的初始化值
float         Temp22[X_LENGTH]  = X_VALUE;              //  辅助变量
float         Temp4[P_LENGTH]   = P_VALUE;              //  辅助变量, 同时保存tCov.PPreOpt[]的初始化值


//============================================================================//
//==                          卡尔曼滤波                                    ==//
//============================================================================//
//==入口参数: 无                                                            ==//
//==出口参数: 无                                                            ==//
//==返回值:   无                                                            ==//
//============================================================================//
float Watch1[N]={0};
float Watch2[N]={0};
float Watch3[N]={0};

void KalMan_Init(void)
{
	unsigned char   i;
	for (i=0; i<X_LENGTH; i++)
	{
		tOpt.XPreOpt[i] = Temp2[i];           //零值初始化
	}
	for (i=0; i<P_LENGTH; i++)
	{
		tCov.PPreOpt[i] = Temp4[i];           //零值初始化
	}
}
float KalMan(float input)
{
	unsigned char   i,j;
  //for (j=0; j<N; j++)
  {
    //Watch1[j] = 100 + j*2;
    //Watch1[j] = input;

    //Y[0] = Watch1[j] + Random1(0, 0.4);
   // Y[0] = Watch1[j] + (rand()%20)-10;
    Y[0] = input;
    //Watch2[j] = Y[0];
    MatrixMul(A, tOpt.XPreOpt, X, A_ROW, X_ROW, X_COLUMN);       //  基于系统的上一状态而预测现在状态; X(k|k-1) = A(k,k-1)*X(k-1|k-1)
    
    MatrixCal1(A, tCov.PPreOpt, Temp4, SYS_ORDER);
    MatrixAdd(Temp4, Q, P, P_ROW, P_COLUMN);                     //  预测数据的协方差矩阵; P(k|k-1) = A(k,k-1)*P(k-1|k-1)*A(k,k-1)'+Q
    
    MatrixCal2(C, P, Temp1, C_ROW, C_COLUMN);
    MatrixAdd(Temp1, R, Temp1, R_ROW, R_COLUMN);
    Gauss_Jordan(Temp1, C_ROW);
    MatrixTrans(C, Temp2, C_ROW, C_COLUMN);
    MatrixMul(P, Temp2, Temp22, P_ROW, C_COLUMN, C_ROW);
    MatrixMul(Temp22, Temp1, K, P_ROW, C_ROW, C_ROW);            //  计算卡尔曼增益; K(k) = P(k|k-1)*C' / (C(k)*P(k|k-1)*C(k)' + R)
    
    MatrixMul(C, X, Temp1, C_ROW, X_ROW, X_COLUMN);
    MatrixMinus(Y, Temp1, Temp1, Y_ROW, Y_COLUMN);
    MatrixMul(K, Temp1, Temp2, K_ROW, Y_ROW, Y_COLUMN);
    MatrixAdd(X, Temp2, tOpt.XNowOpt, X_ROW, X_COLUMN);          //  根据估测值和测量值计算当前最优值; X(k|k) = X(k|k-1)+Kg(k)*(Y(k)-C*X(k|k-1))
    
    MatrixMul(K, C, Temp4, K_ROW, C_ROW, C_COLUMN);
    MatrixMinus(I, Temp4, Temp4, I_ROW, I_COLUMN);
    MatrixMul(Temp4, P, tCov.PNowOpt, I_ROW, P_ROW, P_COLUMN);   //  计算更新后估计协防差矩阵; P(k|k) =（I-Kg(k)*C）*P(k|k-1)
    
    for (i=0; i<X_LENGTH; i++)
    {
      tOpt.XPreOpt[i] = tOpt.XNowOpt[i];
    }
    for (i=0; i<P_LENGTH; i++)
    {
      tCov.PPreOpt[i] = tCov.PNowOpt[i];
    }
   // Watch3[j] = tOpt.XNowOpt[0];
    
  }//end of for
  return tOpt.XNowOpt[0];
}





//============================================================================//
//==                  产生服从正态分布的随机数序列                          ==//
//============================================================================//
//==正态分布的概率密度函数  均值为u   方差为σ2(或标准差σ)                 ==//
//==                  Y=(x-μ)/σ ～ N(0,1)                                 ==//
//==入口参数: Num               产生随机数的个数                            ==//
//==          *S                保存随机数的数组                            ==//
//==          mu                需要的均值                                  ==//
//==          sigma             需要的方差                                  ==//
//==出口参数: *c                指向结果矩阵的指针                          ==//
//==返回值:   无                                                            ==//
//============================================================================//
void Random(unsigned long Num, float *S, float mu, float sigma)
{
  unsigned long  j;
  unsigned int   Temp;
  float r;
  
  for(j=0; j<Num; j++)                //循环产生随机数
  {
    Temp = rand();
    r = (Temp+0.00)/1073741823.00;
    S[j] = sqrt(-2*log(r))*cos(2*3.14159265*rand())*sigma+mu;
  }
}





//============================================================================//
//==                   产生一个服从正态分布的随机数                         ==//
//============================================================================//
//==入口参数: mu                需要的均值                                  ==//
//==          sigma             需要的方差                                  ==//
//==出口参数: 无                                                            ==//
//==返回值:   随机数                                                        ==//
//============================================================================//
float Random1(float mu, float sigma)
{
  unsigned int Temp;
  float r;
  float Ret;
  
  Temp = rand();
  r = (Temp+0.00)/1073741823.00;
  Ret = sqrt(-2*log(r))*cos(2*3.14159265*rand())*sigma+mu;
  
  return Ret;
}
