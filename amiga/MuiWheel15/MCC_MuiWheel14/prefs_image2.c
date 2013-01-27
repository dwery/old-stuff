#define PREFS_IMAGE2_WIDTH        11
#define PREFS_IMAGE2_HEIGHT       11
#define PREFS_IMAGE2_DEPTH         3
#define PREFS_IMAGE2_COMPRESSION   0
#define PREFS_IMAGE2_MASKING       2

#ifdef USE_PREFS_IMAGE_HEADER
const struct BitMapHeader prefs_image2_header =
{ 11,11,265,398,3,2,0,0,0,18,18,11,11 };
#endif

#ifdef USE_PREFS_IMAGE_BODY
const UBYTE prefs_image2_body[66] = {
0x04,0x00,0x00,0x00,0x00,0x00,0x6a,0xc0,0x64,0xc0,0x00,0x00,0x5b,0x40,0x2e,
0x80,0x00,0x00,0x20,0x80,0x11,0x00,0x00,0x00,0x60,0xc0,0x28,0x80,0x00,0x00,
0x86,0x20,0x62,0xc0,0x00,0x00,0x66,0xc0,0x26,0x80,0x00,0x00,0x20,0x80,0x11,
0x00,0x00,0x00,0x5b,0x40,0x2e,0x80,0x00,0x00,0x6a,0xc0,0x64,0xc0,0x00,0x00,
0x04,0x00,0x00,0x00,0x00,0x00, };
#endif
