# MT7688AN_demo
基于openwrt系统开发的一些应用      
## 1.serial_demo编译步骤：     
```     
  * 设置交叉编译环境：     
    # export PATH=/opt/OpenWrt-Toolchain-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$PATH     
    # export STAGING_DIR=~/openwrt_widora/staging_dir/		（这个路径，取决于openwrt源码里面的staging_dir路径）     
  * 交叉编译:      
    # mipsel-openwrt-linux-gcc serial_test.c -lpthread -ldl -o serial_test  （编译应用程序serial_test）     
```         
## 2.mqtt_demo编译步骤:     
```     
  * mqtt源码生成动态库说明：      
    # libpaho-mqtt3a ------------------------》异步不支持SSL     
    # libpaho-mqtt3as -----------------------》异步支持SSL     
    # libpaho-mqtt3c ------------------------》同步不支持SSL     
    # libpaho-mqtt3cs -----------------------》同步支持SSL     
  * x86编译：      
    # mqtt_lib文件夹里面的动态库是编译paho mqtt源码生成的，mqtt_src文件夹里面的文件是从paho mqtt源码src目录里面抠取出来的。      
    # paho mqtt源码下载：git clone https://github.com/eclipse/paho.mqtt.c.git
    # gcc mqtt_test.c mqtt_client.c -I./mqtt_src -L./mqtt_lib_x86 -lpaho-mqtt3a -lpaho-mqtt3as -lpaho-mqtt3c -lpaho-mqtt3cs -o mqtt_test     
```     
