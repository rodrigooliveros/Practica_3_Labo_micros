
#define F_CPU 16000000                                                          // Define CPU Frequency 16MHz 
#include <avr/io.h>                                                             
#include <util/delay.h>    

#define LCD_Dir     DDRD                                                        // Define LCD data port direction 
#define LCD_Port    PORTD                                                       // Define LCD data port 
#define RS          PD2                                                         // Define Register Select pin 
#define EN          PD3                                                         // Define Enable signal pin 
                                                     
long num = 0;                                                                   // Distance integer
char buf[3];                                                                    // Buffer (distance in cm) as string

void LCD_Command(unsigned char cmnd) {
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0);                               // Sending upper nibble 
	LCD_Port &= ~ (1<<RS);                                                      // RS=0, command reg. 
	LCD_Port |= (1<<EN);                                                        // Enable pulse 
	_delay_us(10);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);                                 // Sending lower nibble
	LCD_Port |= (1<<EN);
	_delay_us(10);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(20);
}

void LCD_Init(void) {                                                           // LCD Initialize function 
	LCD_Dir = 0xFF;                                                             // Make LCD port direction as o/p 
	_delay_ms(20);                                                              // LCD Power ON delay always >15ms 

	LCD_Command(0x02);                                                          // send for 4 bit initialization of LCD  
	LCD_Command(0x28);                                                          // 2 line, 5*7 matrix in 4-bit mode 
	LCD_Command(0x0c);                                                          // Display on cursor off
	LCD_Command(0x06);                                                          // Increment cursor (shift cursor to right) 
	LCD_Command(0x01);                                                          // Clear display screen
	_delay_ms(2);
}

void LCD_Char(unsigned char data) {
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0);                               // Sending upper nibble 
	LCD_Port |= (1<<RS);                                                        // RS=1, data reg. 
	LCD_Port |= (1<<EN);
	_delay_us(10);
	LCD_Port &= ~ (1<<EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0x0F) | (data << 4);                                 // Sending lower nibble 
	LCD_Port |= (1<<EN);
	_delay_us(10);
	LCD_Port &= ~ (1<<EN);
	_delay_ms(20);
}

void LCD_String(char *str) {                                                    // Send string to LCD function 
	int i;
	for(i=0;str[i]!=0;i++)                                                      // Send each char of string till the NULL 
		LCD_Char(str[i]);
}

void LCD_String_xy(char row, char pos, char *str) {                             // Send string to LCD with xy position 
	if (row == 0 && pos<16)
        LCD_Command((pos & 0x0F)|0x80);                                         // Command of first row and required position<16 
	else if (row == 1 && pos<16)
        LCD_Command((pos & 0x0F)|0xC0);                                         // Command of first row and required position<16 
	LCD_String(str);                                                            // Call LCD string function 
}

void LCD_Clear() {
	LCD_Command (0x01);                                                         // Clear display 
	_delay_ms(2);
	LCD_Command (0x80);                                                         // Cursor at home position 
}

int main() {
    DDRC |= (1<<DDC1);                                                          // Trigger pin
    DDRC &= ~ (1<<DDC2);                                                        // Echo pin
    PORTC &= ~ ((1<<PC2) | (1<<PC1));                                           // Initialize in 0
    DDRD = 0xFF;                                                                // LCD port
    TCCR1A = 0x00;                                                              // Init Timer1
    TCCR1B |= (1<<CS12) | (0<<CS11) | (0<<CS10);                                // Prescaler for 16MHz
    
	LCD_Init();	
    LCD_String("Tec de Monterrey");
    LCD_Command(0xC0);                                                          // Go to 2nd line
    LCD_String("***  Puebla  ***");
    _delay_ms(2000);
    LCD_Clear();
    
    while(1) {
        PORTC |= (1<<PIN1);                                                    // Sending 1
        _delay_us(10);                                                          // Triggering the sensor for 10usec
        PORTC &= ~ (1<<PIN1);                                                  // Sending 0
                                                                                // Read value 'till timer is 1
        while(((PINC>>PC2)&1) == 0);                                            
        TCNT1 = 0;
        while((PINC>>PC2)&1);
        
        num = TCNT1*16*0.017;                                                   // Formula based on prescaler and frequence
        ltoa(num, buf, 10);                                                     // Convertion from long to string
        
        LCD_String("Reading...");	
        LCD_Command(0xC0);                                                      // Go to 2nd line
        LCD_String("Distance:");
        LCD_String_xy(1, 11, "   ");
        LCD_String_xy(1, 11, buf);
        LCD_String_xy(1, 14, "cm");
        LCD_Command(0x80);                                                      // Goes back to second line
    }
}