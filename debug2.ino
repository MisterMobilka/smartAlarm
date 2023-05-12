#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <iarduino_RTC.h>
#include <DHT.h>
#include <EEPROM.h>
#define BUTTON1 13
#define BUTTON2 12
#define BUTTON3 11
#define BUTTON4 10
#define CLOCK_RST 9
#define CLOCK_DAT 7
#define CLOCK_CLK 8
#define PIEZO 6
#define SENSOR 4
#define DISPLAY_TIME 0
#define DISPLAY_TEMPERATURE 1
#define DISPLAY_TIME_AND_TEMPERATURE 2
#define DISPLAY_OFF 3
byte StateOfDisplay = DISPLAY_TIME;
#define STANDART_STATE 0
#define SETTINGS_OF_TIME 1
#define SETTINGS_OF_SENSOR 2
#define GLOBAL_SETTING 3
#define SETTING_ALARM 4
#define SETTING_TIMER 5
#define STOPWATCH 6
#define SETTING_SENSOR_TEMP 7
#define SETTING_SENSOR_HUMID 8
#define SETTING_GLOBAL_TIME 9
#define SETTING_GLOBAL_WEEKDAY 10
#define SETTING_GLOBAL_DATE 11
#define DISPLAY_INFO_ALARM 12
#define DISPLAY_INFO_TIMER 13
#define DISPLAY_INFO_SENSOR_TEMP 14
#define DISPLAY_INFO_SENSOR_HUMID 15


#define MAX_SONGS 3


#define CHANGE_TIME_ALARM 16
#define CHANGE_TIME_TIMER 17
#define CHANGE_WEEKDAY_ALARM 18
#define CHANGE_SONG_ALARM 19
#define CHANGE_SONG_TIMER 20
int *delays, *times, *notes;
int numOfNotes;
int notes1[] = {
  392, 392, 392, 311, 466, 392, 311, 466, 392
  };
int times1[] = {
  350, 350, 350, 250, 100, 350, 250, 100, 700
  };
int notes2[] = {
 1318, 1318, 1318, 1046, 1318, 1568, 784
  };
int times2[] = {
 150, 300, 150, 150, 300, 600, 600
  };
int delays2[] = {
 150, 300, 300, 150, 300, 600, 600
  };
int notes3[] {
  400, 300, 400, 500, 100, 200
  };
int times3[] {
  200, 200, 400, 200, 200, 400
  };
byte StateOfProgramm=STANDART_STATE;
#define FIRST 0
#define SECOND 1
#define THIRD 2
byte numOfPoint=FIRST;
#define ADRESS_OF_ALARMS 128
#define ADRESS_OF_SENSORS 256
struct alarmType {
  byte hours;
  byte minutes;
  byte seconds;
  bool weekDays[7];
  bool isLoop;
  byte songType;
  };
struct sensorType {
    byte conditionFirst;
    byte conditionSecond;
    byte songType;
  };
alarmType Alarms[6];
sensorType Sensors[6];
void updateAlarms();
void updateSensors();
void deleteROM(int posOfData,byte numOfBytes);
DHT sensr(SENSOR, DHT11);
iarduino_RTC watch(RTC_DS1302, CLOCK_RST, CLOCK_CLK, CLOCK_DAT);
LiquidCrystal_PCF8574 lcd(0x27);
byte buffer[12];
long bufferTime[2];
float humid,temp;
bool display_is_on=true;
void showStandartStateOfDisplay();
void onDisplay();
void playPiezoSongWithDelay(int typeOfSong) {
    if (typeOfSong==0)
    {
      notes=notes2;
      delays=delays2;
      times=times2;
      numOfNotes=7;
    }
    else if (typeOfSong==1)
    {
      notes=notes1;
      delays=times1;
      times=times1;
      numOfNotes=9;
    }
    else if (typeOfSong==2)
    {
      notes=notes3;
      delays=times3;
      times=times3;
      numOfNotes=7;
    }
    for (buffer[7] = 0; buffer[7] < 5; buffer[7]++)
    {
      for (buffer[8] = 0; buffer[8] < numOfNotes; buffer[8]++)
      {
        tone(PIEZO, notes[buffer[8]], times[buffer[8]]);
        delay(delays[buffer[8]]);
      }
    }
  }
