/*
 * Minimal embedded framebuffer library.
 * Currently only supports 16-bit RGB565 format.
 */
#ifndef __VVC_UFB
#define __VVC_UFB

// C library includes
#include <stdint.h>

// Framebuffer macros.
#define UFB_ORIENT_V    ( 0 )
#define UFB_ORIENT_H    ( 1 )

// Define a simple monospace font; each character is 6x8 pixels,
// which comes out to 6 bytes or 3 words for every 2 characters.
#define UFB_CH_A0       0x1F688868
#define UFB_CH_B0       0xFF898989
#define UFB_CH_A1B1     0x1F007600
#define UFB_CH_C0       0x7E818181
#define UFB_CH_D0       0xFF818181
#define UFB_CH_C1D1     0x66007E00
#define UFB_CH_E0       0xFF898989
#define UFB_CH_F0       0xFF888888
#define UFB_CH_E1F1     0x81008000
#define UFB_CH_G0       0x7E818989
#define UFB_CH_H0       0xFF080808
#define UFB_CH_G1H1     0x6E00FF00
#define UFB_CH_I0       0x8181FF81
#define UFB_CH_J0       0x868181FE
#define UFB_CH_I1J1     0x81008000
#define UFB_CH_K0       0xFF182442
#define UFB_CH_L0       0xFF010101
#define UFB_CH_K1L1     0x81000100
#define UFB_CH_M0       0xFF403040
#define UFB_CH_N0       0xFF601806
#define UFB_CH_M1N1     0xFF00FF00
#define UFB_CH_O0       0x7E818181
#define UFB_CH_P0       0xFF888888
#define UFB_CH_O1P1     0x7E007000
#define UFB_CH_Q0       0x7E818582
#define UFB_CH_R0       0xFF888C8A
#define UFB_CH_Q1R1     0x7D007100
#define UFB_CH_S0       0x66919989
#define UFB_CH_T0       0x8080FF80
#define UFB_CH_S1T1     0x66008000
#define UFB_CH_U0       0xFE010101
#define UFB_CH_V0       0x701C031C
#define UFB_CH_U1V1     0xFE00E000
#define UFB_CH_W0       0xFE010601
#define UFB_CH_X0       0xC3241824
#define UFB_CH_W1X1     0xFE00C300
#define UFB_CH_Y0       0xE0100F10
#define UFB_CH_Z0       0x838599A1
#define UFB_CH_Y1Z1     0xE000C100
#define UFB_CH_a0       0x06292929
#define UFB_CH_b0       0xFF090909
#define UFB_CH_a1b1     0x1F000600
#define UFB_CH_c0       0x1E212121
#define UFB_CH_d0       0x060909FF
#define UFB_CH_c1d1     0x12000100
#define UFB_CH_e0       0x3E494949
#define UFB_CH_f0       0x087F8888
#define UFB_CH_e1f1     0x3A006000
#define UFB_CH_g0       0x32494949
#define UFB_CH_h0       0xFF080808
#define UFB_CH_g1h1     0x3E000700
#define UFB_CH_i0       0x00004F00
#define UFB_CH_j0       0x0006015E
#define UFB_CH_i1j1     0x00000000
#define UFB_CH_k0       0x00FF1C23
#define UFB_CH_l0       0x0000FF00
#define UFB_CH_k1l1     0x00000000
#define UFB_CH_m0       0x3F101F10
#define UFB_CH_n0       0x3F10100F
#define UFB_CH_m1n1     0x0F000000
#define UFB_CH_o0       0x0E111111
#define UFB_CH_p0       0x003F2424
#define UFB_CH_o1p1     0x0E001800
#define UFB_CH_q0       0x3048487E
#define UFB_CH_r0       0x003F1010
#define UFB_CH_q1r1     0x01000800
#define UFB_CH_s0       0x00324949
#define UFB_CH_t0       0x20FE2121
#define UFB_CH_s1t1     0x26000200
#define UFB_CH_u0       0x3C02023E
#define UFB_CH_v0       0x18060106
#define UFB_CH_u1v1     0x03001800
#define UFB_CH_w0       0x1E010201
#define UFB_CH_x0       0x110A040A
#define UFB_CH_w1x1     0x1E001100
#define UFB_CH_y0       0x3209093E
#define UFB_CH_z0       0x11131519
#define UFB_CH_y1z1     0x00001100
#define UFB_CH_00       0x7EE19987
#define UFB_CH_10       0x2141FF01
#define UFB_CH_0111     0x7E000100
#define UFB_CH_20       0x63878D99
#define UFB_CH_30       0x66818989
#define UFB_CH_2131     0x71007600
#define UFB_CH_40       0xF80808FF
#define UFB_CH_50       0xE2919191
#define UFB_CH_4151     0x08008E00
#define UFB_CH_60       0x7E919191
#define UFB_CH_70       0x60838CB0
#define UFB_CH_6171     0x4E00C000
#define UFB_CH_80       0x6E919191
#define UFB_CH_90       0x72898989
#define UFB_CH_8191     0x6E007E00
#define UFB_CH_col0     0x00002400
#define UFB_CH_per0     0x00000002
#define UFB_CH_col1per1 0x00000000
#define UFB_CH_exc0     0x007A0000
#define UFB_CH_fws0     0x00061860
#define UFB_CH_exc1fws1 0x00000000
#define UFB_CH_hyp0     0x00080808
#define UFB_CH_pls0     0x00081C08
#define UFB_CH_hyp1pls1 0x00000000
#define UFB_CH_lct0     0x00081422
#define UFB_CH_rct0     0x00442810
#define UFB_CH_lct1rct1 0x00000000
#define UFB_CH_cma0     0x00010306
#define UFB_CH_pct0     0x460C1830
#define UFB_CH_cma1pct1 0x00006200
#define UFB_CH_rpn0     0x000081C3
#define UFB_CH_lpn0     0x007EC381
#define UFB_CH_rpn1lpn1 0x7E000000

