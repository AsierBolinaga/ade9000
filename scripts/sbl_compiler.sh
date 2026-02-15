#!/bin/bash

PRJ_TOPDIR="${PWD}/../.."

if [[ -d "${PRJ_TOPDIR}/adsn_sbl/target/evkmimxrt1060/gcc-arm-none-eabi-9-2019-q4-major" ]]; then
    echo "Compiler direcotry exists"
    # Add your actions here, such as reading the file or processing its contents
else
    echo "Compiler direcotry does not exists"
    wget -P "${PRJ_TOPDIR}/adsn_sbl/target/evkmimxrt1060/" "https://nexus.internal.ainguraiiot.com/repository/Software/toolchains/remote_sensing/sbl/gcc-arm-none-eabi-9-2019-q4-major.tar.xz" --no-check-certificate
    sleep 15  # Esperar 5 segundos antes de volver a comprobar
    tar -xJf "${PRJ_TOPDIR}/adsn_sbl/target/evkmimxrt1060/gcc-arm-none-eabi-9-2019-q4-major.tar.xz" -C "${PRJ_TOPDIR}/adsn_sbl/target/evkmimxrt1060/"
fi