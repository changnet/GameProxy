#gameproxy

install libev first

apt-get install libev-dev
or
http://software.schmorp.de/pkg/libev.html

cd build

make BIT=64 or make BIT=32

you may want to edit config file session.json first
and then start

./gameproxy -s session.json
