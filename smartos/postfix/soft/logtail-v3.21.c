/*---------------------------------------------------------------------------------*
* Log tail utility                                                                 *
*----------------------------------------------------------------------------------*
*  logtail.c -- ASCII file tail program that remembers last position.              *
*                                                                                  *
*  Author:                                                                         *
*  Ross Moffatt <ross.stuff@telstra.com>                                           *
*                                                                                  *
*  Please send me any hacks/bug fixes you make to the code. All comments are       *
*  welcome!                                                                        *
*                                                                                  *
* Modified from the utility: logtail                                               *
* Written by Craig H. Rowland <crowland@psionic.com>                               *
* Based upon original utility: retail (c)Trusted Information Systems               *
* Including the utility: dirname                                                   *
* Written by Piotr Domagalski <szalik@szalik.net>                                  *
* This program is covered by the GNU license.                                      *
*                                                                                  *
* Usage: logtail [log_file] <offset_file>                                          *
*    or: logtail [-l log_file] <-o offset_file> <-d rolled_log_directory>          *
*      : <-f rolled_log_pattern> <-t> <-b> <-r buffer size> <-s>                   *
*    or: logtail -h                                                                *
*                                                                                  *
* This program covered by the GNU License. This program is free to use as long as  *
* the above copyright notices are left intact.                                     *
* This program has no warranty of any kind.                                        *
*                                                                                  *
* To compile:                                                                      *
* Unix: gcc -D_OS_UNIX -o logtail logtail.c                                        *
* Windows: gcc -D_OS_DOS -o logtail logtail.c                                      *
* add -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 for large file aware build        *
*  I used mingw64 to compile the windows version.                                  *
*                                                                                  *
*  VERSION 2.0 : Log roll aware                                                    *
*  VERSION 2.1 : Fixes and Windows compatibility                                   *
*  VERSION 2.11: Minor help text and layout update                                 *
*  VERSION 3.0 : Path related fixes                                                *
*  VERSION 3.1 : Changed -? to -h, DOS_ to D_OS_; added -t, 32bit file too big chk *
*  VERSION 3.11: Fixed 32bit file too big chk bug                                  *
*  VERSION 3.2 : fseek(o),ftell(o),fgets to fgetpos,fsetpos,fgetc                  *
*                fgets,fprintf to fread,fwrite added -b -r -s options              *
*                fixed win extra line feed out bug                                 *
*  VERSION 3.21 : usage function, 32/64 bit aware, added debug, changed the logic. *
*----------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sysexits.h>
#ifdef _OS_DOS
  #include <fcntl.h>
#endif

#define MAX 2048
#define MY_MAX_PATH 2048
#define VERSION "3.21"

// Prototypes for functions
char* dirname(char* path);
char* nondirname(char* path);
int check_log(char* logname, char* offset_filename, char* oldlog_directory, char* oldlog_filename_pat, int testflag,int readbuffersize,int suppressflag, int debugflag);
void usage(int debugflag);
void short_usage(void);
char* right_string(char* my_path_file,int start_pos);

// It all starts here
int main(int argc, char *argv[])
{
  #ifdef _OS_DOS
    _setmode(1,_O_BINARY);
  #endif
  int status = 1; // Set status flag to error
  char offset_filename[MAX], log_filename[MAX], oldlog_dir[MAX], oldlog_pat[MAX], tempstr[MAX];
  char* tempstr_ptr;
  int i, my_switch, checkflag,
      readbuffersize=4096, // default read buffer size
      testflag=0, // default test off
      suppressflag=0, // default output stuff
      debugflag=0; // default debug off
 
  // ok now, we gotta sorta outa the clps
  strcpy(log_filename,"");
  strcpy(offset_filename,"");
  strcpy(oldlog_dir,"");
  strcpy(oldlog_pat,"");

  // any clps, too few, too many, well get outta here
  if ((argc == 1) || (argc > 14)) {
    short_usage();
    exit(EX_USAGE);
  }

  if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "-l") == 0) || (strcmp(argv[1], "-o") == 0) || (strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "-f") == 0) || (strcmp(argv[1], "-r") == 0) || (strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "-t") == 0) || (strcmp(argv[1], "-b") == 0)) {
    for ( i = 1; i < (argc); ++i ) {
      checkflag = 0;
      strcpy(tempstr,"");
      strcat(tempstr,argv[i]);
      if ((tempstr[0] == 45) && (strlen(argv[i]) == 2)) {
        my_switch=tempstr[1];

        switch (my_switch) {
        case 98: // and how about one of these -b ? remember to shove debug stuff out
                   debugflag = 1;
                   checkflag = 1;
                   break;
        case 100: // also looking fora -d ? if so set log dir to use
                   if (argc-1 > i) {
                     if ((strlen(argv[i+1])) > MY_MAX_PATH - 8 ) {
                       fprintf(stderr,"ERROR 118 - Log directory name %s is too long.\n",argv[i+1]);
                       exit(EX_DATAERR);
                     } else {
                       strcpy(oldlog_dir,argv[i+1]);
                       i++;
                       checkflag = 1;
                     }
                   }
                   break;
        case 102: // and how about one of these -f ? if so set log pattern to use
                   if (argc-1 > i) {
                     if ((strlen(argv[i+1])) > MY_MAX_PATH - 8 ) {
                       fprintf(stderr,"ERROR 132 - Log pattern name %s is too long.\n",argv[i+1]);
                       exit(EX_DATAERR);
                     } else {
                       strcpy(oldlog_pat,argv[i+1]);
                       i++;
                       checkflag = 1;
                     }
                   }
                   break;
        case 104: // do we have a -h, well then tellem what, how and exit.
                   usage(debugflag);
                   exit(EX_USAGE);
                   break;
        case 108: // have a -l ? if so set log file to use
                   if (argc-1 > i) {
                     if ((strlen(argv[i+1])) > MY_MAX_PATH - 8 ) {
                       fprintf(stderr,"ERROR 150 - Log filename %s is too long.\n",argv[i+1]);
                       exit(EX_DATAERR);
                     } else {
                       strcpy(log_filename,argv[i+1]);
                       i++;
                       checkflag = 1;
                     }
                   }
                   break;
        case 111: // what abouta -o ? if so set offset file to use
                   if (argc-1 > i) {
                     if ((strlen(argv[i+1])) > MY_MAX_PATH - 8 ) {
                       fprintf(stderr,"ERROR 164 - Offset filename %s is too long.\n",argv[i+1]);
                       exit(EX_DATAERR);
                     } else {
                       strcpy(offset_filename,argv[i+1]);
                       i++;
                       checkflag = 1;
                     }
                   }
                   break;
        case 114: // and how about one of these -r ? if so set read buffer size
                   if (argc-1 > i) {
                     readbuffersize=atoi(argv[i+1]);
                     if (( readbuffersize < 1 ) || ( readbuffersize > 1048576 )) {
                       fprintf(stderr,"ERROR 170 - Read Buffer %s is too small or too large.\n",argv[i+1]);
                       exit(EX_DATAERR);
                     }
                   i++;
                   checkflag = 1;
                   }
                   break;
        case 115: // and how about one of these -s ? suppress output
                   suppressflag = 1;
                   checkflag = 1;
                   break;
        case 116: // and how about one of these -t ? if remember we don't update the offset file
                   testflag = 1;
                   checkflag = 1;
                   break;
                }
      }
      // if the checkflag is still 0 - exit with error
      if ( checkflag == 0 ) {
        fprintf(stderr,"ERROR 181 - Input parameter error, %s.\n", argv[i]);
        exit(EX_DATAERR);
      }
    }
  } else {
    if (argc == 2) {
      // only program name and log name supplied
      if ((strlen(argv[1])) > MY_MAX_PATH - 8) {
        fprintf(stderr,"ERROR 191 - Log filename %s is too long.\n",argv[1]);
        exit(EX_DATAERR);
      }else strcpy(log_filename,argv[1]);
    }
    if (argc == 3) {
      // program name, log name and offset name supplied
      if ((strlen(argv[1])) > MY_MAX_PATH - 8 ) {
        fprintf(stderr,"ERROR 202 - Log filename %s is too long.\n",argv[1]);
        exit(EX_DATAERR);
      }else strcpy(log_filename,argv[1]);
      if ((strlen(argv[2])) > MY_MAX_PATH - 8 ) {
        fprintf(stderr,"ERROR 209 - Offset filename %s is too long.\n",argv[2]);
        exit(EX_DATAERR);
      }else strcpy(offset_filename,argv[2]);
    }
  }

  // ok, lets see we got everything . . .
  if (strcmp(log_filename, "") == 0 ) {
    if ( debugflag != 0 ) fprintf(stderr,"Debug 200 - No log_filename provided.\n");
    short_usage();
    exit(EX_DATAERR);
  }
  if (strcmp(offset_filename, "") == 0 ) {
    strcpy(tempstr,log_filename);
    strcpy(offset_filename,dirname(tempstr));
    strcpy(tempstr,log_filename);
    #ifdef _OS_UNIX
      strcat(offset_filename,"/offset.");
    #endif
    #ifdef _OS_DOS
      strcat(offset_filename,"\\offset.");
    #endif
    strcat(offset_filename,nondirname(tempstr));
  }

  if (strcmp(oldlog_dir, "") == 0 ) {
  // work out the directory from the filename
    strcpy(tempstr,log_filename);
    tempstr_ptr=dirname(tempstr);
    strcpy(oldlog_dir,tempstr_ptr);
  }

  if (strcmp(oldlog_pat, "") == 0 ) {
    strcpy(tempstr,log_filename);
    strcpy(oldlog_pat,nondirname(tempstr));
  }

  // Make sure the old log pattern is just a filename and no path
  i = strlen(oldlog_pat)+1;
  while (( oldlog_pat[i-1] != 47 ) && ( i > 1 )) i--;
  if (( oldlog_pat[i-1] != 47 ) && ( i == 1 )) i=i-1;
  tempstr_ptr=right_string(oldlog_pat,strlen(oldlog_pat)-i);
  strcpy(oldlog_pat,tempstr_ptr);

  if ( debugflag != 0 ) {
  fprintf(stderr,"Debug 259 - log_filename: %s\n",log_filename);
  fprintf(stderr,"Debug 260 - offset_filename: %s\n",offset_filename);
  fprintf(stderr,"Debug 261 - oldlog_dir: %s\n",oldlog_dir);
  fprintf(stderr,"Debug 262 - oldlog_pat: %s\n",oldlog_pat);
  fprintf(stderr,"Debug 263 - Read buffer size: %i\n",readbuffersize);
  if ( suppressflag != 0 ) fprintf(stderr,"Debug 264 - No log output\n");
  if ( testflag != 0 ) fprintf(stderr,"Debug 265 - No offset_file update\n");
  }

  status=check_log(log_filename, offset_filename, oldlog_dir, oldlog_pat, testflag, readbuffersize, suppressflag, debugflag); // check the logs

  if (status == 0) exit(EX_OK);
  else if(status == 1) exit(EX_SOFTWARE);
  else if(status == 2) exit(EX_NOINPUT);
  else if(status == 3) exit(EX_DATAERR);
  else if(status == 4) exit(EX_CANTCREAT);
  else {
    fprintf(stderr,"ERROR 280 - An unknown error has occurred\n\n");
     exit(EX_SOFTWARE);
  }
}

// Called functions follow . . . 

// a function to return the right part of a string from a
// given number of character into the string
char* right_string(char* my_path_file,int start_pos) {
  char* resultstr;
  char* tempstr;
  int counter=0;

  if (start_pos <= 0) start_pos = 1; // Minimum length
  resultstr = (char*)malloc(start_pos + 1);
  tempstr = resultstr;

  if (my_path_file)
  {
    while (*my_path_file++)
      counter++;
    while (start_pos-- >=0 && counter-- >=0) 
      my_path_file--;
    if (*my_path_file) { // don't copy an empty string
      while (*my_path_file)
      *tempstr++=*my_path_file++;
    }
  }
  *tempstr='\0';
  return resultstr;
}

// a function to return the path from a path/filename
char* dirname(char* path)
{
  int i;

  i = strlen(path)+1;
  #ifdef _OS_UNIX
    while (( path[i-1] != 47 ) && ( i > 1 )) { i--; }
    if (( path[i-1] != 47 ) && ( i == 1 )) {
  #endif
  #ifdef _OS_DOS
    while (( path[i-1] != 92 ) && ( i > 1 )) { i--; }
    if (( path[i-1] != 92 ) && ( i == 1 )) {
  #endif
     path = ".";
     return path;
  }
  path[i-1] = '\0';
  return path;
}

char* nondirname(char* path)
{
  int i;
  char* tempstr_ptr;

  i = strlen(path)+1;
  #ifdef _OS_UNIX
    while (( path[i-1] != 47 ) && ( i > 1 )) i--;
    if (( path[i-1] != 47 ) && ( i == 1 )) i=i-1;
  #endif
  #ifdef _OS_DOS
    while (( path[i-1] != 92 ) && ( i > 1 )) i--;
    if (( path[i-1] != 92 ) && ( i == 1 )) i=i-1;
  #endif
  tempstr_ptr=right_string(path,strlen(path)-i);
  strcpy(path,tempstr_ptr);
  return path;
}

// a function to check a log file
int check_log(char* logname, char* offset_filename, char* oldlog_directory, char* oldlog_filename_pat, int testflag, int readbuffersize, int suppressflag, int debugflag)
{
  FILE *input,    // Value user supplies for input file
  *old_input,     // Found filename log rolled to
  *offset_output; // name of the offset output file

  struct stat file_stat, file_stat_old;

  char old_logfile[MAX],      // Moved log file name
       old_logpathfile[MAX],  // Moved log path/file name
       my_tmp_str[MAX];

  char* buffer;

  DIR *dp;
  struct dirent *ep;

  fpos_t offset_position, offset_cur_posn;   // position in the file to offset

  long file_mod_time;
  int charsread=0;

  #if _FILE_OFFSET_BITS == 64
    long long inode_buffer=0, filesize_buffer=0;
  #else
    long inode_buffer=0, filesize_buffer=0;
  #endif

  // allocate buffer memory:
  buffer = (char*) malloc (readbuffersize+1);
  if (buffer == NULL) {
    fputs ("Memory error",stderr);
    exit (2);
  }

  // Check if the file exists in specified directory
  // Open as a binary in case the user reads in non-text files
  if((input=fopen(logname, "rb")) == NULL) {
    fprintf(stderr,"ERROR 390 - File %s cannot be read.\n",logname);
    exit(EX_OSFILE);
  }

  if((stat(logname,&file_stat)) != 0) { // load struct
    fprintf(stderr,"ERROR 396 - Cannot get %s file size.\n",logname);
    exit(EX_OSFILE);
  }

  if ( debugflag != 0 ) {
    #ifdef _OS_UNIX
      fprintf(stderr,"Debug 390 - file_stat.st_ino size: %i\n",sizeof(file_stat.st_ino));
    #endif
    fprintf(stderr,"Debug 391 - file_stat.st_size size: %i\n",sizeof(file_stat.st_size));
    fprintf(stderr,"Debug 392 - fpos_t size: %i\n",sizeof(fpos_t));
  }
  // 32 bits, exit if file too big
  if ((sizeof(fpos_t) == 4) || sizeof(file_stat.st_size) == 4) {
    if ((file_stat.st_size > 2147483646) || (file_stat.st_size < 0)) {
      fprintf(stderr,"ERROR 403 - log file, %s, is too large at %d bytes.\n", logname, file_stat.st_size);
      exit(EX_DATAERR);
    }
  }

  // see if we can open an existing offset file and read in the inode (unix),
  // offset and filesize
  if((offset_output=fopen(offset_filename, "rb")) != NULL) {
    #ifdef _OS_UNIX
      fread(&inode_buffer,sizeof(inode_buffer),1,offset_output);
    #endif
    fread(&offset_position,sizeof(offset_position),1,offset_output);
    fread(&filesize_buffer,sizeof(filesize_buffer),1,offset_output);
    fclose(offset_output); // We're done, clean up
  }else{ // can't read the file? then assume no offset file exists
    fgetpos (input,&offset_position);
    #ifdef _OS_UNIX
      // set the old inode number to the current file
      inode_buffer=file_stat.st_ino;
    #endif
    if (debugflag != 0) fprintf(stderr,"ERROR 425 - Assumed no offset file exists!\n");
  }

  if ( debugflag != 0 ) {
    if ( filesize_buffer > file_stat.st_size ) fprintf(stderr,"Debug 621 - smaller\n");
    if ( filesize_buffer < file_stat.st_size ) fprintf(stderr,"Debug 622 - larger\n");
    if ( filesize_buffer == file_stat.st_size ) fprintf(stderr,"Debug 623 - same\n");
  }

  #ifdef _OS_UNIX
  // if the current file inode is the same, but the file size has
  // grown SMALLER than the last time we checked, then assume
  // log has been rolled via a copy and delete.
  // look for the old file to output any extra,
  // reset the offset to zero.
  if((inode_buffer == file_stat.st_ino) && (filesize_buffer > file_stat.st_size)) {
    if ( debugflag != 0 ) fprintf(stderr,"Debug 457 - Log file has copied/smaller!, Log dir is: %s\n",oldlog_directory);
    // look in the log file directory for the most recent old_log_filename_pat<extn>
    strcpy(old_logfile,"NoFileFound");
    dp = opendir(oldlog_directory);
    if (dp != NULL) {
      file_mod_time = 0;
      while (ep = readdir (dp))
      if (strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0) {
        // put the directory and the filename back together
        strcpy(old_logpathfile, oldlog_directory);
        strcat(old_logpathfile, "/");
        strcat(old_logpathfile, ep->d_name);

        if((stat(old_logpathfile,&file_stat_old)) != 0) { // load struct
          fprintf(stderr,"ERROR 476 - Cannot get %s file size.\n",ep->d_name);
          exit(EX_OSFILE);
        }
        if ((strncmp(ep->d_name,oldlog_filename_pat,strlen(oldlog_filename_pat)) ==0) && (strlen(ep->d_name) > strlen(oldlog_filename_pat)) && (file_stat_old.st_mtime > file_mod_time)) {
          file_mod_time = file_stat_old.st_mtime;
          strcpy(old_logfile,ep->d_name);
        }
      }
    }else{
      fprintf(stderr,"ERROR 488 - Couldn't open the directory: %s\n",oldlog_directory);
      exit(EX_OSFILE);
    }
    (void) closedir (dp);

    if ( debugflag != 0 ) fprintf(stderr,"Debug 494 - Old log file found: %s\n",old_logfile);

    // if we found a file, then deal with it, or bypass
    if (strcmp(old_logfile,"NoFileFound") != 0 ) {
      // put the directory and old filename back together
      strcpy(old_logpathfile, oldlog_directory);
      strcat(old_logpathfile, "/");
      strcat(old_logpathfile, old_logfile);
      if ( debugflag != 0 ) fprintf(stderr,"Debug 506 - Copied file is here: %s\n", old_logpathfile);

      // open the found filename
      if((old_input=fopen(old_logpathfile, "rb")) == NULL) {
        fprintf(stderr,"ERROR 512 - File %s cannot be read.\n",old_logpathfile);
        exit(EX_OSFILE);
      }

      // print out the old log stuff
      fsetpos(old_input,&offset_position);

      // Print the file
      do {
        buffer[0]=0;
        charsread = fread (buffer,1,readbuffersize,old_input);
        if ( suppressflag == 0 ) fwrite (buffer,1,charsread,stdout);
      } while (charsread == readbuffersize);
      fclose(old_input); // clean up
    }
    // set the offset back to zero and off we go looking at the current file
    // reset offset and report everything
    fgetpos(input,&offset_position);
  }

  // if the current file inode is different than that in the
  // offset file then assume it has been rotated via mv and recreated,
  // find the orig file and output latest from it, then set offset to zero
  if (inode_buffer != file_stat.st_ino) {
    if ( debugflag != 0 ) fprintf(stderr,"Debug 540 - Log file has moved!, Log dir is: %s\n",oldlog_directory);

    // look in the current log file directory for the same inode number
    strcpy(old_logfile,"NoFileFound");
    dp = opendir(oldlog_directory);
    if (dp != NULL) {
      while (ep = readdir (dp))
      if (strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0) {
        // put the directory and old filename back together
        strcpy(old_logpathfile, oldlog_directory);
        strcat(old_logpathfile, "/");
        strcat(old_logpathfile, ep->d_name);
        if((stat(old_logpathfile,&file_stat_old)) != 0) { /* load struct */
          fprintf(stderr,"ERROR 557 - Cannot get file %s details.\n",old_logpathfile);
          exit(EX_OSFILE);
        }
        if (inode_buffer == file_stat_old.st_ino) {
          if ( debugflag != 0 ) fprintf(stderr,"Debug 563 - Log file found!, Log is: %s\n",ep->d_name);
          strcpy(old_logfile,ep->d_name);
        }
      }
      (void) closedir (dp);
    }else{
      fprintf(stderr,"ERROR 570 - Couldn't open the directory: %s",oldlog_directory);
      exit(EX_OSFILE);
    }

    // if we found a file, then deal with it, or bypass
    if (strcmp(old_logfile,"NoFileFound") != 0 ) {
      // put the directory and old filename back together
      strcpy(old_logpathfile, oldlog_directory);
      strcat(old_logpathfile, "/");
      strcat(old_logpathfile, old_logfile);
      if ( debugflag != 0 ) fprintf(stderr,"Debug 583 - Moved file is here: %s\n", old_logpathfile);

      // open the found filename
      if((old_input=fopen(old_logpathfile, "rb")) == NULL) {
        fprintf(stderr,"ERROR 589 - File %s cannot be read.\n",old_logpathfile);
        exit(EX_OSFILE);
      }

      // print out the old log stuff
      fsetpos(old_input,&offset_position);

      // Print the file
      do {
        buffer[0]=0;
        charsread = fread (buffer,1,readbuffersize,old_input);
        if ( suppressflag == 0 ) fwrite (buffer,1,charsread,stdout);
      } while (charsread == readbuffersize);
      fclose(old_input); // clean up
    }
  
    // set the offset back to zero and off we go looking at the current file
    fgetpos(input,&offset_position);
  }
  #endif

  #ifdef _OS_DOS
    // if the current file has grown SMALLER than the last time we checked,
    // then assume the log has been rolled via a copy and delete.
    // Look for the old file to output any extra,

  if ( filesize_buffer > file_stat.st_size ) {
    if ( debugflag != 0 ) fprintf(stderr,"Debug 621 - Log file has copied/smaller!, Log dir is: %s\n",oldlog_directory);
    // look in the log file directory for the most recent old_log_filename_pat<extn>
    strcpy(old_logfile,"NoFileFound");
    dp = opendir(oldlog_directory);
    if (dp != NULL) {
      file_mod_time = 0;
      while (ep = readdir (dp))
      if (strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0) {
        // put the directory and the filename back together
        strcpy(old_logpathfile, oldlog_directory);
        strcat(old_logpathfile, "\\");
        strcat(old_logpathfile, ep->d_name);

        if((stat(old_logpathfile,&file_stat_old)) != 0) { // load struct
          fprintf(stderr,"ERROR 640 - Cannot get %s file size.\n",ep->d_name);
          exit(3);
        }
        if ((strncmp(ep->d_name,oldlog_filename_pat,strlen(oldlog_filename_pat)) ==0) && (strlen(ep->d_name) > strlen(oldlog_filename_pat)) && (file_stat_old.st_mtime > file_mod_time)) {
          file_mod_time = file_stat_old.st_mtime;
          strcpy(old_logfile,ep->d_name);
        }
      }
    }else{
      fprintf(stderr,"ERROR 652 - Couldn't open the directory: %s\n",oldlog_directory);
      exit(3);
    }
    (void) closedir (dp);

    if ( debugflag != 0 ) fprintf(stderr,"Debug 658 - Old log file found: %s\n",old_logfile);

    // if we found a file, then deal with it, or bypass
    if (strcmp(old_logfile,"NoFileFound") != 0 ) {
      // put the directory and old filename back together
      strcpy(old_logpathfile, oldlog_directory);
      strcat(old_logpathfile, "/");
      strcat(old_logpathfile, old_logfile);
      if ( debugflag != 0 ) fprintf(stderr,"Debug 670 - Copied file is here: %s\n", old_logpathfile);

      // open the found filename
      if((old_input=fopen(old_logpathfile, "rb")) == NULL) {
        fprintf(stderr,"ERROR 676 - File %s cannot be read.\n",old_logpathfile);
        exit(2);
      }

      // print out the old log stuff
      fsetpos(old_input,&offset_position);

      // Print the file
      do {
        buffer[0]=0;
        charsread = fread (buffer,1,readbuffersize,old_input);
        if ( suppressflag == 0 ) fwrite (buffer,1,charsread,stdout);
      } while (charsread == readbuffersize);
      printf("\n");
      fclose(old_input); // clean up
    }
    // set the offset back to zero and off we go looking at the current file
    // reset offset and report everything
    fgetpos(input,&offset_position);
  }
  #endif

  // print out the current log stuff
  if ( debugflag != 0 ) fprintf(stderr,"Debug 695 - print out the current log stuff: %s\n",logname);
  fsetpos(input,&offset_position);

  // Print the file
    if ( (suppressflag != 0 ) && ( debugflag != 0 )) fprintf(stderr,"Debug 709 - Output suppressed.\n");
    do {
      buffer[0]=0;
      charsread = fread(buffer,1,readbuffersize,input);
      if ( suppressflag == 0 ) fwrite(buffer,1,charsread,stdout);
    } while (charsread == readbuffersize);
  fgetpos(input,&offset_position);
  fclose(input);

  // after we are done we need to write the new offset
  // if testflag set, skip writing out the offset file
  if ( testflag == 0 ) {
    if((offset_output=fopen(offset_filename, "w")) == NULL) {
      fprintf(stderr,"ERROR 720 - File %s cannot be created. Check your permissions.\n",offset_filename);
      exit(EX_NOPERM);
    }else{
      if ((chmod(offset_filename,00660)) != 0) { // Don't let anyone read offset
        fprintf(stderr,"ERROR 725 - Cannot set permissions on file %s\n",offset_filename);
        exit(EX_NOPERM);
      }else{
        // write it
        #ifdef _OS_UNIX
          fwrite(&file_stat.st_ino,sizeof(file_stat.st_ino),1,offset_output);
        #endif
        fwrite(&offset_position,sizeof(offset_position),1,offset_output);
        fwrite(&file_stat.st_size,sizeof(file_stat.st_size),1,offset_output);
        fclose(offset_output);
      }
    }
  }
  free (buffer);
  return(0); /* everything A-OK */
}

