#pragma once
#include "TelegramContainer.h"

class TelegramReader
{
public:
  TelegramReader(TelegramContainer *tc)
  {
    _telegramCnt=tc;
    _telegramCnt->Reset();
    _pos=1;
    _block=0;
  }
  bool ReadTelegram()
  {
    if (!WaitForStart())
      return false;
    do
    {
      if (!ReadNextBlock())
        return false;
    }
    while (!isCrcBlock());
    auto crc=crc16();
    auto crcCode=_telegramCnt->GetBlock(_block-1)+1;
    crcCode[4]='\0';
    if (strtol(crcCode, nullptr, 16)!=crc)
    {
#ifdef DEBUG
      Serial.printf("Invalid crc %s != %0x\n",crcCode,crc); 
#endif
      return false;
    }
#ifdef DEBUG
    Serial.println("Valid crc"); 
#endif

    return true;

    
  }  
  unsigned BlockCount()const
  {
    return _block;
  }
private:  
  unsigned int crc16()
  {
    unsigned int crc=0x0000;
    unsigned char *buf=(unsigned char *)_telegramCnt->Telegram();
    auto len=_telegramCnt->GetBlockOffset(_block-1)+1;
    for (int pos = 0; pos < len; pos++)
    {
        crc ^= (unsigned int)buf[pos];

        for (int i = 8; i != 0; i--)
        {
            if ((crc & 0x0001) != 0)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
  }
  bool ReadStartBlock()
  {
    _telegramCnt->Telegram()[0]='/';
    auto len=Serial2.readBytesUntil('\n',_telegramCnt->Telegram()+1,P1_MAXLINELENGTH-1);
    if (len==0)
      return false;
    _telegramCnt->Telegram()[len+1]='\0';
#ifdef DEBUG    
    Serial.println(_telegramCnt->Telegram());
#endif
    _telegramCnt->Telegram()[len+1]='\n';
    _telegramCnt->SetBlock(0,0);
    _block=1;
    _pos=len+2;
    return true;
  }

  bool WaitForStart()
  {
    if (!Serial2.available())
      return false;
    #ifdef DEBUG
      Serial.println("Wait for start");
    #endif
    for (;;)
    {
      auto len=Serial2.readBytesUntil('/',_telegramCnt->Telegram(),P1_MAXLINELENGTH);
      if (len==0)
        return false;
      if (!ReadStartBlock())
        return false;
      if (_pos>=5&&_telegramCnt->Telegram()[4]=='5')
        break;
    }
    return true;
  }
  bool isCrcBlock()
  {    
    return _telegramCnt->GetBlock(_block-1)[0]=='!';
  }
  bool ReadNextBlock()
  {
    auto len=Serial2.readBytesUntil('\n',_telegramCnt->Telegram()+_pos,P1_MAXLINELENGTH-_pos);
    if (len==0)
      return false;
//    _telegramCnt->Telegram()[len+_pos]='\0';
//    Serial.println(_telegramCnt->Telegram()+_pos);
    _telegramCnt->Telegram()[len+_pos]='\n';    
    _telegramCnt->SetBlock(_block,_pos);
    _block++;
    _pos+=(len+1);
    return true;
  }
  unsigned _pos;  
  unsigned _block;
  TelegramContainer *_telegramCnt;
};
