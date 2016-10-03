#!/usr/bin/env bash
curl -sS https://getcomposer.org/installer | sudo php -- --disable-tls --install-dir=/usr/local/bin --filename=composer
composer require monolog/monolog