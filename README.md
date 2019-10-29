# open62541-influxdb
Automatical logging of OPC UA Variables to InfluxDB

## Dependencies

libconfig++

```bash
sudo apt install libconfig++-dev
```

CLI11:

```bash
mkdir /tmp/cli11_tmp_install
cd /tmp/cli11_tmp_install
git clone --branch v1.7.1 https://github.com/CLIUtils/CLI11.git
cd /tmp/cli11_tmp_install/CLI11
mkdir build
cd /tmp/cli11_tmp_install/CLI11/build
cmake -DCLI11_TESTING=OFF -DCLI11_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_SHARED_LIBS=ON ..
sudo make -j install
rm -rf /tmp/cli11_tmp_install
```

influxdb-cxx:

```bash
mkdir /tmp/influxdb-cxx_install
cd /tmp/influxdb-cxx_install
git clone https://github.com/awegrzyn/influxdb-cxx.git
cd influxdb-cxx; mkdir build
cd build
cmake ..
sudo make -j install
rm -rf /tmp/influxdb-cxx_install
```