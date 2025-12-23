/*****************************************************************************
 *   WDMAPUTL.C   V0.61                                 Worlds of Darkness   *
 *   Map Utility                                                             *
 *                                                                           *
 *   Description:    Worlds of Darkness is a 3D role-playing game.           *
 *                   During the course of the game many races, classes,      *
 *                   and items can be found through out the game-world       *
 *                                                                           *
 *                   GraphicWares & Wizard Software v0.61 03/21/1995         *
 *                   WG3.2+                        - R. Hadsall 02/25/2021   *
 *                   MBBS V10                      - R. Hadsall 12/23/2025   *
 *                                                                           *
 * Copyright (C) 2005-2025 Rick Hadsall.  All Rights Reserved.               *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      * 
 * it under the terms of the GNU Affero General Public License as published  *
 * by the Free Software Foundation, either version 3 of the License, or      *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *                                                                           *
 * Additional Terms for Contributors:                                        *
 * 1. By contributing to this project, you agree to assign all right, title, *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies.                              *
 * 2. You grant Rick Hadsall and Elwynor Technologies a non-exclusive,       *
 *    royalty-free, worldwide license to use, reproduce, prepare derivative  *
 *    works of, publicly display, publicly perform, sublicense, and          *
 *    distribute your contributions                                          *
 * 3. You represent that you have the legal right to make your contributions *
 *    and that the contributions do not infringe any third-party rights.     *
 * 4. Rick Hadsall and Elwynor Technologies are not obligated to incorporate *
 *    any contributions into the project.                                    *
 * 5. This project is licensed under the AGPL v3, and any derivative works   *
 *    must also be licensed under the AGPL v3.                               *
 * 6. If you create an entirely new project (a fork) based on this work, it  *
 *    must also be licensed under the AGPL v3, you assign all right, title,  *
 *    and interest, including all copyrights, in and to your contributions   *
 *    to Rick Hadsall and Elwynor Technologies, and you must include these   *
 *    additional terms in your project's LICENSE file(s).                    *
 *                                                                           *
 * By contributing to this project, you agree to these terms.                *
 *                                                                           *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#if defined(_MSC_VER)
#include <tlhelp32.h>
#include <tchar.h>
#include <io.h>
#include <stdint.h>
#include <fcntl.h>
#else
#include <dir.h>
#include <dirent.h>
#endif
#include "excphand.h"
#include "gcomm.h"

#define MAPWIDTH 70
#define FOPWB  "wb"
#define FOPRA  "rt" 
#define FAMDIR 0x10
#define FAMHID 0x02
#define FAMSYS 0x04
#define SPECIAL_BITS (FAMDIR|FAMHID|FAMSYS)

#ifndef MAXPATH
#define MAXPATH 260
#endif

#ifdef free
  #undef free
#endif
#ifdef malloc
  #undef malloc
#endif
#ifdef printf
  #undef printf
#endif

short calcrc(short oldcrc, char ch);
short mapcrc(char *bp, unsigned short blen);
void *alcmem(unsigned int size);
short fndnxt(struct ffblk *fbptr);
short fndfile(struct ffblk *fb, const char *fname, int attr);
short fnd1st(struct ffblk *fbptr, const char *filspc, int attr);
int check_directory_exists(const char *path);
int create_directory(const char *path);
int count_map_files(const char *path);
void sqzmaps(void);

short calcrc(short oldcrc, char ch) 
{
	short j;
	
	oldcrc^=(ch<<8);
	for (j=0; j < 8; j++) {
		if (oldcrc&0x8000) {
			oldcrc=(oldcrc<<1)^0x1021;
		} else {
			oldcrc=oldcrc<<1;
		}
	}
	
	return (oldcrc);	
}

short mapcrc(char *bp,unsigned short blen) {
  short crc=0,i;
 
  for (i=0; i<blen; i++) {
    crc=calcrc(crc,bp[i]);
  }
 
  return(crc);
}

void *                             /*   returns ptr to newly allocated mem */
alcmem(                            /* allocate dynamic memory              */
unsigned int size)                         /*   requested memory size              */
{
     void *retval;

     if ((retval=(void *)malloc(size)) == NULL) {
          printf("ERROR: Out of memory!!");
          exit(1);
     }
     return(retval);
}

