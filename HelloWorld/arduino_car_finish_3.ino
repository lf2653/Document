#include <Servo.h>
#include <SoftwareSerial.h>
#include <Arduino.h>

#define _DBG_RXPIN_ 5
#define _DBG_TXPIN_ 6

#define debugBaudRate 115200


#define UNO			//uncomment this line when you use it with UNO board
//#define MEGA		//uncomment this line when you use it with MEGA board


#define DEBUG

#ifdef UNO
#define _cell	Serial
#define DebugSerial	mySerial

#endif  
#ifdef MEGA
#define _cell	Serial1
#define DebugSerial	Serial
#endif  
	
#ifdef UNO
extern SoftwareSerial mySerial;
#endif

#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_RX_BUFFER_SIZE 128

#ifdef UNO

SoftwareSerial mySerial(_DBG_RXPIN_,_DBG_TXPIN_);

#endif

#ifdef DEBUG
#define DBG(message)    DebugSerial.print(message)
#define DBGW(message)    DebugSerial.write(message)
#else
#define DBG(message)
#define DBGW(message)
#endif // DEBUG

int chlID;		//client id(0-4)
//#Define SSID "TEST" //设置接入点名称
//#Define password "123456123456"//设置密码
/******************小车引脚配置******************/
int In1 = 2;
int In2 = 3;
int In3 = 7;
int In4 = 8;
int Frared_right=1;//使用引脚4，接收左侧传感器信号，为"1"说明前方无障碍
int Frared_left=1;//使用引脚13，接收右侧传感器信号
/*****************机械臂引脚配置*****************/
Servo myservo1;//控制上下摇头动作
Servo myservo2;//控制抓取

/**********************全局变量******************/
int order=0;//接收无线命令，开始时置于0，小车停止
boolean bug=0;//用来防止正常情况下进入死循环区（用来实现进入自主避障）
int mode_change=2;//标记模式：1为全自动模式；2为半自动模式；3为全手动模式。初始为半自主模式
void setup()
{
  /*************小车部分初始化**************/
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(4,INPUT);
  pinMode(13,INPUT);  
  /*****************机械臂部分初始化*****************/
  myservo1.attach(9);//确定引脚
  myservo2.attach(10);
  
  myservo1.write(90);//初始化舵机位置
  myservo2.write(20);
  /*****************WIFI部分初始化*****************/
  _cell.begin(115200);	//设置波特率
  DebugSerial.begin(debugBaudRate);		
  //_cell.println("AT+CWSAP=SSID,passward,1,3");
  //delay(200);
   _cell.println("AT+CIPMUX=1");
  delay(200);
  _cell.println("AT+CIPSERVER=1,8080");
  delay(200);
  /*******************小车启动*******************/
  send_message(chlID,"The car works.");
}
/******************小车动作********************/
void Turn_left() //左转
{
  digitalWrite(In1, HIGH);
  digitalWrite(In2, LOW);
  digitalWrite(In3, LOW);
  digitalWrite(In4, HIGH);
}

void Turn_right() //右转
{
  digitalWrite(In1,LOW);
  digitalWrite(In2, HIGH);
  digitalWrite(In3, HIGH);
  digitalWrite(In4, LOW);
}

void Run_forward() //前进
{
  digitalWrite(In1,LOW);
  digitalWrite(In2, HIGH);
  digitalWrite(In3, LOW);
  digitalWrite(In4, HIGH);
}

void Run_behind() //后退
{
  digitalWrite(In1,HIGH);
  digitalWrite(In2, LOW);
  digitalWrite(In3, HIGH);
  digitalWrite(In4, LOW);
}

void pause() //暂停
{
  digitalWrite(In1, LOW);
  digitalWrite(In2, LOW);
  digitalWrite(In3, LOW);
  digitalWrite(In4, LOW);
}

/***********************机械臂动作***********************/