/* Helper macros. */
// Convert RGB to 565 format. Masks: R=0xF800 G=0x07E0 B=0x001F.
// TODO: Should this ( >> 3 ) instead of ( & 0x1F )?
#define rgb565( r, g, b ) \
  ( ( ( r & 0x1F ) << 11 ) | ( ( g & 0x3F ) << 5 ) | ( b & 0x1F ) )
// Draw a single pixel.
#define ufb_px( fb, c, x, y ) fb->buf[ x + y * fb->w ] = c;

/* Core framebuffer struct. */
typedef struct uFB {
  uint32_t w, h;
  uint16_t *buf;
} uFB;

/* Simple drawing functions. */
// Draw a horizontal line.
void ufb_hline( uFB* fb, uint16_t c, int32_t x, int32_t y, int32_t l );

// Draw a vertical line.
void ufb_vline( uFB* fb, uint16_t c, int32_t x, int32_t y, int32_t l );

// Draw a line between two points.
// TODO

// Outline a rectangle, with interior border thickness of 'o'.
void ufb_draw_rect( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t o );

// Fill a rectangle.
void ufb_fill_rect( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t w, uint32_t h );

// Outline a circle with a given interior width.
void ufb_draw_circle( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t r, uint32_t o );

// Fill a circle.
void ufb_fill_circle( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t r );

/* Text-drawing methods. */
// Draw a single letter of monospace 6x8 font.
// TODO: Support different orientations at 90-degree increments.
void ufb_draw_letter( uFB* fb, uint16_t color,
                        int x, int y, uint32_t w0, uint32_t w1,
                        int scale, int orient );

// Draw a single character to the framebuffer.
void ufb_draw_char( uFB* fb, uint16_t color,
                    int x, int y, char c,
                    int scale, int orient );

// Draw a <=32-bit integer to the display.
void ufb_draw_int( uFB* fb, uint16_t color, int x, int y, int ic, int scale, int orient );

// Draw a null-terminated C-string.
void ufb_draw_str( uFB* fb, uint16_t color, int x, int y, const char* cstr, int scale, int orient );

// Draw a null-terminated C-string, moving down a line every newline.
void ufb_draw_lines( uFB* fb, uint16_t color, int x, int y, const char* cstr, int scale, int orient );

#endif