void usage(int debugflag)
{
struct stat test_stat;

printf("\nlogtail\n");
printf("\nUsage: logtail [log_file] <offset_file>");
printf("\n   or: logtail [-l log_file] <-o offset_file> <-d rolled_log_directory>");
printf("\n     : <-f rolled_log_filename> <-r> <-s> <-t> <-b>");
printf("\n   or: logtail -h");
printf("\n\n Required Parameters:");
printf("\n      [log_file]           :the log file to open and tail output.");
printf("\n or   [-l log_file]");
printf("\n\n Optional parameters:");
printf("\n   -o offset_file          : filename to use for the Offset file.");
printf("\n      Default: offset.[log_file], in the same directory as [log_file])");
printf("\n   -f rolled_log_filename  : base Filename for rolled logs");
printf("\n      Default: [log_file]");
printf("\n   -d rolled_log_directory : Directory to look for rolled logs.");
printf("\n      Default: the directory [log_file] is located");
printf("\n   -r read buffer size     : size of the log file reads and output writes.");
printf("\n      Default: 4096");
printf("\n   -s                      : suppress output, update offset file only");
printf("\n   -t                      : test, no update to offset file, output only");
printf("\n   -b                      : deBug output");
printf("\n   -h                      : this help");
printf("\n\nlogtail will read in a file and output to stdout, unless the -s option");
printf("\n is specified. allowing a quick first time output of the <offset_file>.");
printf("\n\nAfter outputing the file, logtail will create a file called");
printf("\n<offset_file> that will contain the decimal offset and inode of the file");
printf("\nin ASCII format unless the -t option is specified.  The -t option allows");
printf("\nfor test runs without changing the where logtail reads the log from each");
printf("\ntime logtail is called by not updating/creating the offset file.");
printf("\n\nNext time logtail is run on [log_file] the <offset_file> is read and");
printf("\noutput begins at the saved offset. The -b option outputs debug messages");
printf("\nto stderr. The -r option specifies the log file read buffer, in bytes,");
printf("\nvariation may improve performance.");
printf("\nRotated log files will be automatically accounted for by looking at the");
printf("\nlog and,");
#ifdef _OS_UNIX
  printf(" if the inode has changed, looking for the old log with the");
  printf("\noriginal inode number in the <rolled_log_directory>. If the inode hasn't");
  printf("\nhasn't changed, and log is now smaller, looking for the old log, the most");
  printf("\nrecent file starting with same filename as [log_file].");
#endif
#ifdef _OS_DOS
 printf(" if the log is now smaller, looking for the rolled log in the");
 printf("\n<rolled_log_directory>, the most recent file starting with same filename");
 printf("\nas [log_file].");
#endif
printf("\n\nVersion: %s",VERSION);
#ifdef _OS_UNIX
  printf("\nComplied for *nix");
#endif
#ifdef _OS_DOS
 printf("\nComplied for DOS");
#endif

if ((sizeof(test_stat.st_size) < 8) || (sizeof(fpos_t) < 8)) printf("\nComplied with 32bit file pointers, warning: 2G file size limit.");
if ((sizeof(test_stat.st_size) >= 8) && (sizeof(fpos_t) >= 8)) printf("\nComplied with 64bit file pointers, large file aware.");
if ( debugflag != 0 ) printf("\nSize of test_stat.st_size = %i bytes",sizeof(test_stat.st_size));
if ( debugflag != 0 ) printf("\nSize of fpos_t = %i bytes",sizeof(fpos_t));
if ( debugflag != 0 ) printf("\nCompiled with DEBUG on");

printf("\n\nWritten by Ross Moffatt <ross.stuff@telstra.com>");
printf("\nModified from the utility: logtail");
printf("\nWritten by Craig H. Rowland <crowland@psionic.com>");
printf("\nBased upon original utility: retail (c)Trusted Information Systems");
printf("\nIncluding the utility: dirname");
printf("\nWritten by Piotr Domagalski <szalik@szalik.net>");
printf("\nThis program is covered by the GNU license.");
printf("\n\n This program covered by the GNU License. This program is free to");
printf("\n use as long as the above copyright notices are left intact. This");
printf("\n program has no warranty of any kind.\n");
#ifdef _OS_UNIX
  printf("\n");
#endif
}

