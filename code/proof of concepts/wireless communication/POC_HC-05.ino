void setup() 
{
Serial1.begin(9600);
pinMode(13, OUTPUT); 
 }

void loop() 
{
 if(Serial1.available()>0)
   {     
      char data= Serial1.read(); 
      switch(data)
      {
        case 'A': digitalWrite(13, HIGH);break; // led aan
        case 'B': digitalWrite(13, LOW);break; // led uit
        default : break;
      }
      Serial1.println(data);
   }
   
}   
