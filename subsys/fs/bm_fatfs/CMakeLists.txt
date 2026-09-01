#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
zephyr_library()
zephyr_library_sources(
  diskio.c
  ${ZEPHYR_NRFXLIB_MODULE_DIR}/../fatfs/ff.c
)

zephyr_include_directories_ifdef(CONFIG_BM_FATFS
  "${ZEPHYR_NRFXLIB_MODULE_DIR}/../fatfs/include"
)

zephyr_compile_definitions(FS_FATFS_WINDOW_ALIGNMENT=CONFIG_FS_FATFS_WINDOW_ALIGNMENT)
