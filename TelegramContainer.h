#pragma once
#include "globals.h"

class TelegramContainer
{
public:
  TelegramContainer()
  {    
    _blocks=nullptr;
    _nrOfBlocks=0;
  }
  void Initialize(short blocks)
  {
    _blocks=new unsigned[blocks];
    _nrOfBlocks=blocks;
  }
  void Reset()
  {
    memset(_telegram, 0, sizeof(_telegram));
    memset (_blocks,0,sizeof(unsigned)*_nrOfBlocks);
  }
  char *Telegram()
  {
    return _telegram;
  }
  void SetBlock(int block,unsigned val)
  {
    _blocks[block]=val;
  }
  char *GetBlock(int block)
  {
    return _telegram+_blocks[block];
  }
  unsigned GetBlockOffset(int block)
  { 
    return _blocks[block];
  }
  private:
    char _telegram[P1_MAXLINELENGTH+2];
    unsigned *_blocks;
    short _nrOfBlocks;
};

extern TelegramContainer telegram;