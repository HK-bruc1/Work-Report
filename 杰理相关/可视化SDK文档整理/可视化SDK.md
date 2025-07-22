# cfg_tool.bin的更改时机

**使用可视化工具修改后，cfg_tool.bin并不会同步更改。只有点击编译时才会更新？只有可视化工具还是调用makefile构建脚本都可以？**

`customer_copy.bat`注释`cfg_tool.bin`的`copy`看，是不是makefile会改变`cfg_tool.bin`？

- 修改了蓝牙名，src\蓝牙配置.json中的配置会被修改。
  - 注释copy脚本的src目录后

**cfg_tool.bin才是决定蓝牙名的**

- 原来`make_prompt.bat`是用来被可视化工具调用的。
  - 新架构中没有调用此脚本
- 提示音是单独导出的，导出在output中
  - 估计也会被`make_prompt.bat`copy到SDK目录下参与编译。

![image-20250717144936522](./可视化SDK.assets/image-20250717144936522.png)

**点击菜单栏“编译”->”编译下载”，工具会先导出配置信息到工程目录/output文件夹，然后启动SDK的编译，编译完成后自动下载代码到开发板**

`cfg_tool.bin`文件的导出需要依靠可视化工具的编译功能。

# 工程目录结构说明

```c
工程目录/
     |
     -----.git            // 本地git仓库,请勿删除, 否则无法自动合并补丁
     |
     -----.gitignore      // git配置文件,请勿删除
     |
     -----src             // 配置项页面描述文件---------这个会实时同步
     |
     -----SDK             // SDK目录
     |
     -----output          // 存放配置工具导出的配置文件，SDK编译生成的ufw文件也会复制到此目录下
     |
     -----cache           // 缓存文件
     |
     -----project.jlproj  // 工程描述文件
```

# SDK编译说明

1. codeblock编译:

   codeblock编译使用的是cbp文件编译,添加和删除文件也是修改的cbp文件,不支持导出配置,有可能存在代码和配置文件不对应问题，所以不推荐使用

2. 配置工具编译:

   虽然codeblcok支持命令行方式启动编译,但是存在启动速度慢,闪屏等问题,所以配置工具使用Makefile文件编译，如果要新增文件,需要手动修改Makefile。 支持打开VS Code跳转到错误位置。

3. VS Code编译:

   在VS Code中打开”终端”, 先执行make_prompt.bat设置环境变量，之后可以通过make -j命令编译

# 顶部菜单功能说明

![image-20250718193033483](./可视化SDK.assets/image-20250718193033483.png)

- 说明不需要可视化工具的编译也可以更新配置文件？

## 配置文件说明

![image-20250718193158544](./可视化SDK.assets/image-20250718193158544.png)

**导出这一些？**

- 修改蓝牙名，工具会同步更新`src`目录以及`sdk_config.h和sdk_config.c`。
- 导出配置会得到
  - `apps\earphone\board\br56\jlstream_node_cfg.h`
  - 更新`src`目录以及`sdk_config.h和sdk_config.c`
  - `\output\stream.bin`
  - `\output\cfg_tool.bin`
- `tone_en.cfg`单独导出的