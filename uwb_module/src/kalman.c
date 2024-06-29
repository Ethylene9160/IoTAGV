#include "kalman.h"
#include "stdlib.h"
#include "math.h"
#include "matrix.h"


/*==============================================================================
1.Ԥ����
   X(k|k-1) = A(k,k-1)*X(k-1|k-1)                 //������Ϊ0


2.����Ԥ����Э�������
   P(k|k-1) = A(k,k-1)*P(k-1|k-1)*A(k,k-1)'+Q(k)
   Q(k) = U(k)��U(k)'


3.���㿨�����������
   K(k) = P(k|k-1)*C(k)' / (C(k)*P(k|k-1)*C(k)' + R(k))
   R(k) = N(k)��N(k)'


4.���¹���
   X(k|k) = X(k|k-1)+K(k)*(Y(k)-C(k)*X(k|k-1))


5.������º����Э�������
   P(k|k) =��I-K(k)*C(k)��*P(k|k-1)


6. ��������ֵ


A(k,k-1):     ״̬ת�ƾ���
B(k,k-1):     ״̬�Ŀ�����
C(k):         �۲����ת�ƾ���
X(k|k-1):     ����k-1ʱ�̵�����ֵ����kʱ�̵�ֵ
X(k-1|k-1):   k-1ʱ�̵�����ֵ
P(k|k-1):     X(k|k-1)��Ӧ��covariance
P(k-1|k-1):   X(k-1|k-1)��Ӧ��covariance
Q(k):         ϵͳ���̵�covariance(����һ�β�������ֵ�����γ̶�)
R(k):         �������̵�Э����
Y(k):         kʱ�̵Ĳ���ֵ
K(k):         ����������
U(k):         kʱ�̶�̬����
N(k):         kʱ�̹۲�����


����:     ϵͳ״̬X-----------ʵ���¶�
          ϵͳ����A-----------�¶ȱ仯ת��
          ״̬�Ŀ�����B-------(ͨ��û��)
          �۲�ֵY-------------�¶ȼƶ���
          �۲����C-----------���϶�-�����϶�
          ��������Q-----------�¶ȱ仯ƫ��
          ��������R-----------�������

����˼·: ���ȸ�����һ��(����ǵ�һ�������Ԥ��ֵ����)�����ݼ�������εĹ���ֵ,
          ͬ��,������һ�ε����ݼ�������ι���ֵ��Э����;  ����,�ɱ��ι���ֵ��Э
          ������������������;  ���,���ݹ���ֵ�Ͳ���ֵ���㵱ǰ����ֵ����Э����
          ����,�������˲�ֻ��ʱ���ϵĴ���,������ΪƵ��Ӱ���С
==============================================================================*/



//================================================//
//==             ����ֵ����ṹ��               ==//
//================================================//
typedef struct  _tCovariance
{
    float PNowOpt[P_LENGTH];
    float PPreOpt[P_LENGTH];
}tCovariance;



//================================================//
//==               ����ֵ�ṹ��                 ==//
//================================================//
typedef struct  _tOptimal
{
    float XNowOpt[X_LENGTH];
    float XPreOpt[X_LENGTH];
}tOptimal;



tOptimal      tOpt;                                     //  ���˲������н��г�ʼ����ֵ
tCovariance   tCov;                                     //  ���˲������н��г�ʼ����ֵ
float         Y[Y_LENGTH]  = Y_VALUE;                   //  ����ֵ(ÿ�β�����������Ҫ���������)
float         I[I_LENGTH]  = I_VALUE;                   //  ��λ����
float         X[X_LENGTH]  = X_VALUE;                   //  ��ǰ״̬��Ԥ��ֵ
float         P[P_LENGTH]  = P_VALUE;                   //  ��ǰ״̬��Ԥ��ֵ��Э����
float         K[K_LENGTH]  = K_VALUE;                   //  ����������
float         Temp1[1]     = {0};                       //  ��������
//============================================================================//
//==                    �������˲���Ҫ���õı���                            ==//
//============================================================================//
float         A[A_LENGTH]       = A_VALUE;              //  ״̬ת�ƾ���
float         B[B_LENGTH]       = B_VALUE;              //  ϵͳ����
float         Q[Q_LENGTH]       = Q_VALUE;              //  ϵͳ���̵�Э����
float         C[C_LENGTH]       = C_VALUE;              //  �۲����ת�ƾ���
float         R[R_LENGTH]       = R_VALUE;              //  �������̵�Э����
float         Temp2[X_LENGTH]   = X_VALUE;              //  ��������, ͬʱ����tOpt.XPreOpt[]�ĳ�ʼ��ֵ
float         Temp22[X_LENGTH]  = X_VALUE;              //  ��������
float         Temp4[P_LENGTH]   = P_VALUE;              //  ��������, ͬʱ����tCov.PPreOpt[]�ĳ�ʼ��ֵ


