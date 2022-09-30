//---------------------------------------------------------------------------------------------------
//
// EPX Test Printer
//
// A virtual IPP Printer used to prototype IPP Enterprise Printing Extensions v2.0 (EPX)
//
// Copyright © 2022 Printer Working Group.
// Written by Smith Kennedy (HP Inc.)
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//
//---------------------------------------------------------------------------------------------------


#ifndef _TESTEPX_H_
#  define _TESTEPX_H_

#include <pappl/pappl.h>


extern pappl_pr_driver_t epx_drivers[11];

#ifdef PWG_DRIVER
pappl_pr_driver_t epx_drivers[11] =
{     /* name */                            /* description */                       /* device ID */ /* extension */
    { "pwg_common-300dpi-black_1",	        "EPX Office 300DPI Black",		            NULL,	        NULL },
    { "pwg_common-300dpi-sgray_8",	        "EPX Office 300DPI sGray 8-bit",		    NULL,	        NULL },
    { "pwg_common-300dpi-srgb_8",		    "EPX Office 300DPI sRGB 8-bit",		        NULL,	        NULL },
    { "pwg_common-300dpi-600dpi-black_1",	"EPX Office 300DPI 600DPI Black",		    NULL,	        NULL },
    { "pwg_common-300dpi-600dpi-sgray_8",	"EPX Office 300DPI 600DPI sGray 8-bit",	    NULL,	        NULL },
    { "pwg_common-300dpi-600dpi-srgb_8",	"EPX Office 300DPI 600DPI sRGB 8-bit",	    NULL,	        NULL },
    { "pwg_fail-300dpi-black_1",	        "EPX Always Fails 300DPI Black",		    NULL,	        NULL }
};
#endif // PWG_DRIVER


extern const char *epx_pappl_autoadd_cb(const char *device_info, const char *device_uri, const char *device_id, void *data);
extern bool	epx_pappl_driver_cb(pappl_system_t *system, const char *driver_name, const char *device_uri, const char *device_id, pappl_pr_driver_data_t *driver_data, ipp_t **driver_attrs, void *data);


#endif // !_TESTEPX_H_
