#ifndef _SERIAL_COMMAND_H
#define _SERIAL_COMMAND_H

class SerialCommand{
  public:
    SerialCommand();
    SerialCommand(void*serial);
    SerialCommand(void*serial,int commandCount);
    void registerCommand(char*cmd,void (*pCallback)(const char * arg));
    void read(void);
  private:
    void* _serial;
    int cmdCount = 0;
    int maxCmd = 32;
    struct SerialCommandCallback{
      const char * cmd;
      void (*pCallback)(const char * arg);
    };
    SerialCommandCallback * commandList;
    char arg_buf[256];
    char cmd_buf[32];
    char*arg_ptr;
    char*cmd_ptr;
    bool has_cmd = false;
    
};

SerialCommand::SerialCommand()
  :SerialCommand(&Serial,32)
{
}

SerialCommand::SerialCommand(void* serial)
  :SerialCommand(serial,32)
{ 
}

SerialCommand::SerialCommand(void *serial,int commandCount)
:  _serial(serial),maxCmd(commandCount)
{
    commandList =  (SerialCommandCallback *) realloc(commandList, (maxCmd + 1) * sizeof(SerialCommandCallback));
    cmd_ptr = cmd_buf;
    arg_ptr = arg_buf;
}

void SerialCommand::registerCommand(char*cmd,void (*pCallback)(const char * arg))
{
  commandList[cmdCount].cmd = (const char*)cmd;
  commandList[cmdCount++].pCallback = pCallback;
}

void SerialCommand::read(void){  
  while(((Stream*)_serial)->available()){
    char c = ((Stream*)_serial)->read(); 
    if(!has_cmd){
      if(cmd_ptr>cmd_buf && (c==' ' || c=='\n' ||c==':')){
        has_cmd = true; 
      }else{
        *cmd_ptr++ = c;
        *cmd_ptr = 0;
      } 
    }else if(c>=32){
      *arg_ptr++ = c;
      *arg_ptr = 0;
    }
    if(c=='\n'){
        for(int i = 0;i<cmdCount;i++){
          if(strcmp(commandList[i].cmd,cmd_buf)==0){
            commandList[i].pCallback(arg_buf);            
            break;
          }
        }        
        has_cmd = false; 
        
        arg_ptr = arg_buf;
        cmd_ptr = cmd_buf;
    }
  }
}  
#endif
