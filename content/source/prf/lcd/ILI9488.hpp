#ifndef ILI9488_hpp
#include "ILI9488_regs.h"
#include <stdint.h>
#include <algorithm>
namespace robo{
	namespace prf {
		namespace lcd {
			template<class D> class ILI9488_driver_t: public D {
				public:
					static void write_cmd(uint8_t cmd){
						D::cd::off();
						D::transfer(cmd);
					}
					static void write_data(uint8_t cmd){
						D::cd::on();
						D::transfer(cmd);
					}
					static void write_data(const uint8_t* pDat, uint16_t count){
						D::cd::on();
						D::transfer(pDat,count);
					}
			};


			template<class D> class ILI9488_t: public D {
					public:
					struct command{
						enum {
							 NOP = ILI9488_NOP //0x00
							, SWRESET = ILI9488_SWRESET //0x01
							, RDDID = ILI9488_RDDID   //0x04
							, RDDST = ILI9488_RDDST   //0x09
							, SLPIN = ILI9488_SLPIN   //0x10
							, SLPOUT = ILI9488_SLPOUT  //0x11
							, PTLON = ILI9488_PTLON   //0x12
							, NORON = ILI9488_NORON   //0x13

							, RDMODE = ILI9488_RDMODE  //0x0A
							, RDMADCTL = ILI9488_RDMADCTL  //0x0B
							, RDPIXFMT = ILI9488_RDPIXFMT  //0x0C
							, RDIMGFMT = ILI9488_RDIMGFMT  //0x0D
							, RDSELFDIAG = ILI9488_RDSELFDIAG  //0x0F

							, INVOFF = ILI9488_INVOFF  //0x20
							, INVON = ILI9488_INVON   //0x21
							, GAMMASET = ILI9488_GAMMASET //0x26
							, DISPOFF = ILI9488_DISPOFF //0x28
							, DISPON = ILI9488_DISPON  //0x29

							, CASET = ILI9488_CASET   //0x2A
							, PASET = ILI9488_PASET   //0x2B
							, RAMWR = ILI9488_RAMWR   //0x2C
							, RAMRD = ILI9488_RAMRD   //0x2E

							, PTLAR = ILI9488_PTLAR   //0x30
							, MADCTL = ILI9488_MADCTL  //0x36
							, PIXFMT = ILI9488_PIXFMT  //0x3A

							, FRMCTR1 = ILI9488_FRMCTR1 //0xB1
							, FRMCTR2 = ILI9488_FRMCTR2 //0xB2
							, FRMCTR3 = ILI9488_FRMCTR3 //0xB3
							, INVCTR = ILI9488_INVCTR  //0xB4
							, DFUNCTR = ILI9488_DFUNCTR //0xB6

							, PWCTR1 = ILI9488_PWCTR1  //0xC0
							, PWCTR2 = ILI9488_PWCTR2  //0xC1
							, PWCTR3 = ILI9488_PWCTR3  //0xC2
							, PWCTR4 = ILI9488_PWCTR4  //0xC3
							, PWCTR5 = ILI9488_PWCTR5  //0xC4
							, VMCTR1 = ILI9488_VMCTR1  //0xC5
							, VMCTR2 = ILI9488_VMCTR2  //0xC7

							, RDID1 = ILI9488_RDID1   //0xDA
							, RDID2 = ILI9488_RDID2   //0xDB
							, RDID3 = ILI9488_RDID3   //0xDC
							, RDID4 = ILI9488_RDID4   //0xDD

							, GMCTRP1 = ILI9488_GMCTRP1 //0xE0
							, GMCTRN1 = ILI9488_GMCTRN1 //0xE1
						};
					};
					enum class rotations{
						dg0
						, dg90
						, dg180
						, dg270
					};
					struct results{
						enum class begin{ success, fault , no_feedback_test };
						enum class command{ 
							success = 0
							, mem_overflow = -4 
							, not_support = -3
							, param = -2
							, error = -1
							, out_range = 1
						};
					};
				public:
					ILI9488_t(){
					}
					

					
				public:
					uint16_t width = ILI9488_TFTWIDTH;
					uint16_t height = ILI9488_TFTHEIGHT;
					struct rect_s{
						uint16_t x0 = 0;
						uint16_t x1 = 0;
						uint16_t y0 = 0;
						uint16_t y1 = 0;
						rect_s(){};
						rect_s( const rect_s & _s){ 
							x0 = _s.x0;
							x1 = _s.x1;
							y0 = _s.y0;
							y1 = _s.y1;
						};
						rect_s( const uint16_t ( & _rect) [4]){
							x0 = _rect[0];
							x1 = _rect[1];
							y0 = _rect[2];
							y1 = _rect[3];
						};
					};

					union color_s{
						uint16_t value;
						struct{
							uint16_t r:5;
							uint16_t g:6;
							uint16_t b:5;
						};
					};
							
					struct color_t{
						union{
							struct{
								uint8_t r;
								uint8_t g;
								uint8_t b;
								uint8_t a;
							};
							uint32_t value;
							uint8_t rgba[4];
						} ;
						color_t(){value=0;}
						color_t(uint32_t _value){ value=_value; }
						
						color_t(uint16_t _value){
							color_s s;
							s.value = _value;
							r = 255L * s.r / 31L;
							g = 255L * s.g / 63L;
							b = 255L * s.b / 31L;
						}
						
						color_t(const color_s & _color){
							r = 255L * _color.r / 31;
							g = 255L * _color.g / 63;
							b = 255L * _color.b / 31;
						}
						
						color_t(const std::initializer_list<uint8_t> & _rgb){
							int ix = 0;
							for (const auto x : _rgb) {
								switch (ix){
									case 0: r = x; ix++; break; 
									case 1: g = x; ix++; break; 
									case 2: b = x; break;
								}
							}
						}
						operator  color_s  ()  {
							color_s s;
							s.r =  31L * r / 255;
							s.g =  63L * g / 255;
							s.b =  31L * b / 255;
							return s;
						}
					};

					#if 0
					struct zscreen_s{
						enum{ zbufsize = bufsize };
						uint8_t pixels[zbufsize] = {};
						rect_s rect = {};
						color_s background =  color_t{0xF0,0xF0,0x00};
						color_s color = color_t{0xFF,0xFF,0xFF};
						void reset(){
							rect.x0 = width-1;
							rect.x1 = 0;
							rect.y0 = height-1;
							rect.y1 = 0;
							std::fill_n(pixels,zbufsize,0);
						}
						
						void update( const rect_s & _s){
							if( rect.x0 > _s.x0) rect.x0 = _s.x0;
							if( rect.x1 < _s.x1) rect.x1 = _s.x1;
							if( rect.y0 > _s.y0) rect.y0 = _s.y0;
							if( rect.y1 < _s.y1) rect.y1 = _s.y1;
						}
						enum class light { o =1, off =0};
						void pixel( uint16_t _x, uint16_t _y, light _l){
							update({_x, _y, _x, _y});
							pixels[_y*width + _x] = (uint8_t)_l;
							/*uint16_t tmp = (_y*width + _x);
							uint16_t bn = tmp >> 3;
							uint16_t pn = tmp - bn * 8;
							uint8_t b = pixels[bn];
							if(_l){
								pixels[ bn ] = (b | (1<<pn));
							} else {
								pixels[ bn ] = (b | (1<<pn));
							}*/
							
						}
						
						void h_line( uint16_t _x, uint16_t _y, uint16_t _l, light _lt){
							uint16_t x2 =_x+ _l;
							uint16_t r [4] = {_x, _y, x2, _y};
							update(r);
							int offset1 = _y*width+_x;
							uint8_t * px = pixels+offset1;
							uint8_t * pxe = px+_l;
							for( ; px!=pxe; ++px) *px =  (uint8_t)_lt;
							
							/*int offset1 = _y*width+_x;
							int offset2 = offset1 + _l;
							int begin_b = offset1>>3;
							int begin_px = offset1 - begin_b * 8;
							
							int end_b = offset2>>3;
							int end_px = offset2 - end_b * 8;
							
							//uint8_t begin_mask = 1<<(8-begin_px)
							uint8_t * px = pixels+begin_b+1;
							if(_lt){
								for( int i=begin_b+1;i<end_b;++i,++px) {
									*px = 0xff;
								}
							}else{
								for( int i=begin_b+1;i<end_b;++i,++px) {
									*px = 0x00;
								}
							}*/
							
						}
						
					};
					
					zscreen_s screens[2];
					
					zscreen_s * sending = screens;
					zscreen_s * zet = screens + 1;
					uint8_t pack_zet [bufsize*3];
					int odd = 0;
					
					
					
				public:

					
				void z_apply(void){
					odd = 1 - odd;
					sending = screens + odd;
					zet = screens + (1-odd);
					zet->reset();
					color_t  *  pz;
					uint8_t  * pzz = pack_zet;
					D::cs::off();
					D::write_cmd(0x02c); //write_memory_start
				
					for( int y = sending->rect.y0; y < sending->rect.y1; ++y ){
						address_set(sending->rect.x0,y,sending->rect.x1,y);
						uint8_t * src = sending->pixel + y*width + sending->rect.x0;
						
						for( int i = 0;i< sending->rect.y1-sending->rect.y0; ++i, ++src){
							D::write_data(rgb_till,3*til);
						}
					}
					D::cs::on();	
				}
				void z_pixel( uint16_t _x, uint16_t _y, color_t _color){
					zet->pixel(_x,_y,_color);
				}
				
				void z_h_line( uint16_t _x, uint16_t _y, uint16_t _l, color_t _color){
					zet->h_line(_x,_y,_l,_color);
				}
				#endif
				public:
				void address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2)
				{
					D::write_cmd(0x2a);
					D::write_data(x1>>8);
					D::write_data(x1);
					D::write_data(x2>>8);
					D::write_data(x2);
					D::write_cmd(0x2b);
					D::write_data(y1>>8);
					D::write_data(y1);
					D::write_data(y2>>8);
					D::write_data(y2);
					D::write_cmd(0x2c); 							 
				}
				enum {til = 16};
				uint8_t rgb_till[3*til];
				void rgb_set(color_t _color){
					uint8_t * p = rgb_till;
					uint8_t r = _color.r;
					uint8_t g = _color.g;
					uint8_t b = _color.b;
					for ( int i=0; i< til; ++i){
						*p++ = r;
						*p++ = g;
						*p++ = b;
					}
				}
				void clear(const color_t & _color)                   
				{	
					unsigned int i,m;
					rgb_set( _color );
					D::cs::off();
					D::write_cmd(0x02c); //write_memory_start
					address_set(0,0,320,480);
					for(m=0;m<height;m++){					
						//uint8_t rgb[3] = { (uint8_t)((j>>8)&0xF8), (uint8_t)((j>>3)&0xFC), (uint8_t)(j<<3) };
						for(i=0;i<=width/til;i++)
						{
							D::write_data(rgb_till,3*til);
						}
					}

					D::cs::on();   
				}
				
