#ifndef _KALMAN_H
#define _KALMAN_H


#define   N           500
#define   SEED        1567


//===========================================================================================================//
//==  ע����Ҫ:  A_COLUMN==X_ROW   C_COLUMN==X_ROW
//==             ������Ҫ�ı��м�����Ĵ�С
//===========================================================================================================//
#define   SYS_ORDER   2



#define   A_ROW       2           //  ����A���������
#define   A_COLUMN    2           //  ����A���������
#define   A_LENGTH    4           //  ����A���������Ĵ�С(A_ROW*A_COLUMN)
//  A����ĳ�ʼֵ
//  A���󱣳ֲ���(Ҳ���Զ�A���е���)
//#define   A_VALUE     {1, -0.01, 0, 1}
#define   A_VALUE     {1, -0.01, 0, 1}




//��ĳЩ�����B������Բ�ʹ��
#define   B_ROW       1           //  ����B���������
#define   B_COLUMN    2           //  ����B���������
#define   B_LENGTH    2           //  ����B���������Ĵ�С(B_ROW*B_COLUMN)
//  B����ĳ�ʼֵ
//  B���󱣳ֲ���
//#define   B_VALUE     {0.01, 0}
#define   B_VALUE     {0.01, 0}




#define   C_ROW       1           //  ����C���������
#define   C_COLUMN    2           //  ����C���������
#define   C_LENGTH    2           //  ����C���������Ĵ�С(C_ROW*C_COLUMN)
//  C����ĳ�ʼֵ: C�����ǹ۲�ת�ƾ���, ������Խ��е�λת��,���ȸı��
//  C���󱣳ֲ���
//#define   C_VALUE     {1, 0}
#define   C_VALUE     {1, 0}





#define   I_ROW       2           //  ���嵥λ���������
#define   I_COLUMN    2           //  ���嵥λ���������
#define   I_LENGTH    4           //  ���浥λ���������Ĵ�С(I_ROW*I_COLUMN)
//  I����ĳ�ʼֵ
#define   I_VALUE     {1, 0, 0, 1}





#define   K_ROW       2           //  ����K���������
#define   K_COLUMN    1           //  ����K���������
#define   K_LENGTH    2           //  ����K���������Ĵ�С(K_ROW*K_COLUMN)
//  K����ĳ�ʼֵ: K�ǿ���������,�����ڼ�������й�������ֵ,���ǳ�ʼ����֮һ,�������⸳ֵ
//  K��������ż���ı�
#define   K_VALUE     {0, 0}





#define   P_ROW       2           //  ����P���������
#define   P_COLUMN    2           //  ����P���������
#define   P_LENGTH    4           //  ����P���������Ĵ�С(P_ROW*P_COLUMN)
//  P����ĳ�ʼֵ: P�ĳ�ֵ�ǶԵ�һ��ֵ��Ԥ�����γ̶�,���⸳ֵ����(����Ϊ0);֮��P��ֵ�Ǳ�ʾ��Ԥ��ֵ�����γ̶�;
//  P��������ż���ı�
//#define   P_VALUE     {1, 0, 0, 1}
#define   P_VALUE     {10, 0, 0, 10}





#define   Q_ROW       2           //  ����Q���������
#define   Q_COLUMN    2           //  ����Q���������
#define   Q_LENGTH    4           //  ����Q���������Ĵ�С(Q_ROW*Q_COLUMN)
//  Q����ĳ�ʼֵ: Q��ʾ�Ը�����һ������ֵ���������Ԥ��ֵ�����γ̶�,QԽ���ʾԽ������;��QԽ��Խ��ƽ��
//  Q���󱣳ֲ���(Q��P��������Q���ֲ���,Q�Ǹ���ģ�͵õ�,P�ǿ����������м���õ�)
//#define   Q_VALUE     {0.0001, 0, 0, 0}
#define   Q_VALUE     {0.01, 0, 0, 0}





#define   R_ROW       1           //  ����R���������
#define   R_COLUMN    1           //  ����R���������
#define   R_LENGTH    1           //  ����R���������Ĵ�С(R_ROW*R_COLUMN)
//  R����ĳ�ʼֵ: R��ʾ���β���ֵ����������(Ҳ�������Ϊ�Բ���ֵ�����γ̶�),RԽ���ʾ���������е�����Խ��;��R��Сʱ,Ҫ�ʵ���СQ��ֵ,����Ԥ��ֵ�����γ̶�Ҫ�Ӵ�
//  R���󱣳ֲ���
//#define   R_VALUE     {0.4}
#define   R_VALUE     {0.4}





#define   X_ROW       2           //  ����X����(����ֵ������ֵ)������
#define   X_COLUMN    1           //  ����X����(����ֵ������ֵ)������
#define   X_LENGTH    2           //  ����X����(����ֵ������ֵ)������Ĵ�С(X_ROW*X_COLUMN)
//  X����ĳ�ʼֵ: X�ĳ�ֵ�ǶԵ�һ��ֵ��Ԥ��,���⸳ֵ����,�������ƽ���ʵ�ĳ�ʼֵ;֮��X��ֵ�ǿ�����������Ԥ��ֵ
//  X��������ż���ı�
//#define   X_VALUE     {1, 1}
#define   X_VALUE     {1, 1}





#define   Y_ROW       1           //  ����Y��������
#define   Y_COLUMN    1           //  ����Y���������
#define   Y_LENGTH    1           //  ����Y���������Ĵ�С(Y_ROW*Y_COLUMN)
//  Y����ĳ�ʼֵ,���ǳ�ʼ������֮һ,���⸳ֵ����
//  Y��������ż���ı�
#define   Y_VALUE     {0}


extern float         Y[Y_LENGTH];                   //  ����ֵ(ÿ�β�����������Ҫ���������)
extern float         I[I_LENGTH];                   //  ��λ����
extern float         X[X_LENGTH];                   //  ��ǰ״̬��Ԥ��ֵ
extern float         P[P_LENGTH];                   //  ��ǰ״̬��Ԥ��ֵ��Э����
extern float         K[K_LENGTH];                   //  ����������
extern float         Temp1[1];                      //  ��������
extern float         A[A_LENGTH];                   //  ״̬ת�ƾ���
extern float         B[B_LENGTH];                   //  ϵͳ����
extern float         Q[Q_LENGTH];                   //  ϵͳ���̵�Э����
extern float         C[C_LENGTH];                   //  �۲����ת�ƾ���
extern float         R[R_LENGTH];                   //  �������̵�Э����
extern float         Temp2[X_LENGTH];               //  ��������, ͬʱ����tOpt.XPreOpt[]�ĳ�ʼ��ֵ
extern float         Temp22[X_LENGTH];              //  ��������
extern float         Temp4[P_LENGTH];               //  ��������, ͬʱ����tCov.PPreOpt[]�ĳ�ʼ��ֵ


//extern void   Random(unsigned long Num, float *S, float mu, float sigma);
//extern float  Random1(float mu, float sigma);
extern float   KalMan(float);
extern void   KalMan_Init(void);

#endif