void updateMenu();
void quit_menu();
void choise_menu(byte n);
bool menu_is_activity;
byte choiseOfMenu,maximumOfChoises;
char *stringChoisesOfMenu[10];
void displayMenu();
void displayInfoAlarm();
void initSettingOfTime();
void minusTime(byte hours,byte minutes,byte seconds);
void plusTime(byte hours,byte minutes,byte seconds);
byte positionOfSetting, maxPosSetting;
bool setting_is_activity;
void increase_buffer();
void decrease_buffer();
void displaySettingBuffer();
void settingSucsess();
void NothingHapens();
void ButtonPressed(byte n);
void Button1Pressed();
void Button2Pressed();
void Button3Pressed();
void Button4Pressed();
void doSomething();
void checkEvents();
bool butt[4] {false,false,false,false};
bool stateButton[5] {false,false,false,false,false};
void setButtonStates() {
  butt[0] = digitalRead(BUTTON1);
  butt[1] = digitalRead(BUTTON2);
  butt[2] = digitalRead(BUTTON3);
  butt[3] = digitalRead(BUTTON4);

  if (butt[0] == HIGH && butt[1] == HIGH && butt[2] == HIGH && butt[3] == HIGH)
  {
    if (stateButton[0] == false)
    {
      NothingHapens();
      stateButton[0] = !stateButton[0];
    }
    stateButton[1] = false;
    stateButton[2] = false;
    stateButton[3] = false;
    stateButton[4] = false;
  }
  else
  {
    stateButton[0] = false;
  }
  if (butt[0] == LOW && stateButton[1] == false)
  {
    Button1Pressed();
    ButtonPressed(0);
    stateButton[1] = !stateButton[1];
  }
  else if (butt[1] == LOW && stateButton[2] == false)
  {
    Button2Pressed();
    ButtonPressed(1);
    stateButton[2] = !stateButton[2];
  }
  else if (butt[2] == LOW && stateButton[3] == false)
  {
    Button3Pressed();
    ButtonPressed(2);
    stateButton[3] = !stateButton[3];
  }
  else if (butt[3] == LOW && stateButton[4] == false)
  {
    Button4Pressed();
    ButtonPressed(3);
    stateButton[4] = !stateButton[4];
  }
  }
void setup() {
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);
  pinMode(BUTTON4, INPUT);
  watch.begin();
  sensr.begin();
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  watch.period(5);
  bufferTime[0]=millis();
  bufferTime[1]=millis();
  updateAlarms();
  }
void loop() {
  setButtonStates();
  doSomething();
  checkEvents();
  }
void NothingHapens() {
  }
void Button1Pressed() {
    if (menu_is_activity)
    {
      menu_is_activity=false;
      choise_menu(choiseOfMenu);
    }
    else if (setting_is_activity)
    {
      increase_buffer();
    }
  }
void Button2Pressed() {
    if (menu_is_activity)
    {
      if (choiseOfMenu != maximumOfChoises)
      {
        choiseOfMenu += 1;
      }
    }
    else if (setting_is_activity)
    {
      decrease_buffer();
    }
  }
void Button3Pressed() {
    if (menu_is_activity)
    {
      if (choiseOfMenu != 0)
      {
        choiseOfMenu -= 1;
      }
    }
    else if (setting_is_activity)
    {
      if (positionOfSetting!=0)
      {
        positionOfSetting-=1;
      }
    }
  }
void Button4Pressed() {
    if (menu_is_activity)
    {
      menu_is_activity=false;
      quit_menu();
    }
    else if (setting_is_activity)
    {
      if (positionOfSetting!=maxPosSetting)
      {
        positionOfSetting+=1;
      }
      else
      {
        settingSucsess();
      }
    }
    else if(StateOfProgramm==DISPLAY_INFO_ALARM)
    {
      StateOfProgramm=SETTING_ALARM;
      updateMenu();
    }
    else if(StateOfProgramm==DISPLAY_INFO_TIMER)
    {
      StateOfProgramm=SETTING_TIMER;
      updateMenu();
    }
    else if(StateOfProgramm==DISPLAY_INFO_SENSOR_TEMP)
    {
      StateOfProgramm=SETTING_SENSOR_TEMP;
      updateMenu();
    }
    else if(StateOfProgramm==DISPLAY_INFO_SENSOR_HUMID)
    {
      StateOfProgramm=SETTING_SENSOR_HUMID;
      updateMenu();
    }
  }
void ButtonPressed(byte n){
    if (StateOfProgramm==STANDART_STATE) {
      if (n==3)
      {
        StateOfDisplay += 1;
        if (StateOfDisplay == DISPLAY_OFF + 1)
        {
          StateOfDisplay = DISPLAY_TIME;
        }
      }
      else
      {
        onDisplay();
        StateOfDisplay = STANDART_STATE;
        StateOfProgramm += n+1;
        updateMenu();
      }
    }
  }
void doSomething() {


    if (millis() > bufferTime[0]+900)
    {
      humid = sensr.readHumidity();
      temp = sensr.readTemperature();
      watch.gettime();
      updateAlarms();
      updateSensors();
      bufferTime[0]=millis();
    }
    if (millis() > bufferTime[1])
    {


      bufferTime[1]=millis();
    }





    lcd.home();
    if (StateOfProgramm==STANDART_STATE)
    {
      showStandartStateOfDisplay();
    }
    else if (menu_is_activity)
    {
      displayMenu();
    }
    else if (setting_is_activity)
    {
      displaySettingBuffer();
    }
    else if (StateOfProgramm==DISPLAY_INFO_ALARM)
    {
      displayInfoAlarm();
    }
    else if (StateOfProgramm==DISPLAY_INFO_TIMER)
    {
      displayInfoTimer();
    }
  }
