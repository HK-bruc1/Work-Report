# 调试后

调试后会有一张参数图类似：

![image-20250814184823991](./调试ANC注意事项.assets/image-20250814184823991.png)

代码上保持参数一致

`cpu\br36\audio\audio_config.h`

- DAC增益

![image-20250814190511293](./调试ANC注意事项.assets/image-20250814190511293.png)

`apps\earphone\board\br36\board_ac700n_demo_cfg.h`

- MIC的ANC动态增益。

![image-20250814190640751](./调试ANC注意事项.assets/image-20250814190640751.png)

其他暂时不用修改，根据机器实际情况修改。