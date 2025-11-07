export LIBGL_ALWAYS_SOFTWARE=1
export LD_LIBRARY_PATH=/tmp/protobuf-3.6.1/lib:$LD_LIBRARY_PATH
taskset -c 6,7 /home/karissa/haptech/core/bin/core