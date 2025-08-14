# bin文件格式

## ANC文件相关位置

`DHF-ac700-v138\cpu\br36\tools`

直接同名替换。

- 可视化可以直接替换
- 非可视化需要使用配置工具加载并保存一下anc_gains参数
  - 注意**anc对应的dac的模拟增益**是否跟代码中的模拟音量是否一致，不一致需要备注一下，声学修改。
  - `cpu\br36\audio\audio_config.h`
  
- anc_gains
- anc_coeff

~~`cpu\br36\tools\download\earphone`~~

- 这个目录下的没有用。

需要把ANC压缩包存起来。

