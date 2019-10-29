/*
 * Minimal embedded framebuffer library.
 * Currently only supports 16-bit RGB565 format.
 */
#include "ufb.h"

// Draw a horizontal line.
void ufb_hline( uFB* fb, uint16_t c, int32_t x, int32_t y, int32_t l ) {
  if ( y < 0 || y >= fb->h ) { return; }
  for ( int i = x; i < ( x + l ); ++i ) {
    if ( i >= 0 && i < fb->w ) { ufb_px( fb, c, i, y ); }
  }
}

// Draw a vertical line.
void ufb_vline( uFB* fb, uint16_t c, int32_t x, int32_t y, int32_t l ) {
  if ( x < 0 || x >= fb->w ) { return; }
  for ( int i = y; i < ( y + l ); ++i ) {
    if ( i >= 0 && i < fb->h ) { ufb_px( fb, c, x, i ); }
  }
}

// Draw a line between two points.
// TODO

// Outline a rectangle, with interior border thickness of 'o'.
void ufb_draw_rect( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t o ) {
  for ( int b = 0; b < o; ++b ) {
    ufb_hline( fb, col, x, y + b, w );
    ufb_hline( fb, col, x, ( y + h ) - ( b + 1 ), w );
    ufb_vline( fb, col, x + b, y, h );
    ufb_vline( fb, col, ( x + w ) - ( b + 1 ), y, h );
  }
}

// Fill a rectangle.
void ufb_fill_rect( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t w, uint32_t h ) {
  for ( int i = x; i < x + w; ++i ) {
    for ( int j = y; j < y + h; ++j ) {
      ufb_px( fb, col, i, j );
    }
  }
}

// Outline a circle with a given interior width.
void ufb_draw_circle( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t r, uint32_t o ) {
  int32_t x_min = x - r;
  int32_t y_min = y - r;
  int32_t x_max = x + r;
  int32_t y_max = y + r;
  if ( x_min < 0 ) { x_min = 0; }
  if ( x_max > fb->w ) { x_max = fb->w; }
  if ( y_min < 0 ) { y_min = 0; }
  if ( y_max > fb->h ) { y_max = fb->h; }
  uint32_t r_squared = r * r;
  uint32_t r_border = ( r - o ) * ( r - o );
  for ( int i = x_min; i < x_max; ++i ) {
    for ( int j = y_min; j < y_max; ++j ) {
      int32_t dx = ( i - x );
      int32_t dy = ( j - y );
      if ( ( dx * dx + dy * dy < r_squared ) &&
           ( dx * dx + dy * dy > r_border ) ) {
        ufb_px( fb, col, i, j );
      }
    }
  }
}

// Fill a circle.
void ufb_fill_circle( uFB* fb, uint16_t col, int32_t x, int32_t y, uint32_t r ) {
  int32_t x_min = x - r;
  int32_t y_min = y - r;
  int32_t x_max = x + r;
  int32_t y_max = y + r;
  if ( x_min < 0 ) { x_min = 0; }
  if ( x_max > fb->w ) { x_max = fb->w; }
  if ( y_min < 0 ) { y_min = 0; }
  if ( y_max > fb->h ) { y_max = fb->h; }
  uint32_t r_squared = r * r;
  for ( int i = x_min; i < x_max; ++i ) {
    for ( int j = y_min; j < y_max; ++j ) {
      int32_t dx = ( i - x );
      int32_t dy = ( j - y );
      if ( ( dx * dx + dy * dy < r_squared ) ) {
        ufb_px( fb, col, i, j );
      }
    }
  }
}

