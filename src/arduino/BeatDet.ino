#define NTAPS 30
#define FADE 1
#define BITRES 256
#define AVOIDSAMP 2

#define FASTADC 1

                                                          // defines for setting and clearing register bits because of the default 128 pre-scaler
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

template<int nTaps>
class FIR 
{
  public:
  FIR()                                             //CONSTRUCTS
  {
    k = 0; 
    for (int i=0; i<nTaps; i++) 
    {      
     delayLine[i] = 0; 
    }
  }
  FIR(float newGain, float *newCoefs) 
  {
    k = 0; 
    CoefCalc(newCoefs);

    for (int i=0; i<nTaps; i++) 
    {      
      delayLine[i] = 0; 
    }
    
    GainCalc(newGain);
  }
 
  void CoefCalc(float *newCoefs) 
  {
   for (int i=0; i<nTaps; i++) 
    {      
     coef[i] = newCoefs[i];
    }
  } 
  void GainCalc(float newGain) 
  {
    gain = newGain;
  }
            
  
  float FIRImplmnt(float input) 
  {
    float output = 0;                       
    delayLine[k] = input;                        			// store the input of the routine (contents of the 'in' variable) in the array at the current pointer position
    for (int i=0; i<nTaps; i++)                  			// step through the array
    {                              
      output += coef[i] * delayLine[(i + k) % nTaps];       // ... and add and multiply each value to accumulate the outputput
    }                                                       //  (i + k) % nTaps creates a cyclic way of getting through the array
                  
                  
    output /= gain;                        					// to scale the outputput back to the normal (unless the coefficients provide unity gain in the passband)

    k = (k+1) % nTaps;            							// k is increased and wraps around the nTaps, so next time we will overwrite the oldest saved sample in the array

    return output;                              			// send the revised output value back to the routine
  }
            
  private:
  int k; 													// k stores the index of the current array read to create a circular memory through the array
  float gain; 
  float delayLine[nTaps];
  float coef[nTaps];
  
};

class RGBControl 
{
  public:
  RGBControl() 
  { 
    brightness=0;
    LastBrightness=0;
    UpFade = 0;
    DownFade = 0; 
    ledPin=0;
    micVal=0;
    micMax=0;
    micMin=255;
    counter=0;
    SumDCVal=0;
    DCVal=-1; 				//means not calculated yet
  }

  
  void LedPinNum(int N) 
  { 
    ledPin=N; 
  }
  /*void SetDCVal(int Num) 
  { 
    DCVal=Num; 
  }*/
  void FadeLimitCalc(int Up, int Down) 
  { 
    UpFade=Up; 
    DownFade=Down;
  }
  void MinMaxCalc(int Min, int Max) 
  { 
  	micMin=Min; 
    micMax=Max;
  }
  int DCCalc()                         //to eliminate the DC values by assuming no music is being played and taking the average of the sum of 30 samples.
  {
    if (DCVal==-1)
      {
        if (counter>=AVOIDSAMP) 
           SumDCVal=SumDCVal+micVal;
           counter++;
        if (counter==NTAPS+AVOIDSAMP )
	        {
	          DCVal=SumDCVal/NTAPS; 
	          return 1;
	        }

        return 0;
      }
    else
      {
         return 1; 
      }
  }          
  void BrightnessCalc() 
  {
    //brightness=micVal;
    micVal = abs(micVal-DCVal);
     // Perform 10000 reads. Update mn and mx for each one.
    /*for (int i = 0; i < 10000; ++i) {
    int val = analogRead(analogPin);
    mn = min(mn, val);
    mx = max(mx, val);
  } */
    //micVal = min(micVal+DCALLVal,MAXVAL);
    micMin=min(micVal,micMin);
    micMax=max(micVal,micMax);
    brightness = int((double(micVal-micMin)/double(micMax-micMin))*BITRES)-1;       //normalising the brightness values for evry colour to create a uniform feel

   brightness = min(max(brightness-20,0),BITRES-1);
    
  }
  void WriteBright()                                       //fade denotes by how much amount the brightness will change
  {
    int counter=0;
    while ( abs(LastBrightness - brightness)>0)
    {
       counter++ ;
       if (LastBrightness>brightness) 
         {
            LastBrightness = LastBrightness - FADE;
            if (counter>=DownFade) 
             break;
         }
       else
         { 
            LastBrightness = LastBrightness + FADE;     
            if (counter>=UpFade) 
             break;      
          }
       analogWrite(ledPin, LastBrightness);
    }
  }
 int micVal;
 int micMax;
 int micMin;
 int ledPin; 
 int brightness; 
 int LastBrightness;         
 
