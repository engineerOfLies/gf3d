#ifndef __GF3D_TEXT__H__
#define __GF3D_TEXT__H__
/**
 * gf3d_text
 * @license The MIT License (MIT)
   @copyright Copyright (c) 2015 EngineerOfLies
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

/**
 * @purpose the MGL text library provides common string support for fixed length
 *  strings.  For use with unlimited strings mgl will use glib's GString type
 */

#include "gf3d_types.h"
/**
 * constant lenth character buffers.
 * These will be used when the convenience of GString is not needed
 */

/**
 * @brief 16 character buffer used for short tags
 */
typedef char TextWord[16];
#define GF3DWORDLEN   16
#define gf3d_word_cmp(a,b) (strncmp(a,b,GF3DWORDLEN))
#define gf3d_word_cpy(dst,src) (strncpy(dst,src,GF3DWORDLEN))
#define gf3d_word_clear(a)  (memset(a,0,sizeof(char)*GF3DWORDLEN))
/**
 * @brief 128 character buffer used for statements
 */
typedef char TextLine[128];
#define GF3DLINELEN   128
#define gf3d_line_cmp(a,b) (strncmp(a,b,GF3DLINELEN))
#define gf3d_line_cpy(dst,src) (strncpy(dst,src,GF3DLINELEN))
#define gf3d_line_clear(a)  (memset(a,0,sizeof(char)*GF3DLINELEN))
/**
 * @brief 512 character buffer used for text blocks
 */
typedef char TextBlock[512];
#define GF3DTEXTLEN   512
#define gf3d_block_cmp(a,b) (strncmp(a,b,GF3DTEXTLEN))
#define gf3d_block_cpy(dst,src) (strncpy(dst,src,GF3DTEXTLEN))
#define gf3d_block_clear(a)  (memset(a,0,sizeof(char)*GF3DTEXTLEN))

#endif
