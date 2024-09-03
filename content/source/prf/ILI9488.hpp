#ifndef ILI9488_hpp
#include "ILI9488_regs.h"
#include <stdint.h>
namespace robo{
	namespace prf {
		template<class D> class ILI9488_driver_t: public D {
			public:
				static void write_cmd(uint8_t cmd){
					D::cd::off();
					D::cs::off();
					D::transfer(cmd);
					D::cs::on();
				}
				static void write_data(uint8_t cmd){
					D::cd::on();
					D::cs::off();
					D::transfer(cmd);
					D::cs::on();
				}
				static void write_data(uint8_t* pDat, uint16_t count){
					D::cd::on();
					D::cs::off();
					D::transfer(pDat,count);
					D::cs::on();
				}
				static void read_data(uint8_t* pDat, uint16_t count){
					D::cd::on();
					D::cs::off();
					D::read(pDat,count);
					D::cs::on();
				}
		};
		template<class D> class ILI9488_driver_wcs_t: public D {
			public:
				static void write_cmd(uint8_t cmd){
					D::cd::off();
					D::transfer(cmd);
				}
				static void write_data(uint8_t cmd){
					D::cd::on();
					D::transfer(cmd);
				}
				static void write_data(uint8_t* pDat, uint16_t count){
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
				//virtual int16_t     begin() = 0;
				//virtual void        D::write_cmd(uint8_t cmd) = 0;
				//virtual void        D::write_data(uint8_t dat) = 0;
				//virtual void        D::write_dataBytes(uint8_t* pDat, uint16_t count) = 0;
				//virtual void        writeRepeatPixel(uint16_t color, uint16_t count, uint16_t repeatCount) = 0;
				
			private:

				
				uint16_t width_= ILI9488_TFTWIDTH;
				uint16_t height_= ILI9488_TFTHEIGHT;
				rotations rotation_ =  rotations::dg0;
				void        write_to_ram_(void){
					D::write_cmd(command::RAMWR);
				}
				void set_cursor_addr_(int16_t x0, int16_t y0, int16_t x1, int16_t y1){
					uint8_t addrBuf[4] = {(uint8_t)((uint16_t)x0 >> 8), (uint8_t)x0, (uint8_t)((uint16_t)x1 >> 8), (uint8_t)x1};
					D::write_cmd(command::CASET);
					D::write_data(addrBuf, 4);
					addrBuf[0] = (uint16_t)y0 >> 8; addrBuf[1] = (uint16_t)y0; addrBuf[2] = (uint16_t)y1 >> 8; addrBuf[3] = (uint16_t)y1;
					D::write_cmd(command::PASET);
					D::write_data(addrBuf, 4);
				}
				

			public:
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
							
				void     setRotation(rotations _rotation){
					 D::write_cmd(command::MADCTL);
						switch (_rotation) {
							case rotations::dg0 : {
								D::write_data(MADCTL_MX | MADCTL_BGR);
								width_  = ILI9488_TFTWIDTH;
								height_ = ILI9488_TFTHEIGHT;
							} break;
							case rotations::dg90: {
								D::write_data(MADCTL_MV | MADCTL_BGR);
								width_  = ILI9488_TFTHEIGHT;
								height_ = ILI9488_TFTWIDTH;
							} break;
							case rotations::gd180: {
								D::write_data(MADCTL_MY | MADCTL_BGR);
								width_  = ILI9488_TFTWIDTH;
								height_ = ILI9488_TFTHEIGHT;
							} break;
							case rotations::dg270: {
								D::write_data(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
								width_  = ILI9488_TFTHEIGHT;
								height_ = ILI9488_TFTWIDTH;
							} break;
						}
						rotation_ = _rotation;
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
					D::write_cmd(command::DISPON);
				}
				void        off(void){
					D::write_cmd(command::DISPOFF);
				}
				void        sleep(void){
					D::write_cmd(command::SLPIN);
				}
				void        wakeup(void){
					D::write_cmd(command::SLPOUT);
				}
				uint32_t id = 0;
				uint16_t error = 0;
				void begin(void){
					
					//********Start Initial Sequence*******//
				D::write_cmd(0xE0); //P-Gamma
				D::write_data(0x00);
				D::write_data(0x13);
				D::write_data(0x18);
				D::write_data(0x04);
				D::write_data(0x0F);
				D::write_data(0x06);
				D::write_data(0x3A);
				D::write_data(0x56);
				D::write_data(0x4D);
				D::write_data(0x03);
				D::write_data(0x0A);
				D::write_data(0x06);
				D::write_data(0x30);
				D::write_data(0x3E);
				D::write_data(0x0F);
				D::write_cmd(0XE1); //N-Gamma
				D::write_data(0x00);
				D::write_data(0x13);
				D::write_data(0x18);
				D::write_data(0x01);
				D::write_data(0x11);
				D::write_data(0x06);
				D::write_data(0x38);
				D::write_data(0x34);
				D::write_data(0x4D);
				D::write_data(0x06);
				D::write_data(0x0D);
				D::write_data(0x0B);
				D::write_data(0x31);
				D::write_data(0x37);
				D::write_data(0x0F);
				D::write_cmd(0xC0);
				D::write_data(0x18);
				D::write_data(0x16);
				D::write_cmd(0xC1);
				D::write_data(0x45);
				D::write_cmd(0xC5); //VCOM
				D::write_data(0x00);
				D::write_data(0x63);
				D::write_data(0x01);

				D::write_cmd(0x36); //RAM address mode
				//0xF8 and 0x3C are landscape mode. 0x5C and 0x9C for portrait mode.
				D::write_data(0xF8);

				D::write_cmd(0x3A); //Interface Mode Control
				D::write_data(0x66); //16-bit serial mode
				D::write_cmd(0xB0); //Interface Mode Control
				D::write_data(0x80); //SDO not in use
				D::write_cmd(0xB1); //Frame rate 70HZ
				D::write_data(0x00); //
				D::write_data(0x10);
				D::write_cmd(0xB4);
				D::write_data(0x02);

				D::write_cmd(0xB6); //RGB/MCU Interface Control
				D::write_data(0x02);
				//D::write_data(0x22);

				D::write_cmd(0xE9);
				D::write_data(0x00);
				D::write_cmd(0xF7);
				D::write_data(0xA9);
				D::write_data(0x51);
				D::write_data(0x2C);
				D::write_data(0x82);
				D::write_cmd(0x11);
				D::delay_ms(120);
				D::write_cmd(0x21);


				D::delay_ms(120);
				D::write_cmd(0x29);
	
					/*
					D::write_cmd(0xE0);
				D::write_data(0x00);
				D::write_data(0x03);
				D::write_data(0x09);
				D::write_data(0x08);
				D::write_data(0x16);
				D::write_data(0x0A);
				D::write_data(0x3F);
				D::write_data(0x78);
				D::write_data(0x4C);
				D::write_data(0x09);
				D::write_data(0x0A);
				D::write_data(0x08);
				D::write_data(0x16);
				D::write_data(0x1A);
				D::write_data(0x0F);
	

				D::write_cmd(0XE1);
				D::write_data(0x00);
				D::write_data(0x16);
				D::write_data(0x19);
				D::write_data(0x03);
				D::write_data(0x0F);
				D::write_data(0x05);
				D::write_data(0x32);
				D::write_data(0x45);
				D::write_data(0x46);
				D::write_data(0x04);
				D::write_data(0x0E);
				D::write_data(0x0D);
				D::write_data(0x35);
				D::write_data(0x37);
				D::write_data(0x0F);
	
	
	 
				D::write_cmd(0XC0);      //Power Control 1 
				D::write_data(0x17);    //Vreg1out 
				D::write_data(0x15);    //Verg2out 
				 
				D::write_cmd(0xC1);      //Power Control 2     
				D::write_data(0x41);    //VGH,VGL 
				 
				D::write_cmd(0xC5);      //Power Control 3 
				D::write_data(0x00); 
				D::write_data(0x12);    //Vcom 
				D::write_data(0x80);
				 
				D::write_cmd(0x36);      //Memory Access 
				D::write_data(0x48); 
				 
				D::write_cmd(0x3A);      // Interface Pixel Format 
				D::write_data(0x66); 	  //18 bit    
				 
				D::write_cmd(0XB0);      // Interface Mode Control 
				D::write_data(0x80);     			 //SDO NOT USE
				 
				D::write_cmd(0xB1);      //Frame rate 
				D::write_data(0xA0);    //60Hz 
				 
				D::write_cmd(0xB4);      //Display Inversion Control 
				D::write_data(0x02);    //2-dot 
				 
				D::write_cmd(0XB6);      //Display Function Control  RGB/MCU Interface Control 
				
				D::write_data(0x02);    //MCU 
				D::write_data(0x02);    //Source,Gate scan dieection 
					
				D::write_cmd(0XE9);      // Set Image Functio
				D::write_data(0x00);    // Disable 24 bit data
				 
				D::write_cmd(0xF7);      // Adjust Control 
				D::write_data(0xA9);     
				D::write_data(0x51);     
				D::write_data(0x2C);     
				D::write_data(0x82);    // D7 stream, loose 

				D::write_cmd(0x11); //Sleep out
				D::delay_ms(120);
				D::write_cmd(0x29);
	*/
					/*
					D::write_cmd(command::RDDID);
					D::read_data((uint8_t *) &id,4);

					D::write_cmd(command::RDDID);
					D::read_data((uint8_t *) &id,4);

					D::write_cmd(command::GMCTRP1);//0xE0
					D::write_data(0x00);
					D::write_data(0x03);
					D::write_data(0x09);
					D::write_data(0x08);
					D::write_data(0x16);
					D::write_data(0x0A);
					D::write_data(0x3F);
					D::write_data(0x78);
					D::write_data(0x4C);
					D::write_data(0x09);
					D::write_data(0x0A);
					D::write_data(0x08);
					D::write_data(0x16);
					D::write_data(0x1A);
					D::write_data(0x0F);

					D::write_cmd(command::GMCTRN1);//0xE1
					D::write_data(0x00);
					D::write_data(0x16);
					D::write_data(0x19);
					D::write_data(0x03);
					D::write_data(0x0F);
					D::write_data(0x05);
					D::write_data(0x32);
					D::write_data(0x45);
					D::write_data(0x46);
					D::write_data(0x04);
					D::write_data(0x0E);
					D::write_data(0x0D);
					D::write_data(0x35);
					D::write_data(0x37);
					D::write_data(0x0F);

					D::write_cmd(command::PWCTR1);      //Power Control 1
					D::write_data(0x17);    //Vreg1out
					D::write_data(0x15);    //Verg2out

					D::write_cmd(command::PWCTR2);      //Power Control 2
					D::write_data(0x41);    //VGH,VGL

					D::write_cmd(command::VMCTR1);      //0xC5
					D::write_data(0x00);
					D::write_data(0x12);    //Vcom
					D::write_data(0x80);

					D::write_cmd(command::MADCTL);      //Memory Access //0x36
					D::write_data(0x48);

					D::write_cmd(command::PIXFMT);      // Interface Pixel Format // 0x3A
					D::write_data(0x66); 	  //18 bit

					D::write_cmd(0XB0);      // Interface Mode Control
					D::write_data(0x80);     			 //SDO NOT USE

					D::write_cmd(command::FRMCTR1);      //Frame rate //0xB1
					D::write_data(0xA0);    //60Hz

					D::write_cmd(command::INVCTR);      //Display Inversion Control //0xB4
					D::write_data(0x02);    //2-dot

					D::write_cmd(command::DFUNCTR);      //Display Function Control  RGB/MCU Interface Control //0XB6

					D::write_data(0x02);    //MCU
					D::write_data(0x02);    //Source,Gate scan dieection

					D::write_cmd(0XE9);      // Set Image Functio
					D::write_data(0x00);    // Disable 24 bit data

					D::write_cmd(0xF7);      // Adjust Control
					D::write_data(0xA9);
					D::write_data(0x51);
					D::write_data(0x2C);
					D::write_data(0x82);    // D7 stream, loose

					wakeup();    //Exit Sleep
					D::delay_ms(120);
					on();    //Display on					
					*/
				}
		};
	}
}
#endif