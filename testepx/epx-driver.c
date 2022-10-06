//
// EPX driver for the Printer Application Framework
//
// Copyright © 2022 Printer Working Group
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.
//

#define EPX_DRIVER 1
#include "testepx.h"
#include <pappl/base-private.h>
#include <cups/dir.h>


//
// Driver types...
//

typedef struct pwg_s
{
    cups_raster_t	*ras;			// PWG raster file
    size_t	colorants[4];		// Color usage
} pwg_job_data_t;


//
// Local globals...
//

static const char * const pwg_common_media[] =
{					// Supported media sizes for common printer
    "na_letter_8.5x11in",
    "na_legal_8.5x14in",
    
    "iso_a4_210x297mm",
    
    "custom_max_8.5x14in",
    "custom_min_3x5in"
};


//
// Local functions...
//

static void	epx_identify(pappl_printer_t *printer, pappl_identify_actions_t actions, const char *message);
static bool	epx_print(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool	epx_rendjob(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool	epx_rendpage(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page);
static bool	epx_rstartjob(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device);
static bool	epx_rstartpage(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned page);
static bool	epx_rwriteline(pappl_job_t *job, pappl_pr_options_t *options, pappl_device_t *device, unsigned y, const unsigned char *line);
static bool	epx_status(pappl_printer_t *printer);
static const char *epx_testpage(pappl_printer_t *printer, char *buffer, size_t bufsize);


//
// 'pwg_autoadd()' - Auto-add callback.
//

const char *				                    // O - Driver name or `NULL` for none
epx_pappl_autoadd_cb(const char *device_info,	// I - Device information string (not used)
                     const char *device_uri,	// I - Device URI (not used)
                     const char *device_id,	    // I - IEEE-1284 device ID
                     void       *data)		    // I - Callback data (not used)
{
    const char	*ret = NULL;		// Return value
    
    
    (void)device_info;
    (void)device_uri;
    (void)device_id;
    
    if (!data || strcmp((const char *)data, "testepx"))
    {
//        papplLog(system, PAPPL_LOGLEVEL_ERROR, "Driver callback called with bad data pointer.");
        fprintf(stderr, "Driver callback called with bad data pointer.");
    }
    else
    {
        ret = "epx-driver";
    }
    
    return (ret);
}


//
// 'epx_driver_cb()' - Driver callback for EPX.
//

bool					// O - `true` on success, `false` on failure
epx_pappl_driver_cb(
                    pappl_system_t         *system,	        // I - System
                    const char             *driver_name,    // I - Driver name
                    const char             *device_uri,	    // I - Device URI
                    const char             *device_id,	    // I - IEEE-1284 device ID string (not used)
                    pappl_pr_driver_data_t *driver_data,    // O - Driver data
                    ipp_t                  **driver_attrs,  // O - Driver attributes
                    void                   *data)	        // I - Callback data
{
    
    (void)device_id; // Statement to make the compiler ignore that this isn't used
    
    if (!driver_name || !device_uri || !driver_data || !driver_attrs)
    {
        papplLog(system, PAPPL_LOGLEVEL_ERROR, "Driver callback called without required information.");
        return (false);
    }
    
    if (!data || strcmp((const char *)data, "testepx"))
    {
        papplLog(system, PAPPL_LOGLEVEL_ERROR, "Driver callback called with bad data pointer.");
        return (false);
    }
        
    if (strcmp(driver_name, "epx-driver"))
    {
        papplLog(system, PAPPL_LOGLEVEL_ERROR, "Unsupported driver name '%s'.", driver_name);
        return (false);
    }
    
    driver_data->identify_cb        = epx_identify;
    driver_data->identify_default   = PAPPL_IDENTIFY_ACTIONS_SOUND;
    driver_data->identify_supported = PAPPL_IDENTIFY_ACTIONS_DISPLAY | PAPPL_IDENTIFY_ACTIONS_SOUND;
    driver_data->printfile_cb       = epx_print;
    driver_data->rendjob_cb         = epx_rendjob;
    driver_data->rendpage_cb        = epx_rendpage;
    driver_data->rstartjob_cb       = epx_rstartjob;
    driver_data->rstartpage_cb      = epx_rstartpage;
    driver_data->rwriteline_cb      = epx_rwriteline;
    driver_data->status_cb          = epx_status;
    driver_data->testpage_cb        = epx_testpage;
    driver_data->format             = "image/pwg-raster";
    driver_data->orient_default     = IPP_ORIENT_NONE;
    driver_data->quality_default    = IPP_QUALITY_NORMAL;
    
    return (true);
}


//
// 'pwg_identify()' - Identify the printer.
//

static void
epx_identify(
             pappl_printer_t          *printer,	// I - Printer
             pappl_identify_actions_t actions,	// I - Actions to take
             const char               *message)	// I - Message, if any
{
    (void)printer;
    (void)actions;
    
    // TODO: Open console and send BEL character and message to it instead...
    putchar(7);
    if (message)
        puts(message);
    
    fflush(stdout);
}


//
// 'pwg_print()' - Print a file.
//

static bool				                // O - `true` on success, `false` on failure
epx_print(
          pappl_job_t        *job,		// I - Job
          pappl_pr_options_t *options,	// I - Job options (unused)
          pappl_device_t     *device)   // I - Print device (unused)
{
    int		    fd;                 // Input file
    ssize_t	    bytes;              // Bytes read/written
    char		buffer[65536];      // Read/write buffer
    
    
    (void)options;
    
    papplJobSetImpressions(job, 1);
    
    if ((fd  = open(papplJobGetFilename(job), O_RDONLY)) < 0)
    {
        papplLogJob(job, PAPPL_LOGLEVEL_ERROR, "Unable to open print file '%s': %s", papplJobGetFilename(job), strerror(errno));
        return (false);
    }
    
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
        papplDeviceWrite(device, buffer, (size_t)bytes);
    
    close(fd);
    
    papplJobSetImpressionsCompleted(job, 1);
    
    return (true);
}


//
// 'pwg_rendjob()' - End a job.
//

static bool				// O - `true` on success, `false` on failure
epx_rendjob(
            pappl_job_t        *job,		// I - Job
            pappl_pr_options_t *options,	// I - Job options (unused)
            pappl_device_t     *device)		// I - Print device (unused)
{
    pwg_job_data_t	*pwg = (pwg_job_data_t *)papplJobGetData(job);
    // Job data
    
    (void)options;
    (void)device;
    
    cupsRasterClose(pwg->ras);
    
    free(pwg);
    papplJobSetData(job, NULL);
    
    return (true);
}


//
// 'pwg_rendpage()' - End a page.
//

static bool				// O - `true` on success, `false` on failure
epx_rendpage(
             pappl_job_t        *job,		// I - Job
             pappl_pr_options_t *options,	// I - Job options
             pappl_device_t     *device,		// I - Print device (unused)
             unsigned           page)		// I - Page number
{
    pwg_job_data_t	*pwg = (pwg_job_data_t *)papplJobGetData(job);
    // PWG driver data
    pappl_printer_t	*printer = papplJobGetPrinter(job);
    // Printer
    pappl_supply_t	supplies[5];	// Supply-level data
    
    
    (void)device;
    (void)page;
    
    if (papplPrinterGetSupplies(printer, 5, supplies) == 5)
    {
        // Calculate ink usage from coverage - figure 100 pages at 10% for black,
        // 50 pages at 10% for CMY, and 200 pages at 10% for the waste tank...
        int i;				// Looping var
        int	c, m, y, k, w;			// Ink usage
        pappl_preason_t reasons = PAPPL_PREASON_NONE;
        // "printer-state-reasons" values
        
        papplLogJob(job, PAPPL_LOGLEVEL_DEBUG, "Calculating ink usage (%u,%u,%u,%u)", (unsigned)pwg->colorants[0], (unsigned)pwg->colorants[1], (unsigned)pwg->colorants[2], (unsigned)pwg->colorants[3]);
        
        c = (int)(pwg->colorants[0] / options->header.cupsWidth / options->header.cupsHeight / 5);
        m = (int)(pwg->colorants[1] / options->header.cupsWidth / options->header.cupsHeight / 5);
        y = (int)(pwg->colorants[2] / options->header.cupsWidth / options->header.cupsHeight / 5);
        k = (int)(pwg->colorants[3] / options->header.cupsWidth / options->header.cupsHeight / 10);
        w = (int)((pwg->colorants[0] + pwg->colorants[1] + pwg->colorants[2] + pwg->colorants[3]) / options->header.cupsWidth / options->header.cupsHeight / 20);
        
        // Keep levels between 0 and 100...
        if ((supplies[0].level -= c) < 0)
            supplies[0].level = 100;		// Auto-refill
        if ((supplies[1].level -= m) < 0)
            supplies[1].level = 100;		// Auto-refill
        if ((supplies[2].level -= y) < 0)
            supplies[2].level = 100;		// Auto-refill
        if ((supplies[3].level -= k) < 0)
            supplies[3].level = 100;		// Auto-refill
        if ((supplies[4].level += w) > 100)
            supplies[4].level = 0;		// Auto-replace
        
        // Update printer-state-reasons accordingly...
        for (i = 0; i < 4; i ++)
        {
            if (supplies[i].level == 0)
                reasons |= PAPPL_PREASON_MARKER_SUPPLY_EMPTY;
            else if (supplies[i].level < 10)
                reasons |= PAPPL_PREASON_MARKER_SUPPLY_LOW;
        }
        
        if (supplies[4].level == 100)
            reasons |= PAPPL_PREASON_MARKER_WASTE_FULL;
        else if (supplies[4].level >= 90)
            reasons |= PAPPL_PREASON_MARKER_WASTE_ALMOST_FULL;
        
        papplPrinterSetSupplies(printer, 5, supplies);
        papplPrinterSetReasons(printer, reasons, PAPPL_PREASON_DEVICE_STATUS);
    }
    
    return (true);
}


//
// 'pwg_rstartjob()' - Start a job.
//

static bool				// O - `true` on success, `false` on failure
epx_rstartjob(
              pappl_job_t        *job,		// I - Job
              pappl_pr_options_t *options,	// I - Job options
              pappl_device_t     *device)		// I - Print device (unused)
{
    pwg_job_data_t *pwg = (pwg_job_data_t *)calloc(1, sizeof(pwg_job_data_t));
    // PWG driver data
    
    
    (void)options;
    
    papplJobSetData(job, pwg);
    
    pwg->ras = cupsRasterOpenIO((cups_raster_cb_t)papplDeviceWrite, device, CUPS_RASTER_WRITE_PWG);
    
    return (1);
}


//
// 'pwg_rstartpage()' - Start a page.
//

static bool				// O - `true` on success, `false` on failure
epx_rstartpage(
               pappl_job_t        *job,		// I - Job
               pappl_pr_options_t *options,	// I - Job options
               pappl_device_t     *device,		// I - Print device (unused)
               unsigned           page)		// I - Page number
{
    pwg_job_data_t	*pwg = (pwg_job_data_t *)papplJobGetData(job);
    // PWG driver data
    
    (void)device;
    (void)page;
    
    memset(pwg->colorants, 0, sizeof(pwg->colorants));
    
    return (cupsRasterWriteHeader(pwg->ras, &options->header) != 0);
}


//
// 'pwg_rwriteline()' - Write a raster line.
//

static bool				// O - `true` on success, `false` on failure
epx_rwriteline(
               pappl_job_t         *job,		// I - Job
               pappl_pr_options_t  *options,	// I - Job options
               pappl_device_t      *device,	// I - Print device (unused)
               unsigned            y,		// I - Line number
               const unsigned char *line)		// I - Line
{
    const unsigned char	*lineptr,	// Pointer into line
    *lineend;	// End of line
    pwg_job_data_t	*pwg = (pwg_job_data_t *)papplJobGetData(job);
    // PWG driver data
    
    (void)device;
    (void)y;
    
    // Add the colorant usage for this line (for simulation purposes - normally
    // this is tracked by the printer/ink cartridge...)
    lineend = line + options->header.cupsBytesPerLine;
    
    switch (options->header.cupsColorSpace)
    {
        case CUPS_CSPACE_K :
            if (options->header.cupsBitsPerPixel == 1)
            {
                // 1-bit K
                static unsigned short amounts[256] =
                {				// Amount of "ink" used for 8 pixels
                    0,    255,  255,  510,  255,  510,  510,  765,
                    255,  510,  510,  765,  510,  765,  765,  1020,
                    255,  510,  510,  765,  510,  765,  765,  1020,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    255,  510,  510,  765,  510,  765,  765,  1020,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    255,  510,  510,  765,  510,  765,  765,  1020,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    1020, 1275, 1275, 1530, 1275, 1530, 1530, 1785,
                    255,  510,  510,  765,  510,  765,  765,  1020,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    1020, 1275, 1275, 1530, 1275, 1530, 1530, 1785,
                    510,  765,  765,  1020, 765,  1020, 1020, 1275,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    1020, 1275, 1275, 1530, 1275, 1530, 1530, 1785,
                    765,  1020, 1020, 1275, 1020, 1275, 1275, 1530,
                    1020, 1275, 1275, 1530, 1275, 1530, 1530, 1785,
                    1020, 1275, 1275, 1530, 1275, 1530, 1530, 1785,
                    1275, 1530, 1530, 1785, 1530, 1785, 1785, 2040
                };
                
                for (lineptr = line; lineptr < lineend; lineptr ++)
                    pwg->colorants[3] += amounts[*lineptr];
            }
            else
            {
                // 8-bit K
                for (lineptr = line; lineptr < lineend; lineptr ++)
                    pwg->colorants[3] += *lineptr;
            }
            break;
            
        case CUPS_CSPACE_W :
        case CUPS_CSPACE_SW :
            // 8-bit W (luminance)
            for (lineptr = line; lineptr < lineend; lineptr ++)
                pwg->colorants[3] += 255 - *lineptr;
            break;
            
        case CUPS_CSPACE_RGB :
        case CUPS_CSPACE_SRGB :
        case CUPS_CSPACE_ADOBERGB :
            // 24-bit RGB
            for (lineptr = line; lineptr < lineend; lineptr += 3)
            {
                // Convert RGB to CMYK using simple transform...
                unsigned char cc = 255 - lineptr[0];
                unsigned char cm = 255 - lineptr[1];
                unsigned char cy = 255 - lineptr[2];
                unsigned char ck = cc;
                
                if (ck > cm)
                    ck = cm;
                if (ck > cy)
                    ck = cy;
                
                cc -= ck;
                cm -= ck;
                cy -= ck;
                
                pwg->colorants[0] += cc;
                pwg->colorants[1] += cm;
                pwg->colorants[2] += cy;
                pwg->colorants[3] += ck;
            }
            break;
            
        case CUPS_CSPACE_CMYK :
            // 32-bit CMYK
            for (lineptr = line; lineptr < lineend; lineptr += 4)
            {
                pwg->colorants[0] += lineptr[0];
                pwg->colorants[1] += lineptr[1];
                pwg->colorants[2] += lineptr[2];
                pwg->colorants[3] += lineptr[3];
            }
            break;
            
        default :
            break;
    }
    
    return (cupsRasterWritePixels(pwg->ras, (unsigned char *)line, options->header.cupsBytesPerLine) != 0);
}


//
// 'pwg_status()' - Get current printer status.
//

static bool				// O - `true` on success, `false` on failure
epx_status(
           pappl_printer_t *printer)		// I - Printer
{
    if (!strncmp(papplPrinterGetDriverName(printer), "pwg_common-", 11))
    {
        // Supply levels...
        static pappl_supply_t supply[5] =	// Supply level data
        {
            { PAPPL_SUPPLY_COLOR_CYAN,     "Cyan Ink",       true, 100, PAPPL_SUPPLY_TYPE_INK },
            { PAPPL_SUPPLY_COLOR_MAGENTA,  "Magenta Ink",    true, 100, PAPPL_SUPPLY_TYPE_INK },
            { PAPPL_SUPPLY_COLOR_YELLOW,   "Yellow Ink",     true, 100, PAPPL_SUPPLY_TYPE_INK },
            { PAPPL_SUPPLY_COLOR_BLACK,    "Black Ink",      true, 100, PAPPL_SUPPLY_TYPE_INK },
            { PAPPL_SUPPLY_COLOR_NO_COLOR, "Waste Ink Tank", true, 0, PAPPL_SUPPLY_TYPE_WASTE_INK }
        };
        
        if (papplPrinterGetSupplies(printer, 0, NULL) == 0)
            papplPrinterSetSupplies(printer, (int)(sizeof(supply) / sizeof(supply[0])), supply);
    }
    
    // Every 10 seconds, set the "media-empty" reason for one second...
    if ((time(NULL) % 10) == 0)
        papplPrinterSetReasons(printer, PAPPL_PREASON_MEDIA_EMPTY, PAPPL_PREASON_NONE);
    else
        papplPrinterSetReasons(printer, PAPPL_PREASON_NONE, PAPPL_PREASON_MEDIA_EMPTY);
    
    return (true);
}


//
// 'pwg_testpage()' - Return a test page file to print
//

static const char *			                // O - Filename or `NULL`
epx_testpage(
             pappl_printer_t *printer,		// I - Printer
             char            *buffer,		// I - File Buffer
             size_t          bufsize)		// I - Buffer Size
{
    const char		*testfile;	// Test File
    pappl_pr_driver_data_t data;		// Driver data
    
    
    // Get the printer capabilities...
    papplPrinterGetDriverData(printer, &data);
    
    // Find the right test file...
    if (data.color_supported & PAPPL_COLOR_MODE_COLOR)
        testfile = "portrait-color.png";
    else
        testfile = "portrait-gray.png";
    
    papplCopyString(buffer, testfile, bufsize);
    if (access(buffer, R_OK))
        snprintf(buffer, bufsize, "testsuite/%s", testfile);
    
    if (access(buffer, R_OK))
    {
        *buffer = '\0';
        return (NULL);
    }
    else
    {
        return (buffer);
    }
}