				void h_line(unsigned int x, unsigned int y, unsigned int l, const color_t & _color)                   
				{	
					D::cs::off();
					D::write_cmd(0x02c); //write_memory_start
					address_set(x,y,x+l,y);
					for(int i=0;i<l;++i)
					{
						D::write_data(_color.rgba,3);
					}
					D::cs::on();	
				}
				
				void pixel(unsigned int x, unsigned int y, const color_t & _color)                   
				{	
					D::cs::off();
					D::write_cmd(0x02c); //write_memory_start
					address_set(x,y,x,y);
					D::write_data(_color.rgba,3);
					D::cs::on();	
				}
				void v_line(unsigned int x, unsigned int y, unsigned int l, const color_t & _color)                   
				{	
					D::cs::off();
					D::write_cmd(0x02c); //write_memory_start
					//digitalWrite(RS,HIGH);
					address_set(x,y,x,y+l);
					for(int i=0;i<l;++i)
					{ 
						D::write_data(_color.rgba,3);
					}
					D::cs::on();   
				}
				
				void rect(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const color_t & _color)
				{
					h_line(x  , y  , w, _color);
					h_line(x  , y+h, w, _color);
					v_line(x  , y  , h, _color);
					v_line(x+w, y  , h, _color);
				}

				void rectf(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const color_t & _color)
				{
					unsigned int i;
					for(i=0;i<h;i++)
					{
						h_line(x  , y+i  , w, _color);
					}
				}
				static inline uint8_t z_buffer[3*4096] ;
				void glif(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const color_s * _glif ){
						int sz = w*h;
						const color_s * sb = _glif;
						const color_s * se = _glif+sz;
						uint8_t * db = z_buffer;				
						for(; sb!=se; ++sb ){
							color_t c = *sb;
							*db++ = c.r;
							*db++ = c.g;
							*db++ = c.b;
						}
						D::cs::off();
						D::write_cmd(0x02c); //write_memory_start
						address_set(x,y,x+w,y+h);
						D::write_data(z_buffer,3*sz);
						D::cs::on();
				}				
				