void hand_catch()//抓取物品
{
  int angle=180;//设置角度

  for(angle=90;angle<175;angle++)//将头“摇下”，delay控制速度
  {
    myservo1.write(angle);
    delay(15);
  }
  
  for(angle=20;angle<80;angle++)//夹子合拢，抓取物品
  {
    myservo2.write(angle);
    delay(15);
  }

  for(angle=175;angle>90;angle--)//将头“摇回”，delay控制速度
  {
    myservo1.write(angle);
    delay(15);
  }

}
void hand_throw()//释放物品
{
  int angle;

  for(angle=90;angle<175;angle++)//将头“摇下”，delay控制速度
  {
    myservo1.write(angle);
    delay(15);
  }
  
  for(angle=80;angle>20;angle--)//夹子张开
  {
    myservo2.write(angle);
    delay(15);
  }

  for(angle=175;angle>90;angle--)//将头“摇回”，delay控制速度
  {
    myservo1.write(angle);
    delay(15);
  }


}
/********************WIFI动作*********************/

void closeMux(byte id)//关闭链接
{
    _cell.print("AT+CIPCLOSE=");
    _cell.println(String(id));
    String data;
    unsigned long start;
	start = millis();
	while (millis()-start<500) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("OK")!=-1 || data.indexOf("Link is not")!=-1 || data.indexOf("Cant close")!=-1)
     {
         break;
     }
  }

}

void closeMux(void)//关闭链接
{
    _cell.println("AT+CIPCLOSE");

    String data;
    unsigned long start;
	start = millis();
	while (millis()-start<500) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("Linked")!=-1 || data.indexOf("ERROR")!=-1 || data.indexOf("we must restart")!=-1)
     {
         break;
     }
  }
}


boolean send_message(byte id, String str)//发送数据
{
    _cell.print("AT+CIPSEND=");

    _cell.print(String(id));
    _cell.print(",");
    _cell.println(str.length());
    unsigned long start;
	start = millis();
	bool found;
	while (millis()-start<1000) {                          
        if(_cell.find(">")==true )
        {
			found = true;
           break;
        }
     }
	 if(found)
		_cell.print(str);
	else
	{
		closeMux(id);
		return false;
	}


    String data;
    start = millis();
	while (millis()-start<1000) {
     if(_cell.available()>0)
     {
     char a =_cell.read();
     data=data+a;
     }
     if (data.indexOf("SEND OK")!=-1)
     {
         return true;
     }
  }
  return false;
}



int receive_message(char *buf)//接收数据
{
	//+IPD,<len>:<data>
	//+IPD,<id>,<len>:<data>
	String data = "";
	if (_cell.available()>0)
	{
		unsigned long start;
		start = millis();
		char c0 = _cell.read();
		if (c0 == '+')
		{
			
			while (millis()-start<1000) 
			{
				if (_cell.available()>0)
				{
					char c = _cell.read();
					data += c;
				}
				if (data.indexOf("\nOK")!=-1)
				{
					break;
				}
			}
		
   
    char c0 = _cell.read();
    if (c0 == '+')
    {
      while (millis()-start<1000) 
      {
        if (_cell.available()>0)
        {
          char c = _cell.read();
          data += c;
        }
        if (data.indexOf("\nOK")!=-1)
        {
          break;
        }
      }
    }
    
			Serial.println(data);
			int sLen = strlen(data.c_str());
			int i,j;
			for (i = 0; i <= sLen; i++)
			{
				if (data[i] == ':')
				{
					break;
				}
				
			}
			boolean found = false;
			for (j = 4; j <= i; j++)
			{
				if (data[j] == ',')
				{
					found = true;
					break;
				}
				
			}
			int iSize;
			DBG(data);////////
			DBG("\r\n");
			if(found ==true)
			{
			String _id = data.substring(4, j);
			chlID = _id.toInt();
			String _size = data.substring(j+1, i);
			iSize = _size.toInt();
			//DBG(_size);
			String str = data.substring(i+1, i+1+iSize);
			strcpy(buf, str.c_str());	
			//DBG(str);
						
			}
			else
			{			
			String _size = data.substring(4, i);
			iSize = _size.toInt();
			//DBG(iSize);
			//DBG("\r\n");
			String str = data.substring(i+1, i+1+iSize);
			strcpy(buf, str.c_str());
			//DBG(str);
			}
			return iSize;
		}
	}
	return 0;
}


