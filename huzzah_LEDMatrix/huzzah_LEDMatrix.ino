unsigned char i;
 unsigned char j; 
/*Port Definitions*/
int Max7219_pinCLK = 14;
int Max7219_pinCS = 12;
int Max7219_pinDIN = 13;
 
unsigned char disp1[4][8]={
{0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF},//Square 1
{0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00},//Square 2
{0x00,0x00,0x3C,0x24,0x24,0x3C,0x00,0x00},//Square 3
{0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00},//Square 4
};

unsigned char heart[1][8]={
{0x00,0x66,0xFF,0xFF,0x7E,0x3C,0x18,0x00},
};

unsigned char check[1][8]={
{0x00,0x03,0x06,0x8C,0xD8,0x70,0x20,0x00},
};
 
 
void Write_Max7219_byte(unsigned char DATA) 
{   
            unsigned char i;
      digitalWrite(Max7219_pinCS,LOW);    
      for(i=8;i>=1;i--)
          {     
             digitalWrite(Max7219_pinCLK,LOW);
             digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
             DATA = DATA<<1;
             digitalWrite(Max7219_pinCLK,HIGH);
           }                                 
}
 
 
void Write_Max7219(unsigned char address,unsigned char dat)
{
        digitalWrite(Max7219_pinCS,LOW);
        Write_Max7219_byte(address);           //address，code of LED
        Write_Max7219_byte(dat);               //data，figure on LED 
        digitalWrite(Max7219_pinCS,HIGH);
}
 
void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x03);       //brightness 
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
}
 
 
 
void setup()
{
 
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  delay(50);
  Init_MAX7219();
}
 
 
void loop()
{ 
  //Flashing Squares
  /*
  for(j=0;j<4;j++)
  {
   for(i=1;i<9;i++)
    Write_Max7219(i,disp1[j][i-1]);
   delay(50);
  }     
  for(j=2;j>0;j--)
  {
   for(i=1;i<9;i++)
    Write_Max7219(i,disp1[j][i-1]);
   delay(50);
  }    */

  //Check Mark
  for(i=1;i<9;i++)
    Write_Max7219(i,check[0][i-1]);

  delay(5000);

  //Heart
  for(i=1;i<9;i++)
    Write_Max7219(i,heart[0][i-1]);

  delay(5000);
  
}
