
byte MyAddress = 0x07;  //My i2c address
byte Target = 0x00;  	// who i will be talking to
//all the bytes ALL THE BYTES
byte command[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
int commandlength = 0; //how many bytes i actually use
int serialClaimedLen = 0;
#include <Wire.h> // the part that makes i2c work without all that painful stuff
int request = 0;

void setup() //setup all the things that need to be done
{
  Wire.begin(MyAddress);        		//claim your place on the i2c bus
  Wire.onReceive(receiveEvent); 		//someone sent me some stuff w00t
  Serial.begin(115200); 				// spew this out as fast as I can
  Serial.println("GGround Support Booted.");
}

void loop()
{
  parseSerial(); 	//get stuff from the computer
  if ( 0 == request )
  {
    sendCommand();  	//send the stuff to i2c target
  }
  else if ( 1 == request && 0 != commandlength )
  {
    sendRequest();	
  }
}	

void receiveEvent(int howMany) //i2c recieve then spew to the serial port
{
  //byte databiatch;				//be prepared to get incoming data
  while(0 < Wire.available()) 	// loop through all the incoming data
  {
    //databiatch = Wire.read(); 	// receive byte
    Serial.write(Wire.read()); 	// print the byte
  }
  Serial.println();
}

void sendRequest()
{ 
  byte requestlength = command[0];
  Wire.requestFrom(Target,requestlength);  
  Serial.println("Grequest");
  if(requestlength <= Wire.available() || 0 != requestlength )    
  {
    while( 0 == Wire.available() )
    {
      Serial.print(Wire.read());
    }	
    request = 0;
    Serial.println();	
  }
}

void sendCommand(){
  //Serial.println("Gi2csending");
  int sendstatus = 1; 	//status is not successfull yet
  int fail=0;				//FAIL
  if ( 0 != commandlength ) // if i have something
  {
    while ( 0 != sendstatus && 10 >= fail ) //try until it goes through or you fail to much
    {
      Wire.beginTransmission(Target); // transmit to target
      for ( int data = 0; data < commandlength; data++)   //while still have data loop
      {
        Wire.write(command[data]);    //send it
      }
      sendstatus = Wire.endTransmission();    // stop transmitting
      if ( 0 != sendstatus ) 
      {
        Serial.print("GFail:");
        Serial.println(fail);
        fail++;								//haha fail
        delay(random(0,1000));				//wait for random
      }
    }
    if ( 0 == sendstatus )	
    {
      Serial.println("GIACK");				//YAY you win
    }
    else
    {
      Serial.println("GINACK");				//haha fail
    }

  }
  commandlength = 0;
}

void parseSerial(){
  //Serial.println("GS");
  if (0 < Serial.available()) {				//if you have data start doing stuff
    Serial.println("Gsrxcmd");
    delay(300);  //  wait for all serial data to come in to buffer
    serialClaimedLen = Serial.available();  //Save the claimed serial len

    commandlength = Serial.read(); 	//message length number of bytes excluding this byte
    if ( 0 == commandlength )
    {
      request = 1;
      Serial.println("GSACK");

    }

    else if ((commandlength == serialClaimedLen-1) && (commandlength > 1)) {   //Check to see if serial data available is the same len as the first byte says it should be
      Target = Serial.read(); 		//target device address
      commandlength--;				// already took 1 thing

      for ( int n = 0; n < commandlength; n++ )	//do until you have no stuff
      {	

        command[n] = Serial.read();  			//read the serial until you have it


      }

      serialClaimedLen = 0;
      Serial.println("GSACK");							//YAY you win
    }
    else {  
      while(0 != Serial.available()){
        Serial.read(); 
      }
      Serial.println("GSNACK");						//haha fail
      commandlength = 0;
      serialClaimedLen = 0;
    }
  }

}



