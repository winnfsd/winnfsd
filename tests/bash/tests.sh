#!/usr/bin/env bash
SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
$SCRIPTPATH/connectathon.sh
$SCRIPTPATH/git.sh
$SCRIPTPATH/composer.sh
