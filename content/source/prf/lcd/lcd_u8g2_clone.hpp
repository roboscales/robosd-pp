#include <stdint.h>
#ifndef lcd_u8g2_clone_hpp
#define lcd_u8g2_clone_hpp
namespace robo{
	namespace prf {
		namespace lcd {
			template<class D> class u8g2_clone_t{
			public:
				struct font_info_t{
					/* offset 0 */
					uint8_t glyph_cnt;
					uint8_t bbx_mode;
					uint8_t bits_per_0;
					uint8_t bits_per_1;
					
					/* offset 4 */
					uint8_t bits_per_char_width;
					uint8_t bits_per_char_height;		
					uint8_t bits_per_char_x;
					uint8_t bits_per_char_y;
					uint8_t bits_per_delta_x;
					
					/* offset 9 */
					int8_t max_char_width;
					int8_t max_char_height; /* overall height, NOT ascent. Instead ascent = max_char_height + y_offset */
					int8_t x_offset;
					int8_t y_offset;
					
					/* offset 13 */
					int8_t  ascent_A;
					int8_t  descent_g;	/* usually a negative value */
					int8_t  ascent_para;
					int8_t  descent_para;
						
					/* offset 17 */
					uint16_t start_pos_upper_A;
					uint16_t start_pos_lower_a; 
					
					/* offset 21 */
					uint16_t start_pos_unicode;
					
					const uint8_t * memo = nullptr;
					uint8_t utf8_state = 0;
					uint16_t encoding = 0;
					bool is_font_inverse_mode = false;
					
					static uint16_t get_word(const uint8_t *font, uint8_t offset){
						uint16_t pos;
						font += offset;
						pos = D::read( font );
						font++;
						pos <<= 8;
						pos += D::read( font);
						return pos;
					}

					void set_font( const uint8_t * _memo){
						memo = _memo;
			//			const void * content = D::skeep( memo, 23);
						
							/* offset 0 */
						glyph_cnt = D::get_byte(memo, 0);
						bbx_mode = D::get_byte(memo, 1);
						bits_per_0 = D::get_byte(memo, 2);
						bits_per_1 = D::get_byte(memo, 3);

						/* offset 4 */
						bits_per_char_width = D::get_byte(memo, 4);
						bits_per_char_height = D::get_byte(memo, 5);
						bits_per_char_x = D::get_byte(memo, 6);
						bits_per_char_y = D::get_byte(memo, 7);
						bits_per_delta_x = D::get_byte(memo, 8);

						/* offset 9 */
						max_char_width = D::get_byte(memo, 9);
						max_char_height = D::get_byte(memo, 10);
						x_offset = D::get_byte(memo, 11);
						y_offset = D::get_byte(memo, 12);

						/* offset 13 */
						ascent_A = D::get_byte(memo, 13);
						descent_g = D::get_byte(memo, 14);
						ascent_para = D::get_byte(memo, 15);
						descent_para = D::get_byte(memo, 16);

						/* offset 17 */
						start_pos_upper_A = get_word(memo, 17);
						start_pos_lower_a = get_word(memo, 19); 

						/* offset 21 */
						start_pos_unicode = get_word(memo, 21); 
					}

					virtual uint16_t get_next_char(uint8_t b) = 0;
					
					const uint8_t *  get_glyph_data(uint16_t _e) {
						const uint8_t * font  = memo + 23;
						if ( _e <= 255UL ) {
							if ( _e >= 'a' ) {
								font += start_pos_lower_a;
							} else if ( _e >= 'A' ) {
								font += start_pos_upper_A;
							}
							
							for(;;)	{
								if ( D::read( font + 1 ) == 0 )
									break;
								if (  D::read( font ) == _e )	{
									return font+2;	/* skip _e and glyph size */
								}
								font +=  D::read( font + 1 );
							}
						}	else {
							uint16_t e;
							const uint8_t *unicode_lookup_table;

							font += start_pos_unicode;
							unicode_lookup_table = font; 
					
							/* issue 596: search for the glyph start in the unicode lookup table */
							do
							{
								font += get_word(unicode_lookup_table, 0);
								e = get_word(unicode_lookup_table, 2);
								unicode_lookup_table+=4;
							} while( e < _e );
						
					
							for(;;)	{
								e = D::read( font );
								e <<= 8;
								e |= D::read( font + 1 );
								if ( e == 0 )
									break;
					
								if ( e == _e )
								{
									return font+3;	/* skip _e and glyph size */
								}
								font += D::read( font + 2 );
							}  
						}
					
						return nullptr;
					}
					
					
					uint16_t offset_x = 0;
					uint16_t offset_y = 0;
					typename D::scale_t scale = 1;
					uint16_t draw(uint16_t x, uint16_t y, typename D::scale_t _scale, const char *s)	{
						scale = _scale;
						uint16_t e;
						uint16_t delta, sum;
						sum = 0;

						utf8_state = 0;
						
						offset_x = x;
						offset_y = y;

						target_x = 0;
						target_y = 0;

						for(;;)
						{
							e = get_next_char((uint8_t)*s);
							if ( e == 0x0ffff )
								break;
							s++;
							if ( e != 0x0fffe )
							{
								//const uint8_t * glif = get_glyph_data( e );
								delta = draw_char( e );
								target_x+=delta;
								target_y = 0;

								sum += delta;  
							}
						}
						return sum;
					}
					

					uint16_t target_x;
					uint16_t target_y;
					
					
					struct painter_s	{
						const uint8_t *decode_ptr;			/* pointer to the compressed data */
						
						
						int8_t local_x;						/* local coordinates, (0,0) is upper left */
						int8_t local_y;
						int8_t glyph_width;	
						int8_t glyph_height;

						uint8_t decode_bit_pos;			/* bitpos inside a byte of the compressed data */
						uint8_t is_transparent;
						font_info_t & font_info;
					#ifdef U8G2_WITH_FONT_ROTATION  
						uint8_t dir;				/* direction */
					#endif
						uint8_t get_unsigned_bits( uint8_t _cnt ) 	{
							uint8_t val;
							uint8_t bit_pos = decode_bit_pos;
							uint8_t bit_pos_plus_cnt;
							
							//val = *(f->decode_ptr);
							val = D::read( decode_ptr );  
							
							val >>= bit_pos;
							bit_pos_plus_cnt = bit_pos;
							bit_pos_plus_cnt += _cnt;
							if ( bit_pos_plus_cnt >= 8 )
							{
								uint8_t s = 8;
								s -= bit_pos;
								decode_ptr++;
								//val |= *(f->decode_ptr) << (8-bit_pos);
								val |= D::read( decode_ptr ) << (s);
								//bit_pos -= 8;
								bit_pos_plus_cnt -= 8;
							}
							val &= (1U<<_cnt)-1;
							
							decode_bit_pos = bit_pos_plus_cnt;
							return val;
						}
						
						void setup(const uint8_t *glyph_data){
							decode_ptr = glyph_data;
							decode_bit_pos = 0;
				
							glyph_width = get_unsigned_bits(font_info.bits_per_char_width);
							glyph_height = get_unsigned_bits(font_info.bits_per_char_height);
						};
					
						int8_t get_signed_bits(uint8_t _cnt) {
							int8_t v, d;
							v = (int8_t)get_unsigned_bits(_cnt);
							d = 1;
							_cnt--;
							d <<= _cnt;
							v -= d;
							return v;
						}
						
						void draw_line(uint8_t len, uint8_t is_foreground)	{
							uint8_t cnt;	/* total number of remaining pixels, which have to be drawn */
							uint8_t rem; 	/* remaining pixel to the right edge of the glyph */
							uint8_t current;	/* number of pixels, which need to be drawn for the draw procedure */
							/* current is either equal to cnt or equal to rem */
							
							/* local coordinates of the glyph */
							uint8_t lx,ly;
							
								/* target position on the screen */
							uint16_t x, y;
							
							cnt = len;
							
							/* get the local position */
							lx = local_x;
							ly = local_y;
							
							for(;;)
							{
								/* calculate the number of pixel to the right edge of the glyph */
								rem = glyph_width;
								rem -= lx;
								
								/* calculate how many pixel to draw. This is either to the right edge */
								/* or lesser, if not enough pixel are left */
								current = rem;
								if ( cnt < rem )
									current = cnt;
								
								
								/* now draw the line, but apply the rotation around the glyph target position */
								//u8g2_font_decode_draw_pixel(u8g2, lx,ly,current, is_foreground);
								/* get target position */
								x = font_info.target_x;
								y = font_info.target_y;

								/* apply rotation */
						#ifdef U8G2_WITH_FONT_ROTATION
								
								x = u8g2_add_vector_x(x, lx, ly, decode->dir);
								y = u8g2_add_vector_y(y, lx, ly, decode->dir);
								
								//u8g2_add_vector(&x, &y, lx, ly, decode->dir);
								
						#else
								x += lx;
								y += ly;
						#endif
								
								/* draw foreground and background (if required) */
								if ( is_foreground )
								{
									D::draw_line(
										font_info.offset_x,
										font_info.offset_y,
										x, 
										y, 
										current ,
										font_info.scale
									);
								}
								else if ( is_transparent == 0 )    
								{
									D::draw_bk_line(
										font_info.offset_x,
										font_info.offset_y,
										x, 
										y, 
										current ,
										font_info.scale
									);
								}
								
								/* check, whether the end of the run length code has been reached */
								if ( cnt < rem )
									break;
								cnt -= rem;
								lx = 0;
								ly++;
							}
							lx += cnt;
							
							local_x = lx;
							local_y = ly;  
						}

						painter_s( font_info_t & _font_info ) : font_info(_font_info) {}
							
						int8_t draw(const uint8_t *glyph_data)	{
							uint8_t a, b;
							int8_t x, y;
							int8_t d;
							int8_t h;
								
							setup(glyph_data);     /* set values in u8g2->font_decode data structure */
							h = glyph_height;
							
							x = get_signed_bits( font_info.bits_per_char_x );
							y = get_signed_bits( font_info.bits_per_char_y );
							d = get_signed_bits( font_info.bits_per_delta_x );
							
							if ( glyph_width > 0 ) {
							font_info.target_x += x;
							font_info.target_y -= h+y;

						
							 
							/* reset local x/y position */
							local_x = 0;
							local_y = 0;
							
							/* decode glyph */
							for(;;)	{
								a = get_unsigned_bits(font_info.bits_per_0);
								b = get_unsigned_bits(font_info.bits_per_1);
								do
								{
									draw_line(a, 0);
									draw_line(b, 1);
								} while( get_unsigned_bits( 1) != 0 );

								if ( local_y >= h )
									break;
								}
							
							}
							return d;
						}
					};
					
					
					
					uint16_t draw_char(uint16_t _e){
						uint16_t dx = 0;
						const uint8_t *glyph_data = get_glyph_data(_e);
						if ( glyph_data != nullptr ){
							painter_s painter(*this);
							dx = painter.draw( glyph_data);
						}
						//target_x += dx;
						return dx;
					}
					
				};
				
				struct utf8_t : public font_info_t{
					virtual uint16_t get_next_char(uint8_t b){
						if ( b == 0 || b == '\n' )	/* '\n' terminates the string to support the string list procedures */
								return 0x0ffff;	/* end of string detected, pending UTF8 is discarded */
						if ( font_info_t::utf8_state == 0 ) {
							if ( b >= 0xfc )	/* 6 byte sequence */
							{
								font_info_t::utf8_state = 5;
								b &= 1;
							}  else if ( b >= 0xf8 )  {
									font_info_t::utf8_state = 4;
								b &= 3;
							} else if ( b >= 0xf0 ) {
								font_info_t::utf8_state = 3;
								b &= 7;      
							}  else if ( b >= 0xe0 )  {
									font_info_t::utf8_state = 2;
									b &= 15;
							}   else if ( b >= 0xc0 )  {
								font_info_t::utf8_state = 1;
								b &= 0x01f;
							}   else   {
								/* do nothing, just use the value as encoding */
								return b;
							}
							font_info_t::encoding = b;
							return 0x0fffe;
						} else {
							font_info_t::utf8_state--;
							/* The case b < 0x080 (an illegal UTF8 encoding) is not checked here. */
							font_info_t::encoding<<=6;
							b &= 0x03f;
							font_info_t::encoding |= b;
							if ( font_info_t::utf8_state != 0 )
								return 0x0fffe;	/* nothing to do yet */
						}
						return font_info_t::encoding;
					}
				};

			};
		}
	}
}
#endif