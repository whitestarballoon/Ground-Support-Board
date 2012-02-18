byte MyAddress = 0x01;  //My i2c address
byte Target = 0x00;  	// who i will be talking to
//all the bytes ALL THE BYTES
byte command[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; 
int commandlength = 0; //how many bytes i actually use
#include <Wire.h> // the part that makes i2c work without all that painful stuff

void setup() //setup all the things that need to be done
{
	Wire.begin(MyAddress);        		//claim your place on the i2c bus
	Wire.onReceive(receiveEvent); 		//someone sent me some stuff w00t
	Serial.begin(115200); 				// spew this out as fast as I can
}

void loop()
{
	sendCommand;  	//send the stuff to i2c target
	parseSerial; 	//get stuff from the computer
}

void receiveEvent(int howMany) //i2c recieve then spew to the serial port
{
	byte databiatch;				//be prepared to get incoming data
	while(0 < Wire.available()) 	// loop through all the incoming data
	{
		databiatch = Wire.read(); 	// receive byte
		Serial.print(databiatch); 	// print the byte
	}
	Serial.println();
}

void sendCommand(){
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
			if ( 0 != sendstatus && fail < 10 ) 
			{
				fail++;								//haha fail
				delay(random(0,100));				//wait for random
			}
		}
		if ( 0 == sendstatus )	
		{
			Serial.println("ack");				//YAY you win
		}
		else
		{
			Serial.println("nack");				//haha fail
		}
		
	}
	commandlength = 0;
}

void parseSerial(){
	if (Serial.available() > 0)  				//if you have data start doing stuff
	{			
		if ( 0 == commandlength )				//if it is a new dataset get the header
		{	
			commandlength = Serial.read(); 	//message length number of bytes excluding this byte
			Target = Serial.read(); 		//target device address
			commandlength--;				// already took 1 thing
		}
		for ( int step=0; step = (commandlength-1); step++ )	//do until you have no stuff
		{	
			//make sure that you have as much data as you hoped
			if (Serial.available() > 0 && step < commandlength )	
			{
				command[commandlength] = Serial.read();  			//read the serial until you have it
			}
			else
			{
				Serial.println("NACK");						//haha fail
			}
		}
		Serial.println("ACK");							//YAY you win
	}
}