 private:
  int UpFade;
  int DownFade;
  int DCVal;
  int counter;
  int SumDCVal;
 
};

///long timeChange=0;

/*long Time1=0;
long Time2=0;
long timeDT=0;*/

int INITmicVal[NTAPS]={0};
int micPin = A5;

RGBControl RCtrl,GCtrl,BCtrl;
FIR<NTAPS> firR,firG,firB;


void setup()  
{ 
	 Serial.begin(115200);

#if FASTADC
 // set prescale to 64||Sampling rate=17.8Khz will give 8KHz analysable spectrum
 sbi(ADCSRA, ADPS2);
 sbi(ADCSRA, ADPS1);
 cbi(ADCSRA, ADPS0);
#endif

  RCtrl.LedPinNum(5);
  GCtrl.LedPinNum(9);
  BCtrl.LedPinNum(3); 

  pinMode(RCtrl.ledPin, OUTPUT);
  pinMode(GCtrl.ledPin, OUTPUT);
  pinMode(BCtrl.ledPin, OUTPUT); 

 pinMode(micPin, INPUT);
float Rcoef[NTAPS] ={0.021987,0.024035,0.026037,0.027974,0.029828,0.03158,0.033212,0.034709,0.036054,0.037234,0.038238,0.039053,0.039672,0.040089,0.040298,0.040298,0.040089,0.039672,0.039053,0.038238,0.037234,0.036054,0.034709,0.033212,0.03158,0.029828,0.027974,0.026037,0.024035,0.021987};
float Bcoef[NTAPS] ={-0.011762,-0.038701,0.0050889,0.056559,0.015417,-0.034795,-0.0098747,-0.011244,-0.051569,0.030127,0.14934,0.018955,-0.22001,-0.12084,0.20886,0.20886,-0.12084,-0.22001,0.018955,0.14934,0.030127,-0.051569,-0.011244,-0.0098747,-0.034795,0.015417,0.056559,0.0050889,-0.038701,-0.011762};
float Gcoef[NTAPS] ={0.014581,0.015791,-0.017186,-0.018813,0.020738,0.023056,-0.025904,-0.029495,0.034171,0.040525,-0.049674,-0.064014,0.089775,0.1498,-0.44965,0.44965,-0.1498,-0.089775,0.064014,0.049674,-0.040525,-0.034171,0.029495,0.025904,-0.023056,-0.020738,0.018813,0.017186,-0.015791,-0.014581};



  firR.CoefCalc(Rcoef);
  firR.GainCalc(1);

  firB.CoefCalc(Bcoef);
  firB.GainCalc( 1.9663); 

  firG.CoefCalc(Gcoef);    
  firG.GainCalc( 2.0864); 

  RCtrl.FadeLimitCalc(30,15);
  BCtrl.FadeLimitCalc(40,20);
  GCtrl.FadeLimitCalc(50,13); 

  //RCtrl.MinMaxCalc(0,20); //DOUBT???
  BCtrl.MinMaxCalc(0,20);
  GCtrl.MinMaxCalc(0,10); 

  //int start ;
 //int i ;
 



 /*Serial.print("ADCTEST: ") ;
 start = millis() ;
 for (i = 0 ; i < 1000 ; i++)
   analogRead(0) ;
 Serial.print(millis() - start) ;
 Serial.println(" msec (1000 calls)") ;
  */
 
} 

void loop()  
{ 
  //Serial.println(micInput);
 
  int Buff=0;
 
  while (Buff<NTAPS) //taking 30 samples
  {
    //Time2= micros();
    
    
      //Time2= micros();
      INITmicVal[Buff] = analogRead(micPin);//analogRead(A0);
    //analogRead(micPin);
      //timeDT = Time2 - Time1;
      //Time1 = micros(); 
      Serial.println(INITmicVal[Buff]);
      Buff++;
    
  }


  
  for ( Buff=0;Buff<NTAPS;Buff++)
  { 
    RCtrl.micVal = int(firR.FIRImplmnt(float(INITmicVal[Buff])));
    GCtrl.micVal = int(firG.FIRImplmnt(float(INITmicVal[Buff])));
    BCtrl.micVal = int(firB.FIRImplmnt(float(INITmicVal[Buff]))); 
  }
  //Serial.println(RCtrl.micVal);
    
 
  if ( RCtrl.DCCalc() && GCtrl.DCCalc() && BCtrl.DCCalc()) 
  {
     
    RCtrl.BrightnessCalc();
    GCtrl.BrightnessCalc();
    BCtrl.BrightnessCalc();  
           
    RCtrl.WriteBright();
    BCtrl.WriteBright();
    GCtrl.WriteBright(); 
  }
  
  
}