/*********************综合运用**********************/
int receiveorder()//接收指令
{
int result=10;
char buf[100];
String text="";
int iLen = receive_message(buf);
if(iLen>0) {
  text=buf;
DebugSerial.println( text);
DebugSerial.println( text.length());
if(text.endsWith("stop")) result=0;
if(text.endsWith("forward")) result=1;
if(text.endsWith("behind")) result=2;
if(text.endsWith("left")) result=3;
if(text.endsWith("right")) result=4;
if(text.endsWith("catch")) result=5;
if(text.endsWith("throw")) result=6;
if(text.endsWith("fullauto")) result=7;
if(text.endsWith("halfauto")) result=8;
if(text.endsWith("noauto")) result=9;
if(result<10)
{
send_message(chlID,"OK");
//delay(300);
//closeMux(chlID);
//delay(1000);
}else {
send_message(chlID,"ERROR");
//delay(300);
//closeMux(chlID);
//delay(1000);
}
return result;
}
}




void Auto_mode()//自主避障模式
{ 
    Frared_right = digitalRead(4);
    Frared_left = digitalRead(13);
           if(Frared_right==0) //若右前方有障碍
         {
             if( Frared_left)//判断左前方有无障碍
               { 
                 Turn_left();//若右侧有障碍，左侧无，则左转   
                 delay(200);
               }
               else
               {
                  Run_behind();//若左右都有障碍，则后退并左转找路
                  delay(1000);
                  Turn_left();
                  delay(1000);
                }
         }
          if(Frared_right) //若右前方无障碍
         {
            if( Frared_left) //同时左前方无障碍
               { 
                 Run_forward();//前方无障碍，则前进
                 delay(200);
               }
               else
               {
                  Turn_right();//否则，说明左前方有障碍，应右转 
                  delay(200);
                }           
          }
}

/*********************实际运动*************************/

boolean order_control1()//全自动模式
{
  int order=receiveorder();//获得指令
  
  switch(order)
  {
    case 0:pause();return 0;
    case 7:return 1;
    case 8:pause();order=0;mode_change=2;return 0;
    case 9:pause();order=0;mode_change=3;send_message(chlID,"inmode3");return 0;
    case 10:return 1;
  
   }
}

boolean order_control2()//半自动模式
{
  order=receiveorder();//获得指令
  
  switch(order)
  {
    case 0:pause();return 0;
    case 1:Run_forward();delay(1500);order=8;return 1;
    case 2:Run_behind();delay(1500);order=8;return 1;
    case 3:Turn_left();delay(1500);order=8;return 1;
    case 4:Turn_right();delay(1500);order=8;return 1;
    //case 5:pause();hand_catch();order=8;return 1;
    //case 6:pause();hand_throw();order=8;return 1;
    case 7:pause();order=0;mode_change=1;return 0;
    case 8:return 1;
    case 9:pause();order=0;mode_change=3;send_message(chlID,"inmode3");return 0;
    case 10:return 1;
  }
}

boolean order_control3()//全手动模式，在此状态下才可以使用机械臂
{
  int order=receiveorder();//获得指令
  
  switch(order)
  {
    case 0:pause();return 0;
    case 1:Run_forward();delay(500);return 1;
    case 2:Run_behind();delay(500);return 1;
    case 3:Turn_left();delay(800);return 1;
    case 4:Turn_right();delay(800);return 1;
    case 5:pause();hand_catch();return 1;
    case 6:pause();hand_throw();return 1;
    case 7:pause();order=0;mode_change=1;send_message(chlID,"outmode3");return 0;
    case 8:pause();order=0;mode_change=2;send_message(chlID,"outmode3");return 0;
    case 9:return 1;
    case 10:return 1;
  
   }
}

void loop() 
{
  if(mode_change==1){
    order_control1();
     if(order==7)
     goto flag1;
     /***********以下为静默区，只能通过goto语句访问***********/
     if(bug)
     {
flag1:
    send_message(chlID,"inmode1");//向上位机发送，说明已进入全自动模式
    while(order_control1()!=0)
    {
      Auto_mode();
      }
      send_message(chlID,"outmode1");//向上位机发送，说明已进入全自动模式
     }
}
  
     if(mode_change==2){
     order_control2();
     if(order==8)
     goto flag2;
     /***********以下为静默区，只能通过goto语句访问***********/
     if(bug)
     {
flag2:
    send_message(chlID,"inmode2");//向上位机发送，说明已进入半自动模式
    while(order_control2()!=0)
    {
      Auto_mode();
      }
      send_message(chlID,"outmode2");//向上位机发送，说明已离开半自动模式
      }
    }

     if(mode_change==3){
     order_control3();
    }
}

