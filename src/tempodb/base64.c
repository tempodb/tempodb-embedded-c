/*
 * Copyright (C), 2000-2007 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include "base64.h"

static char encode(unsigned char u);

/**
 *  Implementation of base64 encoding/decoding. 
 *
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: base64.c,v 1.19 2007/07/25 12:54:31 hauk Exp $
 *
 *  @file
 */



/* ------------------------------------------------------------------ Public */



/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char *encode_base64(int size, unsigned char *src) {

  int i;
  char *out, *p;

  if(!src)
    return NULL;

  if(!size)
    size= strlen((char *)src);
    
  out= malloc(sizeof(char) * size*4/3+4);

  p= out;
    
  for(i=0; i<size; i+=3) {
      
    unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
      
    b1 = src[i];
      
    if(i+1<size)
      b2 = src[i+1];
      
    if(i+2<size)
      b3 = src[i+2];
      
    b4= b1>>2;
    b5= ((b1&0x3)<<4)|(b2>>4);
    b6= ((b2&0xf)<<2)|(b3>>6);
    b7= b3&0x3f;
      
    *p++= encode(b4);
    *p++= encode(b5);
      
    if(i+1<size) {
      *p++= encode(b6);
    } else {
      *p++= '=';
    }
      
    if(i+2<size) {
      *p++= encode(b7);
    } else {
      *p++= '=';
    }

  }

  return out;

}


/* ----------------------------------------------------------------- Private */


/**
 * Base64 encode one byte
 */
static char encode(unsigned char u) {

  if(u < 26)  return 'A'+u;
  if(u < 52)  return 'a'+(u-26);
  if(u < 62)  return '0'+(u-52);
  if(u == 62) return '+';
  
  return '/';

}
