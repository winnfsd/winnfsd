#!/usr/bin/env bash
cd /home/vagrant/
git clone https://github.com/marcharding/connectathon-winnfsd.git
cd /home/vagrant/connectathon-winnfsd
git checkout connectathon-winnfsd
sudo make
export WINNFSD=yes
export HARDLINKS=no
export CIFS=yes
export GB2=yes
sudo -E ./server -o rw,vers=3,udp,nolock 192.168.56.1
sudo -E ./server -o rw,vers=3,tcp,nolock 192.168.56.1