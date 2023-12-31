#编译器 gcc, g++
CC = g++
#源文件目录 ./src/study_01
DIR_SRC=./src/gtest_code
#中间*.o文件目录
DIR_OBJ=./obj

RUN_FILE=test_1

#通过扩展通配符函数wildcard在多个原文件目录寻找源文件
# SrcFiles=$(wildcard ${DIR_SRC}/*.c) #  other/*.cpp # 针对多个文件
SrcFiles=$(wildcard ${DIR_SRC}/${RUN_FILE}.cpp) #  other/*.cpp # 针对单个文件
##通过模式替换函数patsubst与去除目录函数notdir获取目标文件列表
ObjFiles=$(patsubst %.c, ${DIR_OBJ}/%.o, $(notdir ${SrcFiles}))

#LIBS= -L ./lib(链接库路径) -l(链接库名) \
#	-L ./lib -l
#LIBS= -L ./lib -l mul -lpthread
# LIBS= -L ./lib -lsqlite3
LIBS= -L ./lib/googletest -lgtest

#头文件路径
# INCLUDE   = -I ./include/sqlite3
INCLUDE   = -I ./include/googletest
# 编译选项 -std=c11, -std=c++17 $(INCLUDE) $(LIBS)
CFLAGS    = -g -Wall -std=c++17 $(INCLUDE) $(LIBS)

#生成的可执行文件
# Target=main
Target=$(RUN_FILE)
all:$(Target)

#=========================================
# #目标文件:依赖于.o文件
# $(Target):$(ObjFiles)
# 	$(CC) $(ObjFiles) -o $@ $(CFLAGS)
# #$(CC) $^ -o $@ $(CFLAGS) #$(LIBS)
	
# #.o文件依赖于.c文件
# $(DIR_OBJ)/%.o:$(DIR_SRC)/%.cpp
# 	$(CC) -c $< -o $@ $(CFLAGS) 
#==========================================


#或一步到位
$(Target):$(SrcFiles)
	$(CC) $(SrcFiles) -o $@ $(CFLAGS)


# 清理
.PHONY:clean all
clean:
# win10
	del *.exe /q /s
	del $(DIR_OBJ)/*.o /q /s
# linux
#	rm -f $(Target).exe
#	rm -f $(DIR_OBJ)/*.o

#@ : 隐藏命令内容本身
#wildcard：，扩展通配符，就是获取指定的文件
#notdir ： 去除路径，把展开的文件去除掉路径信息 dir=(notdir(notdir(src))
#patsubst：替换通配符，将.c/.cpp替换为.o
#$(Files)，取File变量的值。
#$@ 目标文件
#$^ 全部依赖
#$< 第一个依赖
#$? 第一个变化的依赖
#
#g++ test.c -I./include/ -L/lib -l(库文件名libtest.a->test) -o test
# -I：指定头文件路径
# -L：指定链接库的路径