void checkEvents(){
    for (buffer[9]=0;buffer[9]<6;buffer[9]++)
    {
      if (Alarms[buffer[9]].minutes==255)
      {
        continue;
      }
      if (Alarms[buffer[9]].minutes==watch.minutes and Alarms[buffer[9]].hours==watch.Hours and Alarms[buffer[9]].seconds==watch.seconds and Alarms[buffer[9]].weekDays[watch.weekday])
      {
        playPiezoSongWithDelay(Alarms[buffer[9]].songType);
        if (Alarms[buffer[9]].isLoop==false or buffer[9]>2)
        {
          deleteROM(ADRESS_OF_ALARMS+5*buffer[9],5);
        }
      }
    }
  } 
void choise_menu(byte n) {
    if (StateOfProgramm==SETTINGS_OF_TIME) {
      StateOfProgramm=4+n/3;
      numOfPoint=n%3;
      updateMenu();
    }
    else if (StateOfProgramm==SETTINGS_OF_SENSOR) {
      StateOfProgramm=7+n/3;
      numOfPoint=n%3;
      updateMenu();
    }
    else if (StateOfProgramm==GLOBAL_SETTING) {
      StateOfProgramm=9+n;
      setting_is_activity=true;
      if (n==0)
      {
        buffer[0]=0;
        buffer[1]=0;
        buffer[2]=0;
        buffer[3]=0;
        buffer[4]=0;
        buffer[5]=0;
        maxPosSetting=5;
        positionOfSetting=0;
      }
      else if(n==1)
      {
        buffer[0]=1;
        maxPosSetting=0;
        positionOfSetting=0;
      }
      else if(n==2)
      {
        buffer[0]=0;
        buffer[1]=1;
        buffer[2]=0;
        buffer[3]=1;
        buffer[4]=0;
        buffer[5]=0;
        maxPosSetting=5;
        positionOfSetting=0;
      }
    }
    else if(StateOfProgramm==SETTING_ALARM)
    {
      if (n==4)
      {
        deleteROM(ADRESS_OF_ALARMS+5*numOfPoint,5);
        StateOfProgramm=SETTINGS_OF_TIME;
        updateMenu();
      }
      else if (n==3)
      {
        setting_is_activity=true;
        buffer[0]=0;
        maxPosSetting=0;
        positionOfSetting=0;
        StateOfProgramm=CHANGE_SONG_ALARM;
      }
      else if (n==2)
      {
        setting_is_activity=true;
        buffer[0]=0;
        buffer[1]=0;
        buffer[2]=0;
        buffer[3]=0;
        buffer[4]=0;
        buffer[5]=0;
        buffer[6]=0;
        maxPosSetting=6;
        positionOfSetting=0;
        StateOfProgramm=CHANGE_WEEKDAY_ALARM;
      }
      else if (n==1)
      {
        setting_is_activity=true;
        buffer[0]=0;
        buffer[1]=0;
        buffer[2]=0;
        buffer[3]=0;
        buffer[4]=0;
        buffer[5]=0;
        maxPosSetting=5;
        positionOfSetting=0;
        StateOfProgramm=CHANGE_TIME_ALARM;
      }
      else if (n==0)
      {
        StateOfProgramm=DISPLAY_INFO_ALARM;
      }
    }
    else if (StateOfProgramm==SETTING_TIMER)
    {
      if (n==3)
      {
        deleteROM(ADRESS_OF_ALARMS+5*numOfPoint+15,5);
        StateOfProgramm=SETTINGS_OF_TIME;
        updateMenu();
      }
      else if (n==0)
      {
        StateOfProgramm=DISPLAY_INFO_TIMER;
      }
      else if (n==1)
      {
        setting_is_activity=true;
        buffer[0]=0;
        buffer[1]=0;
        buffer[2]=0;
        buffer[3]=0;
        buffer[4]=0;
        buffer[5]=0;
        maxPosSetting=5;
        positionOfSetting=0;
        StateOfProgramm=CHANGE_TIME_TIMER;
      }
      else if (n==2)
      {
        setting_is_activity=true;
        buffer[0]=0;
        maxPosSetting=0;
        positionOfSetting=0;
        StateOfProgramm=CHANGE_SONG_TIMER;
      }
    }
    else if (StateOfProgramm==STOPWATCH)
    {

    }
    else if (StateOfProgramm==SETTING_SENSOR_TEMP)
    {
      if (n==2)
      {
        deleteROM(ADRESS_OF_SENSORS+3*numOfPoint,3);
        StateOfProgramm=SETTINGS_OF_SENSOR;
        updateMenu();
      }
      else if (n==0)
      {
        StateOfProgramm=DISPLAY_INFO_SENSOR_TEMP;
      }
    }
    else if (StateOfProgramm==SETTING_SENSOR_HUMID)
    {
      if (n==2)
      {
        deleteROM(ADRESS_OF_SENSORS+3*numOfPoint+9,3);
        StateOfProgramm=SETTINGS_OF_SENSOR;
        updateMenu();
      }
      else if (n==0)
      {
        StateOfProgramm=DISPLAY_INFO_SENSOR_HUMID;
      }
    }
  }
