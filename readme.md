# 说明
如何使用libcoap <br/>
**作者** 徐凯 <br/>

# libcoap安装概述
* git clone https://github.com/obgm/libcoap.git
* cd libcoap
* autogen.sh
* ./configure --disable-documentation
* make
* sudo make install

# 示例
## tiny-server

## simple-server
CoAP Server示例 <br />
该示例修改为libcoap/example/coap-server <br />
### 编译和运行
```
cd simple-server
# 生成makefile文件
cmake .
# 执行makefile，生成可执行文件 server和clinet
make
# 启动server
./server
# 启动client
./client
```