				void glif(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const uint16_t * _glif ){
						int sz = w*h;
						const uint16_t * sb = _glif;
						const uint16_t * se = _glif+sz;
						uint8_t * db = z_buffer;				
						for(; sb!=se; ++sb ){
							color_t c = *sb;
							*db++ = c.r;
							*db++ = c.g;
							*db++ = c.b;
						}
						D::cs::off();
						D::write_cmd(0x02c); //write_memory_start
						address_set(x,y,x+w,y+h);
						D::write_data(z_buffer,3*sz);
						D::cs::on();
				}			
				
							
				void glif(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const uint8_t * _glif, const color_t & _color, const color_t & _bk){
						int sz = w*h;
						const uint8_t * sb = _glif;
						const uint8_t * se = _glif+(sz>>3);
						uint8_t * db = z_buffer;				
						for(; sb!=se; ++sb ){
							uint8_t b = *sb;
							for(int n = 7;n>=0;n--){
								static uint8_t masks[8] = { 1, 1<<1, 1<< 2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7};
								uint8_t m = masks[n];
								if( (b & m) == m ) {
									*db++ = _color.r;
									*db++ = _color.g;
									*db++ = _color.b;
								} else {
									*db++ = _bk.r;
									*db++ = _bk.g;
									*db++ = _bk.b;
								}
							}
						}
						D::cs::off();
						D::write_cmd(0x02c); //write_memory_start
						address_set(x,y,x+w-1,y+h-1);
						D::write_data(z_buffer,3*sz);
						D::cs::on();
				}	
				