void quit_menu() {
    if (StateOfProgramm==SETTINGS_OF_TIME or StateOfProgramm==SETTINGS_OF_SENSOR or StateOfProgramm==GLOBAL_SETTING)
    {
      StateOfProgramm=STANDART_STATE;
      StateOfDisplay = DISPLAY_OFF;
    }
    if (StateOfProgramm==SETTING_ALARM or StateOfProgramm==SETTING_TIMER or StateOfProgramm==STOPWATCH)
    {
      StateOfProgramm=SETTINGS_OF_TIME;
      updateMenu();
    }
    if (StateOfProgramm==SETTING_SENSOR_TEMP or StateOfProgramm==SETTING_SENSOR_HUMID)
    {
      StateOfProgramm=SETTINGS_OF_SENSOR;
      updateMenu();
    }
  }
void updateMenu() {
    choiseOfMenu=0;
    menu_is_activity=true;
    if (StateOfProgramm==SETTINGS_OF_TIME)
    {
      maximumOfChoises=6;
      stringChoisesOfMenu[0]="Alarm Number 1 ";
      stringChoisesOfMenu[1]="Alarm Number 2 ";
      stringChoisesOfMenu[2]="Alarm Number 3 ";
      stringChoisesOfMenu[3]="Timer Number 1 ";
      stringChoisesOfMenu[4]="Timer Number 2 ";
      stringChoisesOfMenu[5]="Timer Number 3 ";
      stringChoisesOfMenu[6]="Stop Watch     ";
    }
    else if (StateOfProgramm==SETTINGS_OF_SENSOR)
    {
      maximumOfChoises=5;
      stringChoisesOfMenu[0]="Temperature 1  ";
      stringChoisesOfMenu[1]="Temperature 2  ";
      stringChoisesOfMenu[2]="Temperature 3  ";
      stringChoisesOfMenu[3]="Humidity    1  ";
      stringChoisesOfMenu[4]="Humidity    2  ";
      stringChoisesOfMenu[5]="Humidity    3  ";
    }
    else if (StateOfProgramm==GLOBAL_SETTING)
    {
      maximumOfChoises=2;
      stringChoisesOfMenu[0]="Global Time    ";
      stringChoisesOfMenu[1]="Global WeekDay ";
      stringChoisesOfMenu[2]="Global Date    ";
    }
    else if (StateOfProgramm==SETTING_ALARM)
    {
      maximumOfChoises=4;
      stringChoisesOfMenu[0]="Display Info   ";
      stringChoisesOfMenu[1]="Change Time    ";
      stringChoisesOfMenu[2]="Change WeekDay ";
      stringChoisesOfMenu[3]="Change Song    ";
      stringChoisesOfMenu[4]="Delete Alarm   ";
    }
    else if (StateOfProgramm==SETTING_TIMER)
    {
      maximumOfChoises=3;
      stringChoisesOfMenu[0]="Display Info   ";
      stringChoisesOfMenu[1]="Change Time    ";
      stringChoisesOfMenu[2]="Change Song    ";
      stringChoisesOfMenu[3]="Delete Timer   ";
    }
    else if (StateOfProgramm==STOPWATCH)
    {
      maximumOfChoises=2;
      stringChoisesOfMenu[0]="Start StopWatch";
      stringChoisesOfMenu[1]="Stop StopWatch ";
      stringChoisesOfMenu[2]="Show result    ";
    }
    else if (StateOfProgramm==SETTING_SENSOR_TEMP)
    {
      maximumOfChoises=2;
      stringChoisesOfMenu[0]="Display Info   ";
      stringChoisesOfMenu[1]="Set Temp Param ";
      stringChoisesOfMenu[2]="Delete Sensor  ";
    }
    else if (StateOfProgramm==SETTING_SENSOR_HUMID)
    {
      maximumOfChoises=2;
      stringChoisesOfMenu[0]="Display Info   ";
      stringChoisesOfMenu[1]="Set Humid Param";
      stringChoisesOfMenu[2]="Delete Sensor  ";
    }
  }
