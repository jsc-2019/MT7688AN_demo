# MT7688AN_demo
基于openwrt系统开发的一些应用      
## 1.serial_demo编译步骤：     
```     
  * 设置交叉编译环境：     
    # export PATH=/opt/OpenWrt-Toolchain-ramips-mt7688_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$PATH     
    # export STAGING_DIR=~/openwrt_widora/staging_dir/		（这个路径，取决于openwrt源码里面的staging_dir路径）     
  * 交叉编译:      
    # mipsel-openwrt-linux-gcc serial_test.c -lpthread -ldl -o serial_test  （编译应用程序serial_test）     
    
