#!/bin/bash

if [ -f ".env" ]; then
  rm .env
fi

## Defines all variables that will be used in tests ##
{
  echo "LIBVIRT_URI=qemu:///system"
  echo "API_URI=http://127.0.0.1:8081/libvirt"
  echo "ARCH=$(uname -m)"
  echo "TEST_DOMAIN_NAME=test"
  echo "TEST_NETWORK_NAME=test"
} >>.env

source .env

virsh -c "$LIBVIRT_URI" define domain-"$ARCH"-host.xml
virsh -c "$LIBVIRT_URI" net-define network.xml

virsh -c "$LIBVIRT_URI" dumpxml "$TEST_DOMAIN_NAME" >domain-"$TEST_DOMAIN_NAME"-dumpxml.xml
virsh -c "$LIBVIRT_URI" net-dumpxml "$TEST_NETWORK_NAME" >network-"$TEST_NETWORK_NAME"-dumpxml.xml