// Visual Studio 2022 doesn't have the same file routines as Borland, so we create them
#if defined(_MSC_VER)

typedef struct _FFSTATE {
    HANDLE h;
    WIN32_FIND_DATAA wfd;
    int attr;
    int started;
} FFSTATE;

// store/retrieve a pointer in ffblk->ff_reserved safely
static void ff_set_state(struct ffblk* fb, FFSTATE* st)
{
    // in borland world, ff_reserved is typically char[21] in ffblk but it's a long in VS2022
    fb->ff_reserved = (long)(intptr_t)st;
}

static FFSTATE *ff_get_state(struct ffblk* fb)
{
    return (FFSTATE *)(intptr_t)(fb->ff_reserved);
}

//attribute filter
static int ff_attr_match(DWORD fileAttr, int wantAttr)
{
   // If requested attribute word includes hidden, system, or
   // subdirectory bits, return normal files AND those with
   // any of the requested attributes.
    if ( ((fileAttr & SPECIAL_BITS) == 0)
        || ((wantAttr & (int)(fileAttr & SPECIAL_BITS)) != 0) ) {
        return 1;
    }
    return 0;
}

static int ff_fill_fb(struct ffblk* fb, const WIN32_FIND_DATAA* wfd)
{
    FILETIME local;

    fb->ff_attrib = (unsigned char)wfd->dwFileAttributes;
    FileTimeToLocalFileTime(&wfd->ftLastWriteTime, &local);
    FileTimeToDosDateTime(&local, &fb->ff_fdate, &fb->ff_ftime);

    fb->ff_fsize = wfd->nFileSizeLow;
    strcpy(fb->ff_name, wfd->cFileName);

    return 0;
}

short find1st(struct ffblk* fbptr, const char* filspc, int attr)
{
    FFSTATE* st = (FFSTATE*)calloc(1, sizeof(FFSTATE));
    int found = 0;

    if (!st)
        return 0;

    st->attr = attr;
    st->h = FindFirstFileA(filspc, &st->wfd);
    if (st->h == INVALID_HANDLE_VALUE) {
        free(st);
        return 0;
    }

    ff_set_state(fbptr, st);

    // find first matching file
    for (;;) {
        if (ff_attr_match(st->wfd.dwFileAttributes, st->attr)) {
            ff_fill_fb(fbptr, &st->wfd);
            found = 1;
            break;
        }
        if (!FindNextFileA(st->h, &st->wfd)) {
            break;
        }
    }

    if (!found) {
        FindClose(st->h);
        fbptr->ff_reserved = 0; // ff_set_state(fbptr, NULL);
        free(st);
        return 0;
    }

    return 1;
}

// Galacticomm provides fndnxt() and fndfile().
// If we did not include the Galacticomm libs, we'd make this fndnxt and bring fndfile into this block
short _fndnxt(struct ffblk* fbptr)
{
    FFSTATE* st = ff_get_state(fbptr);

    if (!st || st->h == INVALID_HANDLE_VALUE)
        return 0;

    while (FindNextFileA(st->h, &st->wfd)) {
        if (ff_attr_match(st->wfd.dwFileAttributes, st->attr)) {
            ff_fill_fb(fbptr, &st->wfd);
            return 1;
        }
    }

    // no more files, close and free
    FindClose(st->h);
    fbptr->ff_reserved = 0; // ff_set_state(fbptr, NULL);
    free(st);
    return 0;
}
#else

short                              /*   returns TRUE=found a file          */
fndnxt(                            /* find next file matching filspc & attr*/
struct ffblk *fbptr)               /*   ptr to data storage area           */
{
     return(findnext((struct ffblk *)fbptr) == 0);
}

short                              /*   returns TRUE=found a file          */
fnd1st(                            /* find file matching filspc and attr   */
    struct ffblk* fbptr,               /*   ptr to data storage area           */
    const char* filspc,                /*   file spec to match                 */
    int attr)                          /*   attribute to match                 */
{
    return(findfirst(filspc, (struct ffblk*)fbptr, attr) == 0);
}


