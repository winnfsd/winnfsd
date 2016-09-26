#!/usr/bin/env bash
sudo apt-get install software-properties-common
sudo add-apt-repository ppa:ondrej/php
apt-get update
sudo apt-get install -y git
sudo apt-get install -y php7.0-cli php7.0-common php7.0-curl php7.0-intl php7.0-mcrypt php7.0-readline php7.0-json
