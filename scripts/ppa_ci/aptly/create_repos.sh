#!/bin/bash

set -e
for i in bionic buster stretch xenial
do
    aptly repo create -distribution=$i -component=main yadedaily-$i
done