short                              /*   TRUE if matching file found        */
fndfile(                           /* check file's existence and stats     */
struct ffblk *fb,                  /*   file info (CLOSED after findfirst) */
const char *fname,                 /*   filespec we're searching for       */
int attr)                          /*   attribute(s) to match              */
{
     short     found;              /* TRUE if we found a match             */

     /*
      *   Most of this is per the logic in Borland's RTL.
      *
      *   The difference is that we close the search handle before
      *   exiting -- Borland leaves the handle open, and we end up
      *   leaking handles like a sieve.
      */
     HANDLE    fhand=INVALID_HANDLE_VALUE;  /* our search handle           */
     WIN32_FIND_DATA ff;           /* WIN32 search info                    */
     FILETIME local;               /* filetime translated to local time    */

     for (;;)
     {
          if (fhand == INVALID_HANDLE_VALUE) {
               fhand=FindFirstFile(fname,&ff);
               found=(fhand != INVALID_HANDLE_VALUE);
          }
          else {
               found=FindNextFile(fhand,&ff);
          }

          if (!found) {
               break;
          }
          /* If requested attribute word includes hidden, system, or
           * subdirectory bits, return normal files AND those with
           * any of the requested attributes.
           */
          else if ((ff.dwFileAttributes & SPECIAL_BITS) == 0
                || (attr & ff.dwFileAttributes & SPECIAL_BITS) != 0) {
               break;
          }
     }

     if (fhand != INVALID_HANDLE_VALUE) {
          FindClose(fhand);
     }

     if (found) {
          /* Fill in the find_t structure from the NT structure. */
          fb->ff_attrib = ff.dwFileAttributes;

          /* Convert times from GM to Local */
          FileTimeToLocalFileTime(&ff.ftLastWriteTime, &local);
          FileTimeToDosDateTime(&local, &fb->ff_fdate, &fb->ff_ftime);

          fb->ff_fsize = ff.nFileSizeLow;
          strcpy(fb->ff_name, ff.cFileName);
     }
     fb->ff_reserved=0;

     return(found);
}
#endif



// returns 1 if the directory exists
int check_directory_exists(const char *path) {
  struct stat info;
    
  if (stat(path, &info) != 0) {
    return 0; // Directory does not exist
  } else if (info.st_mode & S_IFDIR) {
    return 1; // Directory exists
  }

  return 0; // Not a directory
}

// checks for the output directory, creates it if not present
// returns failure if unable to do so
int create_directory(const char *path) {
#ifdef _WIN32
    return mkdir(path);
#else
    return mkdir(path, 0755); // for LINUX
#endif
}

// checks for the presence of MAP files in a folder
// returns the count of files that match
// returns 0 if unable to open the directory
int count_map_files(const char *path) {
  int count = 0;

#if defined(_MSC_VER)
  struct _finddata_t fd;
  intptr_t hFile;
  char search[MAX_PATH];

  // build search pattern: path\*.MAP
  snprintf(search, sizeof(search), "%s\\*.MAP", path);

  hFile = _findfirst(search, &fd);
  if (hFile == -1) {
        return 0; // Unable to open directory or no files found
    }

  do {
      if (!(fd.attrib & _A_SUBDIR)) {
          count++;
      }
  } while (_findnext(hFile, &fd) == 0);

  _findclose(hFile);

#else
  DIR* dp;
  struct dirent *entry;

  dp = opendir(path);
  if (dp == NULL) {
    return 0; // Unable to open directory
  }

  while ((entry = readdir(dp)) != NULL) {
    if (strstr(entry->d_name, ".MAP") != NULL) {
      count++;
    }
  }

  closedir(dp);
#endif

  return count;
}