					void glif(unsigned int x,unsigned int y,unsigned int w,unsigned int h, const uint8_t * _rgb ){
						int sz = w*h*3;
						D::cs::off();
						D::write_cmd(0x02c); //write_memory_start
						address_set(x,y,x+w,y);
						D::write_data(_rgb,sz);
						D::cs::on();
				}	
				
				//bool limitPixel(int16_t x, int16_t y){
					//return x>=0 && x<width && y>=0 && x<height;					
				//}
				rotations rotation = rotations::dg0;
				void     set_rotation(rotations _rotation){
					D::cs::off();
					D::write_cmd(command::MADCTL);
					switch (_rotation) {
						case rotations::dg0 : {
							D::write_data(0);
							width  = ILI9488_TFTWIDTH;
							height = ILI9488_TFTHEIGHT;
						} break;
						case rotations::dg90: {
							D::write_data(MADCTL_MV|MADCTL_MY);
							width  = ILI9488_TFTHEIGHT;
							height = ILI9488_TFTWIDTH;
						} break;
						case rotations::dg180: {
							D::write_data(MADCTL_MY | MADCTL_MX);
							width  = ILI9488_TFTWIDTH;
							height = ILI9488_TFTHEIGHT;
						} break;
						case rotations::dg270: {
							D::write_data(MADCTL_MX |  MADCTL_MV );
							width  = ILI9488_TFTHEIGHT;
							height = ILI9488_TFTWIDTH;
						} break;
					}
					rotation = _rotation;
			}

