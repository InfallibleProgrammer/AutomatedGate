#include "CMFRC522.h"
#include "Arduino.h"
#include <SPI.h>
CMFRC522::CMFRC522(int pin1 =53, int pin2 = 5){

chipSelectPin = pin1;
NRSTPD = pin2;
}

int CMFRC522::InitialRun1(int pin1 =53){
  chipSelectPin = pin1;
pinMode(chipSelectPin,OUTPUT);
 digitalWrite(chipSelectPin,LOW);
 }
 
int CMFRC522::InitialRun2(int pin2 =5){
  NRSTPD = pin2;
  pinMode(NRSTPD, OUTPUT);
 digitalWrite(NRSTPD, HIGH);
}

void CMFRC522::MFRC522_Init(void){
digitalWrite(NRSTPD,HIGH);

	MFRC522_Reset();
    Write_MFRC522(TModeReg, 0x8D);	
    Write_MFRC522(TPrescalerReg, 0x3E);	
    Write_MFRC522(TReloadRegL, 30);           
    Write_MFRC522(TReloadRegH, 0);
	
	Write_MFRC522(TxAutoReg, 0x40);		
	Write_MFRC522(ModeReg, 0x3D);		

	AntennaOn();		
}

int CMFRC522::MFRC522runtime(){
  uchar i,tmp;
  uchar status;
  int out;
        uchar str[MAX_LEN];
        uchar RC_size;
        uchar blockAddr;  


    status = MFRC522_Request(PICC_REQIDL, str); 
    if (status == MI_OK)
    {
    }

   
    status = MFRC522_Anticoll(str);
    memcpy(serNum, str, 5);
    if (status == MI_OK)
    {
                        Serial.println("The card's number is  : ");
      for(int i =0; i<5; i++){
        out += serNum[i];
      }
      Serial.println(out);
    
    }

    RC_size = MFRC522_SelectTag(serNum);
    if (RC_size != 0)
    {}
                
    
    blockAddr = 7;      
    status = MFRC522_Auth(PICC_AUTHENT1A, blockAddr, sectorKeyA[blockAddr/4], serNum);  //认证
    if (status == MI_OK)
    {
      
      status = MFRC522_Write(blockAddr, sectorNewKeyA[blockAddr/4]);
                        Serial.print("set the new card password, and can modify the data of the Sector: ");
                        Serial.print(blockAddr/4,DEC);
   
                        //写数据
                        blockAddr = blockAddr - 3 ; 
                        status = MFRC522_Write(blockAddr, writeDate);
                        if(status == MI_OK)
                        {
                           Serial.println("OK!");
                        }
    }

   
    blockAddr = 7;        
    status = MFRC522_Auth(PICC_AUTHENT1A, blockAddr, sectorNewKeyA[blockAddr/4], serNum); //认证
    if (status == MI_OK)
    {
      
                        blockAddr = blockAddr - 3 ; 
                        status = MFRC522_Read(blockAddr, str);
      if (status == MI_OK)
      {
                                Serial.println("Read from the card ,the data is : ");
        for (i=0; i<16; i++)
        {
                          Serial.print(str[i]);
        }
                                Serial.println(" ");
      }
    }
               
    MFRC522_Halt();                   
  return out;        
}

void CMFRC522::Write_MFRC522(uchar addr, uchar val){
  digitalWrite(53, LOW);
  SPI.transfer((addr<<1)&0x7E);
  SPI.transfer(val);

  digitalWrite(53, HIGH);
}

uchar CMFRC522::Read_MFRC522(uchar addr){
  uchar val;

  digitalWrite(53, LOW);

  
  SPI.transfer(((addr<<1)&0x7E) | 0x80);  
  val =SPI.transfer(0x00);
  
  digitalWrite(53, HIGH);
  
  return val; 
}
void CMFRC522::SetBitMask(uchar reg, uchar mask){
  uchar tmp;
  tmp = Read_MFRC522(reg);
  Write_MFRC522(reg, tmp | mask);
}
void CMFRC522::ClearBitMask(uchar reg, uchar mask){
  uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask)); 
}
void CMFRC522::AntennaOn(void){
  uchar temp;
  temp = Read_MFRC522(TxControlReg);
  if (!(temp & 0x03))
  {
    SetBitMask(TxControlReg, 0x03);
  }
}
void CMFRC522::AntennaOff(void)
{
  ClearBitMask(TxControlReg, 0x03);
}
void CMFRC522::MFRC522_Reset(void)
{
    Write_MFRC522(CommandReg, PCD_RESETPHASE);
}
uchar CMFRC522::MFRC522_Request(uchar reqMode, uchar *TagType)
{
  uchar status;  
  uint backBits;     

  Write_MFRC522(BitFramingReg, 0x07);   
  
  TagType[0] = reqMode;
  status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

  if ((status != MI_OK) || (backBits != 0x10))
  {    
    status = MI_ERR;
  }
   
  return status;
}

