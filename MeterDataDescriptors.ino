#include "MeterDataDescriptor.h"
#include "Globals.h"

class HandleFloat:public IHandleValue
{
public:
    virtual void HandleValue(MeterDataDescriptor *pDesc,IBlock *block)
    {
      auto value=block->StartOfBlock()+pDesc->Length+ValueOffset();//+1 to skip the (.
      pDesc->ValueString=value;
      auto endpos=block->EndOfBlock();
      auto dot=value+1;
      while (dot<endpos&&*dot!='.')
        dot++;
      if (*dot!='.')
        return;
      *dot='\0';
      long lval=atol(value);
      *dot='.'; 
      value=dot+1;
      while (dot<endpos&&*dot!='*')
        dot++;
      if (*dot!='*')
        return;
      *dot='\0';
      long decimalValue=atol(value);
      if (lval!=pDesc->Value || decimalValue!=pDesc->DecimalValue)
      {
        pDesc->Value=lval;
        pDesc->DecimalValue=decimalValue;
        pDesc->Send=true;
      }
    }
    virtual int ValueOffset()
    {
      return 1; //1 to skip the (.
    }
};
class HandleLong:public IHandleValue
{
public:
    virtual void HandleValue(MeterDataDescriptor *pDesc,IBlock *block)
    {
      auto value=block->StartOfBlock()+pDesc->Length+1;//+1 to skip the (.
      pDesc->ValueString=value;
      auto endpos=block->EndOfBlock();
      auto bra=value+1;
      while (bra<endpos&&*bra!=')')
        bra++;
      if (*bra!=')')
        return;
      *bra='\0';
      long lval=atol(value);
      long decimalValue=0;
      if (lval!=pDesc->Value || decimalValue!=pDesc->DecimalValue)
      {
        pDesc->Value=lval;
        pDesc->DecimalValue=decimalValue;
        pDesc->Send=true;
      }
    }
};

class HandlekWhClass:public HandleFloat
{
};
class HandleTarifGroupClass:public HandleLong
{
};
class HandlekWClass:public HandleFloat
{
};
class HandleVClass:public HandleFloat
{
};
class HandleAClass:public HandleLong
{
};
class HandleM3Class:public HandleFloat
{
    virtual int ValueOffset()
    {
      return 16; //16 to skip the date and ()
    }
};
class HandleGJClass:public HandleFloat
{
    virtual int ValueOffset()
    {
      return 16; //16 to skip the date and ()
    }
};

HandleM3Class HandleM3;
HandleGJClass HandleGJ;

class HandleDeviceTypeClass:public IHandleValue
{
  public:
    virtual void HandleValue(MeterDataDescriptor *pDesc,IBlock *block)
    {
      auto type=*(block->StartOfBlock()+pDesc->Length+3); //value of this block is (00x) 
      pDesc->Code="x-1:24.1.0";
      if (type=='3')
      {
        pDesc[2].Name=MQTT_ROOT_TOPIC"gas_meter_m3";
        pDesc[2].HandleValue=&HandleM3;
      }
      else if (type=='4')
      {
        pDesc[2].Name=MQTT_ROOT_TOPIC"thermal_gj";
        pDesc[2].HandleValue=&HandleGJ;
      }
      else //Not documented... Assume water
      {
        pDesc[2].Name=MQTT_ROOT_TOPIC"water_meter_m3";
        pDesc[2].HandleValue=&HandleM3;
      }
    }
};