				/*
					bool limitPixel(int16_t x, int16_t y){
						return x>=0 && x<width_ && y>=0 && x<height_;					
					}
					void drawPixel(int16_t x, int16_t y, uint16_t color)	{
						uint8_t       colorBuf[3] = {(uint8_t)(((color & 0xf800) >> 11) * 255 / 31), (uint8_t)(((color & 0x07e0) >> 5) * 255 / 63), (uint8_t)((color & 0x001f) * 255 / 31)};
						if( !limitPixel(x, y)) return;
						set_cursor_addr_(x, y, x, y);
						write_to_ram_();
						D::write_data(colorBuf, 3);
					}
					
					void writeRepeatPixel(uint16_t color, uint16_t count, uint16_t repeatCount){
						uint8_t       colorBuf[3] = {(uint8_t)(((color & 0xf800) >> 11) * 255 / 31), (uint8_t)(((color & 0x07e0) >> 5) * 255 / 63), (uint8_t)((color & 0x001f) * 255 / 31)};
						uint16_t      i = 0, j = 0;
						for(i = 0; i < repeatCount; i ++) {
							for(j = 0; j < count; j ++) {
								D::write_data(colorBuf, 3);
							}
						}
					}
								
					
					
					void        fillScreen(uint16_t color){
						set_cursor_addr_(0, 0, 319, 479);
						write_to_ram_();
						writeRepeatPixel(color, 320, 480);
					}
					void        drawVLine(int16_t x, int16_t y, int16_t height, uint16_t color){
						if( !limitPixel(x, y)) return;
						set_cursor_addr_(x, y, x, y + height - 1);
						write_to_ram_();
						writeRepeatPixel(color, height, 1);				
					}
					
					void        drawHLine(int16_t x, int16_t y, int16_t width, uint16_t color){
						if( !limitPixel(x, y)) return;
						set_cursor_addr_(x, y, x + width - 1, y);
						write_to_ram_();
						writeRepeatPixel(color, width, 1);
					}
					
					void        on(void){
						D::cs::off();
						D::write_cmd(command::DISPON);
						D::cs::on();
					}
					void        off(void){
						D::cs::off();
						D::write_cmd(command::DISPOFF);
						D::cs::on();
					}
					void        sleep(void){
						D::cs::off();
						D::write_cmd(command::SLPIN);
						D::cs::on();
					}
					void        wakeup(void){
						D::cs::off();
						D::write_cmd(command::SLPOUT);
						D::cs::on();
					}
					uint32_t id = 0;
					uint16_t error = 0;
					
					*/
					void begin(void){
						D::reset::on();
						D::delay_ms(5); 
						D::reset::off();
						D::delay_ms(15); 
						D::reset::on();
						D::delay_ms(15); 

						D::cs::off();

						D::write_cmd(0xF7);  
						D::write_data(0xA9); 
						D::write_data(0x51); 
						D::write_data(0x2C); 
						D::write_data(0x82);  

						D::write_cmd(0xC0);  
						D::write_data(0x11); 
						D::write_data(0x09); 

						D::write_cmd(0xC1);  
						D::write_data(0x41); 

						D::write_cmd(0xC5);  
						D::write_data(0x00); 
						D::write_data(0x0A); 
						D::write_data(0x80);

						D::write_cmd(0xB1);  
						D::write_data(0xB0); 
						D::write_data(0x11); 

						D::write_cmd(0xB4);  
						D::write_data(0x02); 

						D::write_cmd(0xB6);    
						D::write_data(0x02);
						D::write_data(0x22);  

						D::write_cmd(0xB7);    
						D::write_data(0xC6);  

						D::write_cmd(0xBE);    
						D::write_data(0x00);   
						D::write_data(0x04); 

						D::write_cmd(0xE9);    
						D::write_data(0x00);   

						D::write_cmd(0x36);  
						D::write_data(0x08);   

						D::write_cmd(0x3A);    
						D::write_data(0x66); 

						D::write_cmd(0xE0);    
						D::write_data(0x00);  
						D::write_data(0x07); 
						D::write_data(0x10); 
						D::write_data(0x09); 
						D::write_data(0x17); 
						D::write_data(0x0B); 
						D::write_data(0x41); 
						D::write_data(0x89); 
						D::write_data(0x4B); 
						D::write_data(0x0A); 
						D::write_data(0x0C); 
						D::write_data(0x0E); 
						D::write_data(0x18); 
						D::write_data(0x1B); 
						D::write_data(0x0F); 

						D::write_cmd(0xE1);    
						D::write_data(0x00);  
						D::write_data(0x17); 
						D::write_data(0x1A); 
						D::write_data(0x04); 
						D::write_data(0x0E); 
						D::write_data(0x06); 
						D::write_data(0x2F); 
						D::write_data(0x45); 
						D::write_data(0x43); 
						D::write_data(0x02); 
						D::write_data(0x0A); 
						D::write_data(0x09); 
						D::write_data(0x32); 
						D::write_data(0x36); 
						D::write_data(0x0F); 

						D::write_cmd(0x11);    //Exit Sleep 
						D::delay_ms(120); 
						D::write_cmd(0x29);    //Display on 

						D::cs::on();
						
					}
					struct font_s{
						uint8_t width;
						uint8_t height;
						uint8_t size;
						const uint8_t * memo;					
					}; 
					
					void text(uint16_t x, uint16_t y, uint8_t _c, const color_t & _color , const color_t & _bk, bool mode, const font_s & _font)
					{
						if((x >= width) || (y >= height ) || ((x + _font.width* _font.size - 1) < 0) || ((y + _font.height* _font.size - 1) < 0)){
								return;
						}		
						//?
						if(_c >= 176){
							_c++; 
						}
						
						for (int8_t i=0; i<_font.width; i++){
								uint8_t line;
								if (i == _font.width-1)
								{
										line = 0x0;
								}
								else
								{
										line = _font.memo [ ((int)_c*(_font.width-1)) +i ];
								}
								for (int8_t j = 0; j<8; j++){
										if (line & 0x1)	{
											if (_font.size == 1) {
												pixel(x+i, y+j, _color);
											}	else {  
													rectf(x+(i*_font.size), y+(j*_font.size), _font.size, _font.size, _color);
											}
										} else if ( _bk.value != _color.value) {
											if(!mode) {
												if (_font.size == 1) {
													pixel(x+i, y+j, _bk);
												}	else 	{  
													rectf(x+i*_font.size, y+j*_font.size, _font.size, _font.size, _bk);
												}
											}
										}
										line >>= 1;
								}
							}
						}
				};
		}
	}
}
#endif