//============================================================================//
//==                          �������˲�                                    ==//
//============================================================================//
//==��ڲ���: ��                                                            ==//
//==���ڲ���: ��                                                            ==//
//==����ֵ:   ��                                                            ==//
//============================================================================//
float Watch1[N]={0};
float Watch2[N]={0};
float Watch3[N]={0};

void KalMan_Init(void)
{
    unsigned char   i;
    for (i=0; i<X_LENGTH; i++)
    {
        tOpt.XPreOpt[i] = Temp2[i];           //��ֵ��ʼ��
    }
    for (i=0; i<P_LENGTH; i++)
    {
        tCov.PPreOpt[i] = Temp4[i];           //��ֵ��ʼ��
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
        MatrixMul(A, tOpt.XPreOpt, X, A_ROW, X_ROW, X_COLUMN);       //  ����ϵͳ����һ״̬��Ԥ������״̬; X(k|k-1) = A(k,k-1)*X(k-1|k-1)

        MatrixCal1(A, tCov.PPreOpt, Temp4, SYS_ORDER);
        MatrixAdd(Temp4, Q, P, P_ROW, P_COLUMN);                     //  Ԥ�����ݵ�Э�������; P(k|k-1) = A(k,k-1)*P(k-1|k-1)*A(k,k-1)'+Q

        MatrixCal2(C, P, Temp1, C_ROW, C_COLUMN);
        MatrixAdd(Temp1, R, Temp1, R_ROW, R_COLUMN);
        Gauss_Jordan(Temp1, C_ROW);
        MatrixTrans(C, Temp2, C_ROW, C_COLUMN);
        MatrixMul(P, Temp2, Temp22, P_ROW, C_COLUMN, C_ROW);
        MatrixMul(Temp22, Temp1, K, P_ROW, C_ROW, C_ROW);            //  ���㿨��������; K(k) = P(k|k-1)*C' / (C(k)*P(k|k-1)*C(k)' + R)

        MatrixMul(C, X, Temp1, C_ROW, X_ROW, X_COLUMN);
        MatrixMinus(Y, Temp1, Temp1, Y_ROW, Y_COLUMN);
        MatrixMul(K, Temp1, Temp2, K_ROW, Y_ROW, Y_COLUMN);
        MatrixAdd(X, Temp2, tOpt.XNowOpt, X_ROW, X_COLUMN);          //  ���ݹ���ֵ�Ͳ���ֵ���㵱ǰ����ֵ; X(k|k) = X(k|k-1)+Kg(k)*(Y(k)-C*X(k|k-1))

        MatrixMul(K, C, Temp4, K_ROW, C_ROW, C_COLUMN);
        MatrixMinus(I, Temp4, Temp4, I_ROW, I_COLUMN);
        MatrixMul(Temp4, P, tCov.PNowOpt, I_ROW, P_ROW, P_COLUMN);   //  ������º����Э�������; P(k|k) =��I-Kg(k)*C��*P(k|k-1)

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
//==                  ����������̬�ֲ������������                          ==//
//============================================================================//
//==��̬�ֲ��ĸ����ܶȺ���  ��ֵΪu   ����Ϊ��2(���׼���)                 ==//
//==                  Y=(x-��)/�� �� N(0,1)                                 ==//
//==��ڲ���: Num               ����������ĸ���                            ==//
//==          *S                ���������������                            ==//
//==          mu                ��Ҫ�ľ�ֵ                                  ==//
//==          sigma             ��Ҫ�ķ���                                  ==//
//==���ڲ���: *c                ָ���������ָ��                          ==//
//==����ֵ:   ��                                                            ==//
//============================================================================//
void Random(unsigned long Num, float *S, float mu, float sigma)
{
    unsigned long  j;
    unsigned int   Temp;
    float r;

    for(j=0; j<Num; j++)                //ѭ�����������
    {
        Temp = rand();
        r = (Temp+0.00)/1073741823.00;
        S[j] = sqrt(-2*log(r))*cos(2*3.14159265*rand())*sigma+mu;
    }
}





//============================================================================//
//==                   ����һ��������̬�ֲ��������                         ==//
//============================================================================//
//==��ڲ���: mu                ��Ҫ�ľ�ֵ                                  ==//
//==          sigma             ��Ҫ�ķ���                                  ==//
//==���ڲ���: ��                                                            ==//
//==����ֵ:   �����                                                        ==//
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