void showStandartStateOfDisplay() {
    if (StateOfDisplay == DISPLAY_TIME)
    {
      onDisplay();
      lcd.print(watch.gettime("    H:i:s    "));
      lcd.setCursor(0, 1);
      lcd.print(watch.gettime("   d M Y  "));
    }
    else if (StateOfDisplay == DISPLAY_TEMPERATURE)
    {
      lcd.print("Humidity  :");
      lcd.print(humid);
      lcd.setCursor(0, 1);
      lcd.print("Temperatur:");
      lcd.print(temp);
      if (temp < 10)
        lcd.print("  ");
      else
        lcd.print(' ');
    }
    else if (StateOfDisplay == DISPLAY_TIME_AND_TEMPERATURE)
    {
      lcd.print(watch.gettime("    H:i:s    "));
      lcd.setCursor(0, 1);
      lcd.print("Temperatur:");
      lcd.print(temp);
      if (temp < 10)
        lcd.print("  ");
      else
        lcd.print(' ');
    }
    else if (StateOfDisplay == DISPLAY_OFF)
    {
      if (display_is_on)
      {
        lcd.noDisplay();
        lcd.setBacklight(0);
      }
      else
      {
        return;
      }
      display_is_on=false;
    }
  }
void onDisplay() {
    if (!display_is_on)
      {
        lcd.display();
        lcd.setBacklight(255);
        display_is_on=true;
      }
  }
void displayMenu() {
    if (choiseOfMenu == maximumOfChoises)
    {
      lcd.print(' ');
      lcd.print(stringChoisesOfMenu[choiseOfMenu - 1]);
      lcd.setCursor(0, 1);
      lcd.print(char(0xFF));
      lcd.print(stringChoisesOfMenu[choiseOfMenu]);
    }
    else
    {
      lcd.print(char(0xFF));
      lcd.print(stringChoisesOfMenu[choiseOfMenu]);
      lcd.setCursor(0, 1);
      lcd.print(' ');
      lcd.print(stringChoisesOfMenu[choiseOfMenu + 1]);
    }
  }
void increase_buffer(){
    if (StateOfProgramm==SETTING_GLOBAL_TIME or StateOfProgramm==CHANGE_TIME_ALARM or StateOfProgramm==CHANGE_TIME_TIMER)
    {
      if (positionOfSetting == 0)
      {
        if (buffer[0] == 1 and buffer[1] < 4)
        {
          buffer[0] += 1;
        }
        else if (buffer[0] == 0)
        {
          buffer[0] += 1;
        }
      }
      else if (positionOfSetting == 1)
      {
        if (buffer[1] < 3)
        {
          buffer[1] += 1;
        }
        else if (buffer[1] < 9 and buffer[0] != 2)
        {
          buffer[1] += 1;
        }
      }
      else if (positionOfSetting == 2)
      {
        if (buffer[2] != 5)
        {
          buffer[2] += 1;
        }
      }
      else if (positionOfSetting == 3)
      {
        if (buffer[3] != 9)
        {
          buffer[3] += 1;
        }
      }
      else if (positionOfSetting == 4)
      {
        if (buffer[4] != 5)
        {
          buffer[4] += 1;
        }
      }
      else
      {
        if (buffer[5] != 9)
        {
          buffer[5] += 1;
        }
      }
    }
    else if (StateOfProgramm==SETTING_GLOBAL_DATE)
    {
      if (positionOfSetting == 0)
      {
        if (buffer[0] == 2 and buffer[1] < 2)
        {
          buffer[0] += 1;
        }
        else if (buffer[0] != 2 and buffer[0] != 3)
        {
          buffer[0] += 1;
        }
      }
      else if (positionOfSetting == 1)
      {
        if (buffer[0] == 3 and buffer[1] != 1)
        {
          buffer[1] += 1;
        }
        else if (buffer[0] != 3 and buffer[1] != 9)
        {
          buffer[1] += 1;
        }
      }
      else if (positionOfSetting == 2)
      {
        if (buffer[2] != 1 and buffer[3] < 3)
        {
          buffer[2] += 1;
        }
      }
      else if (positionOfSetting == 3)
      {
        if (buffer[2] == 0 and buffer[3] != 9)
        {
          buffer[3] += 1;
        }
        else if (buffer[2] == 1 and buffer[3] != 2)
        {
          buffer[3] += 1;
        }
      }
      else
      {
        if (buffer[positionOfSetting] != 9)
          buffer[positionOfSetting] += 1;
      }
    }
    else if (StateOfProgramm==SETTING_GLOBAL_WEEKDAY)
    {
      if (buffer[0]!=7)
      {
        buffer[0]+=1;
      }
    }
    else if (StateOfProgramm==CHANGE_SONG_ALARM or StateOfProgramm==CHANGE_SONG_TIMER)
    {
      if (buffer[0]!=MAX_SONGS-1)
      {
        buffer[0]++;
      }
    }
    else if (StateOfProgramm==CHANGE_WEEKDAY_ALARM)
    {
      if (buffer[positionOfSetting]==0)
      {
        buffer[positionOfSetting]++;
      }
    }
  }
