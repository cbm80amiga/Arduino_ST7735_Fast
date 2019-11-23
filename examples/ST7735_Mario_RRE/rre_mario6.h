#ifndef __font_mario6_h__
#define __font_mario6_h__

/*
  *** Generated by rrefontgen ***
  Font:         [mario6] 48x64
  Total chars:  4 (' ' to '#')
  Total rects:  48 * 3 bytes
  Total pixels: 1099 (409 overlapping)
  Total bytes:  152 (144 rects + 8 offs)
  Bitmap size:  1536 (48x64 * 4) (+4 opt)
*/

const unsigned char fontmario6_Rects[144] PROGMEM = {
	0x9e,0x58,0x07, 0xed,0x87,0x01, 0xa9,0x15,0x45, 0xab,0x52,0x42, 0x61,0x1f,0x04, 0xef,0x48,0x00, 0x25,0xd7,0x03, 0x1d,0x17,0x03, 0xac,0x07,0x00, 0xac,0x0f,0x00, 0x6e,0x27,0x01, 0x23,0x21,0x01, 0x27,0x27,0x01, 0x2a,0x14,0x00, 0x1d,0x16,0x00, 0x28,0x16,0x00, 
	0x1d,0x1d,0x00, 0x20,0x1f,0x00, 0x25,0x26,0x00, 0xa6,0xd8,0x09, 0x80,0x9b,0x0b, 0x0b,0x4c,0x45, 0x51,0x95,0x01, 0xc0,0x0a,0x03, 0x40,0x26,0x06, 0x89,0x98,0x06, 0xc9,0x0c,0x01, 0x53,0x56,0x00, 0x06,0xda,0x03, 0x40,0x08,0x01, 0x4e,0x0a,0x01, 0x05,0x0d,0x03, 
	0x51,0xd2,0x00, 0x00,0x1a,0x02, 0xc8,0x99,0x04, 0x4c,0x07,0x00, 0x4d,0x08,0x00, 0x4a,0x10,0x00, 0xc0,0xd9,0x00, 0x0b,0x06,0x00, 0x02,0x09,0x00, 0x0e,0x09,0x00, 0x10,0x0b,0x00, 0x04,0x0c,0x00, 0x00,0x0e,0x00, 0x08,0x0e,0x00, 0x0d,0x23,0x00, 0x4a,0x17,0x0a, 
};

const unsigned short fontmario6_CharOffs[5] PROGMEM = {
	0x0000, 0x0000, 0x0000, 0x0014, 0x0030, 
};

RRE_Font rre_mario6 = { RRE_24B, 48,64, 0x20, 0x23, (const uint8_t*)fontmario6_Rects, (const uint16_t*)fontmario6_CharOffs };

#endif

