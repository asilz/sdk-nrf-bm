.. _sQSPI_sample:

sQSPI
###########

.. contents::
   :local:
   :depth: 2

The sQSPI sample demonstrates how to use the :ref:`soft_peripherals` library with |BMlong| to perform read, page program, and sector erase operations on the external flash memory of the development kit.

Requirements
************

The sample supports the following development kits with on-board SPI external flash:

.. tabs::

   .. group-tab:: Simple board variants

      The following board variants do not have DFU capabilities:

      **S115**:

      .. list-table::
         :header-rows: 1

         * - Hardware platform
           - PCA
           - Board target
         * - `nRF54L15 DK`_
           - PCA10156
           - ``bm_nrf54l15dk/nrf54l15/cpuapp/s115_softdevice``
         * - `nRF54L15 DK`_ (emulating nRF54L10)
           - PCA10156
           - ``bm_nrf54l15dk/nrf54l10/cpuapp/s115_softdevice``
         * - `nRF54L15 DK`_ (emulating nRF54L05)
           - PCA10156
           - ``bm_nrf54l15dk/nrf54l05/cpuapp/s115_softdevice``
         * - `nRF54LM20 DK`_
           - PCA10184
           - ``bm_nrf54lm20dk/nrf54lm20a/cpuapp/s115_softdevice``



.. note::
   The `nRF54LV10 DK`_ and `nRF54LS05 DK`_ do not have on-board external flash memory and are out of scope for this sample.
   However, the :ref:`soft_peripherals` library itself is fully supported on the nRF54LV10 and nRF54LS05 SoCs.
   This sample is specific to the on-board MX25R6435F flash and its board pin assignments. It does not support these kits without adapting the flash commands, pin assignments, and timing for a connected device.

.. important::
   Before flashing the sample, you must enable :guilabel:`External memory` using the `Board Configurator`_ app in `nRF Connect for Desktop`_, and then write the configuration to the board.
   Otherwise, the on-board flash is not powered or routed to the SoC, and the sample will not work.

Overview
********

The sample uses the sQSPI driver to communicate with the on-board MX25R6435F NOR flash. Reads and page programs use quad SPI; erase and setup commands use single SPI.
All operations target the same flash address (set by the :kconfig:option:`CONFIG_SAMPLE_SQSPI_FLASH_ADDR` Kconfig option). Pressing Button 3 erases the 4 KiB sector at that address before programming the configured message.

The flash pins, including write-protect and reset, are defined by the ``BOARD_EXTERNAL_MEMORY_*`` macros in the :file:`board-config.h` file.

Configuration
*************

You can modify the following options (available in the Kconfig file at :file:`samples/softperipherals/sqspi/Kconfig`):

* :kconfig:option:`CONFIG_SAMPLE_SQSPI_MSG` - Sets the message written to the external flash on program.
* :kconfig:option:`CONFIG_SAMPLE_SQSPI_FLASH_ADDR` - Sets the external flash address targeted by read, program, and erase.
  Keep it a multiple of 0x1000 (4 KiB sector size) to avoid a wrapped, corrupted write.

User interface
**************

LED 0:
   Lit after flash and button initialization succeeds.

Button 1:
   Erase external memory.

Button 2:
   Read external memory.

Button 3:
   Program external memory.

Building and running
********************

This sample can be found under :file:`samples/softperipherals/sqspi/` in the |BMshort| folder structure.

For details on how to create, configure, and program a sample, see :ref:`getting_started_with_the_samples`.

Testing
=======

You can test this sample by performing the following steps:

1. Compile and program the application.
#. Observe that the ``sQSPI sample initialized`` message is printed, followed by the list of available buttons and their operations.
#. Press **Button 2** to read the on-board flash.
   The hex dump covers the configured message length and shows either ``0xFF`` bytes (erased) or the data last written at the address.
#. Press **Button 3** to erase the 4 KiB sector at the configured address and program the configured message (set with :kconfig:option:`CONFIG_SAMPLE_SQSPI_MSG`, default ``Hello World!``).
   The log reports the erased sector address and then ``Flash operation complete``.
#. Press **Button 2** again.
   The hex dump now contains the message bytes, confirming the write succeeded.
   For the default configuration, the bytes are ``48 65 6c 6c 6f 20 57 6f 72 6c 64 21 00`` (ASCII for ``Hello World!`` followed by the string terminator).
#. Power-cycle the board and press **Button 2**.
   The same message reappears, confirming the data persists across power cycles.
#. Press **Button 1** to erase the sector and wait for the flash's self-timed erase operation to complete.
#. Power-cycle the board again and press **Button 2**.
   The hex dump shows all ``0xFF``, confirming the erase succeeded.
