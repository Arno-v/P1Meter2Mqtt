#pragma once
class IBlock
{
  public:
    virtual char *StartOfBlock()const=0;
    virtual const char *EndOfBlock()=0;
};
class MeterDataDescriptor;
class IHandleValue
{
public:
  virtual void HandleValue(MeterDataDescriptor *pDesc,IBlock *block)=0;

};