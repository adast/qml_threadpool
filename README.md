### Simple QML thread pool app

<p float="left">
  <img src="demo/1.png" width="300" />
  <img src="demo/2.png" width="300" /> 
  <img src="demo/3.png" width="300" />
</p>

#### Build and run from source
mkdir build && cd build \
cmake .. && make -j $(( $(nproc) + 1 )) \
./qml_threadpool

#### Build and run using docker
make
