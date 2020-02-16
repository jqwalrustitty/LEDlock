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
#include <sys/stat.h>       // stat()
#include <fcntl.h>          // open()
#include <sys/time.h>       // gettimeofday(), timersub()

#include "global.h"
#include "encoder.h"        // INITSTATE
#include "meta.h"           // struct metadata, bytesToMeta(), metaSize()
#include "crc32.h"          // rc_crc32()
#include "cq.h"             // waitForCQ(), decodeChunk()
#include "filing.h"         // writeContents(), showFileData()

/* ---------------------------------- */

uint8_t lastTrybble;

int verbose = 0;
int foreground = 0;
int run_once = 0;

#define STRERR strerror(errno)

/* ---------------------------------- */

int main( int argc, char* argv[] );
void usage( char* progname );

void loop( FILE *hid, char *outdir );
void goDark();

unsigned char * getPayload( FILE *fh, int fileSize );
int check_dir( const char *path );

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Receive and decode the file contents
 */
unsigned char * getPayload( FILE *fh, int fileSize )
{
    unsigned char *buf;
    buf = malloc(fileSize);
    for (int j=0 ; j < fileSize ; j++) {
        buf[j] = decodeChunk(fh);
        if (verbose>=2) printf(" %#04x %c\n",buf[j],isprint(buf[j])?buf[j]:'_');
    }
    return buf;
}

/* -------------------------------------
 * Check output directory exists
 *  - create if not?
 *  return: 0 on success
 */
#ifndef CREATEDIR
#define CREATEDIR 0
#endif
int check_dir( const char *path )
{
    struct stat sb;
    int rv;

    //return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);

    rv = stat(path, &sb);
    if (rv != 0) {
        syslog(LOG_LOCAL7|LOG_ERR,"Cannot stat directory %s : %s", path, STRERR);
        return rv;
    }
    if (S_ISDIR(sb.st_mode) != 1) {
        syslog(LOG_LOCAL7|LOG_ERR,"Invalid output directory %s", path);
        return 1;
    }

#if CREATEDIR
    // create directory (needs to go into the rv check from stat()
    rv = mkdir(path, 0x755);
    if (rv != 0) {
        syslog(LOG_LOCAL7|LOG_ERR,"Error creating output dir %s : %s", path, STRERR);
        syslog(LOG_LOCAL7|LOG_ERR,"    rv=%d, errno=%d", rv, errno);
        return errno;
    }
#endif
    //syslog(LOG_LOCAL7|LOG_INFO,"Using output dir %s", path);
    return 0;
}

/* -------------------------------------------------------------------------- */

/* -------------------------------------
 * Process a single incoming frame
 */
void loop( FILE *hid, char *outdir )
{
    struct metadata md;
    uint8_t *metaBuf;
    uint8_t *fileBuf;
    uint32_t crc = 0;
    struct timeval ustart, uend, udiff;

    /* Wait for a new transmission */
    syslog(LOG_LOCAL7|LOG_INFO,"Awaiting new transmission");
    lastTrybble = INITSTATE;
    waitForCQ(hid);
    if (verbose) fprintf(stdout,"\n === RECEIVING ===\n");
    syslog(LOG_LOCAL7|LOG_INFO,"  Received CQ");

    /* Process metadata */
    metaBuf = getPayload(hid,metaSize(&md));
    md = bytesToMeta(metaBuf);
    free(metaBuf);

    if (verbose) fprintf(stdout,"INFO: size=%d+%d  crc=%#x\n",
                                metaSize(&md), md.size, md.crc);
    syslog(LOG_LOCAL7|LOG_INFO,"  size=%d+%d  crc=0x%x",
                                metaSize(&md), md.size, md.crc);
#if DEBUG
    if (! metaSanity(&md)) {
        char mdStr[512] = {0};
        showMetadataStr(mdStr, 512, &md);
        fprintf(stderr, "WARN: Metadata error: [%s]",mdStr);
        syslog(LOG_LOCAL7|LOG_WARNING,"  > Metadata error: [%s]",mdStr);
        return;
    }
#endif

    /* Get contents of file */
    gettimeofday(&ustart,NULL);
    fileBuf = getPayload(hid, md.size);
    gettimeofday(&uend,NULL);

    timersub(&uend,&ustart,&udiff);
    float zdiff = (float) udiff.tv_sec + ((float)udiff.tv_usec/1000000);
    if (verbose)
        fprintf(stdout,"INFO: Received %d bytes in %.2f seconds @ %.2f bps [%c]\n",
                md.size, zdiff, md.size/zdiff, *fileBuf);
    syslog(LOG_LOCAL7|LOG_INFO,"  Received %d bytes in %.2f seconds @ %.2f bps",
                md.size, zdiff, md.size/zdiff);

    /* Check CRC and write file */
    crc = rc_crc32(0, fileBuf, md.size);
    if (crc == md.crc) {
        writeContents(md, fileBuf, outdir, FLAGNONE);
    } else {
        fprintf(stderr, "WARN: CRC mismatch: 0x%x != 0x%x\n", crc, md.crc);
        syslog(LOG_LOCAL7|LOG_WARNING,"  > CRC mismatch: 0x%x", crc);
        writeContents(md, fileBuf, outdir, FLAGCRC);
    }
    if (verbose) showFileData(fileBuf, md.size);

    /* Cleanup */
    free(fileBuf);

    if (verbose) fprintf(stdout," === DONE ===\n\n");
}

