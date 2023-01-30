
cmake_minimum_required(VERSION 2.8.12)

project(sample_db)

#set(CMAKE_BUILD_TYPE "Debug")
set(OS "win32 linux arm" CACHE STRING "none")
set(CMAKE_C_FLAGS "-g -O0 -pthread -fPIC")
set(CMAKE_CXX_FLAGS "-O2 -fPIC -std=c++11")	


#hi3516DV300
#[[
set(CMAKE_FIND_ROOT_PATH /opt/hisi-linux/x86-arm/arm-himix200-linux)
set(CMAKE_C_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-gcc)
set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-g++)
link_directories("${PROJECT_SOURCE_DIR}/../lib/hisi/hi3516dv300/")
include_directories("${PROJECT_SOURCE_DIR}/../include")
add_executable(sample_acs "sample.c" )
target_link_libraries(sample_acs  -lACSCloud -lcurl -lmosquitto  -lcares  -lssl -lcrypto -lrt  -ldl -lm -lpthread)
]]
#hi3516DV300


#nt98566
##[[
set(CMAKE_FIND_ROOT_PATH /opt/arm-ca9-linux-uclibcgnueabihf-8.4.01)
set(CMAKE_C_COMPILER /opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-gcc)
set(CMAKE_CXX_COMPILER /opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-g++)
link_directories("${PROJECT_SOURCE_DIR}/../")
link_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/curl/lianyong/nt98566/lib")
link_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/mqtt/lianyong/nt98566/lib")
link_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/openssl/lianyong/nt98566/lib")
include_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/zlog/lianyong/nt98566/include")
include_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/curl/lianyong/nt98566/include")
include_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/mqtt/lianyong/nt98566/include")
include_directories("${PROJECT_SOURCE_DIR}/../../dcit-lib/third_party/openssl/lianyong/nt98566/include")
include_directories("${PROJECT_SOURCE_DIR}/../include")
add_executable(sample_acs "sample.c" )
target_link_libraries(sample_acs  -lacscloud -lcurl -lmosquitto -lssl -lcrypto -lrt  -ldl -lm -lpthread)
#]]
#nt98566

#target_link_libraries(sample_acs  -lACSCloud -lcurl -lmosquitto -luuid -lcares -lssl -lcrypto -lrt  -ldl -lm -lpthread)
#zlp -lZLPACSCloud -ldl -lcurl -lwebsockets -lssl -lcrypto -lm -lpthread
#acs -lACSCloud -lmosquitto -luuid -lcares -lssl -lcrypto -lrt -lcurl -ldl -lm -lpthread 