void sqzmaps(void) {
	short done;
  unsigned short bptr; //,rrow; //srclen
  char fch,tchar,srcpos,stopme,
       *bufr;
  unsigned char chcnt;
  char ifn[60],ofn[60],srcline[210];
  short crc;
  FILE *sysin,*sysout;
  struct ffblk utlfblk;
  
  printf("Beginning the map squeezing process...\n");
 
  done=fnd1st(&utlfblk,"MAP\\*.MAP",0); // was fnd1st
  bufr=(char *)alcmem(8000);
  while(done) {
    strcpy(ifn,"MAP\\");
    strcpy(ofn,"SQZMAP\\");
    strcat(ifn,utlfblk.ff_name);
    strcat(ofn,utlfblk.ff_name);
    printf("squeezing %s to %s \n",ifn,ofn);

    if ( (sysin=fopen(ifn,FOPRA))!=NULL
         && (sysout=fopen(ofn,FOPWB))!=NULL) {
      fgets(srcline,200,sysin);
      srcpos=1;
      bptr=0;
      fch=srcline[0];
      chcnt=1;
      //srclen=1;
      //rrow=1;
      stopme=0;
      while(!stopme) {
        tchar=srcline[srcpos];
        //srclen++;
        if (fch==tchar) {
          if (chcnt<255) chcnt++;
          else {
            bufr[bptr]=fch;
            bptr++;
            bufr[bptr]=chcnt;
            bptr++;
            chcnt=1;
          }
        } else {
          bufr[bptr]=fch;
          bptr++;
          bufr[bptr]=chcnt;
          bptr++;
          chcnt=1;
          fch=tchar;
        }
        srcpos++;
        if (srcpos==MAPWIDTH) {
          if (feof(sysin)) stopme=1;
          else {
            fgets(srcline,200,sysin);
            //rrow++;
            srcpos=0;
          }
        }
      }
      bufr[bptr]=fch;
      bptr++;
      bufr[bptr]=chcnt;
      bptr++;

      crc=mapcrc(bufr,bptr);

      fwrite(&bptr,sizeof(unsigned short),1,sysout);
      fwrite(&crc,sizeof(short),1,sysout);
      fwrite(bufr,1,bptr,sysout);
      fclose(sysin);
      fclose(sysout);
    }
    done=fndnxt(&utlfblk);
  }
  free(bufr);
  printf("Map squeezing process complete!\n");
  
}

#if defined(_MSC_VER)

static void ensure_console(void)
{
    // If launched from an existing console (cmd.exe), attach to it.
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // redirect stdout/stderr to the attached console
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        return;
    }

    // Otherwise (double-clicked), create our own console
    if (AllocConsole()) {
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }
}


int WINAPI WinMain(
	_In_     HINSTANCE hInstance,               // handle to current instance
	_In_opt_ HINSTANCE hPrevInstance,           // handle to previous instance
	_In_     LPSTR lpCmdLine,                   // pointer to command line
	_In_     int nCmdShow)                      // show state of window
{
  ensure_console();

#else
int main(int argc, char *argv[]) 
{
#endif
  printf("\nWDMAPUTL v0.62 - Copyright (c) 2025 Elwynor Technologies\n\n");
  
  // avoid compiler warnings
#if defined(_MSC_VER)
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nCmdShow;
#else
  (void)argc;
  (void)argv;
#endif
  
  // Check for the presence of the MAP subdirectory
  if (!check_directory_exists("MAP")) {
      fprintf(stderr, "Error: MAP subdirectory does not exist.\n");
      return EXIT_FAILURE;
  }
 
  // Check for the presence of the SQZMAP subdirectory, and create it if it doesn't exist
  if (!check_directory_exists("SQZMAP")) {
    if (create_directory("SQZMAP") != 0) {
      fprintf(stderr, "Error: Failed to create SQZMAP subdirectory.\n");
      return EXIT_FAILURE;
    } else {
      printf("Created SQZMAP subdirectory.\n");
    }
  }

  // Check for the presence of files that match the pattern *.MAP in the MAP subdirectory
  if (count_map_files("MAP") == 0) {
    fprintf(stderr, "Error: No *.MAP files found in the MAP subdirectory!\n");
    return EXIT_FAILURE;
  }

  // All seems ready to go!   
  sqzmaps();

#if defined(_MSC_VER)
  if (GetConsoleWindow() != NULL) {
      printf("\nPress Enter to exit...");
      (void) getchar();
  }
#endif

  return EXIT_SUCCESS;
}
