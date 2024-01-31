
/* TEST code on/off here. undef this during interation */
#define ISOLATED_TEST_EN   
/* serial debug prints on off here*/
#define SERIAL_DEBUG

#define CHARGE_CHECK_INTERVAL_MS     10000
#define ADC_SAMPLING_INTERVAL_MS     500
#define ADC_SAMPLES_FOR_AVG_CALC     10



/*ADC pin*/
#define BATT_VOLT_SENSE_ADC_PIN            A0

/*ADC CALIBRATION*/
#define BATT_CHARGE_MIN_ADC_VOLT     39  //31 :9.0V , 39 :10.8v
#define BATT_CHARGE_MAX_ADC_VOLT     46  //46:12.64
#define CUTOFF_PERCENT_LEVEL     (uint32_t)(100)/(BATT_CHARGE_MAX_ADC_VOLT-BATT_CHARGE_MIN_ADC_VOLT)
/*--------ADC vs V trend
  ADC  V
  31   9.0
  39  10.8
  40  11.1
  41  11.4
  42  11.79
  45  12.43
  46  12.64

*/
/* serial comm wrappers */
#ifdef SERIAL_DEBUG
  #define SERIAL_BEGIN(x)     {Serial.begin(x);}
  #define SERIAL_PRNT(x)      {Serial.print(x);}
  #define SERIAL_PRNTLN(x)    {Serial.println(x);}
#else
  #define SERIAL_BEGIN(x)   {}
  #define SERIAL_PRNT(x)    {}
  #define SERIAL_PRNTLN(x)  {}
#endif

uint16_t adcBattVoltAvg_g;

bool isBattGood=false;

void  batterypackMonInit()
{
  pinMode(BATT_VOLT_SENSE_ADC_PIN,INPUT);

}

bool isBatterypackGood()
{
  return isBattGood;
}

uint8_t getBattLvlPercent()
{
  uint8_t battlvlRet = 0;
  if (adcBattVoltAvg_g < BATT_CHARGE_MIN_ADC_VOLT)
  {
    battlvlRet = 0;
    isBattGood=false;
    SERIAL_PRNTLN("Please charge battery pack!!..");
  }
  else
  {
    battlvlRet = (uint8_t)(((float)( adcBattVoltAvg_g-BATT_CHARGE_MIN_ADC_VOLT) / (BATT_CHARGE_MAX_ADC_VOLT - BATT_CHARGE_MIN_ADC_VOLT)) * 100);
    if(battlvlRet >=CUTOFF_PERCENT_LEVEL )
    {
      isBattGood=true;
      
    }
  }
  return battlvlRet;
}

void Task_BatteryPackMonitor()
{
  static uint64_t chargeCheckElapsed;
  static boolean doADCSampling;
  static uint8_t chargerMosfetOffOnceFlag = 0;


  /*
     Enable volt measurement every CHARGE_CHECK_INTERVAL_MS
  */
  if (millis() - chargeCheckElapsed > CHARGE_CHECK_INTERVAL_MS)
  {
    
    SERIAL_PRNTLN("Measuring sense voltage..");
    doADCSampling = true;
    chargeCheckElapsed = millis() ;
  }
  /*
     Volt measurement and averaging every sampling interval ADC_SAMPLING_INTERVAL_MS
  */
  if (doADCSampling) {
    static uint64_t chargeSampleElapsed;
    static uint16_t sumAvg;
    static uint8_t samplCtr;
    if (millis() - chargeSampleElapsed > ADC_SAMPLING_INTERVAL_MS)
    {
      sumAvg += analogRead(BATT_VOLT_SENSE_ADC_PIN);
      SERIAL_PRNT("Raw Adc val for sense pin:");
      SERIAL_PRNTLN(analogRead(BATT_VOLT_SENSE_ADC_PIN));
      samplCtr++;
      if (samplCtr == ADC_SAMPLES_FOR_AVG_CALC)
      {
        adcBattVoltAvg_g = sumAvg / ADC_SAMPLES_FOR_AVG_CALC;
        sumAvg = 0;
        samplCtr = 0;
        doADCSampling = false;
        SERIAL_PRNT("Avg sense volt :");
        SERIAL_PRNTLN(adcBattVoltAvg_g);
        SERIAL_PRNT("Batt % :");
        SERIAL_PRNTLN(getBattLvlPercent());
      }
      chargeSampleElapsed = millis() ;
    }


  }


}

#ifdef ISOLATED_TEST_EN
void setup()
{
 
    SERIAL_BEGIN(9600);

  batterypackMonInit();
}

void loop()
{
  Task_BatteryPackMonitor();
}

#endif