/* Text-drawing methods. */
// Draw a single letter of monospace 6x8 font.
// TODO: Support different orientations at 90-degree increments.
void ufb_draw_letter( uFB* fb, uint16_t color,
                        int x, int y, uint32_t w0, uint32_t w1,
                        int scale, int orient ) {
  if ( scale <= 0 ) { return; }
  int line_h = 8 * scale;
  int cur_x = x;
  int cur_y = y;
  for ( int w_iter = 31; w_iter >= 0; --w_iter ) {
    if ( ( w0 & ( 1 << w_iter ) ) != 0 && cur_x > 0 && cur_y > 0 ) {
      ufb_fill_rect( fb, color, cur_x, cur_y, scale, scale );
    }
    if ( orient == UFB_ORIENT_V ) {
      cur_y += scale;
      if ( cur_y == y + line_h ) {
        cur_y = y;
        cur_x += scale;
      }
    }
    else if ( orient == UFB_ORIENT_H ) {
      cur_x -= scale;
      if ( cur_x == x - line_h ) {
        cur_x = x;
        cur_y += scale;
      }
    }
  }
  for ( int w_iter = 15; w_iter >= 0; --w_iter ) {
    if ( ( w1 & ( 1 << w_iter ) ) != 0 && cur_x > 0 && cur_y > 0 ) {
      ufb_fill_rect( fb, color, cur_x, cur_y, scale, scale );
    }
    if ( orient == UFB_ORIENT_V ) {
      cur_y += scale;
      if ( cur_y == y + line_h ) {
        cur_y = y;
        cur_x += scale;
      }
    }
    else if ( orient == UFB_ORIENT_H ) {
      cur_x -= scale;
      if ( cur_x == x - line_h ) {
        cur_x = x;
        cur_y += scale;
      }
    }
  }
}

