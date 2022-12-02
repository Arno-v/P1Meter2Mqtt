#include "TelegramReader.h"
#include "IHandleValue.h"
#include "MeterDataDescriptor.h"



TelegramContainer telegram;
 
class BlockPointer:public IBlock
{
public:
    BlockPointer(char *start,char *end)
    {
        _start=start;
        _end=end;
    }
    virtual char *StartOfBlock()const
    {
        return _start;
    }
    virtual const char *EndOfBlock()
    {
        return _end;
    }
private:
    char *_start;
    char *_end;
};

bool readTelegram()
{
  TelegramReader reader(&telegram);
  if(reader.ReadTelegram())
  {
    for (auto i=0;i<reader.BlockCount()-1;i++)
    {
        BlockPointer bp(telegram.GetBlock(i),telegram.GetBlock(i+1)-1);
        HandleBlock(&bp);
    }
    return true;
  }
  return false;
}