void decrease_buffer(){
    if (StateOfProgramm==SETTING_GLOBAL_TIME or StateOfProgramm==CHANGE_TIME_ALARM or StateOfProgramm==CHANGE_TIME_TIMER)
    {
      if (buffer[positionOfSetting] != 0)
      {
        buffer[positionOfSetting] -= 1;
      }
    }
    else if (StateOfProgramm==SETTING_GLOBAL_DATE)
    {
      if (positionOfSetting == 0 and buffer[0] != 0 and buffer[1] != 0)
      {
        buffer[0] -= 1;
      }
      else if (positionOfSetting == 1)
      {
        if (buffer[1] > 1)
        {
          buffer[1] -= 1;
        }
        else if (buffer[1] == 1 and buffer[0] != 0)
        {
          buffer[1] -= 1;
        }
      }
      else if (positionOfSetting == 2 and buffer[2] == 1 and buffer[3] != 0)
      {
        buffer[2] -= 1;
      }
      else if (positionOfSetting == 3)
      {
        if (buffer[2] == 1 and buffer[3] != 0)
        {
          buffer[3] -= 1;
        }
        else if (buffer[2] == 0 and buffer[3] != 1)
        {
          buffer[3] -= 1;
        }
      }
      else if (buffer[positionOfSetting] != 0)
      {
        buffer[positionOfSetting] -= 1;
      }
    }
    else if (StateOfProgramm==SETTING_GLOBAL_WEEKDAY)
    {
      if (buffer[0]!=1)
      {
        buffer[0]-=1;
      }
    }
    else if (StateOfProgramm==CHANGE_SONG_ALARM or StateOfProgramm==CHANGE_SONG_TIMER)
    {
      if (buffer[0]!=0)
      {
        buffer[0]--;
      }
    }
    else if (StateOfProgramm==CHANGE_WEEKDAY_ALARM)
    {
      if (buffer[positionOfSetting]==1)
      {
        buffer[positionOfSetting]--;
      }
    }
  }
void displaySettingBuffer() {
    if (StateOfProgramm==SETTING_GLOBAL_TIME or StateOfProgramm==CHANGE_TIME_ALARM or StateOfProgramm==CHANGE_TIME_TIMER)
    {
      lcd.print("Time : ");
      lcd.print(buffer[0]);
      lcd.print(buffer[1]);
      lcd.print(':');
      lcd.print(buffer[2]);
      lcd.print(buffer[3]);
      lcd.print(':');
      lcd.print(buffer[4]);
      lcd.print(buffer[5]);
      lcd.print(' ');
      lcd.setCursor(0, 1);
      if (positionOfSetting == 0)
      {
        lcd.print("       ");
        lcd.print(char(0xFF));
        lcd.print("       ");
      }
      else if (positionOfSetting == 1)
      {
        lcd.print("        ");
        lcd.print(char(0xFF));
        lcd.print("      ");
      }
      else if (positionOfSetting == 2)
      {
        lcd.print("          ");
        lcd.print(char(0xFF));
        lcd.print("    ");
      }
      else if (positionOfSetting == 3)
      {
        lcd.print("           ");
        lcd.print(char(0xFF));
        lcd.print("   ");
      }
      else if (positionOfSetting == 4)
      {
        lcd.print("             ");
        lcd.print(char(0xFF));
        lcd.print(" ");
      }
      else if (positionOfSetting == 5)
      {
        lcd.print("              ");
        lcd.print(char(0xFF));
      }
    }
    else if(StateOfProgramm==SETTING_GLOBAL_DATE)
    {
      lcd.print("Date:");
      lcd.print(buffer[0]);
      lcd.print(buffer[1]);
      lcd.print('-');
      lcd.print(buffer[2]);
      lcd.print(buffer[3]);
      lcd.print(' ');
      lcd.print("20");
      lcd.print(buffer[4]);
      lcd.print(buffer[5]);
      lcd.print(' ');
      lcd.setCursor(0, 1);
      if (positionOfSetting == 0)
      {
        lcd.print("     ");
        lcd.print(char(0xFF));
        lcd.print("         ");
      }
      else if (positionOfSetting == 1)
      {
        lcd.print("      ");
        lcd.print(char(0xFF));
        lcd.print("        ");
      }
      else if (positionOfSetting == 2)
      {
        lcd.print("        ");
        lcd.print(char(0xFF));
        lcd.print("      ");
      }
      else if (positionOfSetting == 3)
      {
        lcd.print("         ");
        lcd.print(char(0xFF));
        lcd.print("     ");
      }
      else if (positionOfSetting == 4)
      {
        lcd.print("             ");
        lcd.print(char(0xFF));
        lcd.print(" ");
      }
      else if (positionOfSetting == 5)
      {
        lcd.print("              ");
        lcd.print(char(0xFF));
      }
    }
    else if (StateOfProgramm==SETTING_GLOBAL_WEEKDAY)
    {
      lcd.print("Week Number:");
      lcd.print(buffer[0]);
      lcd.print("   ");
      lcd.setCursor(0,1);
      lcd.print("            ");
      lcd.print(char(0xFF));
      lcd.print("   ");
    }
    else if (StateOfProgramm==CHANGE_SONG_ALARM or StateOfProgramm==CHANGE_SONG_TIMER)
    {
      lcd.print("Song Number:");
      lcd.print(buffer[0]);
      lcd.print("   ");
      lcd.setCursor(0,1);
      lcd.print("            ");
      lcd.print(char(0xFF));
      lcd.print("   ");
    }
    else if (StateOfProgramm==CHANGE_WEEKDAY_ALARM)
    {
      lcd.print("Weeks : ");
      if (buffer[0])
        lcd.print('M');
      else
        lcd.print(' ');
      if (buffer[1])
        lcd.print('T');
      else
        lcd.print(' ');
      if (buffer[2])
        lcd.print('W');
      else
        lcd.print(' ');
      if (buffer[3])
        lcd.print('T');
      else
        lcd.print(' ');
      if (buffer[4])
        lcd.print('F');
      else
        lcd.print(' ');
      if (buffer[5])
        lcd.print('S');
      else
        lcd.print(' ');
      if (buffer[6])
        lcd.print('S');
      else
        lcd.print(' ');
      lcd.print(' ');
      lcd.setCursor(0,1);
      lcd.print("        ");
      for (buffer[10]=0;buffer[10]!=positionOfSetting;buffer[10]++)
      {
        lcd.print(' ');
      }
      lcd.print(char(0xFF));
      lcd.print("        ");
    }
  }
