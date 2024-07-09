#ifndef _KALMAN_H
#define _KALMAN_H


#define   N           500
#define   SEED        1567


//===========================================================================================================//
//==  注意需要:  A_COLUMN==X_ROW   C_COLUMN==X_ROW
//==             可能需要改变中间变量的大小
//===========================================================================================================//
#define   SYS_ORDER   2



#define   A_ROW       2           //  定义A矩阵的行数
#define   A_COLUMN    2           //  定义A矩阵的列数
#define   A_LENGTH    4           //  保存A矩阵的数组的大小(A_ROW*A_COLUMN)
                                  //  A矩阵的初始值
                                  //  A矩阵保持不变(也可以对A进行迭代)
//#define   A_VALUE     {1, -0.01, 0, 1}
#define   A_VALUE     {1, -0.01, 0, 1}




//在某些情况下B矩阵可以不使用
#define   B_ROW       1           //  定义B矩阵的行数
#define   B_COLUMN    2           //  定义B矩阵的列数
#define   B_LENGTH    2           //  保存B矩阵的数组的大小(B_ROW*B_COLUMN)
                                  //  B矩阵的初始值
                                  //  B矩阵保持不变
//#define   B_VALUE     {0.01, 0}
#define   B_VALUE     {0.01, 0}




#define   C_ROW       1           //  定义C矩阵的行数
#define   C_COLUMN    2           //  定义C矩阵的列数
#define   C_LENGTH    2           //  保存C矩阵的数组的大小(C_ROW*C_COLUMN)
                                  //  C矩阵的初始值: C矩阵是观测转移矩阵, 例如可以进行单位转换,幅度改变等
                                  //  C矩阵保持不变
//#define   C_VALUE     {1, 0}
#define   C_VALUE     {1, 0}





#define   I_ROW       2           //  定义单位矩阵的行数
#define   I_COLUMN    2           //  定义单位矩阵的列数
#define   I_LENGTH    4           //  保存单位矩阵的数组的大小(I_ROW*I_COLUMN)
                                  //  I矩阵的初始值
#define   I_VALUE     {1, 0, 0, 1}





#define   K_ROW       2           //  定义K矩阵的行数
#define   K_COLUMN    1           //  定义K矩阵的列数
#define   K_LENGTH    2           //  保存K矩阵的数组的大小(K_ROW*K_COLUMN)
                                  //  K矩阵的初始值: K是卡尔曼增益,用于在计算过程中估计最优值,不是初始条件之一,可以任意赋值
                                  //  K矩阵会随着计算改变
#define   K_VALUE     {0, 0}





#define   P_ROW       2           //  定义P矩阵的行数
#define   P_COLUMN    2           //  定义P矩阵的列数
#define   P_LENGTH    4           //  保存P矩阵的数组的大小(P_ROW*P_COLUMN)
                                  //  P矩阵的初始值: P的初值是对第一次值的预测信任程度,任意赋值即可(不能为0);之后P的值是表示对预测值的信任程度;
                                  //  P矩阵会随着计算改变
//#define   P_VALUE     {1, 0, 0, 1}
#define   P_VALUE     {10, 0, 0, 10}





#define   Q_ROW       2           //  定义Q矩阵的行数
#define   Q_COLUMN    2           //  定义Q矩阵的列数
#define   Q_LENGTH    4           //  保存Q矩阵的数组的大小(Q_ROW*Q_COLUMN)
                                  //  Q矩阵的初始值: Q表示对根据上一次最优值所计算出的预测值的信任程度,Q越大表示越不信任;故Q越大越不平滑
                                  //  Q矩阵保持不变(Q与P的区别是Q保持不变,Q是根据模型得到,P是卡尔曼过程中计算得到)
//#define   Q_VALUE     {0.0001, 0, 0, 0}
#define   Q_VALUE     {0.01, 0, 0, 0}





#define   R_ROW       1           //  定义R矩阵的行数
#define   R_COLUMN    1           //  定义R矩阵的列数
#define   R_LENGTH    1           //  保存R矩阵的数组的大小(R_ROW*R_COLUMN)
                                  //  R矩阵的初始值: R表示本次测量值的噪声方差(也可以理解为对测量值的信任程度),R越大表示测量过程中的噪声越大;当R很小时,要适当减小Q的值,即对预测值的信任程度要加大
                                  //  R矩阵保持不变
//#define   R_VALUE     {0.4}
#define   R_VALUE     {0.4}





#define   X_ROW       2           //  定义X矩阵(估计值和最优值)的行数
#define   X_COLUMN    1           //  定义X矩阵(估计值和最优值)的列数
#define   X_LENGTH    2           //  保存X矩阵(估计值和最优值)的数组的大小(X_ROW*X_COLUMN)
                                  //  X矩阵的初始值: X的初值是对第一次值的预测,任意赋值即可,但尽量逼近真实的初始值;之后X的值是卡尔曼过程中预测值
                                  //  X矩阵会随着计算改变
//#define   X_VALUE     {1, 1}
#define   X_VALUE     {1, 1}





#define   Y_ROW       1           //  定义Y矩阵行数
#define   Y_COLUMN    1           //  定义Y矩阵的列数
#define   Y_LENGTH    1           //  保存Y矩阵的数组的大小(Y_ROW*Y_COLUMN)
                                  //  Y矩阵的初始值,不是初始化条件之一,任意赋值即可
                                  //  Y矩阵会随着计算改变
#define   Y_VALUE     {0}


extern float         Y[Y_LENGTH];                   //  测量值(每次测量的数据需要存入该数组)
extern float         I[I_LENGTH];                   //  单位矩阵
extern float         X[X_LENGTH];                   //  当前状态的预测值
extern float         P[P_LENGTH];                   //  当前状态的预测值的协方差
extern float         K[K_LENGTH];                   //  卡尔曼增益
extern float         Temp1[1];                      //  辅助变量
extern float         A[A_LENGTH];                   //  状态转移矩阵
extern float         B[B_LENGTH];                   //  系统参数
extern float         Q[Q_LENGTH];                   //  系统过程的协方差
extern float         C[C_LENGTH];                   //  观测矩阵转移矩阵
extern float         R[R_LENGTH];                   //  测量过程的协方差
extern float         Temp2[X_LENGTH];               //  辅助变量, 同时保存tOpt.XPreOpt[]的初始化值
extern float         Temp22[X_LENGTH];              //  辅助变量
extern float         Temp4[P_LENGTH];               //  辅助变量, 同时保存tCov.PPreOpt[]的初始化值


//extern void   Random(unsigned long Num, float *S, float mu, float sigma);
//extern float  Random1(float mu, float sigma);
extern float   KalMan(float);
extern void   KalMan_Init(void);

#endif