// Draw a single character to the framebuffer.
void ufb_draw_char( uFB* fb, uint16_t color,
                    int x, int y, char c,
                    int scale, int orient ) {
  if ( x < 0 || x > ( fb->w + scale * 6 ) ) { return; }
  if ( y < 0 || y > ( fb->h + scale * 8 ) ) { return; }
  uint32_t w0 = 0x00000000;
  uint32_t w1 = 0x00000000;
  if ( c == 'A' ) {
    w0 = UFB_CH_A0;
    w1 = UFB_CH_A1B1 >> 16;
  }
  else if ( c == 'B' ) {
    w0 = UFB_CH_B0;
    w1 = UFB_CH_A1B1 & 0x0000FFFF;
  }
  else if ( c == 'C' ) {
    w0 = UFB_CH_C0;
    w1 = UFB_CH_C1D1 >> 16;
  }
  else if ( c == 'D' ) {
    w0 = UFB_CH_D0;
    w1 = UFB_CH_C1D1 & 0x0000FFFF;
  }
  else if ( c == 'E' ) {
    w0 = UFB_CH_E0;
    w1 = UFB_CH_E1F1 >> 16;
  }
  else if ( c == 'F' ) {
    w0 = UFB_CH_F0;
    w1 = UFB_CH_E1F1 & 0x0000FFFF;
  }
  else if ( c == 'G' ) {
    w0 = UFB_CH_G0;
    w1 = UFB_CH_G1H1 >> 16;
  }
  else if ( c == 'H' ) {
    w0 = UFB_CH_H0;
    w1 = UFB_CH_G1H1 & 0x0000FFFF;
  }
  else if ( c == 'I' ) {
    w0 = UFB_CH_I0;
    w1 = UFB_CH_I1J1 >> 16;
  }
  else if ( c == 'J' ) {
    w0 = UFB_CH_J0;
    w1 = UFB_CH_I1J1 & 0x0000FFFF;
  }
  else if ( c == 'K' ) {
    w0 = UFB_CH_K0;
    w1 = UFB_CH_K1L1 >> 16;
  }
  else if ( c == 'L' ) {
    w0 = UFB_CH_L0;
    w1 = UFB_CH_K1L1 & 0x0000FFFF;
  }
  else if ( c == 'M' ) {
    w0 = UFB_CH_M0;
    w1 = UFB_CH_M1N1 >> 16;
  }
  else if ( c == 'N' ) {
    w0 = UFB_CH_N0;
    w1 = UFB_CH_M1N1 & 0x0000FFFF;
  }
  else if ( c == 'O' ) {
    w0 = UFB_CH_O0;
    w1 = UFB_CH_O1P1 >> 16;
  }
  else if ( c == 'P' ) {
    w0 = UFB_CH_P0;
    w1 = UFB_CH_O1P1 & 0x0000FFFF;
  }
  else if ( c == 'Q' ) {
    w0 = UFB_CH_Q0;
    w1 = UFB_CH_Q1R1 >> 16;
  }
  else if ( c == 'R' ) {
    w0 = UFB_CH_R0;
    w1 = UFB_CH_Q1R1 & 0x0000FFFF;
  }
  else if ( c == 'S' ) {
    w0 = UFB_CH_S0;
    w1 = UFB_CH_S1T1 >> 16;
  }
  else if ( c == 'T' ) {
    w0 = UFB_CH_T0;
    w1 = UFB_CH_S1T1 & 0x0000FFFF;
  }
  else if ( c == 'U' ) {
    w0 = UFB_CH_U0;
    w1 = UFB_CH_U1V1 >> 16;
  }
  else if ( c == 'V' ) {
    w0 = UFB_CH_V0;
    w1 = UFB_CH_U1V1 & 0x0000FFFF;
  }
  else if ( c == 'W' ) {
    w0 = UFB_CH_W0;
    w1 = UFB_CH_W1X1 >> 16;
  }
  else if ( c == 'X' ) {
    w0 = UFB_CH_X0;
    w1 = UFB_CH_W1X1 & 0x0000FFFF;
  }
  else if ( c == 'Y' ) {
    w0 = UFB_CH_Y0;
    w1 = UFB_CH_Y1Z1 >> 16;
  }
  else if ( c == 'Z' ) {
    w0 = UFB_CH_Z0;
    w1 = UFB_CH_Y1Z1 & 0x0000FFFF;
  }
  else if ( c == 'a' ) {
    w0 = UFB_CH_a0;
    w1 = UFB_CH_a1b1 >> 16;
  }
  else if ( c == 'b' ) {
    w0 = UFB_CH_b0;
    w1 = UFB_CH_a1b1 & 0x0000FFFF;
  }
  else if ( c == 'c' ) {
    w0 = UFB_CH_c0;
    w1 = UFB_CH_c1d1 >> 16;
  }
  else if ( c == 'd' ) {
    w0 = UFB_CH_d0;
    w1 = UFB_CH_c1d1 & 0x0000FFFF;
  }
  else if ( c == 'e' ) {
    w0 = UFB_CH_e0;
    w1 = UFB_CH_e1f1 >> 16;
  }
  else if ( c == 'f' ) {
    w0 = UFB_CH_f0;
    w1 = UFB_CH_e1f1 & 0x0000FFFF;
  }
  else if ( c == 'g' ) {
    w0 = UFB_CH_g0;
    w1 = UFB_CH_g1h1 >> 16;
  }
  else if ( c == 'h' ) {
    w0 = UFB_CH_h0;
    w1 = UFB_CH_g1h1 & 0x0000FFFF;
  }
  else if ( c == 'i' ) {
    w0 = UFB_CH_i0;
    w1 = UFB_CH_i1j1 >> 16;
  }
  else if ( c == 'j' ) {
    w0 = UFB_CH_j0;
    w1 = UFB_CH_i1j1 & 0x0000FFFF;
  }
  else if ( c == 'k' ) {
    w0 = UFB_CH_k0;
    w1 = UFB_CH_k1l1 >> 16;
  }
  else if ( c == 'l' ) {
    w0 = UFB_CH_l0;
    w1 = UFB_CH_k1l1 & 0x0000FFFF;
  }
  else if ( c == 'm' ) {
    w0 = UFB_CH_m0;
    w1 = UFB_CH_m1n1 >> 16;
  }
  else if ( c == 'n' ) {
    w0 = UFB_CH_n0;
    w1 = UFB_CH_m1n1 & 0x0000FFFF;
  }
  else if ( c == 'o' ) {
    w0 = UFB_CH_o0;
    w1 = UFB_CH_o1p1 >> 16;
  }
  else if ( c == 'p' ) {
    w0 = UFB_CH_p0;
    w1 = UFB_CH_o1p1 & 0x0000FFFF;
  }
  else if ( c == 'q' ) {
    w0 = UFB_CH_q0;
    w1 = UFB_CH_q1r1 >> 16;
  }
  else if ( c == 'r' ) {
    w0 = UFB_CH_r0;
    w1 = UFB_CH_q1r1 & 0x0000FFFF;
  }
  else if ( c == 's' ) {
    w0 = UFB_CH_s0;
    w1 = UFB_CH_s1t1 >> 16;
  }
  else if ( c == 't' ) {
    w0 = UFB_CH_t0;
    w1 = UFB_CH_s1t1 & 0x0000FFFF;
  }
  else if ( c == 'u' ) {
    w0 = UFB_CH_u0;
    w1 = UFB_CH_u1v1 >> 16;
  }
  else if ( c == 'v' ) {
    w0 = UFB_CH_v0;
    w1 = UFB_CH_u1v1 & 0x0000FFFF;
  }
  else if ( c == 'w' ) {
    w0 = UFB_CH_w0;
    w1 = UFB_CH_w1x1 >> 16;
  }
  else if ( c == 'x' ) {
    w0 = UFB_CH_x0;
    w1 = UFB_CH_w1x1 & 0x0000FFFF;
  }
  else if ( c == 'y' ) {
    w0 = UFB_CH_y0;
    w1 = UFB_CH_y1z1 >> 16;
  }
  else if ( c == 'z' ) {
    w0 = UFB_CH_z0;
    w1 = UFB_CH_y1z1 & 0x0000FFFF;
  }
  else if ( c == '0' ) {
    w0 = UFB_CH_00;
    w1 = UFB_CH_0111 >> 16;
  }
  else if ( c == '1' ) {
    w0 = UFB_CH_10;
    w1 = UFB_CH_0111 & 0x0000FFFF;
  }
  else if ( c == '2' ) {
    w0 = UFB_CH_20;
    w1 = UFB_CH_2131 >> 16;
  }
  else if ( c == '3' ) {
    w0 = UFB_CH_30;
    w1 = UFB_CH_2131 & 0x0000FFFF;
  }
  else if ( c == '4' ) {
    w0 = UFB_CH_40;
    w1 = UFB_CH_4151 >> 16;
  }
  else if ( c == '5' ) {
    w0 = UFB_CH_50;
    w1 = UFB_CH_4151 & 0x0000FFFF;
  }
  else if ( c == '6' ) {
    w0 = UFB_CH_60;
    w1 = UFB_CH_6171 >> 16;
  }
  else if ( c == '7' ) {
    w0 = UFB_CH_70;
    w1 = UFB_CH_6171 & 0x0000FFFF;
  }
  else if ( c == '8' ) {
    w0 = UFB_CH_80;
    w1 = UFB_CH_8191 >> 16;
  }
  else if ( c == '9' ) {
    w0 = UFB_CH_90;
    w1 = UFB_CH_8191 & 0x0000FFFF;
  }
  else if ( c == ':' ) {
    w0 = UFB_CH_col0;
    w1 = UFB_CH_col1per1 >> 16;
  }
  else if ( c == '.' ) {
    w0 = UFB_CH_per0;
    w1 = UFB_CH_col1per1 & 0x0000FFFF;
  }
  else if ( c == '!' ) {
    w0 = UFB_CH_exc0;
    w1 = UFB_CH_exc1fws1 >> 16;
  }
  else if ( c == '/' ) {
    w0 = UFB_CH_fws0;
    w1 = UFB_CH_exc1fws1 & 0x0000FFFF;
  }
  else if ( c == '-' ) {
    w0 = UFB_CH_hyp0;
    w1 = UFB_CH_hyp1pls1 >> 16;
  }
  else if ( c == '+' ) {
    w0 = UFB_CH_pls0;
    w1 = UFB_CH_hyp1pls1 & 0x0000FFFF;
  }
  else if ( c == '<' ) {
    w0 = UFB_CH_lct0;
    w1 = UFB_CH_lct1rct1 >> 16;
  }
  else if ( c == '>' ) {
    w0 = UFB_CH_rct0;
    w1 = UFB_CH_lct1rct1 & 0x0000FFFF;
  }
  ufb_draw_letter( fb, color, x, y, w0, w1, scale, orient );
}

