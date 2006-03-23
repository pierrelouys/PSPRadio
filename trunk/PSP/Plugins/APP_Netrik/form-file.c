/*
   netrik -- The ANTRIK Internet Viewer
   Copyright (C) Olaf D. Buddenhagen AKA antrik, et al (see AUTHORS)
   Published under the GNU GPL; see LICENSE for details.
*/

/*
 * form-file.c -- file handling for "textarea" and "file" form controls
 *
 * (C) 2003 antrik
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "form-file.h"

enum File_err {
   ERR_NO=0,
   ERR_OPEN,
   ERR_SIZE,
   ERR_READ
};

static enum File_err read_file(const char *file_name, struct Data_string *value);

/* 
 * Read file into memory.
 *
 * The file file_named "file_name" is opened and its content read into a newly
 * malloced buffer; this buffer is stored in the "value" data string.
 *
 * If some error occured while reading the file, an appropriate error code is
 * returned.
 */

static enum File_err read_file(file_name, value)
const char		*file_name;
struct Data_string	*value;
{
   int		fildes;
   struct stat	stat_buf;

   int		size;
   char		*data;

   fildes=open(file_name, O_RDONLY);
   if(fildes==-1) {
      unlink(file_name);
      return ERR_OPEN;
   }

   if(fstat(fildes, &stat_buf)==-1) {
      close(fildes);
      unlink(file_name);
      return ERR_SIZE;
   }
   size=stat_buf.st_size;

   data=malloc(size);
   if(read(fildes, data, size)!=size) {
      close(fildes);
      unlink(file_name);
      free(data);
      return ERR_READ;
   }

   close(fildes);

   /* store new data */
   free(value->data);
   value->data=data;
   value->size=size;

   return ERR_NO;
}    /* read file */

/*
 * Use external editor to edit the value of a "textarea" form input field.
 *
 * Creates a temporary file with the old contents of the input field, launches
 * editor on that file, and reads new contents back.
 *
 * If some error occured in the system calls, a string describing the error is
 * returned; NULL means no error.
 */

char *edit_textarea(value, name)
struct Data_string	*value;
char			*name;
{
   /* external editor to use */
   char	*editor;
   editor=getenv("EDITOR");
   if(editor==NULL)
      editor="vi";

   /* write file/edit/read file */
   {
      /* get temp file name */
      static const char	format[]="netrik-textarea-%s-XXXXXX";
      const int		size=sizeof(format)+strlen(name);
      char		temp_name[size];
      snprintf(temp_name, size, format, name);

      /* write temporary file */
      {
	 int	fildes;
	 
	 fildes=mkstemp(temp_name);
	 if(fildes==-1) {
	    return "failed to create temporary file";
	 }

	 if(write(fildes, value->data, value->size)!=value->size) {
	    close(fildes);
	    unlink(temp_name);
	    return "failed to write to temporary file";
	 }

	 if(close(fildes)==-1) {
	    unlink(temp_name);
	    return "failed to close temporary file after writing";
	 }
      }

      /* edit/read file */
      {
	 /* get edit command */
	 static const char	format[]="%s %s";
	 const int		size=sizeof(format)+strlen(temp_name)+strlen(editor);
	 char			edit_cmd[size];
	 snprintf(edit_cmd, size, format, editor, temp_name);

	 /* edit */
	 if(system(edit_cmd)!=0) {
	    unlink(temp_name);
	    return "invoking editor failed";
	 }

	 /* read temporary file */
	 {
	    enum File_err	error;

	    error=read_file(temp_name, value);
	    switch(error) {
	       case ERR_NO: break;
	       case ERR_OPEN: return "failed to reopen temporary file";
	       case ERR_SIZE: return "failed to get file size";
	       case ERR_READ: return "failed to read from temporary file";
	    }
	 }    /* read file */

      }    /* edit/read file */

      unlink(temp_name);
   }    /* write file/edit/read file */

   return NULL;    /* no error */
}

/* 
 * Read a file into memory.
 *
 * The file contents is stored in a data string which is returned to the
 * caller.
 *
 * If some error occured while reading the file, the "size" attribute of the
 * data string is set to -1 to indicate no valid data.
 */

struct Data_string form_read_file(file_name)
const char		*file_name;
{
   struct Data_string	value={NULL, 0};
   enum File_err	error;

   error=read_file(file_name, &value);
   if(error!=ERR_NO) {
      static const char	*err_str[]={
	 "",    /* ERR_NO */
	 "open",
	 "get size of",
	 "read from"
      };
      
      fprintf(stderr, "\n\nfailed to %s file \"%s\" (to be submitted with form)\n", err_str[error], file_name);
      value.size=-1;
   }

   return value;
}
