         #define IMU_WRITE 0b11010000  // Write Address      AD0=0
         #define IMU_READ 0b11010001   // Read Address       AD0=0
         
         unsigned char TempH;
         unsigned char TempL;
void IMU_Init()
{
     I2C1_Init(400000); //Fast Frequency(400K), we could use 100K also
     I2C1_Start();
     //Write PWR_MGMT_1 Address
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(0x6B);  //Should I wait for acknowledgement  ?                                     **********
     // Reset, Sleep, Cycle, N/A, TempSensor Disable, Clk SC: PLL X Gyro
     I2C1_Wr(0b00001001);
     I2C1_Stop();
     Delay_ms(1);  //allow time for stop to be sent *MIT*

     I2C1_Start();
     //Write GYRO_CONFIG Address
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(0x1B);  //Should I wait for acknowledgement  ?                                     **********
     // Self test off[7:5], Full Scale Range: +-250 Deg/Sec
     I2C1_Wr(0x00);
     I2C1_Stop();
     Delay_ms(1);  //allow time for stop to be sent *MIT*

     I2C1_Start();
     //Write ACCEL_CONFIG Address
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(0x1C);  //Should I wait for acknowledgement  ?                                     **********
     // Self test off[7:5], Full Scale Range: +-2g
     I2C1_Wr(0x00);
     I2C1_Stop();
     Delay_ms(1);  //allow time for stop to be sent *MIT*
}
void IMU_Write(unsigned char regAddress, unsigned char regData )
{
     I2C1_Start();
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(regAddress);  //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regData);
     I2C1_Stop();
     Delay_ms(1);  //allow time for stop to be sent *MIT*
}
void IMU_Read2(unsigned char regAddress, unsigned char *outReg )  // Used for two consecutive reads
{
     I2C1_Start();
     //Write ACC X Address
     I2C1_Wr(IMU_WRITE);   //Should I wait for acknowledgement  ?                                     **********
     I2C1_Wr(regAddress);
     //Read regAddress Data
     I2C1_Start();              // The Data sheet states that we should "start", we are not sure if we should "Restart"
     I2C1_Wr(IMU_READ);
     TempH=I2C1_Rd(1);
     TempL=I2C1_Rd(0);
     *outReg=TempH<<8 | TempL;   // Concatenate the High and Low Registers
     I2C1_Stop();
     Delay_ms(1);  //allow time for stop to be sent *MIT*
}

void main() {
     signed int ACC_X;
     signed int GYRO_X;
     IMU_Init();
     IMU_Write(0x1A, 0x3);  //CONFIG Register, Digital Low Pass Filter: 3 *MIT*
    while(1)
     {
     IMU_Read2(0x3B,ACC_X);
     IMU_Read2(0x43,GYRO_X);
     }
     

}

 /* First Trial
   I2C1_Start();
     //Write ACC X Address
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(0x3B);
     //Read ACC X Data
     I2C1_Start();              // The Data sheet states that we should "start", we are not sure if we should "Restart"
     I2C1_Wr(IMU_READ);
     TempH=I2C1_Rd(1);
     TempL=I2C1_Rd(0);
     ACC_X=TempH<<8 | TempL;
     I2C1_Repeated_Start();

     //Write GYRO X Address
     I2C1_Wr(IMU_WRITE);
     I2C1_Wr(0x43);
     //Read GYRO X Data
     I2C1_Start();
     I2C1_Wr(IMU_READ);
     TempH=I2C1_Rd(1);
     TempL=I2C1_Rd(0);
     GYRO_X=TempH<<8 | TempL;
     I2C_Stop();*/