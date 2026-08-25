#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
zephyr_library()
zephyr_library_sources(
  diskio.c
)

zephyr_include_directories_ifdef(CONFIG_SQSPI
  "${ZEPHYR_NRFXLIB_MODULE_DIR}/../fatfs/include"
)