void settingSucsess() {
    setting_is_activity=false;
    if (StateOfProgramm==SETTING_GLOBAL_TIME)
    {
      watch.settime(buffer[4] * 10 + buffer[5], buffer[2] * 10 + buffer[3], buffer[0] * 10 + buffer[1]);
    }
    else if (StateOfProgramm==SETTING_GLOBAL_WEEKDAY)
    {
      watch.settime(-1,-1,-1,-1,-1,-1,buffer[0]);
    }
    else if(StateOfProgramm==SETTING_GLOBAL_DATE)
    {
      watch.settime(-1, -1, -1, buffer[0] * 10 + buffer[1], buffer[2] * 10 + buffer[3], buffer[4] * 10 + buffer[5]);
    }
    else if (StateOfProgramm==CHANGE_TIME_ALARM)
    {
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 0,buffer[0]*10+buffer[1]);
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 1,buffer[2]*10+buffer[3]);
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 2,buffer[4]*10+buffer[5]);
    }
    else if (StateOfProgramm==CHANGE_SONG_ALARM)
    {
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 3,buffer[0]);
    }
    else if (StateOfProgramm==CHANGE_SONG_TIMER)
    {
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 18,buffer[0]);
    }
    else if (StateOfProgramm==CHANGE_TIME_TIMER)
    {
      buffer[9]=watch.Hours;
      buffer[8]=watch.minutes;
      buffer[7]=watch.seconds;
      plusTime(buffer[0]*10+buffer[1] , buffer[2]*10+buffer[3] , buffer[4]*10+buffer[5]);
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 15,buffer[9]);
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 16,buffer[8]);
      EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 17,buffer[7]);
    }
    else if (StateOfProgramm==CHANGE_WEEKDAY_ALARM)
    {
      buffer[8]=0;
      for (buffer[9]=0;buffer[9]!=7;buffer[9]++)
      {
        buffer[8]+=(buffer[buffer[9]] << buffer[9]);
      }
      if (buffer[8]!=0)
      {
        EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 4,buffer[8] + 128);
      }
      else
      {
        EEPROM.update(ADRESS_OF_ALARMS+5*numOfPoint + 4,buffer[8]);
      }
    }
    StateOfProgramm=STANDART_STATE;
    StateOfDisplay=DISPLAY_OFF;
  }
void initSettingOfTime() {

  }
void updateAlarms(){
    for (buffer[9]=0;buffer[9]!=6;buffer[9]++)
    {
      Alarms[buffer[9]].hours = EEPROM.read(ADRESS_OF_ALARMS+5*buffer[9]);
      Alarms[buffer[9]].minutes = EEPROM.read(ADRESS_OF_ALARMS+1+5*buffer[9]);
      Alarms[buffer[9]].seconds = EEPROM.read(ADRESS_OF_ALARMS+2+5*buffer[9]);
      Alarms[buffer[9]].songType = EEPROM.read(ADRESS_OF_ALARMS+3+5*buffer[9]);
      buffer[11] = EEPROM.read(ADRESS_OF_ALARMS+4+5*buffer[9]);
      Alarms[buffer[9]].isLoop=buffer[11] & 128;
      for (buffer[8]=0, buffer[7]=1;buffer[8]!=7;buffer[8]++, buffer[7]<<=1)
      {
        Alarms[buffer[9]].weekDays[buffer[8]]=buffer[11] & buffer[7];
      }
    }
  }
