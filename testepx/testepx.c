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


#include "testepx.h"
#include <pappl/base-private.h>
#include <config.h>
#include <libgen.h>

#define FOOTER_HTML         "Copyright © 2022 Printer Working Group."
#define VERSION_STRING      "0.1.0.1"


static pappl_system_t *epx_system_cb(int optionCount, cups_option_t *options, void *data);

//---------------------------------------------------------------------------------------------------
// main()

int main(int  argc, char *argv[])
{
    int result = 0;
    char *whoami = basename(argv[0]);
    
    printf("%s - Starting papplMainLoop\n", whoami);
//    result = papplMainloop(argc,                                                // I - Number of command line arguments
//                           argv,                                                // I - Command line arguments
//                           VERSION_STRING,                                      // I - Version number
//                           FOOTER_HTML,                                         // I - Footer HTML or `NULL` for none
//                           (int)(sizeof(epx_drivers) / sizeof(epx_drivers[0])), // I - Number of drivers
//                           epx_drivers,                                         // I - Drivers
//                           NULL,                                                // I - Auto-add callback or `NULL` for none
//                           epx_pappl_driver_cb,                                 // I - Driver callback
//                           NULL,                                                // I - Sub-command name or `NULL` for none
//                           NULL,                                                // I - Sub-command callback or `NULL` for none
//                           epx_system_cb,                                       // I - System callback or `NULL` for default
//                           NULL,                                                // I - Usage callback or `NULL` for default
//                           whoami);                                             // I - Context pointer

    result = papplMainloop(argc,                                                // I - Number of command line arguments
                           argv,                                                // I - Command line arguments
                           VERSION_STRING,                                      // I - Version number
                           FOOTER_HTML,                                         // I - Footer HTML or `NULL` for none
                           0,                                                   // I - Number of drivers
                           NULL,                                                // I - Drivers
                           epx_pappl_autoadd_cb,                                // I - Auto-add callback or `NULL` for none
                           epx_pappl_driver_cb,                                 // I - Driver callback
                           NULL,                                                // I - Sub-command name or `NULL` for none
                           NULL,                                                // I - Sub-command callback or `NULL` for none
                           epx_system_cb,                                       // I - System callback or `NULL` for default
                           NULL,                                                // I - Usage callback or `NULL` for default
                           whoami);                                             // I - Context pointer

    printf("%s - papplMainLoop stopped with result %d\n", whoami, result);

    return result;
}

//---------------------------------------------------------------------------------------------------
// 'epx_system_cb()' - System callback.

pappl_system_t *epx_system_cb(int           optionCount,   // I - Number of options
                              cups_option_t *options,      // I - Options
                              void          *data)         // I - Callback data
{
    pappl_system_t      *system;            // System object
    const char          *val,               // Current option value
                        *hostname,          // Hostname, if any
                        *logfile,           // Log file, if any
                        *system_name;       // System name, if any
    pappl_loglevel_t    loglevel;           // Log level
    int                 port = 0;           // Port number, if any
    char                *whoami = (char*)data;
    
    // System options
    static pappl_contact_t contact =    // Contact information
    {
        "Smith Kennedy",
        "epx@pwg.org",
        "+1-208-555-1212"
    };
    static pappl_version_t versions[1] =    // Firmware version info
    {
        {
            "Test Application",             // "xxx-firmware-name" value
            "",                             // "xxx-firmware-patches" value
            VERSION_STRING,                 // "xxx-firmware-string-version" value
            {                               // "xxx-firmware-version" value (short[4])
                0,
                1,
                0,
                1
            }
            
        }
    };
    

    
    // Verify that this was the right callback called by validating that data is what was provided in main()
    if (!data || strcmp(whoami, "testepx"))
    {
        fprintf(stderr, "%s - epx_system_cb: Bad callback data %p.\n", whoami, data);
        return (NULL);
    }
    
    // Parse options...
    if ((val = cupsGetOption("log-level", (cups_len_t)optionCount, options)) != NULL)
    {
        if (!strcmp(val, "fatal"))
            loglevel = PAPPL_LOGLEVEL_FATAL;
        else if (!strcmp(val, "error"))
            loglevel = PAPPL_LOGLEVEL_ERROR;
        else if (!strcmp(val, "warn"))
            loglevel = PAPPL_LOGLEVEL_WARN;
        else if (!strcmp(val, "info"))
            loglevel = PAPPL_LOGLEVEL_INFO;
        else if (!strcmp(val, "debug"))
            loglevel = PAPPL_LOGLEVEL_DEBUG;
        else
        {
            fprintf(stderr, "%s - epx_system_cb: Bad log-level value '%s'.\n", whoami, val);
            return (NULL);
        }
    }
    else
    {
        loglevel = PAPPL_LOGLEVEL_UNSPEC;
    }
    
    logfile     = cupsGetOption("log-file", (cups_len_t)optionCount, options);
    hostname    = cupsGetOption("server-hostname", (cups_len_t)optionCount, options);
    system_name = cupsGetOption("system-name", (cups_len_t)optionCount, options);
    
    if ((val = cupsGetOption("server-port", (cups_len_t)optionCount, options)) != NULL)
    {
        if (!isdigit(*val & 255))
        {
            fprintf(stderr, "%s - epx_system_cb: Bad server-port value '%s'.\n", whoami, val);
            return (NULL);
        }
        else
            port = atoi(val);
    }
    
    system = papplSystemCreate(PAPPL_SOPTIONS_WEB_INTERFACE | PAPPL_SOPTIONS_WEB_LOG |
                               PAPPL_SOPTIONS_WEB_NETWORK | PAPPL_SOPTIONS_WEB_SECURITY |
                               PAPPL_SOPTIONS_WEB_TLS,                                              // I - Server options
                               system_name ? system_name : "NoSystemName",                          // I - System name
                               port,                                                                // I - Port number or `0` for auto
                               "_print,_universal",                                                 // I - DNS-SD sub-types or `NULL` for none
                               cupsGetOption("spool-directory", (cups_len_t)optionCount, options),  // I - Spool directory or `NULL` for default
                               logfile ? logfile : "-",                                             // I - Log file or `NULL` for default
                               loglevel,                                                            // I - Log level
                               cupsGetOption("auth-service", (cups_len_t)optionCount, options),     // I - PAM authentication service or `NULL` for none
                               false);                                                              // I - Only support TLS connections?
    
    if (system == NULL)
        return (NULL);
    
    papplSystemAddListeners(system, NULL);
    papplSystemSetHostName(system, hostname); // TODO: Why is this even needed?
    
    papplSystemSetPrinterDrivers(system, (int)(sizeof(epx_drivers) / sizeof(epx_drivers[0])), epx_drivers, epx_pappl_autoadd_cb, /*create_cb*/NULL, epx_pappl_driver_cb, "testmainloop");
    
    papplSystemSetFooterHTML(system, FOOTER_HTML);
    papplSystemSetSaveCallback(system, (pappl_save_cb_t)papplSystemSaveState, (void *)"/tmp/testmainloop.state");
    papplSystemSetVersions(system, (int)(sizeof(versions) / sizeof(versions[0])), versions);
    
    if (!papplSystemLoadState(system, "/tmp/testmainloop.state"))
    {
        papplSystemSetContact(system, &contact);
        papplSystemSetDNSSDName(system, system_name ? system_name : "Test Mainloop");
        papplSystemSetGeoLocation(system, "geo:46.4707,-80.9961");
        papplSystemSetLocation(system, "Test Lab 42");
        papplSystemSetOrganization(system, "Example Company");
    }
    
    return (system);
}
