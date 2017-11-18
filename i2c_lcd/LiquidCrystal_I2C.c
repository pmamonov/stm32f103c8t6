#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "I2C.h"
#include "LiquidCrystal_I2C.h"
#include "FreeRTOS.h"
#include "task.h"

//YWROBOT
//last updated on 21/12/2011
//Tim Starling Fix the reset bug (Thanks Tim)
//wiki doc http://www.dfrobot.com/wiki/index.php?title=I2C/TWI_LCD1602_Module_(SKU:_DFR0063)
//Support Forum: http://www.dfrobot.com/forum/
//Compatible with the Arduino IDE 1.0
//Library version:1.1


int LCDI2C_write(uint8_t value){
	return LCDI2C_send(value, Rs);
}



// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystal_I2C_Def lcdi2c;

int LCDI2C_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
  lcdi2c.Addr = lcd_Addr;
  lcdi2c.cols = lcd_cols;
  lcdi2c.rows = lcd_rows;
  lcdi2c.backlightval = LCD_NOBACKLIGHT;

  init_I2C1(); // Wire.begin();
  lcdi2c.displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  return LCDI2C_begin(lcd_cols, lcd_rows);
}

int LCDI2C_begin(uint8_t cols, uint8_t lines) {//, uint8_t dotsize) {
	int ret;

	if (lines > 1) {
		lcdi2c.displayfunction |= LCD_2LINE;
	}
	lcdi2c.numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
/*	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}*/

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	vTaskDelay(50);

	// Now we pull both RS and R/W low to begin commands
	ret = LCDI2C_expanderWrite(lcdi2c.backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	if (ret)
		return ret;
	vTaskDelay(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
    ret = LCDI2C_write4bits(0x03 << 4);
    if (ret)
    	return ret;
    vTaskDelay(5); // wait min 4.1ms

   // second try
    ret = LCDI2C_write4bits(0x03 << 4);
    if (ret)
    	return ret;
    vTaskDelay(5); // wait min 4.1ms

   // third go!
    ret = LCDI2C_write4bits(0x03 << 4);
    if (ret)
    	return ret;
    vTaskDelay(1);

   // finally, set to 4-bit interface
    ret |= LCDI2C_write4bits(0x02 << 4);
    if (ret)
    	return ret;


	// set # lines, font size, etc.
	ret = LCDI2C_command(LCD_FUNCTIONSET | lcdi2c.displayfunction);
    if (ret)
    	return ret;

	// turn the display on with no cursor or blinking default
	lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	ret = LCDI2C_display();
    if (ret)
    	return ret;

	// clear it off
	ret = LCDI2C_clear();
    if (ret)
    	return ret;

	// Initialize to default text direction (for roman languages)
	lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	ret = LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
    if (ret)
    	return ret;

	ret = LCDI2C_home();
	
	return ret;
}

/********** high level commands, for the user! */
int LCDI2C_clear(){
	int ret;
	ret = LCDI2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	vTaskDelay(3);  // this command takes a long time!
    return ret;
}

int LCDI2C_home(){
    int ret = LCDI2C_command(LCD_RETURNHOME);  // set cursor position to zero

	vTaskDelay(3);  // this command takes a long time!
    return ret;
}

int LCDI2C_setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > lcdi2c.numlines ) {
		row = lcdi2c.numlines-1;    // we count rows starting w/0
	}
	return LCDI2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDI2C_noDisplay() {
	lcdi2c.displaycontrol &= ~LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

int LCDI2C_display() {
	lcdi2c.displaycontrol |= LCD_DISPLAYON;
	return LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turns the underline cursor on/off
void LCDI2C_noCursor() {
	lcdi2c.displaycontrol &= ~LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_cursor() {
	lcdi2c.displaycontrol |= LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turn on and off the blinking cursor
void LCDI2C_noBlink() {
	lcdi2c.displaycontrol &= ~LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_blink() {
	lcdi2c.displaycontrol |= LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDI2C_scrollDisplayLeft(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C_scrollDisplayRight(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDI2C_leftToRight(void) {
	lcdi2c.displaymode |= LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This is for text that flows Right to Left
void LCDI2C_rightToLeft(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'right justify' text from the cursor
void LCDI2C_autoscroll(void) {
	lcdi2c.displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'left justify' text from the cursor
void LCDI2C_noAutoscroll(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDI2C_createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	LCDI2C_command(LCD_SETCGRAMADDR | (location << 3));
	int i;
	for (i=0; i<8; i++) {
		LCDI2C_write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
int LCDI2C_noBacklight(void) {
	lcdi2c.backlightval=LCD_NOBACKLIGHT;
	return LCDI2C_expanderWrite(0);
}

int LCDI2C_backlight(void) {
	lcdi2c.backlightval=LCD_BACKLIGHT;
	return LCDI2C_expanderWrite(0);
}



/*********** mid level commands, for sending data/cmds */

int LCDI2C_command(uint8_t value) {
	return LCDI2C_send(value, 0);
}


/************ low level data pushing commands **********/

// write either command or data
int LCDI2C_send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
    int ret = LCDI2C_write4bits((highnib)|mode);
	if (ret)
		return ret;
	return LCDI2C_write4bits((lownib)|mode);
}

int LCDI2C_write4bits(uint8_t value) {
	int ret = LCDI2C_expanderWrite(value);

	if (ret)
		return ret;
	return LCDI2C_pulseEnable(value);
}

int LCDI2C_expanderWrite(uint8_t _data){
	int ret = I2C_StartTransmission (I2C1,
					 I2C_Direction_Transmitter,
					 lcdi2c.Addr);
	if (ret)
		return ret;
	ret = I2C_WriteData(I2C1, (int)(_data) | lcdi2c.backlightval);
	I2C_GenerateSTOP(I2C1, ENABLE); //Wire.endTransmission();

	return ret;
}

static void __delay(int n)
{
	volatile int i = 100 * n;

	while (i--)
		;	
}

int LCDI2C_pulseEnable(uint8_t _data){
	int ret;

	ret = LCDI2C_expanderWrite(_data | En);	// En high
	if (ret)
		return ret;
	__delay(2);		// enable pulse must be >450ns

	ret = LCDI2C_expanderWrite(_data & ~En);	// En low
	if (ret)
		return ret;
	__delay(40);		// commands need > 37us to settle
	return ret;
}


// Alias functions

void LCDI2C_cursor_on(){
	LCDI2C_cursor();
}

void LCDI2C_cursor_off(){
	LCDI2C_noCursor();
}

void LCDI2C_blink_on(){
	LCDI2C_blink();
}

void LCDI2C_blink_off(){
	LCDI2C_noBlink();
}

void LCDI2C_load_custom_character(uint8_t char_num, uint8_t *rows){
		LCDI2C_createChar(char_num, rows);
}

void LCDI2C_setBacklight(uint8_t new_val){
	if(new_val){
//		backlight();		// turn backlight on
	}else{
//		noBacklight();		// turn backlight off
	}
}

//Функция передачи строки через USART
int LCDI2C_write_String(char* str) {
  uint8_t i=0;
  int ret;

  while(str[i])
  {
    ret = LCDI2C_write(str[i]);
    if (ret)
    	return ret;
    i++;
  }
}