HandlekWhClass HandlekWh;
HandleTarifGroupClass HandleTarifGroup;
HandlekWClass HandlekW;
HandleVClass HandleV;
HandleAClass HandleA;
HandleDeviceTypeClass HandleDeviceType;
MeterDataDescriptor MeterDataDescriptors[]=
{
  {"","1-3:0.2.8",nullptr},
  {nullptr,"0-0:1.0.0",nullptr}, //new
  {nullptr,"0-0:96.1.1",nullptr},
  {MQTT_ROOT_TOPIC"consumption_tarif_high","1-0:1.8.1",&HandlekWh},
  {MQTT_ROOT_TOPIC"consumption_tarif_low","1-0:1.8.2",&HandlekWh},
  {MQTT_ROOT_TOPIC"received_tarif_high","1-0:2.8.1",&HandlekWh},
  {MQTT_ROOT_TOPIC"received_tarif_low","1-0:2.8.2",&HandlekWh},
  {MQTT_ROOT_TOPIC"actual_tarif_group","0-0:96.14.0",&HandleTarifGroup},
  {MQTT_ROOT_TOPIC"actual_consumption","1-0:1.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"actual_received","1-0:2.7.0",&HandlekW},
  {nullptr,"0-0:96.7.21",nullptr}, 
  {nullptr,"0-0:96.7.9",nullptr},
  {nullptr,"1-0:99.97.0",nullptr},
  {nullptr,"1-0:32.32.0",nullptr},
  {nullptr,"1-0:52.32.0",nullptr},
  {nullptr,"1-0:72.32.0",nullptr},
  {nullptr,"1-0:32.36.0",nullptr},
  {nullptr,"1-0:52.36.0",nullptr},
  {nullptr,"1-0:72.36.0",nullptr},
  {nullptr,"0-0:96.13.0",nullptr},
  {MQTT_ROOT_TOPIC"instant_voltage_l1","1-0:32.7.0",&HandleV},
  {MQTT_ROOT_TOPIC"instant_voltage_l2","1-0:52.7.0",&HandleV},
  {MQTT_ROOT_TOPIC"instant_voltage_l3","1-0:72.7.0.",&HandleV},
  {MQTT_ROOT_TOPIC"instant_power_current_l1","1-0:31.7.0",&HandleA},
  {MQTT_ROOT_TOPIC"instant_power_current_l2","1-0:51.7.0",&HandleA},
  {MQTT_ROOT_TOPIC"instant_power_current_l3","1-0:71.7.0",&HandleA},
  {MQTT_ROOT_TOPIC"instant_power_usage_l1","1-0:21.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"instant_power_usage_l2","1-0:41.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"instant_power_usage_l3","1-0:61.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"instant_power_return_l1","1-0:22.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"instant_power_return_l2","1-0:42.7.0",&HandlekW},
  {MQTT_ROOT_TOPIC"instant_power_return_l3","1-0:62.7.0",&HandlekW},
  //For the nexr itemt, the order IS important. Handler objects may be added at runtime.
  {nullptr,"0-1:24.1.0",&HandleDeviceType}, //3=gas, 4=stadswarmte
  {nullptr,"0-1:96.1.0",nullptr},
  {nullptr,"0-1:24.2.1",nullptr}, 
  {nullptr,"0-2:24.1.0",&HandleDeviceType}, //3=gas, 4=stadswarmte
  {nullptr,"0-2:96.1.0",nullptr},
  {nullptr,"0-2:24.2.1",nullptr}, 
  {nullptr,"0-3:24.1.0",&HandleDeviceType}, //3=gas, 4=stadswarmte
  {nullptr,"0-3:96.1.0",nullptr},
  {nullptr,"0-3:24.2.1",nullptr}, 
  {nullptr,nullptr,nullptr},   
};

int InitializeMeterData()
{
  auto i=0;
  for (i=0;MeterDataDescriptors[i].Code!=nullptr;i++)
  {
    MeterDataDescriptors[i].Length=strlen(MeterDataDescriptors[i].Code);
    MeterDataDescriptors[i].Value=-1;
    MeterDataDescriptors[i].DecimalValue=-1;
    MeterDataDescriptors[i].Send=false;
    MeterDataDescriptors[i].ValueString=nullptr;
  }
  return i;
}

bool HandleBlock(IBlock *block)
{
  auto desc=MeterDataDescriptors;
  while (desc->Code!=nullptr)
  {
    if (memcmp(desc->Code,block->StartOfBlock(),desc->Length)==0)
    {
      if (desc->HandleValue!=nullptr)
        desc->HandleValue->HandleValue(desc,block);
      return true;
    }
    desc++;
  }
  return false;
}

void SendAll()
{
  auto desc=MeterDataDescriptors;
  while (desc->Code!=nullptr)
  {
    desc->Send=true;
    desc++;
  }
}

