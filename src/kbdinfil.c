/* -----------------------------------------------------------------------------
 * File         : src/kbdinfil.c
 * Description  : Daemon Listener
 * Copyright	: (c) Rodger Allen 2019
 * Licence		: BSD3
 * -------------------------------------------------------------------------- */

#ifndef LISTENER
#define LISTENER
#endif

#include <stdio.h>          // printf()
#include <stdlib.h>         // malloc(), free()
#include <string.h>         // strlen()
#include <ctype.h>          // isprint()
#include <inttypes.h>       // uint32_t

#include <unistd.h>         // getopt(), dup2()
#include <syslog.h>         // syslog()
#include <errno.h>          // errno, strerror()
#include <sys/stat.h>       // umask()
#include <fcntl.h>          // open()
#include <sys/time.h>       // gettimeofday(), timersub()

#include "global.h"
#include "encoder.h"
#include "meta.h"
#include "crc32.h"
#include "cq.h"
#include "filing.h"

/* ---------------------------------- */

uint8_t parity;

int verbose = 0;
int foreground = 0;
int run_once = 0;

#define STRERR strerror(errno)

/* ---------------------------------- */

int main( int argc, char* argv[] );
void usage( const char* progname );
void goDark();
void loop( FILE *hid, const char *outdir );

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Process a single incoming frame
 */
void loop( FILE *hid, const char *outdir )
{
    struct metadata md;
    uint8_t *metaBuf;
    uint8_t *fileBuf;
    uint32_t crc = 0;
    struct timeval ustart, uend, udiff;
    encoder enc = trybbleEncoder;
    int ok = 0;

    /* Wait for a new transmission */
    //syslog(LOG_LOCAL7|LOG_DEBUG,"Awaiting new transmission");
    parity = PARITY;
    while (!ok) {
        if (!waitForEncoding(hid,&enc)) continue;
        if (verbose) fprintf(stdout,"\n === got enc %#04x ===\n",(enc.mask));
        ok = cqSanity(hid, enc);
        if(verbose>=4 && !ok) fprintf(stdout,"Failed CQ for enc=%s",showLEDs(enc.mask));
        //syslog(LOG_LOCAL7|LOG_DEBUG,"Failed CQ for enc=%s",showLEDs(enc.mask));
    }
    if (verbose) fprintf(stdout,"\n === RECEIVING ===\n");
    //syslog(LOG_LOCAL7|LOG_DEBUG,"  Received CQ");

    /* Process metadata */
    metaBuf = getPayload(hid, metaSize(&md), enc);
    md = bytesToMeta(metaBuf);
    free(metaBuf);
    if (! metaSanity(&md,&enc)) {
        fprintf(stderr, "WARN: Metadata error: %s\n",showMetaData(&md));
        syslog(LOG_LOCAL7|LOG_WARNING," Metadata error: %s", showMetaData(&md));
        return;
    }
    if (verbose) fprintf(stdout,"INFO: %s\n", showMetaData(&md));
    syslog(LOG_LOCAL7|LOG_INFO,"< %s", showMetaData(&md));

    /* Get contents of file */
    gettimeofday(&ustart,NULL);
    fileBuf = getPayload(hid, md.size, enc);
    gettimeofday(&uend,NULL);

    timersub(&uend,&ustart,&udiff);
    float zdiff = (float) udiff.tv_sec + ((float)udiff.tv_usec/1000000);
    if (verbose)
        fprintf(stdout,"INFO: Received %d bytes in %.2f seconds @ %.2f bps, mask=%s\n",
                md.size, zdiff, md.size/zdiff, showLEDs(enc.mask));

    /* Check CRC and write file */
    crc = rc_crc32(0, fileBuf, md.size);
    if (crc != md.crc) {
        fprintf(stderr, "WARN: CRC mismatch: 0x%x != 0x%x\n", crc, md.crc);
        syslog(LOG_LOCAL7|LOG_WARNING," CRC mismatch: 0x%x", crc);
    }

    char *stamp = mkStamp(md,showLEDs(enc.mask),(crc==md.crc)?"bin":"crc");
    char *newname = newFileName( outdir, stamp );
    writeContents(md, fileBuf, newname);
    syslog(LOG_LOCAL7|LOG_NOTICE,"> %s [%d: %.2fs @ %.2fbps]",
                                stamp, md.delay, zdiff, md.size/zdiff);
    if (verbose>=4) showFileData(fileBuf, md.size);

    /* Cleanup */
    free(newname);
    free(stamp);
    free(fileBuf);

    if (verbose) fprintf(stdout," === DONE ===\n\n");
}

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Daemonise
 */
void goDark()
{
    pid_t pid = 0;
    pid_t sid = 0;

    // Create child process
    pid = fork();
    if (pid < 0) {
        fprintf(stderr,"ERR: fork failed!\n");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        fprintf(stdout,"spawned pid: %d \n", pid);
        exit(EXIT_SUCCESS);
    }
    umask(0);
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    // Change the current working directory to root.
    if (chdir("/")!=0) {
        fprintf(stderr,"ERR: failed to chdir to \"/\" : %s", STRERR);
        exit(EXIT_FAILURE);
    }

    // Close or dup stdin, stdout and stderr
    int log;
    if ((log = open(DEBUGLOG, O_WRONLY|O_APPEND|O_CREAT, 0644)) < 0) {
        syslog(LOG_LOCAL7|LOG_ERR,"ERR: open(%s) : %s", DEBUGLOG,STRERR);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        return;
    }
    close(STDIN_FILENO); //dup2(log, STDIN_FILENO);
    dup2(log, STDOUT_FILENO);
    dup2(log, STDERR_FILENO);
    close(log);
}

