#!/bin/bash

export LD_LIBRARY_PATH=/home/lighthouse/rpclib/build/install/lib:$LD_LIBRARY_PATH

CORE_DUMP_DIR="/home/lighthouse/rpclib/build/install/coredump"
sudo mkdir -p $CORE_DUMP_DIR

# 设置权限
sudo chmod a+rw $CORE_DUMP_DIR

# 设置核心转储文件的路径和名称格式
# %e: 可执行文件名, %p: 进程ID, %t: 时间戳
echo "${CORE_DUMP_DIR}/core_%e.%p_%t" | sudo tee /proc/sys/kernel/core_pattern

# 确保核心转储大小没有限制
ulimit -c unlimited

# 输出当前设置
echo "Core dump path set to: $(cat /proc/sys/kernel/core_pattern)"

./bin/server


./bin/client
./bin/client
./bin/client
./bin/client