# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)
# 项目信息
project (YeelightSer)
# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加 util 子目录
add_subdirectory(util)

# 指定生成目标
add_executable(YeelightSer ${DIR_SRCS})

target_link_libraries(YeelightSer event)

# 添加链接库
target_link_libraries(YeelightSer util)


#make intall 
install (TARGETS YeelightSer DESTINATION bin)