void updateSensors() {
    for (buffer[9]=0;buffer[9]!=6;buffer[9]++)
    {
      Sensors[buffer[9]].conditionFirst = EEPROM.read(ADRESS_OF_SENSORS+3*buffer[3]);
      Sensors[buffer[9]].conditionSecond = EEPROM.read(ADRESS_OF_SENSORS+1+3*buffer[3]);
      Sensors[buffer[9]].songType = EEPROM.read(ADRESS_OF_SENSORS+2+3*buffer[3]);
    }
  }
void deleteROM(int posOfData,byte numOfBytes) {
    for (buffer[9]=0;buffer[9]!=numOfBytes;buffer[9]++)
    {
      EEPROM.update(posOfData+buffer[9],255);
    }
  }
void displayInfoAlarm() {
    lcd.print("Time-");
    if (Alarms[numOfPoint].minutes == 255)
    {
      lcd.print("NONE     ");
      lcd.setCursor(0, 1);
      lcd.print("NONE NONE NONE  ");
      return;
    }
    if (Alarms[numOfPoint].hours < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint].hours);
    lcd.print(':');
    if (Alarms[numOfPoint].minutes < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint].minutes);
    lcd.print(':');
    if (Alarms[numOfPoint].seconds < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint].seconds);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Lp-");
    if (Alarms[numOfPoint].isLoop)
      lcd.print('Y');
    else
      lcd.print('N');
    lcd.print(" Wek-");



    if (Alarms[numOfPoint].weekDays[0])
      lcd.print('M');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[1])
      lcd.print('T');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[2])
      lcd.print('W');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[3])
      lcd.print('T');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[4])
      lcd.print('F');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[5])
      lcd.print('S');
    else
      lcd.print(' ');
    if (Alarms[numOfPoint].weekDays[6])
      lcd.print('S');
    else
      lcd.print(' ');
  }
void displayInfoTimer() {
    lcd.print("Last-");
    if (Alarms[numOfPoint + 3].minutes == 255)
    {
      lcd.print("NONE     ");
      lcd.setCursor(0, 1);
      lcd.print("NONE NONE NONE  ");
      return;
    }
    buffer[9]=Alarms[numOfPoint + 3].hours;
    buffer[8]=Alarms[numOfPoint + 3].minutes;
    buffer[7]=Alarms[numOfPoint + 3].seconds;
    minusTime(watch.Hours,watch.minutes,watch.seconds);
    if (buffer[9] < 10)
    {
      lcd.print('0');
    }
    lcd.print(buffer[9]);
    lcd.print(':');
    if (buffer[8] < 10)
    {
      lcd.print('0');
    }
    lcd.print(buffer[8]);
    lcd.print(':');
    if (buffer[7] < 10)
    {
      lcd.print('0');
    }
    lcd.print(buffer[7]);
    lcd.print("   ");
    lcd.setCursor(0,1);
    lcd.print("Time=");
    if (Alarms[numOfPoint + 3].hours < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint + 3].hours);
    lcd.print(':');
    if (Alarms[numOfPoint + 3].minutes < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint + 3].minutes);
    lcd.print(':');
    if (Alarms[numOfPoint + 3].seconds < 10)
    {
      lcd.print('0');
    }
    lcd.print(Alarms[numOfPoint + 3].seconds);
    lcd.print("   ");
  }
void displayInfoSensorTemp() {
    lcd.print("First Cond: ");
    if (Sensors[numOfPoint].conditionFirst<10)
      lcd.print('0');
    lcd.print(Sensors[numOfPoint].conditionFirst);
    lcd.print("  ");
    lcd.setCursor(0,1);
    lcd.print("Second Cond: ");
    if (Sensors[numOfPoint].conditionSecond<10)
      lcd.print('0');
    lcd.print(Sensors[numOfPoint].conditionSecond);
    lcd.print(' ');
  }
void displayInfoSensorHumid() {
    numOfPoint+=3;
    displayInfoSensorTemp();
    numOfPoint-=3;
  }
void plusTime(byte hours,byte minutes,byte seconds) {
  buffer[9]+=hours;
  buffer[8]+=minutes;
  buffer[7]+=seconds;
  buffer[8]+=buffer[7]/60;
  buffer[9]+=buffer[8]/60;
  buffer[7]%=60;
  buffer[8]%=60;
  buffer[9]%=24;
  }
void minusTime(byte hours,byte minutes,byte seconds) {
  buffer[9]+=24;
  buffer[8]+=60;
  buffer[7]+=60;
  buffer[9]-=hours;
  buffer[8]-=minutes;
  buffer[7]-=seconds;
  if (buffer[7]/60==0)
  {
    buffer[8]--;
  }
  if (buffer[8]/60==0)
  {
    buffer[9]--;
  }
  buffer[7]%=60;
  buffer[8]%=60;
  buffer[9]%=24;
  }
