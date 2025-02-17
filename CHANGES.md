Changes in PAPPL
================

Changes in v1.4.0
-----------------

- Added support for "job-retain-until" (Issue #14)
- Added new PAPPL-Create-Printers operation, and the PAPPL mainloop API now
  auto-adds local printers the first time a server is run (Issue #245)
- Added new `papplDeviceRemoveScheme` and `papplDeviceRemoveTypes` APIs to
  disable unwanted device types (Issue #259)
- Added support for suspending and resuming jobs at copy boundaries (Issue #266)
- Added support for server configuration files (Issue #279)
- Now preserve the paused state of printers (Issue #286)
- Fixed reporting of "xxx-k-octet-supported" attributes.
- Fixed printing of 1/2/4-bit grayscale PNG images (Issue #267)
- Fixed USB serial number for DYMO printers (Issue #271)
- Fixed a potential buffer overflow in the logging code (Issue #272)
- Fixed DNS-SD advertisements when the server name is set to "localhost"
  (Issue #274)
- Fixed hostname change detection when using mDNSResponder (Issue #282)
- Fixed authentication cookie comparisons for simple password mode.
- Fixed a potential time-of-use issue with PAPPL-created directories.
- Fixed handling of trailing '%' in log format strings.
- Updated the `options` sub-command to list vendor options and values
  (Issue #255)
- Updated web interface to show the age of jobs (Issue #256)
- Updated "devices" sub-command to have the PAPPL server find the devices
  instead of doing it directly (Issue #262)
- Updated default logging to be less chatty (Issue #270)
- Updated the Wi-Fi configuration page to support hidden networks.
- Updated the Wi-Fi configuration page reload time to 30 seconds.
- Updated TLS certificate generation to support more types of certificates and
  to use modern OpenSSL/GNU TLS APIs.
