# lhft
lighthouse matching engine test

### Steps to build
git clone https://github.com/chavanrc/lhft.git
cd lhft/
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
make

if want to perform benchmarks then build with -DBENCHMARK_ENABLED=ON
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLED=ON ..
make

### To Run:
You can find your lib and test exe in build/stage

./tests/bin/Tests -g ../../tests/input.csv "lhft file test"

To run benchmarks
./tests/bin/Tests "me benchmark test"

### You can use docker as well
docker build -f ./docker/Dockerfile -t="ubuntu:lhft" .
docker run -it -v /home/{user}/CLionProjects/lhft/:/home/user/lhft --cap-add=SYS_PTRACE ubuntu:lhft bash