#!/bin/bash

source .env

virsh -c "$LIBVIRT_URI" undefine "$TEST_DOMAIN_NAME"
virsh -c "$LIBVIRT_URI" net-undefine "$TEST_NETWORK_NAME"
rm domain-"$TEST_DOMAIN_NAME"-dumpxml.xml network-"$TEST_NETWORK_NAME"-dumpxml.xml .env