// Draw a <=32-bit integer to the display.
void ufb_draw_int( uFB* fb, uint16_t color, int x, int y, int ic, int scale, int orient ) {
  int magnitude = 1000000000;
  int cur_x = x;
  int cur_y = y;
  int step = 6 * scale;
  int first_found = 0;
  int proc_val = ic;
  if ( proc_val < 0 ) {
    proc_val = proc_val * -1;
    ufb_draw_char( fb, color, cur_x, cur_y, '-', scale, orient );
    if ( orient == UFB_ORIENT_V ) {
      cur_x += step;
    }
    else if ( orient == UFB_ORIENT_H ) {
      cur_y += step;
    }
  }
  for ( magnitude = 1000000000; magnitude > 0; magnitude = magnitude / 10 ) {
    int m_val = proc_val / magnitude;
    proc_val -= ( m_val * magnitude );
    if ( m_val > 0 || first_found || magnitude == 1 ) {
      first_found = 1;
    }
    char mc = ' ';
    if ( m_val == 0 ) {
      mc = '0';
    }
    else if ( m_val == 1 ) {
      mc = '1';
    }
    else if ( m_val == 2 ) {
      mc = '2';
    }
    else if ( m_val == 3 ) {
      mc = '3';
    }
    else if ( m_val == 4 ) {
      mc = '4';
    }
    else if ( m_val == 5 ) {
      mc = '5';
    }
    else if ( m_val == 6 ) {
      mc = '6';
    }
    else if ( m_val == 7 ) {
      mc = '7';
    }
    else if ( m_val == 8 ) {
      mc = '8';
    }
    else if ( m_val == 9 ) {
      mc = '9';
    }
    if ( first_found ) {
      ufb_draw_char( fb, color, cur_x, cur_y, mc, scale, orient );
      if ( orient == UFB_ORIENT_V ) {
        cur_x += step;
        if ( cur_x >= fb->w ) { return; }
      }
      else if ( orient == UFB_ORIENT_H ) {
        cur_y += step;
        if ( cur_y >= fb->h ) { return; }
      }
    }
  }
}