/* -------------------------------------------------------------------------- */

void goDark()
{
    pid_t pid = 0;
    pid_t sid = 0;

    // Create child process
    pid = fork();

    // Indication of fork() failure
    if (pid < 0) {
        fprintf(stderr,"ERR: fork failed!\n");
        exit(EXIT_FAILURE);     // Return failure in exit status
    }

    // PARENT PROCESS. Need to kill it.
    if (pid > 0) {
        fprintf(stdout,"spawned pid: %d \n", pid);
        exit(EXIT_SUCCESS);     // return success in exit status
    }

    // unmask the file mode
    umask(0);

    //set new session
    sid = setsid();
    if(sid < 0) {
        exit(EXIT_FAILURE);     // Return failure
    }

    // Change the current working directory to root.
    chdir("/");

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

void usage( char* progname )
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
#if DEBUG
        fprintf(stderr, "   INITSTATE       : %s\n", showLEDstate(INITSTATE));
        fprintf(stderr, "   __TYDMASK__     : %s\n", showLEDstate(__TYDMASK__));
        fprintf(stderr, "   __SCROLL__      : %s\n", showLEDstate(__SCROLL__));
        fprintf(stderr, "   __CAPITAL__     : %s\n", showLEDstate(__CAPITAL__));
        fprintf(stderr, "   __NUMLOCK__     : %s\n", showLEDstate(__NUMLOCK__));
#else
        fprintf(stderr, "   INITSTATE       : %d\n", INITSTATE);
        fprintf(stderr, "   TYDMASK         : %d\n", __TYDMASK__);
#endif
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

    setvbuf (stdout, NULL, _IONBF, BUFSIZ);

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
    // TODO: tweak the '#defines'; or just never stop?
#define RETRIES 100
#define TIMEOUT 5
    hid = fopen(devicename, "rb");
    int tries = 0;
    while (hid == NULL) {
        tries++;
        fprintf(stderr, "ERR: Error opening %s: %s", devicename, STRERR);
        syslog(LOG_LOCAL7|LOG_ERR,"Error opening %s: %s", devicename, STRERR);
        if (tries >= RETRIES) {
            fprintf(stderr, "ERR: Exceeded maxtries %d. Exitting.\n",RETRIES);
        	syslog(LOG_LOCAL7|LOG_ERR,"Exceeded maxtries %d. Exitting.",RETRIES);
            exit(EXIT_FAILURE);
		}
        else {
            fprintf(stderr, "ERR: Retry %d of %d in %d seconds",tries,RETRIES,TIMEOUT);
            syslog(LOG_LOCAL7|LOG_ERR,"Retry %d of %d in %d seconds",tries,RETRIES,TIMEOUT);
        }
        sleep(TIMEOUT);
        hid = fopen(devicename, "rb");
    }
    syslog(LOG_LOCAL7|LOG_INFO,": Listen on %s", devicename);

    // -----------------
    // Check output location
    if (check_dir(outdir) != 0) {
        fprintf(stderr, "ERR: Error opening %s: %s", devicename, STRERR);
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
