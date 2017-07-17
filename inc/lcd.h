#ifndef __LCD_H__
#define __LCD_H__

#define PCF8574_ADDR	0x3f
#define SC	20
#define SL	4


void lcd_task(void *vpars);
void lcd_setstr(int, int, char *);
char *lcd_getstr(int);

#endif /* __LCD_H__*/