// Draw a null-terminated C-string.
void ufb_draw_str( uFB* fb, uint16_t color, int x, int y, const char* cstr, int scale, int orient ) {
  int i = 0;
  int offset = 0;
  int step = 6 * scale;
  while (cstr[i] != '\0') {
    if ( orient == UFB_ORIENT_V ) {
      ufb_draw_char( fb, color, x + offset, y, cstr[ i ], scale, orient );
    }
    else if ( orient == UFB_ORIENT_H ) {
      ufb_draw_char( fb, color, x, y + offset, cstr[ i ], scale, orient );
    }
    ++i;
    if ( orient == UFB_ORIENT_V || orient == UFB_ORIENT_H ) {
      offset += step;
    }
    else {
      offset -= step;
    }
  }
}

void ufb_draw_lines( uFB* fb, uint16_t color, int x, int y, const char* cstr, int scale, int orient ) {
  int i = 0;
  int char_offset = 0;
  int line_offset = 0;
  int char_step = 6 * scale;
  int line_step = 8 * scale;
  while ( cstr[ i ] != '\0' ) {
    if ( cstr[ i ] == '\n' ) {
      char_offset = 0;
      line_offset += line_step;
      ++i;
    }
    else {
      if ( orient == UFB_ORIENT_V ) {
        ufb_draw_char( fb, color, x + char_offset, y + line_offset, cstr[ i ], scale, orient );
      }
      else if ( orient == UFB_ORIENT_H ) {
        ufb_draw_char( fb, color, x + line_offset, y + char_offset, cstr[ i ], scale, orient );
      }
      ++i;
      char_offset += char_step;
    }
  }
}