uchar CMFRC522::MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
    uchar status = MI_ERR;
    uchar irqEn = 0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;

    switch (command)
    {
        case PCD_AUTHENT:    
    {
      irqEn = 0x12;
      waitIRq = 0x10;
      break;
    }
    case PCD_TRANSCEIVE:  
    {
      irqEn = 0x77;
      waitIRq = 0x30;
      break;
    }
    default:
      break;
    }
   
    Write_MFRC522(CommIEnReg, irqEn|0x80);  
    ClearBitMask(CommIrqReg, 0x80);     
    SetBitMask(FIFOLevelReg, 0x80);     
    
  Write_MFRC522(CommandReg, PCD_IDLE);  

  
    for (i=0; i<sendLen; i++)
    {   
    Write_MFRC522(FIFODataReg, sendData[i]);    
  }

  
  Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {    
    SetBitMask(BitFramingReg, 0x80);    //StartSend=1,transmission of data starts  
  }   
    
 
  i = 2000; 
    do 
    {
    
        n = Read_MFRC522(CommIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    ClearBitMask(BitFramingReg, 0x80);   
  
    if (i != 0)
    {    
        if(!(Read_MFRC522(ErrorReg) & 0x1B))  
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   
        status = MI_NOTAGERR;       
      }

            if (command == PCD_TRANSCEIVE)
            {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits)
                {   
          *backLen = (n-1)*8 + lastBits;   
        }
                else
                {   
          *backLen = n*8;   
        }

                if (n == 0)
                {   
          n = 1;    
        }
                if (n > MAX_LEN)
                {   
          n = MAX_LEN;   
        }
        
        
                for (i=0; i<n; i++)
                {   
          backData[i] = Read_MFRC522(FIFODataReg);    
        }
            }
        }
        else
        {   
      status = MI_ERR;  
    }
        
    }
  
    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE); 

    return status;
}

uchar CMFRC522::MFRC522_Anticoll(uchar *serNum)
{
    uchar status;
    uchar i;
  uchar serNumCheck=0;
    uint unLen;
    

    
  Write_MFRC522(BitFramingReg, 0x00);  
 
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
  {
    
    for (i=0; i<4; i++)
    {   
      serNumCheck ^= serNum[i];
    }
    if (serNumCheck != serNum[i])
    {   
      status = MI_ERR;    
    }
    }

   

    return status;
} 

void CMFRC522::CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
    uchar i, n;

    ClearBitMask(DivIrqReg, 0x04);      
    SetBitMask(FIFOLevelReg, 0x80);     
  

    
    for (i=0; i<len; i++)
    {   
    Write_MFRC522(FIFODataReg, *(pIndata+i));   
  }
    Write_MFRC522(CommandReg, PCD_CALCCRC);

  
    i = 0xFF;
    do 
    {
        n = Read_MFRC522(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));      

  
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}

uchar CMFRC522::MFRC522_SelectTag(uchar *serNum)
{
    uchar i;
  uchar status;
  uchar size;
    uint recvBits;
    uchar buffer[9]; 

  

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
      buffer[i+2] = *(serNum+i);
    }
  CalulateCRC(buffer, 7, &buffer[7]);   //??
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    
    if ((status == MI_OK) && (recvBits == 0x18))
    {   
    size = buffer[0]; 
  }
    else
    {   
    size = 0;    
  }

    return size;
}

uchar CMFRC522::MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum)
{
    uchar status;
    uint recvBits;
    uchar i;
  uchar buff[12]; 

 
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i=0; i<6; i++)
    {    
    buff[i+2] = *(Sectorkey+i);   
  }
    for (i=0; i<4; i++)
    {    
    buff[i+8] = *(serNum+i);   
  }
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

    if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
    {   
    status = MI_ERR;   
  }
    
    return status;
}

uchar CMFRC522::MFRC522_Read(uchar blockAddr, uchar *recvData)
{
    uchar status;
    uint unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalulateCRC(recvData,2, &recvData[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }
    
    return status;
}

uchar CMFRC522::MFRC522_Write(uchar blockAddr, uchar *writeData)
{
    uchar status;
    uint recvBits;
    uchar i;
  uchar buff[18]; 
    
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {   
    status = MI_ERR;   
  }
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)
        {    
          buff[i] = *(writeData+i);   
        }
        CalulateCRC(buff, 16, &buff[16]);
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {   
      status = MI_ERR;   
    }
    }
    
    return status;
}

void CMFRC522::MFRC522_Halt(void)
{
  uchar status;
    uint unLen;
    uchar buff[4]; 

    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
 
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}
