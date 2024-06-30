The code corresponding to TAG is in the uwb_target_module_keil folder. If you need to change it to Anchor0, please modify 179-188 lines:

```c
//#define TAG
#define TAG_ID 0x0F
#define MASTER_TAG 0x0F
#define MAX_SLAVE_TAG 0x02
#define SLAVE_TAG_START_INDEX 0x01

#define ANTHOR
#define ANCHOR_MAX_NUM 3
#define ANCHOR_IND 0  // 0 1 2
//#define ANCHOR_IND ANCHOR_NUM
```

For Anchor1, it should be:

```C
//#define TAG
#define TAG_ID 0x0F
#define MASTER_TAG 0x0F
#define MAX_SLAVE_TAG 0x02
#define SLAVE_TAG_START_INDEX 0x01

#define ANTHOR
#define ANCHOR_MAX_NUM 3
#define ANCHOR_IND 1  // 0 1 2
//#define ANCHOR_IND ANCHOR_NUM
```

