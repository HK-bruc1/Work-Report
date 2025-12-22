# 打开涂鸦

除了打开BLE以及第三方协议中的涂鸦协议，没有添加pid,vid也会弹窗提示。有一个基本的界面。需要登录涂鸦管理平台去新建一个pid,vid。APP的操作界面不知道是不是也可以直接在这里直接配置。跟插件模块类似的？

`apps\common\third_party_profile\tuya_protocol\app\demo\tuya_ble_app_demo.h`

```c
#define APP_PRODUCT_ID          "tmodylku"

#define APP_BUILD_FIRMNAME      "tuya_ble_sdk_app_demo_nrf52832"

//固件版本
#define TY_APP_VER_NUM       0x0100
#define TY_APP_VER_STR	     "1.0"

//硬件版本
#define TY_HARD_VER_NUM      0x0100
#define TY_HARD_VER_STR	     "1.0"
```

- 通过这一个ID呈现不同的界面与功能。是区分其他设置的核心标识。

# 主图以及产品图分辨率要求

1、产品图要求:800*800px；最多可以上传6张

2、主图要求：尺寸要求：1011px*840px, 格式要求：png,jpeg,jpg

# 编译器版本要求

调试涂鸦不能使用2.4.9以及以后版本编译器，否则涂鸦app出现无法搜到ble情况

# 添加APP自定义按键



