#!/bin/sh

set -x
set -e

sh grammar.sh

sh rerun.sh
