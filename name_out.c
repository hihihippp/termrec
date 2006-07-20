#include "config.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "formats.h"
#include "stream.h"
#include "name_out.h"


extern char *optarg;
extern int optopt;

int codec_from_ext_rec(char *name)
{
    int i,len;

    len=0;
    for (i=0;compressors[i].name;i++)
        if (compressors[i].ext && match_suffix(name, compressors[i].ext, 0))
        {
            len=strlen(compressors[i].ext);
            break;
        }
    for (i=0;rec[i].name;i++)
        if (rec[i].ext && match_suffix(name, rec[i].ext, len))
            return i;
    return -1;
}


#if (defined HAVE_GETOPT_LONG) && (defined HAVE_GETOPT_H)
# define _GNU_SOURCE
# include <getopt.h>
struct option rec_opts[]={
{"format",	1, 0, 'f'},
{"exec",	1, 0, 'e'},
{"help",	0, 0, 'h'},
{0,		0, 0, 0},
};

struct option proxy_opts[]={
{"format",	1, 0, 'f'},
{"local-port",	1, 0, 'l'},
{"listen-port",	1, 0, 'l'},
{"port",        1, 0, 'p'},
{"help",	0, 0, 'h'},
{0,		0, 0, 0},
};
#endif

#define REC	0
#define PROXY	1

void get_parms(int argc, char **argv, int prog)
{
    char *cp;
    int i;

    codec=-1;
    command=0;
    record_name=0;
    lport=-1;
    rport=-1;
    
    while(1)
    {
#if (defined HAVE_GETOPT_LONG) && (defined HAVE_GETOPT_H)
        switch(getopt_long(argc, argv, (prog==REC)?"f:e:h":"f:l:h",
                                       (prog==REC)?rec_opts:proxy_opts, 0))
#else
        switch(getopt(argc, argv, (prog==REC)?"f:e:h":"f:l:h"))
#endif
        {
        case -1:
            goto finish_args;
        case ':':
        case '?':
            exit(1);
        case 'f':
            if (codec!=-1)
                error("You can use only one format at a time.\n");
            for(i=0;play[i].name;i++)
                if (!strcasecmp(rec[i].name, optarg))
                {
                    codec=i;
                    break;
                }
            if (codec==-1)
            {
                fprintf(stderr, "No such format: %s\n", optarg);
                fprintf(stderr, "Valid formats:\n");
                for(i=0;play[i].name;i++)
                    fprintf(stderr, " %-15s (%s)\n", play[i].name, play[i].ext);
                exit(1);
            }
            break;
        case 'e':
            if (command)
                error("You can specify -e only once.\n");
            command=optarg;
            break;
        case 'l':
            if (lport!=-1)
                error("You can specify -l only once.\n");
            i=0;
            cp=optarg;
            while(isdigit(*cp))
            {
                i=i*10+*cp++-'0';
                if (i>65535)
                    error("Too high local port number.\n");
            }
            if (*cp || !i)
                error("Invalid local port.\n");
            lport=i;
            break;
        case 'p':
            if (rport!=-1)
                error("You can specify the remote port only once.\n");
            i=0;
            cp=optarg;
            while(isdigit(*cp))
            {
                i=i*10+*cp++-'0';
                if (i>65535)
                    error("Too high remote port number.\n");
            }
            if (*cp || !i)
                error("Invalid remote port.\n");
            rport=i;
            break;
        case 'h':
            switch(prog)
            {
            case REC:
                printf(
                    "Usage: termrec [-f format] [-e command] [file]\n"
                    "    Records the output of a console session to a file, including timing data.\n"
                    "-f, --format X        set output format to X (-f whatever for the list)\n"
                    "-e, --exec X          execute command X instead of spawning a shell\n"
                    "-h, --help            show this usage message\n"
                    "If no filename is given, a name will be generated using the current date\n"
                    "    and the given format.\n"
                    "If no format is given, it will be set according to the extension of the\n"
                    "    filename, or default to .ttyrec.bz2 if nothing is given.\n"
                    "You can specify compression by appending .gz or .bz2 to the file name.\n");
                break;
            case PROXY:
                printf(
                    "Usage: proxyrec [-f format] [-p rport] [-l lport] host[:port] [file]\n"
                    "    Sets up a proxy, recording all TELNET traffic to a file, including timing data.\n"
                    "-f, --format X        set output format to X (-f whatever for the list)\n"
                    "-l, --local-port X    listen on port X locally (default: 9999)\n"
                    "-p, --port X          connect to remote portX (default: 23)\n"
                    "-h, --help            show this usage message\n"
                    "The host to connect to must be specified.\n"
                    "If no filename is given, a name will be generated using the current date\n"
                    "    and the given format; the proxy will also allow multiple connections.\n"
                    "If no format is given, it will be set according to the extension of the\n"
                    "    filename, or default to .ttyrec.bz2 if nothing is given.\n"
                    "You can specify compression by appending .gz or .bz2 to the file name.\n");
            }
            exit(0);
        }
    }
finish_args:
    if (optind+prog>argc)
        error("You MUST specify the host to connect to!\n");
    if (optind+1+prog<argc)
        error("You can specify at most one file to record to.\n");
    if (prog==PROXY)
        if ((cp=strrchr(command=argv[optind], ':')))
        {
            *cp=0;
            cp++;
            i=0;
            while(isdigit(*cp))
            {
                i=i*10+*cp++-'0';
                if (i>65535)
                    error("Too high remote port number.\n");
            }
            if (*cp || !i)
                error("Invalid remote port in the host part.\n");
            if (rport!=-1)
                error("You can specify the remote port just once.\n");
            rport=i;
        }
    if (optind+prog<argc)
    {
        record_name=argv[argc-1];
        if (codec==-1)
            if ((codec=codec_from_ext_rec(record_name))==-1)
                codec=1;  /* default to ttyrec */
    }
    else
        if (codec==-1)
            codec=1;
}


FILE *fopen_out(char **file_name, int nodetach)
{
    int i,fd;
    char add[2],date[24];
    time_t t;
    FILE *f;
    
    if (!*file_name)
    {
        add[0]=add[1]=0;
        time(&t);
        strftime(date, sizeof(date), "%Y-%m-%d.%H-%M-%S", localtime(&t));
        for(i='a';i<='z';i++)
        {
            asprintf(file_name, "%s%s%s"
#if (defined HAVE_LIBBZ2) || (defined SHIPPED_LIBBZ2)
                                      ".bz2"
#else
# if (defined HAVE_LIBZ) || (defined SHIPPED_LIBZ)
                                      ".gz"
# endif
#endif
            , date, add, rec[codec].ext);
            fd=open(*file_name, O_CREAT|O_WRONLY|O_EXCL|O_BINARY, 0666);
            /* We do some autoconf magic to exclude O_BINARY when inappropiate. */
            if (fd!=-1)
            {
                f=fdopen(fd, "wb");
                goto finish;
            }
            free(*file_name);
            *file_name=0;
        }
        error("Can't assign valid name in the current directory.\n");
    }
    if (!(f=fopen(*file_name, "wb")))
        error("Can't write to the record file (%s).\n", *file_name);
finish:
    return stream_open(f, *file_name, "wb", compressors, nodetach);
}