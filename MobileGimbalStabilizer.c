        #define IMU_WRITE 0b11010000  // Write Address      AD0=0
         #define IMU_READ 0b11010001   // Read Address       AD0=0
         #define  PICFrequency 4000000
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
     float RACC_X;
     float RACC_Y;
     float RACC_Z;
     
     float Aax;
     float Aay;
     float Aaz;
     float Racc;
     
     float GYRO_X;
     float GYRO_Y;
     float GYRO_Z;
     
     float Axz;
     float Ayz;
     
     float RxEst;
     float RyEst;
     float RzEst;
     
     float REst;
     
     char firstSample=1;
     
     char signRzGyro;
     
     float wGyro;
     
     unsigned long lastSeconds;
     unsigned long newSeconds;
     unsigned long dt;
     
     T1CON=0b00000001;
     I2C1_Init(400000);      //Fast Frequency(400K), we could use 100K also
     TRISC = 0x00;           //TX o/p
     UART1_Init(9615);       //Baud rate
     Delay_ms(500);
     IMU_Init();
     IMU_Write(0x1A, 0x03);         //CONFIG Register, Digital Low Pass Filter: 3 *MIT*
    while(1)
    {  
       newSeconds= TMR1H<<8 | TMR1L;         //Getting time elapsed since last reading (Delta T)
       newSeconds*=(4*(1/PICFrequency)*1);    //Transforming from Ticks to Seconds for TMR1 Prescaler of 1
       dt= newSeconds -lastSeconds;
       lastSeconds=  newSeconds;
       
       RACC_X=(float)IMU_Read(0x3B)/2048.0;
       RACC_Y=(float)IMU_Read(0x3D)/2048.0;
       RACC_Z=(float)IMU_Read(0x3F)/2048.0;

       //Getting the resultant
       Racc= SQRT(RACC_X*RACC_X + RACC_Y*RACC_Y + RACC_Z*RACC_Z);

       //Normalizing Accelerometer Resultant
       RACC_X/=Racc;
       RACC_Y/=Racc;
       RACC_Z/=Racc;

       if(firstSample)
       {  //Our current estimate are the accelerometer readings
         RxEst = RACC_X;
         RyEst = RACC_Y;
         RzEst = RACC_Z;
       }
       else
       {
          if(abs(RzEst) < 0.1)
          {   //Rz is too small and because it is used as reference for computing Axz, Ayz it's error fluctuations will amplify leading to bad results
              //in this case skip the gyro data and just use previous estimate
              GYRO_X=RxEst;
              GYRO_Y=RyEst;
              GYRO_Z=RzEst;
          }
          else
          {
               GYRO_X=(float)IMU_Read(0x43)/ 131.0;
               GYRO_Y=(float)IMU_Read(0x45)/ 131.0;
               GYRO_Z=(float)IMU_Read(0x47)/ 131.0;
               //Multiply Gyro's Angular velocity by time (seconds) to get Degrees
               GYRO_X*=dt;
               GYRO_Y*=dt;
               //Get the angles Axz, Ayz
               Axz=atan2(RxEst,RzEst) * 180/3.14; //convert from Rad to Degrees
               Ayz=atan2(RyEst,RzEst) * 180/3.14; //convert from Rad to Degrees
               //
               Axz+=GYRO_X;
               Ayz+=GYRO_Y;
          }

          signRzGyro = ( cos(Axz * 3.14 / 180) >=0 ) ? 1 : -1;
          //Equations from Starlino website
         /* GYRO_X= sin(Axz * 3.14 / 180);
          GYRO_X/= sqrt( 1 + (cos(Axz * 3.14 / 180)*(cos(Axz * 3.14 / 180) * tan(Ayz * 3.14 / 180)*tan(Ayz * 3.14 / 180))));
          GYRO_Y= sin(Ayz * 3.14 / 180);
          GYRO_Y/=sqrt( 1 + (cos(Ayz * 3.14 / 180)*(cos(Ayz * 3.14 / 180) * tan(Axz * 3.14 / 180)*tan(Axz * 3.14 / 180)))); */
          //Simplified version of the equations
          GYRO_X= 1/SQRT (1  +   (1/tan(Axz * 3.14 / 180))*(1/tan(Axz * 3.14 / 180)) * (1/cos(Ayz * 3.14 / 180))*(1/cos(Ayz * 3.14 / 180)));
          GYRO_Y= 1/SQRT (1  +   (1/tan(Ayz * 3.14 / 180))*(1/tan(Ayz * 3.14 / 180)) * (1/cos(Axz * 3.14 / 180))*(1/cos(Axz * 3.14 / 180)));

          GYRO_Z  =  signRzGyro*sqrt(1-GYRO_X*GYRO_X-GYRO_Y*GYRO_Y);
       }

       //Combine the Acceleratmor and Gyroscope values
       RxEst=(RACC_X + wGyro* GYRO_X) / (1 + wGyro);
       RyEst=(RACC_Y + wGyro* GYRO_Y) / (1 + wGyro);
       RzEst=(RACC_Z + wGyro* GYRO_Z) / (1 + wGyro);

       REst= SQRT(RxEst*RxEst + RyEst*RyEst + RzEst*RzEst);

       //Normalizing Accelerometer Resultant
       RxEst/=REst;
       RyEst/=REst;
       RzEst/=REst;

       Aax= acos(RxEst/REst);
       Aay= acos(RyEst/REst);
       Aaz= acos(RzEst/REst);

       print(Aax,"Aax");
       print(Aay,"Aay");
       print(Aaz,"Aaz");

     }
}
