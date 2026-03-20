.. _ble_hrs_peripheral_central_sample:

Bluetooth: Heart Rate Service Peripheral Central
################################################

.. contents::
   :local:
   :depth: 2

The Heart Rate Service Peripheral Central sample demonstrates how you can implement the Heart Rate profile as a server and client using |BMlong|.

Requirements
************

The sample supports the following development kits:

.. tabs::

   .. group-tab:: Simple board variants

      The following board variants do **not** have DFU capabilities:

      .. include:: /includes/supported_boards_all_non-mcuboot_variants_s145.txt

   .. group-tab:: MCUboot board variants

      The following board variants have DFU capabilities:

      .. include:: /includes/supported_boards_all_mcuboot_variants_s145.txt

Overview
********

This sample advertises and scans for devices that advertise with the :ref:`lib_ble_service_hrs` UUID (0x180D) and initiates a connection when a device is found.
When a peripheral device is connected, the sample starts the service discovery procedure.
If this succeeds, the sample subscribes to the Heart Rate Measurement characteristic to receive heart rate notifications.
If a central connects to this device the central subscribes to the Heart Rate Measurement characteristic to receive heart rate notifications forwarded from the peripheral device.

.. _ble_hrs_peripheral_central_sample_testing:

User interface
**************

Button 0:
  Press to disable allow list.

Button 1:
  Press to disconnect from the connected peripheral device.

  Keep the button pressed while resetting the board to delete bonding information for all peers stored on the device.

Button 2:
  Press to disconnect from the connected central device.

LED 0:
   Lit when the device is initialized.

LED 1:
   Lit when a peripheral device is connected.

LED 2:
   Lit when a central device is connected.

Building and running
********************

This sample can be found under :file:`samples/bluetooth/ble_hrs_peripheral_central/` in the |BMshort| folder structure.

For details on how to create, configure, and program a sample, see :ref:`getting_started_with_the_samples`.

Testing
=======

We can test this sample with three devices, one running this sample, another one running the :ref:`ble_hrs_sample` sample, and a central, for example, a phone or a tablet with `nRF Connect for Mobile`_ or `nRF Toolbox`_.

Complete the following steps to test the sample:

1. Compile and program the application.
#. Observe that the ``BLE HRS Peripheral Central sample initialized`` message is printed.
#. In the Serial Terminal, observe that the ``Advertising as nRF_BM_HRS_bridge`` message is printed.
   You can configure the advertising name using the :kconfig:option:`CONFIG_SAMPLE_BLE_DEVICE_NAME` Kconfig option.
   For information on how to do this, see `Configuring Kconfig`_.
#. Program the second development kit with the :ref:`ble_hrs_sample` sample.
#. Observe that the ``Scan filter match`` message is printed, followed by ``Connecting to target`` and ``Connected``, when connecting to the peripheral device.
#. Observe that the ``Heart rate service discovered.`` message is printed.
#. Connect to the device from nRF Connect (the device is advertising as "nRF_BM_HRS_bridge").
#. Observe that the ``Connecting to target`` and ``Connected`` messages are printed when connecting to the central device.
#. Observe that the device starts receiving heart rate measurement notifications and forwarding them to the central.
