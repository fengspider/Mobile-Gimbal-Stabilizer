        #define IMU_WRITE 0b11010000  // Write Address      AD0=0
         #define IMU_READ 0b11010001   // Read Address       AD0=0
         unsigned char word [16];
         unsigned char title [16];
         unsigned char TempH;
         unsigned char TempL;


void IMU_Write(unsigned char regAddress, unsigned char regData )
{
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(regAddress);    //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regData);
     I2C1_Stop();
     Delay_ms(1);            //allow time for stop to be sent *MIT*   *********** Why did Karim make it 70 ?
}
signed int IMU_Read(unsigned char regAddress)  // Used for two consecutive reads
{
     signed int outReg;
     I2C1_Start();                           //Write ACC X Address
     I2C1_Wr(IMU_WRITE);     //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regAddress);                          //Read regAddress Data
     I2C1_Repeated_Start();           // The Data sheet states that we should "start", we are not sure if we should "Restart".  K.opinion: ac:Repeated_start  // as mentioned in the answer that we need to start again without a stop and he called it a "restart" which is Repeated_Start method in our UART library.
     i2c1_wr(IMU_READ);      // k., should we write the number of registers that we're targeting?                       k.opinion contt_2    ac:repeated_start_3       // supports s.opinion.
     TempH=I2C1_Rd(1);
     while (!I2C1_Is_Idle()) asm nop;  // Wait for the read cycle to finish
     TempL=I2C1_Rd(0);
     while (!I2C1_Is_Idle()) asm nop;   // Wait for the read cycle to finish
     outReg=TempH<<8|TempL; // Concatenate the High and Low Registers
     I2C1_Stop();
     Delay_ms(1);            //allow time for stop to be sent *MIT*
     return outReg;
}
void print(signed int number,unsigned char title [16])
{
     IntToHex(number, word);
     UART1_Write_Text(title);
     UART1_Write_Text(word);
     UART1_Write_Text("                                                                     ");
     Delay_ms(100);
}
void IMU_Init()
{    IMU_Write(0x6b,0b10000000); //PWR_MGMT_1; Reset
     IMU_Write(0x6b,0b00001001); //PWR_MGMT_1; Temperature Sensor Disabled, PLL X Gyro clock
     IMU_Write(0x1B,0x00); // GYRO_CONFIG; Self test off[7:5], Full Scale Range: +-250 Deg/Sec
     IMU_Write(0x1C,0x00);// ACCEL_CONFIG; Self test off[7:5], Full Scale Range: +-2g
}
signed int getAccAngle(signed int ACC_sub,signed int ACC)
{
     return acos(ACC_sub/ACC);
}
void main() {
     float ACC_X;
     float ACC_Y;
     float ACC_Z;
     
     float Aax;
     float Aay;
     float Aaz;
     float Racc;
     
     float GYRO_X;
     float GYRO_Y;
     float GYRO_Z;
     
     I2C1_Init(400000);      //Fast Frequency(400K), we could use 100K also
     TRISC = 0x00;           //TX o/p
     UART1_Init(9615);       //Baud rate
     Delay_ms(500);
     IMU_Init();
     IMU_Write(0x1A, 0x03);         //CONFIG Register, Digital Low Pass Filter: 3 *MIT*
    while(1)
     {
     ACC_X=(float)IMU_Read(0x3B)/2048.0;
     print(ACC_X,"X.acce:");
     
     GYRO_X=(float)IMU_Read(0x43)/ 131.0;
     print(GYRO_X,"X.Gyro:");
     
     ACC_Y=(float)IMU_Read(0x3D)/2048.0;
     print(ACC_Y,"Y.acce:");
     
     GYRO_Y=(float)IMU_Read(0x45)/ 131.0;
     print(GYRO_Y,"Y.Gyro:");
     
     ACC_Z=(float)IMU_Read(0x3F)/2048.0;
     print(ACC_Z,"Z.acce:");
     
     GYRO_Z=(float)IMU_Read(0x47)/ 131.0;
     print(GYRO_Z,"Z.Gyro:");
     
     Racc= SQRT(ACC_X*ACC_X + ACC_Y*ACC_Y + ACC_Z*ACC_Z)

     Aax= acos(ACC_X/Racc);
     Aay= acos(ACC_Y/Racc);
     Aaz= acos(ACC_Z/Racc);





     }
}
