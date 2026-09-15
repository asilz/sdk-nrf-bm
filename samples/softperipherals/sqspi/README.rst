.. _sqspi_fatfs_sample:

sQSPI FAT Filesystem
#####################

.. contents::
   :local:
   :depth: 2

The sQSPI FAT filesystem sample demonstrates how to use the softperipheral :ref:`sQSPI` along with a FAT filesystem to format, mount, read, and write a file on the external SPI flash of a development kit.

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
           - ``bm_nrf54l15dk/nrf54l15/cpuapp/s115_softdevice/softperipheral``

.. note::
   The `nRF54LV10 DK`_ and `nRF54LS05 DK`_ do not have on-board external flash memory and are out of scope for this sample.
.. important::
   Before flashing the sample, you must enable :guilabel:`External memory` using the `Board Configurator`_ app in `nRF Connect for Desktop`_, and then write the configuration to the board.
   Otherwise, the on-board flash is not powered or routed to the SoC, and the sample will not work.

Overview
********

The sample uses the sQSPI softperipheral to drive the external flash memory in single and quad SPI modes, and uses the FatFs library to format the flash to FAT, and to mount, open, read from, write to, and close a file on it.

The flash is the on-board MX25-class NOR device on the development kit.
Its SPI pins and strap pins that configure fixed hardware settings, such as write-protect and reset state at power on, are defined in :file:`subsys/fs/fatfs/diskio.c`.

Reads and page programs are performed over quad SPI, while status, control, and erase commands are sent over single SPI.

Configuration
*************

This sample does not have any sample-specific Kconfig options.
The file name (``main.txt``) and the value written to it (``0xCAFEBABE``) are fixed in the source code at :file:`src/main.c`.

User interface
**************

LED 0:
   Lit when the file system is mounted.

LED 1:
   Lit while the file is opened.

LED 2:
   Lit while the FAT volume is being created.

Button 0:
   While Button 2 is pressed:
      Opens the file called ``main.txt``.
   While Button 2 is not pressed:
      Reads the first 4 bytes of the file and logs them as a 32-bit value.

Button 1:
   While Button 2 is pressed:
      Closes the file.
   While Button 2 is not pressed:
      Writes the 32-bit value ``0xCAFEBABE`` to the file.

Button 2:
   Used in combination with other buttons for alternate functionalities.

Button 3:
   While Button 2 is pressed:
      Mounts the file system.
   While Button 2 is not pressed:
      Creates a FAT volume on the logical drive.

Building and running
********************

This sample can be found under :file:`samples/softperipherals/sqspi/` in the |BMshort| folder structure.

For details on how to create, configure, and program a sample, see :ref:`getting_started_with_the_samples`.

Testing
=======

You can test this sample by performing the following steps:

1. Compile and program the application.
#. Observe that the ``sQSPI Sample started.`` message is printed.
#. Press **Button 3** to create a FAT volume on the external flash.
   **LED 2** lights up while the volume is being created and turns off once done.
   The log prints ``Created FAT volume on logical drive``.
#. Press **Button 3** while holding **Button 2** to mount the file system.
   **LED 0** turns on and the log prints ``Mounted file system``.
#. Press **Button 0** while holding **Button 2** to open ``main.txt``.
   **LED 1** turns on and the log prints ``Opened file``.
#. Press **Button 1** to write the value ``0xCAFEBABE`` to the file.
   The log prints ``Wrote Value 0xCAFEBABE``.
#. Press **Button 0** to read the value back from the file.
   The log prints ``Read Value 0xCAFEBABE``, confirming the write succeeded.
#. Press **Button 1** while holding **Button 2** to close the file.
   **LED 1** turns off and the log prints ``Closed file``.
#. Power-cycle the board, mount the file system again (**Button 3** + **Button 2**), open the file again (**Button 0** + **Button 2**), and press **Button 0** to read the value.
   The same value reappears, confirming the data persists across power cycles.