void short_usage(void)
{
printf("\n  Usage: logtail [log_file] <offset_file>");
printf("\n     or: logtail [-l log_file] <-o offset_file> <-d rolled_log_directory>");
printf("\n       : <-f rolled_log_filename> <-r> <-s> <-t> <-b>");
printf("\n     or: logtail -h\n");
#ifdef _OS_UNIX
  printf("\n");
#endif
}

/* ------------------------------------------------------------------*/
/* logtail.c -- ASCII file tail program that remembers last position.*/
/*                                                                   */
/* Author:                                                           */
/* Craig H. Rowland <crowland@psionic.com> 15-JAN-96                 */
/*                  <crowland@vni.net>                               */
/*                                                                   */
/* Please send me any hacks/bug fixes you make to the code. All      */
/* comments are welcome!                                             */
/*                                                                   */
/* Idea for program based upon the retail utility featured in the    */
/* Gauntlet(tm) firewall protection package published by Trusted     */
/* Information Systems Inc. <info@tis.com>                           */
/*                                                                   */
/* This program will read in a standard text file and create an      */
/* offset marker when it reads the end. The offset marker is read    */
/* the next time logtail is run and the text file pointer is moved   */
/* to the offset location. This allows logtail to read in the next   */
/* lines of data following the marker. This is good for marking log  */
/* files for automatic log file checkers to monitor system events.   */
/*                                                                   */
/* This program covered by the GNU License. This program is free to  */
/* use as long as the above copyright notices are left intact. This  */
/* program has no warranty of any kind.                              */
/*                                                                   */
/* To compile as normal:                                             */
/*gcc -o logtail logtail.c                                           */
/*                                                                   */
/* To compile as large file aware:                                   */
/*gcc -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -o logtail logtail.c*/
/*                                                                   */
/* VERSION 1.1: Initial release                                      */
/*                                                                   */
/*         1.11: Minor typo fix. Fixed NULL comparison.              */
/*         1.2 : Modified to be large file aware, jhb & rjm          */
/* ------------------------------------------------------------------*/

/*  dirname
 *  Copyright (c) 2003 Piotr Domagalski <szalik@szalik.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