/* -------------------------------------------------------------------------- */

void usage( const char* progname )
{
    fprintf(stderr, "Daemon to receive data from keyboard LEDs\n");
    fprintf(stderr, "  version: %s\n",REVISION);
    fprintf(stderr, "\n");
    fprintf(stderr, "usage: %s [-hvF1] [-o dir] <device>\n", progname);
    fprintf(stderr, "	-h		this help\n");
    fprintf(stderr, "	-F		run in foreground\n");
    fprintf(stderr, "	-1		receive one frame and exit (runs in foreground)\n");
    fprintf(stderr, "	-v		verbose (more like debug)\n");
    fprintf(stderr, "	-vv		more debugging output\n");
    fprintf(stderr, "	-vvv		holy crap, too much\n");
    fprintf(stderr, "	-o		output dir (default: %s)\n", DEFOUTDIR);
    fprintf(stderr, "	device		path to device, e.g., /dev/hidg0\n");
    fprintf(stderr, "\n");
    if (verbose) {
        fprintf(stderr, "Defaults\n");
        fprintf(stderr, "   REVISION        : %s\n", REVISION);
        fprintf(stderr, "   MAXFILESIZE     : %d\n", MAXFILESIZE);
        fprintf(stderr, "   DEFTIMER        : %d\n", DEFTIMER);
        fprintf(stderr, "   DEFDEVICE       : %s\n", DEFDEVICE);
        fprintf(stderr, "   DEFOUTDIR       : %s\n", DEFOUTDIR);
        fprintf(stderr, "   DEBUGLOG        : %s\n", DEBUGLOG);
        fprintf(stderr, "   DEBUG           : %d\n", DEBUG);
        fprintf(stderr, "   PARITY          : %s\n", showLEDs(PARITY));
        fprintf(stderr, "   KANA            : %s\n", showLEDs(KANA));
        fprintf(stderr, "   SCROLL          : %s\n", showLEDs(SCROLL));
        fprintf(stderr, "   CAPITAL         : %s\n", showLEDs(CAPITAL));
        fprintf(stderr, "   NUMLOCK         : %s\n", showLEDs(NUMLOCK));
        fprintf(stderr, "   MAGIX/MAGSZ     : %s %d\n", MAGIX, MAGSZ);
        fprintf(stderr, "\n");
    }
}

/* -------------------------------------
 * Main
 */
int main( int argc, char* argv[] )
{
    int opt;
    char *devicename;
    char *outdir = DEFOUTDIR;
    FILE *hid;

    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    while ((opt = getopt(argc, argv, "h?F1o:v::")) != -1) {
        switch (opt) {
            case 'h':case '?':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'o':
                if (optarg == NULL) {
                    fprintf(stderr, "ERR: -o missing directory\n");
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                outdir = optarg;
                int len = strlen(outdir);
                if (outdir[len-1]=='/')
                    outdir[len-1] = '\0';
                break;
            case 'v':
                verbose = 1;
                if (optarg == NULL)
                    break;
                while (*optarg++ == 'v' && verbose<9)
                    verbose++;
                break;
            case 'F':
                foreground = 1;
                break;
            case '1':
                run_once = 1;
                foreground = 1;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "ERR: missing device\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    devicename = argv[optind];
    //verbose &= foreground;

    // -----------------
    // Starting
    syslog(LOG_LOCAL7|LOG_NOTICE,"====== Keyboard LED Listener v%s ======",REVISION);
    syslog(LOG_LOCAL7|LOG_INFO,": %s",argv[0]);

    // -----------------
    // Daemonise
    if (foreground == 0)
        (void) goDark();
    syslog(LOG_LOCAL7|LOG_INFO,": PID: %d%s", getpid(), foreground?" (foreground)":"");
    fprintf(stdout,"\n\n\n====== Keyboard LED Listener v%s ======\n",REVISION);
    fprintf(stdout,"INFO: running in foreground (pid=%d)\n",getpid());

    // -----------------
    // Open device
    // Device may not be immediately available on boot
    hid = fopen(devicename, "rb");
    while (hid == NULL) {
        fprintf(stderr, "ERR: Error opening %s: %s", devicename, STRERR);
        syslog(LOG_LOCAL7|LOG_ERR,"Error opening %s: %s", devicename, STRERR);
        sleep(RETRY_TIMEOUT);
        hid = fopen(devicename, "rb");
    }
    syslog(LOG_LOCAL7|LOG_INFO,": Listening on %s", devicename);

    // -----------------
    // Check output location
    if (check_dir(outdir) != 0) {
        fprintf(stderr, "ERR: Error opening %s: %s\n", devicename, STRERR);
        syslog(LOG_LOCAL7|LOG_ERR,"Error opening %s: %s", devicename, STRERR);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_LOCAL7|LOG_INFO,": Output directory %s", outdir);
    if (verbose) fprintf(stdout,"Listen on %s\nOutput dir %s\n\n", devicename, outdir);
    if (verbose) syslog(LOG_LOCAL7|LOG_INFO,": Verbose output to %s",
                            foreground?"console":DEBUGLOG);

    // -----------------
    // Ready, Steady, Loop
    if (run_once) loop(hid,outdir);
    else
        while (1) loop(hid,outdir);

    if (hid != NULL) fclose(hid);
    exit(EXIT_SUCCESS);
}

// -----------------------------------------------------------------------------
// vi: et ts=4 ai
